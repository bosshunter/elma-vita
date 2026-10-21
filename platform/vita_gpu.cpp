// vita_gpu.cpp - GPU drawing of gameplay and menus on the PS Vita.
//
// (port) Part of the PS Vita adaptation of the Elasto Mania source code.
//
// The original draws every frame on the CPU, pixel by pixel. Here the Vita's
// GPU (sceGxm, through vita2d) draws gameplay and the menus:
//
// Gameplay. When a level is loaded the original builds the whole level
// picture as runs of pixels (ecset: one set behind the bikes, one in front).
// That picture is cut into 256x256 pieces ("csempe"), which become 8-bit
// palette textures the first time they are needed and stay in a cache. A
// helper thread on another CPU core makes the pieces around each view ahead
// of time, so scrolling rarely has to wait for one. Every frame the GPU draws,
// for each view: the sky (it moves at half speed, so it is drawn on its own),
// the pieces behind the bikes, the objects, the bikes (each part a textured
// parallelogram), the pieces in front of the bikes, the navigator and the
// timer (its pixels take a colour that stands out from the level under them,
// worked out from the pieces and the sky, as the CPU drawing does). The
// zoom only changes the scale of all this, so it costs nothing.
//
// Menus. The background pictures are uploaded when they change; the bouncing
// balls (discs showing the second picture) and the helmet are drawn by the
// GPU every frame.
//
// Textures live in big blocks of GPU memory (one allocation each), not one
// allocation per texture: the Vita's video memory comes in 256 KB blocks.

#include "all.h"
#include "port.h"

#include <psp2/gxm.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/threadmgr.h>
#include <vita2d.h>


// vita_platform.cpp:

#define KEPX (PORT_FIZX)
#define KEPY (PORT_FIZY)

static int Gpuaktiv = 0;
static int Gpukesz = 0;
int Port_gjrajz = 0;
int Port_lgrgen = 1;
void (*Port_csempefv)( int reteg, unsigned char* dest, int stride,
					   int x0, int y0, int w, int h, int kurzorszam ) = NULL;

int port_gpu( void ) {
	return Gpuaktiv;
}

// ---------------------------------------------------------------------------
// GPU memory
// ---------------------------------------------------------------------------

#define IGAZIT( x, a ) (((x) + ((a)-1)) & ~((a)-1))

static void* gpumem( unsigned int meret, SceUID* uid ) {
	unsigned int m = IGAZIT( meret, 256*1024 );
	*uid = sceKernelAllocMemBlock( "elma_gpu", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, m, NULL );
	if( *uid < 0 ) {
		m = IGAZIT( meret, 4096 );
		*uid = sceKernelAllocMemBlock( "elma_gpu", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW_UNCACHE, m, NULL );
		if( *uid < 0 )
			return NULL;
	}
	void* p = NULL;
	if( sceKernelGetMemBlockBase( *uid, &p ) < 0 )
		return NULL;
	if( sceGxmMapMemory( p, m, (SceGxmMemoryAttribFlags)(SCE_GXM_MEMORY_ATTRIB_READ |
														  SCE_GXM_MEMORY_ATTRIB_WRITE) ) < 0 )
		return NULL;
	return p;
}

struct gpupool {
	unsigned char* alap;
	unsigned int meret, pos;
};

static void* poolfog( gpupool* p, unsigned int n, unsigned int igazitas ) {
	if( !p->alap )
		return NULL;
	unsigned int kezd = IGAZIT( p->pos, igazitas );
	if( kezd + n > p->meret )
		return NULL;
	p->pos = kezd + n;
	return p->alap + kezd;
}

static gpupool Tartos = { NULL, 0, 0 };   // menus, palettes, navigator textures
static gpupool Lgrpool = { NULL, 0, 0 };  // pictures of the current LGR file
static gpupool Menupool = { NULL, 0, 0 }; // the helmet's frames (in the menus' colours)

// Vertices: a ring of 4 parts, one per frame. When a frame is started, the
// frame 4 before it is surely drawn already (vita2d keeps at most 2 frames
// waiting for the display), so its part can be written again.
#define CSUCSRESZ (512*1024)
static unsigned char* Csucsok = NULL;
static unsigned int Csucsresz = 0, Csucspos = 0;

static void* csucsfog( unsigned int n ) {
	if( !Csucsok )
		return vita2d_pool_memalign( n, 16 );
	unsigned int kezd = IGAZIT( Csucspos, 16 );
	if( kezd + n > CSUCSRESZ )
		return NULL;
	Csucspos = kezd + n;
	return Csucsok + Csucsresz*CSUCSRESZ + kezd;
}

// Starts drawing a frame on the screen:
static void keretkezd( void ) {
	Csucsresz = (Csucsresz + 1) & 3;
	Csucspos = 0;
	vita2d_start_drawing();
	vita2d_clear_screen();
}

// ---------------------------------------------------------------------------
// Textures
// ---------------------------------------------------------------------------

struct gtex {
	vita2d_texture t;
	int w, h, tw;          // size, and width in memory (multiple of 8)
	unsigned char* adat;
};

static int texkeszit( gtex* g, gpupool* p, int w, int h, int p8, const unsigned int* paletta ) {
	if( w < 1 ) w = 1;
	if( h < 1 ) h = 1;
	int tw = IGAZIT( w, 8 );
	unsigned char* adat = (unsigned char*)poolfog( p, tw*h*(p8 ? 1 : 4), 128 );
	if( !adat )
		return 0;
	memset( &g->t, 0, sizeof( g->t ) );
	if( sceGxmTextureInitLinear( &g->t.gxm_tex, adat,
			p8 ? SCE_GXM_TEXTURE_FORMAT_P8_ABGR : SCE_GXM_TEXTURE_FORMAT_U8U8U8U8_ABGR,
			tw, h, 0 ) < 0 )
		return 0;
	if( p8 )
		sceGxmTextureSetPalette( &g->t.gxm_tex, (void*)paletta );
	sceGxmTextureSetMinFilter( &g->t.gxm_tex, SCE_GXM_TEXTURE_FILTER_POINT );
	sceGxmTextureSetMagFilter( &g->t.gxm_tex, SCE_GXM_TEXTURE_FILTER_POINT );
	sceGxmTextureSetUAddrMode( &g->t.gxm_tex, SCE_GXM_TEXTURE_ADDR_CLAMP );
	sceGxmTextureSetVAddrMode( &g->t.gxm_tex, SCE_GXM_TEXTURE_ADDR_CLAMP );
	g->w = w;
	g->h = h;
	g->tw = tw;
	g->adat = adat;
	return 1;
}

// Palettes (256 x ABGR, 64-byte aligned):
static unsigned int* Szintpal = NULL;   // the level's colours
static unsigned int* Csempepal = NULL;  // the same, with the transparent colour
static unsigned int* Menupal = NULL;    // the menus' colours

// Draws texture g with its 4 corners at screen points (x[i], y[i]), texture
// points (u[i], v[i]) (0..1); order: top left, top right, bottom left, bottom right
static void rajz4( const gtex* g, const float* x, const float* y,
				   const float* u, const float* v ) {
	vita2d_texture_vertex* vt = (vita2d_texture_vertex*)csucsfog( 4*sizeof( vita2d_texture_vertex ) );
	if( !vt )
		return;
	for( int i = 0; i < 4; i++ ) {
		vt[i].x = x[i];
		vt[i].y = y[i];
		vt[i].z = 0.5f;
		vt[i].u = u[i];
		vt[i].v = v[i];
	}
	vita2d_draw_array_textured( &g->t, SCE_GXM_PRIMITIVE_TRIANGLE_STRIP, vt, 4, 0xFFFFFFFF );
}

// 8-bit picture -> RGBA texture (transparent colour: alpha 0, -1: none)
static void rgbabe( gtex* g, const unsigned char* forras, int sorhossz, int w, int h,
					int atlatszo, const unsigned int* pal ) {
	for( int y = 0; y < h; y++ ) {
		const unsigned char* s = forras + y*sorhossz;
		unsigned int* d = (unsigned int*)(g->adat) + y*g->tw;
		for( int x = 0; x < w; x++ ) {
			unsigned char c = s[x];
			d[x] = (c == atlatszo) ? 0 : (pal[c] | 0xFF000000);
		}
	}
}

// ---------------------------------------------------------------------------
// Picture cache (sprites, bike parts, sky, gap pattern) for the current LGR
// file and the menus
// ---------------------------------------------------------------------------

struct kepbejegy {
	const void* kulcs;
	const void* paletta;
	int fajta;      // 0 pic8 RGBA, 1 kiskep RGBA, 2 pic8 P8 (opaque), 3 pic8 sky
	gtex g;
};

#define MAXKEP (1024)
static kepbejegy Kepek[MAXKEP];
static int Kepszam = 0;           // entries of the LGR file (from 0)
static kepbejegy Menukepek[64];   // the menus' helmet frames (kept)
static int Menukepszam = 0;
static int Aktlgrgen = 0;

static gtex* kepkeres( kepbejegy* tomb, int db, const void* kulcs, const void* pal, int fajta ) {
	for( int i = 0; i < db; i++ )
		if( tomb[i].kulcs == kulcs && tomb[i].paletta == pal && tomb[i].fajta == fajta )
			return &tomb[i].g;
	return NULL;
}

// A pic8 as a texture: RGBA (with the sprite's transparent colour) or P8.
static gtex* pic8tex( pic8* p, const unsigned int* pal, int menu ) {
	kepbejegy* tomb = menu ? Menukepek : Kepek;
	int* db = menu ? &Menukepszam : &Kepszam;
	int maxdb = menu ? 64 : MAXKEP;
	gtex* g = kepkeres( tomb, *db, p, pal, 0 );
	if( g )
		return g;
	if( *db >= maxdb )
		return NULL;
	kepbejegy* b = &tomb[*db];
	int w = p->getxsize(), h = p->getysize();
	if( !texkeszit( &b->g, menu ? &Menupool : &Lgrpool, w, h, 0, NULL ) )
		return NULL;
	const unsigned char* spr = p->getsprite();
	if( !spr ) {
		// opaque, row by row (pic8 rows are not one after the other in memory):
		for( int y = 0; y < h; y++ ) {
			const unsigned char* s = p->getptr( y );
			unsigned int* d = (unsigned int*)(b->g.adat) + y*b->g.tw;
			for( int x = 0; x < w; x++ )
				d[x] = pal[s[x]] | 0xFF000000;
		}
	}
	else {
		// A sprite: its runs say which pixels are drawn ('K') and which not ('N')
		memset( b->g.adat, 0, b->g.tw*h*4 );
		unsigned int pos = 0;
		for( int y = 0; y < h; y++ ) {
			const unsigned char* s = p->getptr( y );
			unsigned int* d = (unsigned int*)(b->g.adat) + y*b->g.tw;
			int x = 0;
			while( x < w ) {
				unsigned char parancs = spr[pos++];
				int n = spr[pos++];
				if( n < 1 )
					break;
				if( parancs == 'K' )
					for( int i = x; i < x+n && i < w; i++ )
						d[i] = pal[s[i]] | 0xFF000000;
				x += n;
			}
		}
	}
	b->kulcs = p;
	b->paletta = pal;
	b->fajta = 0;
	(*db)++;
	return &b->g;
}

// An opaque P8 texture of a rectangle of a pic8 (sky, gap pattern):
static gtex* pic8p8( pic8* p, int w, int h, int fajta ) {
	gtex* g = kepkeres( Kepek, Kepszam, p, Szintpal, fajta );
	if( g )
		return g;
	if( Kepszam >= MAXKEP )
		return NULL;
	kepbejegy* b = &Kepek[Kepszam];
	if( w > p->getxsize() ) w = p->getxsize();
	if( h > p->getysize() ) h = p->getysize();
	if( !texkeszit( &b->g, &Lgrpool, w, h, 1, Szintpal ) )
		return NULL;
	for( int y = 0; y < h; y++ )
		memcpy( b->g.adat + y*b->g.tw, p->getptr( y ), w );
	b->kulcs = p;
	b->paletta = Szintpal;
	b->fajta = fajta;
	Kepszam++;
	return &b->g;
}

static gtex* kiskeptex( kiskep* k ) {
	gtex* g = kepkeres( Kepek, Kepszam, k, Szintpal, 1 );
	if( g )
		return g;
	if( Kepszam >= MAXKEP )
		return NULL;
	kepbejegy* b = &Kepek[Kepszam];
	if( !texkeszit( &b->g, &Lgrpool, k->xsize, k->ysize, 0, NULL ) )
		return NULL;
	rgbabe( &b->g, k->tomb, 256, k->xsize, k->ysize, k->lyuk, Szintpal );
	b->kulcs = k;
	b->paletta = Szintpal;
	b->fajta = 1;
	Kepszam++;
	return &b->g;
}

// ---------------------------------------------------------------------------
// Level pieces
// ---------------------------------------------------------------------------

#define CS (256)            // piece size
#define MAXCSEMPE (512)
#define CSHASH (1024)

enum { CS_URES = 0, CS_KESZ, CS_VAR, CS_KESZUL }; // empty, ready, queued, being made

struct csempe {
	int reteg, tx, ty;
	volatile int allapot;
	unsigned int utolso;    // last frame it was drawn in
	int kov;                // next in the hash chain, -1: none
	gtex g;
};

static csempe* Csempek = NULL;
static int Csempeszam = 0;
static int Fej[CSHASH];
static unsigned int Keret = 10;      // gameplay frame counter
static int Aktszintgen = -1;
static int Atlatszo = 0;

static int hashkulcs( int reteg, int tx, int ty ) {
	unsigned int h = (unsigned int)(tx*73856093) ^ (unsigned int)(ty*19349663) ^ (unsigned int)(reteg*83492791);
	return (int)(h % CSHASH);
}

static void lancbol( int i ) {
	csempe* c = &Csempek[i];
	int h = hashkulcs( c->reteg, c->tx, c->ty );
	int* p = &Fej[h];
	while( *p >= 0 ) {
		if( *p == i ) {
			*p = c->kov;
			break;
		}
		p = &Csempek[*p].kov;
	}
	c->kov = -1;
}

static void lancba( int i ) {
	csempe* c = &Csempek[i];
	int h = hashkulcs( c->reteg, c->tx, c->ty );
	c->kov = Fej[h];
	Fej[h] = i;
}

static int csempekeres( int reteg, int tx, int ty ) {
	int i = Fej[hashkulcs( reteg, tx, ty )];
	while( i >= 0 ) {
		csempe* c = &Csempek[i];
		if( c->tx == tx && c->ty == ty && c->reteg == reteg )
			return i;
		i = c->kov;
	}
	return -1;
}

// Helper thread: makes queued pieces
static SceUID Szal = -1;
static SceUID Szalsema = -1;
static SceKernelLwMutexWork Zar;
#define SORHOSSZ (1024)
static int Sor[SORHOSSZ];
static volatile int Sorfej = 0, Sorveg = 0;   // read / write positions
static volatile int Szalmost = -1;            // the piece the thread is making now

static void csempekeszit( int i, int kurzorszam ) {
	csempe* c = &Csempek[i];
	if( Port_csempefv )
		Port_csempefv( c->reteg, c->g.adat, c->g.tw, c->tx*CS, c->ty*CS, CS, CS, kurzorszam );
	else
		memset( c->g.adat, Atlatszo, CS*CS );
}

static int csempeszal( SceSize args, void* argp ) {
	(void)args; (void)argp;
	while( 1 ) {
		sceKernelWaitSema( Szalsema, 1, NULL );
		while( 1 ) {
			sceKernelLockLwMutex( &Zar, 1, NULL );
			if( Sorfej == Sorveg ) {
				sceKernelUnlockLwMutex( &Zar, 1 );
				break;
			}
			int i = Sor[Sorfej];
			Sorfej = (Sorfej + 1) % SORHOSSZ;
			if( Csempek[i].allapot != CS_VAR ) {
				sceKernelUnlockLwMutex( &Zar, 1 );
				continue;
			}
			Csempek[i].allapot = CS_KESZUL;
			Szalmost = i;
			sceKernelUnlockLwMutex( &Zar, 1 );

			csempekeszit( i, 1 );

			sceKernelLockLwMutex( &Zar, 1, NULL );
			__sync_synchronize();
			Csempek[i].allapot = CS_KESZ;
			Szalmost = -1;
			sceKernelUnlockLwMutex( &Zar, 1 );
		}
	}
	return 0;
}

// The helper thread stops working on the level (it may be about to change):
void port_gpu_szunet( void ) {
	if( !Gpukesz || !Csempek )
		return;
	sceKernelLockLwMutex( &Zar, 1, NULL );
	while( Sorfej != Sorveg ) {
		int i = Sor[Sorfej];
		Sorfej = (Sorfej + 1) % SORHOSSZ;
		if( Csempek[i].allapot == CS_VAR ) {
			Csempek[i].allapot = CS_URES;
			lancbol( i );
		}
	}
	sceKernelUnlockLwMutex( &Zar, 1 );
	while( Szalmost >= 0 )
		sceKernelDelayThread( 200 );
	Aktszintgen = -1; // all pieces are made again
}

// A slot for a new piece: an empty one, or the one drawn longest ago (not
// in the last 4 frames, the GPU may still be drawing those):
static int szabadcsempe( void ) {
	int legregebbi = -1;
	for( int i = 0; i < Csempeszam; i++ ) {
		csempe* c = &Csempek[i];
		if( c->allapot == CS_URES )
			return i;
		if( c->allapot == CS_KESZ && c->utolso + 4 <= Keret ) {
			if( legregebbi < 0 || c->utolso < Csempek[legregebbi].utolso )
				legregebbi = i;
		}
	}
	if( legregebbi >= 0 ) {
		lancbol( legregebbi );
		Csempek[legregebbi].allapot = CS_URES;
	}
	return legregebbi;
}

static void csempekuldes( int i, int reteg, int tx, int ty ) {
	csempe* c = &Csempek[i];
	c->reteg = reteg;
	c->tx = tx;
	c->ty = ty;
	c->utolso = Keret;
	lancba( i );
}

// A piece for drawing now (made here if it is not ready):
static csempe* csempekell( int reteg, int tx, int ty ) {
	int i = csempekeres( reteg, tx, ty );
	if( i >= 0 ) {
		csempe* c = &Csempek[i];
		if( c->allapot == CS_VAR ) {
			// Not started yet: make it here
			sceKernelLockLwMutex( &Zar, 1, NULL );
			int sajat = c->allapot == CS_VAR;
			if( sajat )
				c->allapot = CS_KESZUL;
			sceKernelUnlockLwMutex( &Zar, 1 );
			if( sajat ) {
				csempekeszit( i, 0 );
				c->allapot = CS_KESZ;
			}
		}
		while( c->allapot == CS_KESZUL ) // the helper thread is making it
			sceKernelDelayThread( 100 );
		__sync_synchronize();
		c->utolso = Keret;
		return c;
	}
	i = szabadcsempe();
	if( i < 0 )
		return NULL;
	csempekuldes( i, reteg, tx, ty );
	csempekeszit( i, 0 );
	Csempek[i].allapot = CS_KESZ;
	return &Csempek[i];
}

// Queues a piece for the helper thread if it is not there yet:
static int elore( int reteg, int tx, int ty ) {
	if( csempekeres( reteg, tx, ty ) >= 0 )
		return 0;
	int kov = (Sorveg + 1) % SORHOSSZ;
	if( kov == Sorfej )
		return 0;
	int i = szabadcsempe();
	if( i < 0 )
		return 0;
	csempekuldes( i, reteg, tx, ty );
	Csempek[i].utolso = Keret > 2 ? Keret-2 : 0; // not needed yet
	sceKernelLockLwMutex( &Zar, 1, NULL );
	Csempek[i].allapot = CS_VAR;
	Sor[Sorveg] = i;
	Sorveg = kov;
	sceKernelUnlockLwMutex( &Zar, 1 );
	return 1;
}

// ---------------------------------------------------------------------------
// Start-up
// ---------------------------------------------------------------------------

#define NAVSZAM (24)
static gtex Navtex[NAVSZAM];     // the navigator, a ring (4 views x 6 frames)
static unsigned int Navkov = 0;
static gtex Menusima, Menugolyo;
static int Menutex = 0;

void port_gpu_init( void ) {
	if( access( "cpurajz.inf", 0 ) == 0 )
		return; // CPU drawing, as in v1.0
	SceUID uid;
	Tartos.alap = (unsigned char*)gpumem( 4*1024*1024, &uid );
	Tartos.meret = 4*1024*1024;
	Menupool.alap = (unsigned char*)gpumem( 2*1024*1024, &uid );
	Menupool.meret = 2*1024*1024;
	Lgrpool.alap = (unsigned char*)gpumem( 16*1024*1024, &uid );
	Lgrpool.meret = 16*1024*1024;
	if( !Tartos.alap || !Menupool.alap || !Lgrpool.alap )
		return;
	Csucsok = (unsigned char*)gpumem( 4*CSUCSRESZ, &uid ); // (NULL: vita2d's pool)
	// The level pieces, as many as fit (64 KB each):
	int db = MAXCSEMPE;
	void* cs = NULL;
	while( db >= 192 ) {
		cs = gpumem( db*CS*CS, &uid );
		if( cs )
			break;
		db -= 64;
	}
	if( !cs )
		return;
	Csempek = (csempe*)calloc( db, sizeof( csempe ) );
	if( !Csempek )
		return;
	Szintpal = (unsigned int*)poolfog( &Tartos, 1024, 64 );
	Csempepal = (unsigned int*)poolfog( &Tartos, 1024, 64 );
	Menupal = (unsigned int*)poolfog( &Tartos, 1024, 64 );
	if( !Szintpal || !Csempepal || !Menupal )
		return;
	memset( Szintpal, 0, 1024 );
	memset( Csempepal, 0, 1024 );
	memset( Menupal, 0, 1024 );
	Csempeszam = db;
	for( int i = 0; i < db; i++ ) {
		Csempek[i].allapot = CS_URES;
		Csempek[i].kov = -1;
		gtex* g = &Csempek[i].g;
		memset( &g->t, 0, sizeof( g->t ) );
		g->adat = (unsigned char*)cs + i*CS*CS;
		g->w = g->h = g->tw = CS;
		sceGxmTextureInitLinear( &g->t.gxm_tex, g->adat, SCE_GXM_TEXTURE_FORMAT_P8_ABGR, CS, CS, 0 );
		sceGxmTextureSetPalette( &g->t.gxm_tex, Csempepal );
		sceGxmTextureSetMinFilter( &g->t.gxm_tex, SCE_GXM_TEXTURE_FILTER_POINT );
		sceGxmTextureSetMagFilter( &g->t.gxm_tex, SCE_GXM_TEXTURE_FILTER_POINT );
		sceGxmTextureSetUAddrMode( &g->t.gxm_tex, SCE_GXM_TEXTURE_ADDR_CLAMP );
		sceGxmTextureSetVAddrMode( &g->t.gxm_tex, SCE_GXM_TEXTURE_ADDR_CLAMP );
	}
	for( int i = 0; i < CSHASH; i++ )
		Fej[i] = -1;
	for( int i = 0; i < NAVSZAM; i++ )
		if( !texkeszit( &Navtex[i], &Tartos, 256, 128, 1, Szintpal ) )
			return;
	if( !texkeszit( &Menusima, &Tartos, KEPX, KEPY, 1, Menupal ) ||
		!texkeszit( &Menugolyo, &Tartos, KEPX, KEPY, 1, Menupal ) )
		return;

	// The helper thread, on the second CPU core:
	if( sceKernelCreateLwMutex( &Zar, "elma_csempe", 0, 0, NULL ) < 0 )
		return;
	Szalsema = sceKernelCreateSema( "elma_csempe", 0, 0, 1000000, NULL );
	if( Szalsema < 0 )
		return;
	Szal = sceKernelCreateThread( "elma_csempe", csempeszal, 0x10000100, 0x10000, 0,
								  SCE_KERNEL_CPU_MASK_USER_1, NULL );
	if( Szal >= 0 )
		sceKernelStartThread( Szal, 0, NULL );
	Gpukesz = 1;
	Gpuaktiv = 1;
}

// ---------------------------------------------------------------------------
// Gameplay frame
// ---------------------------------------------------------------------------

static float Nsx = 0, Nsy = 0, Nsw = KEPX, Nsh = KEPY; // the view on the screen
static float Kx = 1, Ky = 1;                            // screen pixels per picture pixel
static int Vw = KEPX, Vh = KEPY;                        // the picture's size
static int Nezetszam = 0;                               // views drawn this frame

// What the current view shows, for the timer (port_gj_look): the size of the
// level picture (zoomed: bigger than the view), the level pieces' origin in
// both layers and the sky
static int Lvw = 1, Lvh = 1;
static int Retegvan[2] = { 0, 0 }, Retegx[2], Retegy[2];
static pic8* Egkep = NULL;
static int Egxmod = 1, Egymod = 1, Egdx = 0;

static inline float sx( float px ) { return Nsx + px*Kx; }
static inline float sy( float py ) { return Nsy + Nsh - py*Ky; }

// A texture with its bottom left corner at picture point (px, py), w x h
// texels, texture row 0 at the bottom (like the game's buffer):
static void kepbe( const gtex* g, float px, float py, int w, int h ) {
	float x0 = sx( px ), x1 = sx( px + w );
	float yb = sy( py ), yt = sy( py + h );
	float u1 = (float)w / g->tw, v1 = (float)h / g->h;
	float x[4] = { x0, x1, x0, x1 };
	float y[4] = { yt, yt, yb, yb };
	float u[4] = { 0, u1, 0, u1 };
	float v[4] = { v1, v1, 0, 0 };
	rajz4( g, x, y, u, v );
}

void port_gj_kezd( const void* paletta, int atlatszo, int szintgen ) {
	if( !Gpuaktiv )
		return;
	// A new LGR file: its pictures are made again
	if( Aktlgrgen != Port_lgrgen ) {
		port_gpu_szunet();
		vita2d_wait_rendering_done();
		Kepszam = 0;
		Lgrpool.pos = 0;
		Aktlgrgen = Port_lgrgen;
		Aktszintgen = -1;
	}
	// A new level: all pieces are made again
	if( Aktszintgen != szintgen || Atlatszo != (atlatszo & 255) ) {
		port_gpu_szunet();
		vita2d_wait_rendering_done();
		for( int i = 0; i < Csempeszam; i++ ) {
			Csempek[i].allapot = CS_URES;
			Csempek[i].kov = -1;
		}
		for( int i = 0; i < CSHASH; i++ )
			Fej[i] = -1;
		for( int c = 0; c < 256; c++ )
			Csempepal[c] = Szintpal[c];
		// The sky and ground pictures are loaded again with every level:
		Kepszam = 0;
		Lgrpool.pos = 0;
		Atlatszo = atlatszo & 255;
		Csempepal[Atlatszo] = 0;
		Aktszintgen = szintgen;
	}
	// New colours: the pieces stay (they hold colour numbers), the pictures
	// made with the old colours are made again
	const unsigned int* p = (const unsigned int*)paletta;
	int palvalt = 0;
	for( int c = 0; c < 256; c++ )
		if( Szintpal[c] != (p[c] | 0xFF000000) ) {
			palvalt = 1;
			break;
		}
	if( palvalt ) {
		vita2d_wait_rendering_done();
		for( int c = 0; c < 256; c++ ) {
			Szintpal[c] = p[c] | 0xFF000000;
			Csempepal[c] = Szintpal[c];
		}
		Csempepal[Atlatszo] = 0;
		Kepszam = 0;
		Lgrpool.pos = 0;
	}
	Keret++;
	Nezetszam = 0;
	keretkezd();
	Port_kepforras = 1;
	Port_gjrajz = 1;
}

static void bmpmentes( void ) {
	char nev[20];
	for( int i = 0; i < 100; i++ ) {
		sprintf( nev, "snap%02d.bmp", i );
		if( access( nev, 0 ) != 0 )
			break;
	}
	FILE* h = fopen( nev, "wb" );
	if( !h )
		return;
	const unsigned int* fb = (const unsigned int*)vita2d_get_current_fb();
	int sor = KEPX*3;
	unsigned int meret = 54 + sor*KEPY;
	unsigned char fej[54];
	memset( fej, 0, 54 );
	fej[0] = 'B'; fej[1] = 'M';
	fej[2] = meret; fej[3] = meret >> 8; fej[4] = meret >> 16; fej[5] = meret >> 24;
	fej[10] = 54; fej[14] = 40;
	fej[18] = KEPX & 255; fej[19] = KEPX >> 8;
	fej[22] = KEPY & 255; fej[23] = KEPY >> 8;
	fej[26] = 1; fej[28] = 24;
	fwrite( fej, 1, 54, h );
	static unsigned char puf[KEPX*3];
	for( int y = KEPY-1; y >= 0; y-- ) {
		const unsigned int* s = fb + y*KEPX;
		for( int x = 0; x < KEPX; x++ ) {
			unsigned int c = s[x]; // A8B8G8R8
			puf[3*x]   = (unsigned char)(c >> 16); // B
			puf[3*x+1] = (unsigned char)(c >> 8);  // G
			puf[3*x+2] = (unsigned char)c;         // R
		}
		fwrite( puf, 1, sor, h );
	}
	fclose( h );
}

void port_gj_vege( int kepmentes ) {
	if( !Gpuaktiv )
		return;
	Port_gjrajz = 0;
	vita2d_end_drawing();
	vita2d_swap_buffers();
	if( kepmentes ) {
		vita2d_wait_rendering_done();
		bmpmentes(); // the frame just finished
	}
}

// The gaps between the views: the area (x, y, w, h) of the screen tiled with
// this picture like blt8 does into the game's upside-down screen buffer:
void port_gj_hatter( pic8* minta, int x, int y, int w, int h ) {
	if( !Gpuaktiv || !minta )
		return;
	gtex* g = pic8p8( minta, minta->getxsize(), minta->getysize(), 2 );
	if( !g )
		return;
	port_gj_nezet( x, y, w, h, w, h );
	for( int py = 0; py < h; py += g->h )
		for( int px = 0; px < w; px += g->w )
			kepbe( g, (float)px, (float)py, g->w, g->h );
	port_gj_nezet_vege();
}

void port_gj_nezet( int vx, int vy, int vw2, int vh2, int pw, int ph ) {
	if( !Gpuaktiv )
		return;
	Nsx = (float)vx;
	Nsy = (float)vy;
	Nsw = (float)vw2;
	Nsh = (float)vh2;
	port_gj_skala( pw, ph );
	Lvw = Vw;
	Lvh = Vh;
	Retegvan[0] = Retegvan[1] = 0;
	Egkep = NULL;
	vita2d_set_clip_rectangle( vx, vy, vx + vw2, vy + vh2 );
	vita2d_enable_clipping();
}

void port_gj_skala( int pw, int ph ) {
	if( pw < 1 ) pw = 1;
	if( ph < 1 ) ph = 1;
	Vw = pw;
	Vh = ph;
	Kx = Nsw / pw;
	Ky = Nsh / ph;
}

void port_gj_nezet_vege( void ) {
	if( !Gpuaktiv )
		return;
	vita2d_disable_clipping();
	Nezetszam++;
}

void port_gj_eg( pic8* peg, int egxmod, int egymod, int egdx ) {
	if( !Gpuaktiv || !peg || egxmod < 1 || egymod < 1 )
		return;
	Egkep = peg;
	Egxmod = egxmod;
	Egymod = egymod;
	Egdx = egdx;
	gtex* g = pic8p8( peg, egxmod, egymod, 3 );
	if( !g )
		return;
	// sky pixel (x, y) of the view = sky picture ((egdx + x) % egxmod, y % egymod)
	for( int y = 0; y < Vh; y += g->h )
		for( int x = -(egdx % g->w); x < Vw; x += g->w )
			kepbe( g, (float)x, (float)y, g->w, g->h );
}

void port_gj_reteg( int reteg, int xe1, int ye1 ) {
	if( !Gpuaktiv || !Csempek )
		return;
	Retegvan[reteg & 1] = 1;
	Retegx[reteg & 1] = xe1;
	Retegy[reteg & 1] = ye1;
	int tx0 = xe1 >= 0 ? xe1/CS : -((-xe1+CS-1)/CS);
	int ty0 = ye1 >= 0 ? ye1/CS : -((-ye1+CS-1)/CS);
	int tx1 = (xe1 + Vw - 1) / CS;
	int ty1 = (ye1 + Vh - 1) / CS;
	// Missing pieces of the view (after a new level, or a fast move): the
	// helper thread starts on them from one end, this thread from the other
	if( Szal >= 0 ) {
		int uj = 0;
		for( int ty = ty0; ty <= ty1; ty++ )
			for( int tx = tx0; tx <= tx1; tx++ )
				uj += elore( reteg, tx, ty );
		if( uj )
			sceKernelSignalSema( Szalsema, uj );
	}
	for( int ty = ty1; ty >= ty0; ty-- )
		for( int tx = tx1; tx >= tx0; tx-- ) {
			csempe* c = csempekell( reteg, tx, ty );
			if( c )
				kepbe( &c->g, (float)(tx*CS - xe1), (float)(ty*CS - ye1), CS, CS );
		}
	// Ahead: the ring of pieces around the view, made by the helper thread
	if( Szal >= 0 ) {
		int uj = 0;
		for( int ty = ty0-1; ty <= ty1+1 && uj < 12; ty++ )
			for( int tx = tx0-1; tx <= tx1+1 && uj < 12; tx++ ) {
				if( ty >= ty0 && ty <= ty1 && tx >= tx0 && tx <= tx1 )
					continue;
				uj += elore( reteg, tx, ty );
			}
		if( uj )
			sceKernelSignalSema( Szalsema, uj );
	}
}

void port_gj_kep( pic8* p, int x, int y ) {
	if( !Gpuaktiv || !p )
		return;
	gtex* g = pic8tex( p, Szintpal, 0 );
	if( g )
		kepbe( g, (float)x, (float)y, g->w, g->h );
}

void port_gj_negyszog( kiskep* k, float rx, float ry, float ux, float uy, float vx, float vy ) {
	if( !Gpuaktiv || !k || k->xsize < 2 || k->ysize < 2 )
		return;
	gtex* g = kiskeptex( k );
	if( !g )
		return;
	// Texel (i, j) is centred at r + i*uu + j*vv:
	float uux = ux / (k->xsize-1), uuy = uy / (k->xsize-1);
	float vvx = vx / (k->ysize-1), vvy = vy / (k->ysize-1);
	// Texture corners (texel edges), plus half a pixel like the CPU drawing:
	float ox = rx - 0.5f*uux - 0.5f*vvx + 0.5f;
	float oy = ry - 0.5f*uuy - 0.5f*vvy + 0.5f;
	float ax = uux*k->xsize, ay = uuy*k->xsize; // along the texture's x
	float bx = vvx*k->ysize, by = vvy*k->ysize; // along its y
	float u1 = (float)k->xsize / g->tw, v1 = (float)k->ysize / g->h;
	float x[4] = { sx( ox ), sx( ox+ax ), sx( ox+bx ), sx( ox+ax+bx ) };
	float y[4] = { sy( oy ), sy( oy+ay ), sy( oy+by ), sy( oy+ay+by ) };
	float u[4] = { 0, u1, 0, u1 };
	float v[4] = { 0, 0, v1, v1 };
	rajz4( g, x, y, u, v );
}

void port_gj_resz( pic8* p, int x, int y, int w, int h ) {
	if( !Gpuaktiv || !p || w < 1 || h < 1 )
		return;
	if( w > 256 ) w = 256;
	if( h > 128 ) h = 128;
	gtex* g = &Navtex[Navkov++ % NAVSZAM];
	for( int r = 0; r < h; r++ ) {
		if( y + r < 0 || y + r >= p->getysize() )
			continue;
		memcpy( g->adat + r*g->tw, p->getptr( y + r ) + x, w );
	}
	float u1 = (float)w / g->tw, v1 = (float)h / g->h;
	float x0 = sx( (float)x ), x1 = sx( (float)(x+w) );
	float yb = sy( (float)y ), yt = sy( (float)(y+h) );
	float xs[4] = { x0, x1, x0, x1 };
	float ys[4] = { yt, yt, yb, yb };
	float us[4] = { 0, u1, 0, u1 };
	float vs[4] = { v1, v1, 0, 0 };
	rajz4( g, xs, ys, us, vs );
}

// The colour number under picture pixel (px, py) of the current view (at
// its normal size), from the level and the sky (-1: unknown):
static int alattaszin( int px, int py ) {
	if( Nsw < 1.0f || Nsh < 1.0f )
		return -1;
	// The level picture's pixel, like the CPU zoom scaling picks it:
	int X = (int)(px * (long long)Lvw / (int)(Nsw + 0.5f));
	int Y = (int)(py * (long long)Lvh / (int)(Nsh + 0.5f));
	for( int r = 1; r >= 0; r-- ) {
		if( !Retegvan[r] )
			continue;
		int xe = Retegx[r] + X, ye = Retegy[r] + Y;
		if( xe < 0 || ye < 0 )
			continue;
		int i = csempekeres( r, xe / CS, ye / CS );
		if( i < 0 || Csempek[i].allapot != CS_KESZ )
			continue;
		unsigned char c = Csempek[i].g.adat[(ye % CS)*CS + (xe % CS)];
		if( c != Atlatszo )
			return c;
	}
	if( Egkep && Y >= 0 && X >= 0 )
		return Egkep->getptr( Y % Egymod )[(Egdx + X) % Egxmod];
	return -1;
}

// The timer: like the CPU drawing, every pixel of the rectangle gets the
// colour lookup[colour under it] (dark -> the lightest colour, else the
// darkest). Pixels of the same new colour next to each other are drawn as
// one rectangle.
void port_gj_look( int x, int y, int w, int h, const unsigned char* lookup ) {
	if( !Gpuaktiv || !lookup || w < 1 || h < 1 )
		return;
	for( int j = 0; j < h; j++ ) {
		int kezd = 0, elozo = -2;
		for( int i = 0; i <= w; i++ ) {
			int c = -2;
			if( i < w ) {
				int a = alattaszin( x + i, y + j );
				c = a < 0 ? lookup[0] : lookup[a];
			}
			if( c == elozo )
				continue;
			if( elozo >= 0 && i > kezd ) {
				float x0 = sx( (float)(x + kezd) ), x1 = sx( (float)(x + i) );
				float yt = sy( (float)(y + j + 1) ), yb = sy( (float)(y + j) );
				vita2d_draw_rectangle( x0, yt, x1 - x0, yb - yt, Szintpal[elozo] );
			}
			kezd = i;
			elozo = c;
		}
	}
}

// ---------------------------------------------------------------------------
// Menus
// ---------------------------------------------------------------------------

static int Menuverzio = -1;
static int Menudb = 0;
static port_korong Menukorongok[64];
static pic8* Menusisak = NULL;
static int Menusisakx = 0, Menusisaky = 0;

// The menus use the colours on the screen now:
static void menupaletta( void ) {
	const unsigned int* p = port_paletta();
	int valt = 0;
	for( int c = 0; c < 256; c++ )
		if( Menupal[c] != (p[c] | 0xFF000000) ) {
			valt = 1;
			break;
		}
	if( !valt )
		return;
	vita2d_wait_rendering_done();
	for( int c = 0; c < 256; c++ )
		Menupal[c] = p[c] | 0xFF000000;
	Menukepszam = 0; // the helmet's frames are made again in the new colours
	Menupool.pos = 0;
}

void port_gm_hatter( pic8* sima, pic8* golyo, int verzio ) {
	if( !Gpuaktiv || !sima || !golyo )
		return;
	if( verzio == Menuverzio && Menutex )
		return;
	vita2d_wait_rendering_done(); // the GPU may still be drawing the old ones
	pic8* kepek[2] = { sima, golyo };
	gtex* texek[2] = { &Menusima, &Menugolyo };
	for( int k = 0; k < 2; k++ ) {
		int w = kepek[k]->getxsize(), h = kepek[k]->getysize();
		if( w > KEPX ) w = KEPX;
		if( h > KEPY ) h = KEPY;
		for( int y = 0; y < h; y++ )
			memcpy( texek[k]->adat + y*texek[k]->tw, kepek[k]->getptr( y ), w );
	}
	Menuverzio = verzio;
	Menutex = 1;
}

static void menukeret( int dialoggal ) {
	menupaletta();
	keretkezd();
	// The plain picture, 1:1:
	{
		float x[4] = { 0, KEPX, 0, KEPX };
		float y[4] = { 0, 0, KEPY, KEPY };
		float u1 = (float)KEPX / Menusima.tw, v1 = 1.0f;
		float u[4] = { 0, u1, 0, u1 };
		float v[4] = { 0, 0, v1, v1 };
		rajz4( &Menusima, x, y, u, v );
	}
	// The balls: discs showing the same place of a picture
	for( int i = 0; i < Menudb; i++ ) {
		const port_korong* k = &Menukorongok[i];
		if( k->r < 1.0f )
			continue;
		const gtex* g = k->golyobol ? &Menugolyo : &Menusima;
		const int N = 28;
		vita2d_texture_vertex* vt = (vita2d_texture_vertex*)csucsfog( 3*N*sizeof( vita2d_texture_vertex ) );
		if( !vt )
			break;
		float px = k->x, py = k->y;
		for( int s = 0; s < N; s++ ) {
			float a0 = 6.2831853f*s/N, a1 = 6.2831853f*(s+1)/N;
			float pts[3][2] = { { px, py },
								{ px + k->r*cosf( a0 ), py + k->r*sinf( a0 ) },
								{ px + k->r*cosf( a1 ), py + k->r*sinf( a1 ) } };
			for( int j = 0; j < 3; j++ ) {
				vita2d_texture_vertex* q = &vt[3*s+j];
				q->x = pts[j][0];
				q->y = pts[j][1];
				q->z = 0.5f;
				q->u = pts[j][0] / g->tw;
				q->v = pts[j][1] / g->h;
			}
		}
		vita2d_draw_array_textured( &g->t, SCE_GXM_PRIMITIVE_TRIANGLES, vt, 3*N, 0xFFFFFFFF );
	}
	// The helmet (rows top down, like the menu buffer):
	if( Menusisak ) {
		gtex* g = pic8tex( Menusisak, Menupal, 1 );
		if( g ) {
			float x0 = (float)Menusisakx, y0 = (float)Menusisaky;
			float x[4] = { x0, x0 + g->w, x0, x0 + g->w };
			float y[4] = { y0, y0, y0 + g->h, y0 + g->h };
			float u1 = (float)g->w / g->tw, v1 = 1.0f;
			float u[4] = { 0, u1, 0, u1 };
			float v[4] = { 0, 0, v1, v1 };
			rajz4( g, x, y, u, v );
		}
	}
	vita2d_end_drawing();
	if( dialoggal )
		vita2d_common_dialog_update(); // the on-screen keyboard over the menu
	vita2d_swap_buffers();
	Port_kepforras = 2;
}

void port_gm_rajz( int db, const port_korong* k, pic8* sisak, int sisakx, int sisaky ) {
	if( !Gpuaktiv || !Menutex )
		return;
	if( db > 64 )
		db = 64;
	Menudb = db;
	for( int i = 0; i < db; i++ )
		Menukorongok[i] = k[i];
	Menusisak = sisak;
	Menusisakx = sisakx;
	Menusisaky = sisaky;
	menukeret( 0 );
}

// The same menu frame again (vita_platform.cpp, e.g. after a palette change):
void port_gm_ujra( int dialoggal ) {
	if( Gpuaktiv && Menutex )
		menukeret( dialoggal );
}
