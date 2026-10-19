// vita_platform.cpp - PS Vita implementation of the game's platform layer.
//
// (port) Part of the PS Vita adaptation of the Elasto Mania source code.
// Replaces F_WIN/W_V.CPP (window, main loop, mouse, timer, errors),
// F_WIN/W_DD.CPP (DirectDraw), F_WIN/W_K.CPP + F_WIN/W_DI.CPP (keyboard),
// and F_WIN/W_SOUND2.CPP (DirectSound streaming). The interfaces are the ones
// declared in M_V.H, M_DD.H, M_K.H and HANG.H, so the game code is unchanged.
//
// Video:  vita2d, 8-bit palettized texture (the game draws in 256 colours).
// Input:  sceCtrl buttons + left analog stick, front touch panel.
// Sound:  sceAudioOut, 11025 Hz mono 16-bit, fed from the original mixer.
// Text:   sceImeDialog (Vita on-screen keyboard).

#include "all.h"
#include "dik.h"
#include "port.h"

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/audioout.h>
#include <psp2/ime_dialog.h>
#include <psp2/apputil.h>
#include <psp2/common_dialog.h>
#include <psp2/system_param.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/power.h>
#include <vita2d.h>

#ifndef PORT_ADATKONYVTAR
#define PORT_ADATKONYVTAR "ux0:data/elma"
#endif

// ---------------------------------------------------------------------------
// Globals expected by the game (originally in the Windows layer)
// ---------------------------------------------------------------------------

int Win_95 = 1;
int Margrafikus = 0;
int Locky0_alul = 0;
int Mostvoltegynemcurpos = 0; // used by the mixer (vita_hhigh.cpp)
extern int Waitforretrace;    // main.cpp

int Port_kepx = PORT_FIZX, Port_kepy = PORT_FIZY;
int Port_szeles = 0;
int Port_jatekgombok = 0;

// (port) Gameplay, the menus and the level editor all use the whole 960x544
// screen, drawn 1:1. (A leftover 640x480 picture is centred by bltfront.)
static int klasszikus( void ) {
	return 0;
}

static void beallitkepmeret( void ) {
	if( klasszikus() ) {
		Port_kepx = PORT_KLASSZX;
		Port_kepy = PORT_KLASSZY;
	}
	else {
		Port_kepx = PORT_FIZX;
		Port_kepy = PORT_FIZY;
	}
}

// Called before the game locks the screen, so entering or leaving the editor
// switches the screen size:
void port_kepmeretfrissit( void ) {
	beallitkepmeret();
}

port_jatek_guard::port_jatek_guard( void ) {
	regiszeles = Port_szeles;
	regigombok = Port_jatekgombok;
	Port_szeles = 1;
	Port_jatekgombok = 1;
	beallitkepmeret();
}
port_jatek_guard::~port_jatek_guard( void ) {
	Port_szeles = regiszeles;
	Port_jatekgombok = regigombok;
	beallitkepmeret();
}
port_gombfelvetel_guard::port_gombfelvetel_guard( void ) {
	regigombok = Port_jatekgombok;
	Port_jatekgombok = 1;
}
port_gombfelvetel_guard::~port_gombfelvetel_guard( void ) {
	Port_jatekgombok = regigombok;
}

// ---------------------------------------------------------------------------
// Video (replaces W_DD.CPP)
// ---------------------------------------------------------------------------

static unsigned char* Buff[2] = { NULL, NULL }; // 960x544 8-bit buffers
static int Elso = 0;                              // index of the front buffer
static unsigned char* Sormut[PORT_FIZY];          // row pointers handed out
static int Bufflocked = 0;
static vita2d_texture* Tex = NULL;
static unsigned int Paletta[256];                 // current palette (ABGR)
static int Vitainit = 0;
// Front buffer or palette changed but not shown yet. Shown from mv_check, so
// many small front-buffer updates (level editor) cost one screen refresh:
static int Piszkos = 0;

static void ddinit( void ) {
	for( int i = 0; i < 2; i++ ) {
		Buff[i] = new unsigned char[PORT_FIZX*PORT_FIZY];
		if( !Buff[i] )
			uzenet( "memory ddinit!" );
		memset( Buff[i], 0, PORT_FIZX*PORT_FIZY );
	}
	Tex = vita2d_create_empty_texture_format( PORT_FIZX, PORT_FIZY,
											  SCE_GXM_TEXTURE_FORMAT_P8_ABGR );
	if( !Tex )
		uzenet( "Could not create the screen texture!" );
	vita2d_texture_set_filters( Tex, SCE_GXM_TEXTURE_FILTER_POINT,
								SCE_GXM_TEXTURE_FILTER_POINT );
	for( int i = 0; i < 256; i++ )
		Paletta[i] = RGBA8( i, i, i, 255 );
	Margrafikus = 1;
}

static void ddfinish( void ) {
	if( Tex ) {
		vita2d_wait_rendering_done();
		vita2d_free_texture( Tex );
		Tex = NULL;
	}
}

// Draws the current front buffer (or only uploads it) and shows it:
static void kirajzolkeret( int dialoggal ) {
	vita2d_start_drawing();
	vita2d_clear_screen();
	if( Tex ) {
		if( Port_kepx == PORT_FIZX ) // (port) gameplay and menus: native 1:1
			vita2d_draw_texture( Tex, 0, 0 );
		else
			vita2d_draw_texture_part_scale( Tex, 0, 0,
				PORT_KLASSZ_OFSX, PORT_KLASSZ_OFSY,
				PORT_KLASSZX, PORT_KLASSZY,
				960.0f / PORT_KLASSZX,
				544.0f / PORT_KLASSZY );
	}
	vita2d_end_drawing();
	if( dialoggal )
		vita2d_common_dialog_update();
	vita2d_swap_buffers();
}

static void megjelenit( void ) {
	Piszkos = 0;
	if( !Tex || !Buff[Elso] )
		return;
	vita2d_wait_rendering_done(); // GPU must not read the texture while we write it
	unsigned char* dest = (unsigned char*)vita2d_texture_get_datap( Tex );
	unsigned int stride = vita2d_texture_get_stride( Tex );
	unsigned char* sour = Buff[Elso];
	if( stride == PORT_FIZX )
		memcpy( dest, sour, PORT_FIZX*PORT_FIZY );
	else
		for( int y = 0; y < PORT_FIZY; y++ )
			memcpy( dest+y*stride, sour+y*PORT_FIZX, PORT_FIZX );
	memcpy( vita2d_texture_get_palette( Tex ), Paletta, sizeof( Paletta ) );
	kirajzolkeret( 0 );
}

static unsigned char** sorokat( unsigned char* buff ) {
	int x0 = 0, y0 = 0;
	if( Port_kepx != PORT_FIZX ) {
		x0 = PORT_KLASSZ_OFSX;
		y0 = PORT_KLASSZ_OFSY;
	}
	int ymeret = Port_kepy;
	unsigned char* mut = buff + y0*PORT_FIZX + x0;
	for( int y = 0; y < ymeret; y++ ) {
		if( Locky0_alul )
			Sormut[ymeret-1-y] = mut;
		else
			Sormut[y] = mut;
		mut += PORT_FIZX;
	}
	return Sormut;
}

unsigned char** lockbackbuffer( int fizikai ) {
	fizikai++;
	if( Bufflocked )
		hiba( "lockbackbuffer-ben Bufflocked!" );
	Bufflocked = 1;
	return sorokat( Buff[1-Elso] );
}

void unlockbackbuffer( void ) {
	if( !Bufflocked )
		hiba( "unlockbackbuffer-ben !Bufflocked!" );
	Bufflocked = 0;
	Elso = 1-Elso; // Flip
	megjelenit();
}

unsigned char** lockfrontbuffer( void ) {
	if( Bufflocked )
		hiba( "lockfrontbuffer-ben Bufflocked!" );
	Bufflocked = 1;
	return sorokat( Buff[Elso] );
}

void unlockfrontbuffer( void ) {
	if( !Bufflocked )
		hiba( "unlockfrontbuffer-ben !Bufflocked!" );
	Bufflocked = 0;
	Piszkos = 1; // shown by the next mv_check
}

// Palette: 'tomb' holds 256 RGB triplets with 6-bit components (VGA style).
ddpal::ddpal( unsigned char* tomb ) {
	unsigned int* pal = new unsigned int[256];
	if( !pal )
		hiba( "memory ddpal!" );
	for( int i = 0; i < 256; i++ ) {
		unsigned char r = (unsigned char)(tomb[3*i]<<2);
		unsigned char g = (unsigned char)(tomb[3*i+1]<<2);
		unsigned char b = (unsigned char)(tomb[3*i+2]<<2);
		pal[i] = RGBA8( r, g, b, 255 );
	}
	pddpal = pal;
}

ddpal::~ddpal( void ) {
	delete[] (unsigned int*)pddpal;
	pddpal = NULL;
}

void ddpal::set( void ) {
	memcpy( Paletta, pddpal, sizeof( Paletta ) );
	// Like DirectDraw, a palette change is visible at once (next mv_check):
	Piszkos = 1;
}

// ---------------------------------------------------------------------------
// Buttons, analog stick, touch (replaces W_DI.CPP)
// ---------------------------------------------------------------------------

enum { MOD_MENU = 0, MOD_EDITOR, MOD_JATEK };

static unsigned int Gombok = 0;   // buttons pressed now
static unsigned int Tiltott = 0;  // held through a mode change: ignored until released
static int Aktmod = MOD_MENU;
static unsigned char Lx = 128, Ly = 128;
static int Erintes = 0, Erintx = 0, Erinty = 0;

static int Erintvolt = 0;    // (port) was the screen touched in the previous poll?
static int Erintklikk = 0;   // (port) touch acts as the left mouse button (editor only)
static int Erintlatolt = 0;  // (port) the game has seen this click pressed at least once

static int Erintstx = 0, Erintsty = 0;   // (port) where the current touch started
static SceUInt64 Erintstido = 0;         // (port) when it started
static int Erintvonszol = 0;             // (port) has the finger moved (a drag)?

static int Erintfazis = 0;               // (port) extra click after a drag: 1..3 = steps left

static const unsigned int G_L = SCE_CTRL_LTRIGGER | SCE_CTRL_L1;
static const unsigned int G_R = SCE_CTRL_RTRIGGER | SCE_CTRL_R1;

static int kellmod( void ) {
	extern int Editorban_dialnak; // dialog.cpp
	if( Port_jatekgombok )
		return MOD_JATEK;
	if( Editorban_dialnak )
		return MOD_EDITOR;
	return MOD_MENU;
}

// ---------------------------------------------------------------------------
// (port) Controllers. Players see them as "Pad 1".."Pad 5":
//   Pad 1    = controller port 0: the Vita's own buttons (DIK key codes, so
//              state.dat stays compatible with the PC game)
//   Pad 2..5 = the external controllers, "slots" 0..3 here, each button with
//              its own key code PORT_PADKOD( slot, bit )
// Which controller port a slot reads depends on the setup:
//   - Vita with ds34vita/MiniVitaTV: port 0 is the Vita, ports 1..4 are the
//     controllers -> slot s reads port s+1.
//   - PS TV, or reVita's "Vita as virtual DS4": port 0 already contains the
//     first controller (it is the same as port 1) -> slot s reads port s+2,
//     so the first controller is Pad 1 together with the Vita.
// The game finds out which one it is by watching port 1: if its button
// presses and releases also show up on port 0 in the same frame (or the next),
// the two are the same controller (Osszevont).
// ---------------------------------------------------------------------------

static unsigned int Padgombok[PORT_PADSZAM];  // buttons pressed now, per slot
static unsigned int Padtiltott[PORT_PADSZAM]; // held through a mode change
static unsigned int Port0nyers = 0;            // port 0 buttons (no stick)
static int Osszevont = 0;      // port 0 and port 1 are the same controller
static int Osszevontpont = 0;  // evidence for it: + same, - different
static unsigned int Elozo0 = 0, Elozo1 = 0;    // last frame's port 0 / port 1
static unsigned int Elozovalt0 = 0;            // what changed on port 0 last frame
static unsigned int Fuggomaszk = 0, Fuggoertek = 0; // port 1 change to check next frame

// Buttons a pad can bind (the PS button belongs to the system):
static const unsigned int PADMASZK = 0x0000FFFF;

static unsigned int olvasport( int port, int dpadis ) {
	SceCtrlData pad;
	memset( &pad, 0, sizeof( pad ) );
	// Every port is read every frame, even one that looks empty: controller
	// plugins may move controllers between ports, and checking the port info
	// first made pads stop responding after a while.
	if( sceCtrlPeekBufferPositiveExt2( port, &pad, 1 ) < 0 )
		return 0;
	unsigned int g = pad.buttons & PADMASZK;
	if( dpadis ) {
		// The left stick also works as the D-pad (like on the Vita itself):
		if( pad.lx < 64 )  g |= SCE_CTRL_LEFT;
		if( pad.lx > 192 ) g |= SCE_CTRL_RIGHT;
		if( pad.ly < 64 )  g |= SCE_CTRL_UP;
		if( pad.ly > 192 ) g |= SCE_CTRL_DOWN;
	}
	return g;
}

// Is port 1 the same controller as port 0? (see above)
static void osszevontvizsgalat( unsigned int p0, unsigned int p1 ) {
	int eredmeny = 0; // +1 same, -1 different, 0 nothing to judge
	unsigned int valt0 = p0 ^ Elozo0;
	if( Fuggomaszk ) {
		// A port 1 change from the previous frame that port 0 did not show
		// then: did it show it now (one frame late)?
		if( (valt0 & Fuggomaszk) == Fuggomaszk && (p0 & Fuggomaszk) == Fuggoertek )
			eredmeny = 1;
		else
			eredmeny = -1;
		Fuggomaszk = 0;
	}
	unsigned int valt1 = p1 ^ Elozo1;
	if( valt1 ) {
		if( ((valt0 | Elozovalt0) & valt1) == valt1 && (p0 & valt1) == (p1 & valt1) )
			eredmeny = 1; // the same buttons changed the same way on port 0
						  // (now, or one frame earlier)
		else {
			Fuggomaszk = valt1;
			Fuggoertek = p1 & valt1;
		}
	}
	Elozo0 = p0;
	Elozo1 = p1;
	Elozovalt0 = valt0;
	if( !eredmeny )
		return;
	Osszevontpont += eredmeny;
	if( Osszevontpont > 8 )  Osszevontpont = 8;
	if( Osszevontpont < -8 ) Osszevontpont = -8;
	int uj = Osszevont;
	if( Osszevontpont >= 4 )
		uj = 1;
	if( Osszevontpont <= -4 )
		uj = 0;
	if( uj != Osszevont ) {
		Osszevont = uj;
		// The pads move to other ports: ignore what is held right now
		for( int s = 0; s < PORT_PADSZAM; s++ )
			Padtiltott[s] = 0xFFFFFFFF;
	}
}

static void olvaspadokat( void ) {
	unsigned int portok[6];
	for( int port = 1; port <= 5; port++ )
		portok[port] = port <= 4 ? olvasport( port, 1 ) : 0;
	osszevontvizsgalat( Port0nyers, olvasport( 1, 0 ) );
	for( int s = 0; s < PORT_PADSZAM; s++ ) {
		int port = Osszevont ? s+2 : s+1;
		Padgombok[s] = portok[port];
	}
}

static int bitindex( unsigned int gomb ) {
	for( int b = 0; b < 32; b++ )
		if( gomb == (1u << b) )
			return b;
	return 0;
}

static void olvasbemenet( void ) {
	SceCtrlData pad;
	memset( &pad, 0, sizeof( pad ) );
	sceCtrlPeekBufferPositive( 0, &pad, 1 );
	Gombok = pad.buttons;
	Port0nyers = pad.buttons & PADMASZK; // (port) see olvaspadokat
	Lx = pad.lx;
	Ly = pad.ly;
	// (port) Outside the editor the left stick also acts as the D-pad.
	// (In the editor the stick moves the mouse cursor instead.)
	if( kellmod() != MOD_EDITOR ) {
			if( Lx < 64 )  Gombok |= SCE_CTRL_LEFT;
			if( Lx > 192 ) Gombok |= SCE_CTRL_RIGHT;
			if( Ly < 64 )  Gombok |= SCE_CTRL_UP;
			if( Ly > 192 ) Gombok |= SCE_CTRL_DOWN;
	}
	
	SceTouchData touch;
	memset( &touch, 0, sizeof( touch ) );
	sceTouchPeek( SCE_TOUCH_PORT_FRONT, &touch, 1 );
	Erintes = touch.reportNum > 0;
	if( Erintes ) {
		// Panel is 1920x1088, screen is 960x544:
		Erintx = touch.report[0].x/2;
		Erinty = touch.report[0].y/2;
	}

	// (port) Tablet-style click in the level editor: finger down = left button down.
	// The click ends when the finger is lifted. A very short tap is held until the
	// game has seen it once, so it is not lost; a drag or a long touch is released
	// at once, because the game has surely seen the press by then.
	if( Aktmod != MOD_EDITOR ) {
			Erintklikk = 0;
			Erintfazis = 0;
	}
	else if( Erintes && !Erintvolt ) {
			Erintklikk = 1;
			Erintlatolt = 0;
			Erintvonszol = 0;
			Erintfazis = 0;
			Erintstx = Erintx;
			Erintsty = Erinty;
			Erintstido = sceKernelGetProcessTimeWide();
	}
	else if( Erintes && Erintklikk ) {
			int ddx = Erintx - Erintstx, ddy = Erinty - Erintsty;
			if( ddx*ddx + ddy*ddy > 36 ) // moved more than ~6 pixels
					Erintvonszol = 1;
	}
	else if( !Erintes && Erintklikk ) {
			if( Erintvonszol ) {
					// Finger lifted after a drag: the press was the first click, now
					// give the game a second click at the lift point (see getbutbmou):
					Erintklikk = 0;
					Erintfazis = 1;
			}
			else if( Erintlatolt || sceKernelGetProcessTimeWide() - Erintstido > 150000 )
					Erintklikk = 0; // a plain tap or long press: just release
	}
	Erintvolt = Erintes;

	olvaspadokat(); // (port) external controllers

	int mod = kellmod();
	if( mod != Aktmod ) {
		Aktmod = mod;
		Tiltott = Gombok;
		for( int p = 0; p < PORT_PADSZAM; p++ )
			Padtiltott[p] = Padgombok[p];
	}
	Tiltott &= Gombok;
	for( int p = 0; p < PORT_PADSZAM; p++ )
		Padtiltott[p] &= Padgombok[p];
}

// Fills the key state table (PORT_KODSZAM entries: keyboard codes, then pads):
static void refreshDI( unsigned char* buffer ) {
	memset( buffer, 0, PORT_KODSZAM );
	unsigned int g = Gombok & ~Tiltott;
	#define BE( gomb, kod ) if( g & (gomb) ) buffer[kod] = 0x80

	if( Aktmod == MOD_JATEK ) {
		// Gameplay. These codes are the default key bindings (see
		// di_reset_state), so they can also be reassigned in Options.
		BE( SCE_CTRL_CROSS,    DIK_UP );       // accelerate
		BE( SCE_CTRL_SQUARE,   DIK_DOWN );     // brake
		BE( SCE_CTRL_RIGHT,    DIK_RIGHT );    // rotate clockwise
		BE( SCE_CTRL_LEFT,     DIK_LEFT );     // rotate anticlockwise
		BE( SCE_CTRL_CIRCLE,   DIK_SPACE );    // turn around
		BE( SCE_CTRL_SELECT,   DIK_ESCAPE );   // back to the menu
		BE( SCE_CTRL_UP,       DIK_V );        // minimap on/off
		BE( SCE_CTRL_DOWN,     DIK_T );        // timer on/off (the game's default timer key)
		BE( G_L,               DIK_MINUS );    // smaller screen
		BE( G_R,               DIK_EQUALS );   // bigger screen
		BE( SCE_CTRL_TRIANGLE, DIK_I );        // screenshot
		BE( SCE_CTRL_START,    DIK_RETURN );

		// (port) Controllers: every button has its own code. Select and Start
		// on any controller also work as Esc / Enter, so anyone can leave:
		for( int p = 0; p < PORT_PADSZAM; p++ ) {
			unsigned int pg = Padgombok[p] & ~Padtiltott[p];
			for( int b = 0; b < 32; b++ )
				if( pg & (1u << b) )
					buffer[PORT_PADKOD( p, b )] = 0x80;
			if( pg & SCE_CTRL_SELECT ) buffer[DIK_ESCAPE] = 0x80;
			if( pg & SCE_CTRL_START )  buffer[DIK_RETURN] = 0x80;
		}
	}
	else {
		// Menus and level editor:
		int r = (g & G_R) != 0;
		if( r ) {
			BE( SCE_CTRL_UP,   DIK_PRIOR );    // page up
			BE( SCE_CTRL_DOWN, DIK_NEXT );     // page down
		}
		else {
			BE( SCE_CTRL_UP,   DIK_UP );
			BE( SCE_CTRL_DOWN, DIK_DOWN );
		}
		BE( SCE_CTRL_LEFT,   DIK_LEFT );
		BE( SCE_CTRL_RIGHT,  DIK_RIGHT );
		BE( SCE_CTRL_CIRCLE, DIK_ESCAPE );     // back / cancel
		BE( SCE_CTRL_START,  DIK_RETURN );
		BE( SCE_CTRL_SQUARE, DIK_BACK );       // backspace (text fields)
		if( Aktmod == MOD_EDITOR ) {
			BE( SCE_CTRL_TRIANGLE, DIK_SPACE ); // editor SPACE actions
		}
		if( Aktmod == MOD_MENU ) {
			BE( SCE_CTRL_CROSS, DIK_RETURN );  // select

			// (port) Any controller can also navigate the menus:
			for( int p = 0; p < PORT_PADSZAM; p++ ) {
				unsigned int pg = Padgombok[p] & ~Padtiltott[p];
				#define PBE( gomb, kod ) if( pg & (gomb) ) buffer[kod] = 0x80
				if( pg & (SCE_CTRL_R1 | SCE_CTRL_RTRIGGER) ) {
					PBE( SCE_CTRL_UP,   DIK_PRIOR );
					PBE( SCE_CTRL_DOWN, DIK_NEXT );
				}
				else {
					PBE( SCE_CTRL_UP,   DIK_UP );
					PBE( SCE_CTRL_DOWN, DIK_DOWN );
				}
				PBE( SCE_CTRL_LEFT,   DIK_LEFT );
				PBE( SCE_CTRL_RIGHT,  DIK_RIGHT );
				PBE( SCE_CTRL_CROSS,  DIK_RETURN );
				PBE( SCE_CTRL_START,  DIK_RETURN );
				PBE( SCE_CTRL_CIRCLE, DIK_ESCAPE );
				PBE( SCE_CTRL_SQUARE, DIK_BACK );
				#undef PBE
			}
		}
		// In the editor Cross is the left mouse button (see getbutbmou).
	}
	#undef BE
}

// Default key bindings (called from state_s::resetcontrols):
void di_reset_state( state_s* mut ) {
	mut->opciok1.billgaz    = DIK_UP;     // Cross
	mut->opciok1.billfek    = DIK_DOWN;   // Square
	mut->opciok1.billugras1 = DIK_RIGHT;  // D-Pad Right
	mut->opciok1.billugras2 = DIK_LEFT;   // D-Pad Left
	mut->opciok1.billfordul = DIK_SPACE;  // Circle
	mut->opciok1.billview   = DIK_T;      // D-Pad Up
	mut->opciok1.billtime   = DIK_V;      // D-Pad Down
	mut->opciok1.billshowkep= 0;          // no button left for it

	// (port) Players B, C and D: Pads 2, 3 and 4, same layout as
	// player A (Cross, Square, D-pad, Circle, D-pad Down / Up):
	jatekosopciok* padosok[3] = { &mut->opciok2, &mut->opciok3, &mut->opciok4 };
	for( int p = 0; p < 3; p++ ) {
		jatekosopciok* o = padosok[p];
		o->billgaz    = PORT_PADKOD( p, bitindex( SCE_CTRL_CROSS ) );
		o->billfek    = PORT_PADKOD( p, bitindex( SCE_CTRL_SQUARE ) );
		o->billugras1 = PORT_PADKOD( p, bitindex( SCE_CTRL_RIGHT ) );
		o->billugras2 = PORT_PADKOD( p, bitindex( SCE_CTRL_LEFT ) );
		o->billfordul = PORT_PADKOD( p, bitindex( SCE_CTRL_CIRCLE ) );
		o->billview   = PORT_PADKOD( p, bitindex( SCE_CTRL_DOWN ) );
		o->billtime   = PORT_PADKOD( p, bitindex( SCE_CTRL_UP ) );
		o->billshowkep= 0;
	}

	// (port) L / R zoom by default; the screen size keys start unassigned:
	mut->billplussz = 0;
	mut->billminusz = 0;
	mut->billzoomin  = DIK_EQUALS;        // Pad 1 R
	mut->billzoomout = DIK_MINUS;         // Pad 1 L
	mut->billsnap = DIK_I;                // Triangle
}

// Names shown in Options / Customize controls:
char* kodtobillnev( int billkod ) {
	// (port) Controller buttons: "Pad 2 Cross" etc. (the external controllers
	// are Pad 2..5, the Vita itself is Pad 1). A few rotating buffers, as the
	// caller may ask for several names before copying them:
	if( billkod >= PORT_PADKOD( 0, 0 ) && billkod < PORT_KODSZAM ) {
		static const char* gombnev[32] = {
			"Select", "L3", "R3", "Start", "Up", "Right", "Down", "Left",
			"L2", "R2", "L1", "R1", "Triangle", "Circle", "Cross", "Square" };
		static char puffer[8][24];
		static int kov = 0;
		int p = (billkod - PORT_PADKOD( 0, 0 )) / 32;
		int b = (billkod - PORT_PADKOD( 0, 0 )) % 32;
		char* ki = puffer[kov];
		kov = (kov+1) % 8;
		if( b < 16 && gombnev[b] )
			sprintf( ki, "Pad %d %s", p+2, gombnev[b] );
		else
			sprintf( ki, "Pad %d button %d", p+2, b );
		return ki;
	}
	// The Vita's own buttons (with a PS TV or reVita's "Vita as virtual DS4",
	// also the first controller):
	switch( billkod ) {
		case DIK_UP:     return (char*)"Pad 1 Cross";
		case DIK_DOWN:   return (char*)"Pad 1 Square";
		case DIK_RIGHT:  return (char*)"Pad 1 Right";
		case DIK_LEFT:   return (char*)"Pad 1 Left";
		case DIK_SPACE:  return (char*)"Pad 1 Circle";
		case DIK_V:      return (char*)"Pad 1 Up";
		case DIK_T:      return (char*)"Pad 1 Down";
		case DIK_MINUS:  return (char*)"Pad 1 L";
		case DIK_EQUALS: return (char*)"Pad 1 R";
		case DIK_I:      return (char*)"Pad 1 Triangle";
		case DIK_ESCAPE: return (char*)"Pad 1 Select";
		case DIK_RETURN: return (char*)"Pad 1 Start";
		default:         return NULL;
	}
}

int controlaltnyomva( void ) {
	return 0; // There is no Ctrl+Alt on the Vita
}

// ---------------------------------------------------------------------------
// Keyboard character queue (replaces W_K.CPP)
// ---------------------------------------------------------------------------

#define BUFFSIZE (160)
static int Buffer[BUFFSIZE];
static unsigned char BufferIsm[BUFFSIZE]; // (port) 1 = this character is an auto-repeat
static int Utolsoismetelt = 0;            // (port) was the last returned character a repeat?
static unsigned char Buffer1[PORT_KODSZAM], Buffer2[PORT_KODSZAM]; // (port) incl. controllers
static int Elsojon = 1;
static int Charszam = 0;
static int Konvtomb[256];

static void beirchar( int c, int ismetles = 0 ) {
	if( Charszam < BUFFSIZE ) {
		BufferIsm[Charszam] = (unsigned char)ismetles;
		Buffer[Charszam++] = c;
	}
}

static void initwkonvtomb( void ) {
	for( int i = 0; i < 256; i++ )
		Konvtomb[i] = 0;
	Konvtomb[DIK_ESCAPE] = MK_ESC;
	Konvtomb[DIK_RETURN] = MK_ENTER;
	Konvtomb[DIK_UP] = MK_UP;
	Konvtomb[DIK_DOWN] = MK_DOWN;
	Konvtomb[DIK_LEFT] = MK_LEFT;
	Konvtomb[DIK_RIGHT] = MK_RIGHT;
	Konvtomb[DIK_PRIOR] = MK_PGUP;
	Konvtomb[DIK_NEXT] = MK_PGDOWN;
	Konvtomb[DIK_SPACE] = ' ';
	Konvtomb[DIK_BACK] = MK_BACKSPACE;
	// Gameplay-only codes produce no characters
}

void mk_init( void ) {
	memset( Buffer1, 0, sizeof( Buffer1 ) );
	memset( Buffer2, 0, sizeof( Buffer2 ) );
	Charszam = 0;
	Elsojon = 1;
	initwkonvtomb();
}

int mk_getextchar( void ) {
	while( 1 ) {
		mv_check();
		if( Charszam > 0 ) {
			int c = Buffer[0];
			Utolsoismetelt = BufferIsm[0]; // (port)
			for( int i = 0; i < Charszam-1; i++ ) {
				Buffer[i] = Buffer[i+1];
				BufferIsm[i] = BufferIsm[i+1];
			}
			Charszam--;
			return c;
		}
	}
}

// (port) 1 if the character last returned by mk_getextchar was an auto-repeat:
int mk_utolsoismetelt( void ) {
       return Utolsoismetelt;
}

void mk_emptychar( void ) {
	mv_check();
	Charszam = 0;
}

int mk_kbhit( void ) {
	mv_check();
	return Charszam > 0;
}

int mk_getstate( int kod ) {
	if( kod < 0 || kod >= PORT_KODSZAM ) // (port)
		return 0;
	return Elsojon ? Buffer2[kod] : Buffer1[kod];
}

static void mkw_getDIstate( void ) {
	if( Elsojon )
		refreshDI( Buffer1 );
	else
		refreshDI( Buffer2 );
	Elsojon = !Elsojon;
}

// (port) Auto-repeat for Up / Down / Page Up / Page Down in the menus:
static const unsigned int ISM_KEZDET = 500000; // microseconds held before repeating starts
static const unsigned int ISM_KOZ    = 80000;  // microseconds between repeats
int Port_listaismetles = 0; // (port) set by the editor's list dialogs: enables auto-repeat there
static int Ismkey = -1;
static SceUInt64 Ismkov = 0;

static void mkw_setkeydown( void ) {
	unsigned char *regi, *uj;
	if( Elsojon ) {
		regi = Buffer1;
		uj = Buffer2;
	}
	else {
		regi = Buffer2;
		uj = Buffer1;
	}
	for( int i = 0; i < 256; i++ )
		if( !regi[i] && uj[i] && Konvtomb[i] ) {
			beirchar( Konvtomb[i] );
			if( i == DIK_UP || i == DIK_DOWN || i == DIK_PRIOR || i == DIK_NEXT ) {
				Ismkey = i;
				Ismkov = sceKernelGetProcessTimeWide() + ISM_KEZDET;
			}
		}

	// Only in the menus, and only while the key is still held:
    if( ( Aktmod != MOD_MENU && !Port_listaismetles ) || Ismkey < 0 || !uj[Ismkey] ) {
		Ismkey = -1;
		return;
	}
	SceUInt64 most = sceKernelGetProcessTimeWide();
	// Charszam == 0: never build up a backlog if drawing is slow
	if( most >= Ismkov && Charszam == 0 ) {
		beirchar( Konvtomb[Ismkey], 1 );
		Ismkov = most + ISM_KOZ;
	}
}

// ---------------------------------------------------------------------------
// Mouse emulation for the level editor: left stick / touch move the cursor,
// Cross = left button, Circle = right button.
// ---------------------------------------------------------------------------

static double Egerx = PORT_FIZX/2, Egery = PORT_FIZY/2;
static SceUInt64 Utolsomozgas = 0;

// (port) X button drag: remember where X was pressed and whether the cursor moved
static int Xvolt = 0;
static int Xvonszol = 0;
static double Xstx = 0, Xsty = 0;

static void mozgategeret( void ) {
	SceUInt64 most = sceKernelGetProcessTimeWide();
	double dt = Utolsomozgas ? (most-Utolsomozgas)/1000000.0 : 0.0;
	Utolsomozgas = most;
	if( dt > 0.1 )
		dt = 0.1;

	const int holtsav = 24;
	int dx = (int)Lx-128, dy = (int)Ly-128;
	if( dx > -holtsav && dx < holtsav )
		dx = 0;
	if( dy > -holtsav && dy < holtsav )
		dy = 0;
	// (port) In the editor the D-pad also moves the cursor:
	if( Aktmod == MOD_EDITOR && !Port_listaismetles ) { // not while a level list is open
			const int dpadtemp = 100; // 128 would be full stick speed
			unsigned int g = Gombok & ~Tiltott;
			if( g & SCE_CTRL_LEFT )  dx -= dpadtemp;
			if( g & SCE_CTRL_RIGHT ) dx += dpadtemp;
			if( g & SCE_CTRL_UP )    dy -= dpadtemp;
			if( g & SCE_CTRL_DOWN )  dy += dpadtemp;
	}
	if( dx || dy ) {
		// Faster as the stick is pushed further (up to ~700 pixel/sec):
		double fx = dx/128.0, fy = dy/128.0;
		Egerx += fx*fabs( fx )*700.0*dt;
		Egery += fy*fabs( fy )*700.0*dt;
	}
	if( Erintes ) {
		// (port) the menu/editor picture is stretched over the whole screen
		Egerx = Erintx; // (port) the editor is drawn 1:1 now
		Egery = Erinty;
	}
	if( Egerx < 0 ) Egerx = 0;
	if( Egery < 0 ) Egery = 0;
	if( Egerx > PORT_FIZX-1 ) Egerx = PORT_FIZX-1;
	if( Egery > PORT_FIZY-1 ) Egery = PORT_FIZY-1;

	// (port) X works like the touch screen: after a drag, lifting X gives the
	// game a second click at the release point (a plain press stays one click).
	if( Aktmod != MOD_EDITOR ) {
			Xvolt = 0;
	}
	else {
			int xnyomva = ((Gombok & ~Tiltott) & SCE_CTRL_CROSS) != 0;
			if( xnyomva && !Xvolt ) {
					Xvonszol = 0;
					Xstx = Egerx;
					Xsty = Egery;
			}
			else if( xnyomva ) {
					double ddx = Egerx - Xstx, ddy = Egery - Xsty;
					if( ddx*ddx + ddy*ddy > 36.0 ) // moved more than ~6 pixels
							Xvonszol = 1;
			}
			else if( Xvolt && Xvonszol ) {
					Erintfazis = 1; // starts the 0-1-0 click sequence in getbutbmou
					Xvonszol = 0;
			}
			Xvolt = xnyomva;
	}
}

// (port) Editor dialogs keep their 640x480 layout and are drawn in the middle
// of the screen; while one is open, the mouse is reported relative to it
// (see dialogkozep in edituj.cpp):
int Port_dialofsx = 0, Port_dialofsy = 0;

void getmou( int* px, int* py ) {
	*px = (int)Egerx - Port_dialofsx;
	*py = (int)Egery - Port_dialofsy;
}

void setmou( int x, int y ) {
	Egerx = x + Port_dialofsx;
	Egery = y + Port_dialofsy;
}

int getbutbmou( void ) {
        mv_check();
        if( Aktmod != MOD_EDITOR )
                return 0;
        if( Erintklikk ) { // (port) touch = left button
                Erintlatolt = 1;
                return 1;
        }
        // (port) Second click after a drag: not pressed, pressed, not pressed.
        // Each call to this function moves one step, so the game sees a fresh press:
        if( Erintfazis == 1 ) { Erintfazis = 2; return 0; }
        if( Erintfazis == 2 ) { Erintfazis = 3; return 1; }
        if( Erintfazis == 3 ) { Erintfazis = 0; return 0; }
        return ((Gombok & ~Tiltott) & SCE_CTRL_CROSS) != 0;
}

int getbutjmou( void ) {
	mv_check();
	return Aktmod == MOD_EDITOR && ((Gombok & ~Tiltott) & SCE_CTRL_CIRCLE);
}

// ---------------------------------------------------------------------------
// Main loop helpers, timer, errors (replaces W_V.CPP)
// ---------------------------------------------------------------------------

void mv_check( int nemkellnyomas ) {
	// Not while queued (e.g. injected keyboard) characters are still being
	// processed, so typed text appears at once instead of letter by letter:
	if( Piszkos && !Bufflocked && Charszam == 0 )
		megjelenit();
	hang();
	olvasbemenet();
	mozgategeret();
	mkw_getDIstate();
	if( Aktmod == MOD_JATEK ) // no dimming or sleep during play / replays
		sceKernelPowerTick( SCE_KERNEL_POWER_TICK_DEFAULT );
	if( nemkellnyomas )
		return;
	mkw_setkeydown();
}

void dsoundfinish( void );

void mv_exit( char* text ) {
	text = text;
	dsoundfinish();
	ddfinish();
	if( Vitainit )
		vita2d_fini();
	sceKernelExitProcess( 0 );
	while( 1 )
		;
}

static double Startido = 0;

static double ezredmp( void ) {
	return sceKernelGetProcessTimeWide()/1000.0;
}

// Returns time in 1/182 seconds, like the original:
double mv_stopperido( void ) {
	return ezredmp()*0.182 - Startido;
}

void mv_startstopper( void ) {
	Startido = ezredmp()*0.182;
}

void getbiostort( unsigned short* pbios, unsigned short* ptort ) {
	SceUInt64 t = sceKernelGetProcessTimeWide();
	*pbios = (unsigned short)(t/54925);
	*ptort = (unsigned short)(t%54925);
}

int s_random( int maximum ) {
	return rand()%maximum;
}

// Shows a message with the system font when the game's own graphics are not
// available (e.g. the data files are missing):
static void szovegesuzenet( const char* text ) {
	if( !Vitainit )
		return;
	vita2d_pgf* pgf = vita2d_load_default_pgf();
	while( 1 ) {
		vita2d_start_drawing();
		vita2d_clear_screen();
		if( pgf ) {
			vita2d_pgf_draw_text( pgf, 40, 80, RGBA8( 255, 255, 255, 255 ), 1.0f,
								  "Elasto Mania" );
			vita2d_pgf_draw_text( pgf, 40, 140, RGBA8( 255, 255, 255, 255 ), 1.0f, text );
			vita2d_pgf_draw_text( pgf, 40, 480, RGBA8( 160, 160, 160, 255 ), 1.0f,
								  "Press X or O to exit." );
		}
		vita2d_end_drawing();
		vita2d_swap_buffers();
		SceCtrlData pad;
		memset( &pad, 0, sizeof( pad ) );
		sceCtrlPeekBufferPositive( 0, &pad, 1 );
		if( pad.buttons & (SCE_CTRL_CROSS | SCE_CTRL_CIRCLE) )
			break;
	}
	if( pgf )
		vita2d_free_pgf( pgf );
}

static int Hibaban = 0;
static char Tmphibatext[500];

static void directhiba( char* text1, char* text2, char* text3, char* text4 ) {
	FILE* h = fopen( Hibaban ? "error_b.txt" : "error.txt", "wt" );
	if( h ) {
		fprintf( h, "%s\n", text1 );
		if( text2 ) fprintf( h, "%s\n", text2 );
		if( text3 ) fprintf( h, "%s\n", text3 );
		if( text4 ) fprintf( h, "%s\n", text4 );
		if( Hibaban )
			fprintf( h, "Two errors while processing!\n" );
		fclose( h );
	}
	if( Hibaban )
		mv_exit();
	Hibaban = 1;

	// The error may come while a buffer is locked or during gameplay: release
	// the locks and show the message on the normal screen with menu
	// buttons (Cross/Start = Enter, Circle = Esc):
	Bufflocked = 0;
	void port_picfeloldas( void ); // d_pic.cpp
	port_picfeloldas();
	Locky0_alul = 0;
	Port_szeles = 0;
	Port_jatekgombok = 0;
	beallitkepmeret();

	if( Margrafikus ) {
		if( text4 && text3 ) {
			if( strlen( text3 ) + strlen( text4 ) < 490 ) {
				strcpy( Tmphibatext, text3 );
				strcat( Tmphibatext, " " );
				strcat( Tmphibatext, text4 );
				text3 = Tmphibatext;
			}
		}
		void grafikushiba( char* text1, char* text2 = NULL, char* text3 = NULL );
		grafikushiba( text1, text2, text3 );
		while( 1 ) {
			int c = mk_getextchar();
			if( c == MK_ESC || c == MK_ENTER )
				break;
		}
	}
	else {
		char text[600];
		snprintf( text, sizeof( text ), "%s %s %s %s", text1,
				  text2 ? text2 : "", text3 ? text3 : "", text4 ? text4 : "" );
		szovegesuzenet( text );
	}
	mv_exit();
}

void hiba( char* text1, char* text2, char* text3 ) {
	if( access( "message.inf", 0 ) == 0 )
		directhiba( (char*)"<<message.inf accessed>>", text1, text2, text3 );
	directhiba( (char*)"Sorry, internal error.", NULL, NULL, NULL );
}

void uzenet( char* text1, char* text2, char* text3 ) {
	if( strstr( text1, "memory" ) && !strstr( text1, "about" ) ) {
		if( access( "message.inf", 0 ) == 0 )
			directhiba( (char*)"message.inf accessed\n", text1, text2, text3 );
		directhiba( (char*)"Sorry, out of memory!", NULL, NULL, NULL );
	}
	directhiba( text1, text2, text3, NULL );
}

// ---------------------------------------------------------------------------
// On-screen keyboard
// ---------------------------------------------------------------------------

static void ascii2utf16( SceWChar16* dest, const char* sour, int max ) {
	int i = 0;
	for( ; sour && sour[i] && i < max-1; i++ )
		dest[i] = (SceWChar16)(unsigned char)sour[i];
	dest[i] = 0;
}

void port_ime_bevitel( const char* cim, const char* kezdo, int maxhossz,
					   int csakszam, int torlesszam, int torlokod,
					   int enterrel ) {
	static SceWChar16 cim16[128];
	static SceWChar16 kezdo16[SCE_IME_DIALOG_MAX_TEXT_LENGTH+1];
	static SceWChar16 eredmeny16[SCE_IME_DIALOG_MAX_TEXT_LENGTH+1];
	if( maxhossz < 1 )
		maxhossz = 1;
	if( maxhossz > 100 )
		maxhossz = 100;
	ascii2utf16( cim16, cim, 128 );
	ascii2utf16( kezdo16, kezdo, maxhossz+1 );
	memset( eredmeny16, 0, sizeof( eredmeny16 ) );

	SceImeDialogParam param;
	sceImeDialogParamInit( &param );
	param.supportedLanguages = SCE_IME_LANGUAGE_ENGLISH;
	param.languagesForced = SCE_TRUE;
	param.type = csakszam ? SCE_IME_TYPE_NUMBER : SCE_IME_TYPE_BASIC_LATIN;
	param.option = SCE_IME_OPTION_NO_AUTO_CAPITALIZATION;
	param.textBoxMode = SCE_IME_DIALOG_TEXTBOX_MODE_DEFAULT;
	param.dialogMode = SCE_IME_DIALOG_DIALOG_MODE_WITH_CANCEL;
	param.title = cim16;
	param.maxTextLength = maxhossz;
	param.initialText = kezdo16;
	param.inputTextBuffer = eredmeny16;

	Charszam = 0;
	if( Piszkos && !Bufflocked )
		megjelenit(); // the keyboard is drawn over the current screen
	if( sceImeDialogInit( &param ) < 0 ) {
		beirchar( MK_ESC );
		return;
	}
	while( sceImeDialogGetStatus() != SCE_COMMON_DIALOG_STATUS_FINISHED ) {
		hang();
		kirajzolkeret( 1 );
	}
	SceImeDialogResult eredmeny;
	memset( &eredmeny, 0, sizeof( eredmeny ) );
	sceImeDialogGetResult( &eredmeny );
	sceImeDialogTerm();

	// The button that closed the keyboard is still held: ignore it until released.
	olvasbemenet();
	Tiltott = Gombok;
	for( int p = 0; p < PORT_PADSZAM; p++ ) // (port)
		Padtiltott[p] = Padgombok[p];

	if( eredmeny.button != SCE_IME_DIALOG_BUTTON_ENTER ||
		(csakszam && eredmeny16[0] == 0) ) { // an empty number counts as cancel
		beirchar( MK_ESC );
		return;
	}
	for( int i = 0; i < torlesszam; i++ )
		beirchar( torlokod );
	for( int i = 0; eredmeny16[i] && i < maxhossz; i++ ) {
		SceWChar16 c = eredmeny16[i];
		if( c >= 32 && c < 127 )
			beirchar( (int)c );
	}
	if( enterrel )
		beirchar( MK_ENTER );
	megjelenit();
}

// ---------------------------------------------------------------------------
// Sound output (replaces W_SOUND2.CPP). The mixer (vita_hhigh.cpp) is the
// original one; here we only stream its output to the audio hardware.
// ---------------------------------------------------------------------------

#define HANG_FREKVENCIA (11025)
#define HANG_GRAIN      (256)
#define HANG_GYURU      (8192) // ring buffer, power of two
#define HANG_CEL        (1100) // keep about 1/10 sec queued, like the original

static short Gyuru[HANG_GYURU];
static volatile unsigned int Irpos = 0, Olvpos = 0;
static int Hangport = -1;
static SceUID Hangszal = -1;
static volatile int Hangfut = 0;
static volatile int Hangtorles = 0; // main thread asks the audio thread to drop the queue

static int hangszal( SceSize args, void* argp ) {
	args = args;
	argp = argp;
	static short ki[HANG_GRAIN];
	while( Hangfut ) {
		if( Hangtorles ) {
			Hangtorles = 0;
			Olvpos = Irpos;
		}
		unsigned int van = Irpos - Olvpos;
		__sync_synchronize(); // samples written before Irpos was advanced
		if( van >= HANG_GRAIN && van <= HANG_GYURU ) {
			for( int i = 0; i < HANG_GRAIN; i++ )
				ki[i] = Gyuru[(Olvpos+i) & (HANG_GYURU-1)];
			__sync_synchronize();
			Olvpos += HANG_GRAIN;
		}
		else
			memset( ki, 0, sizeof( ki ) );
		sceAudioOutOutput( Hangport, ki ); // blocks until the hardware needs more
	}
	return 0;
}

void initdsound( int secondaryeloir ) {
	secondaryeloir = secondaryeloir;
	if( Hangport >= 0 )
		return;
	Hangport = sceAudioOutOpenPort( SCE_AUDIO_OUT_PORT_TYPE_BGM, HANG_GRAIN,
									HANG_FREKVENCIA, SCE_AUDIO_OUT_MODE_MONO );
	if( Hangport < 0 ) {
		Hangenabled = 0;
		return;
	}
	int hangero[2] = { SCE_AUDIO_VOLUME_0DB, SCE_AUDIO_VOLUME_0DB };
	sceAudioOutSetVolume( Hangport, (SceAudioOutChannelFlag)(SCE_AUDIO_VOLUME_FLAG_L_CH |
						  SCE_AUDIO_VOLUME_FLAG_R_CH), hangero );
	Irpos = Olvpos = 0;
	Hangfut = 1;
	Hangszal = sceKernelCreateThread( "elma_hang", hangszal, 0x10000100, 0x4000, 0, 0, NULL );
	if( Hangszal >= 0 )
		sceKernelStartThread( Hangszal, 0, NULL );
}

void dsoundfinish( void ) {
	if( Hangszal >= 0 ) {
		Hangfut = 0;
		sceKernelWaitThreadEnd( Hangszal, NULL, NULL );
		sceKernelDeleteThread( Hangszal );
		Hangszal = -1;
	}
	if( Hangport >= 0 ) {
		sceAudioOutReleasePort( Hangport );
		Hangport = -1;
	}
}

void callbackhang( short* sbuff, int buffsize ); // vita_hhigh.cpp

// Called often from mv_check: tops up the ring buffer with mixed sound.
void hang( void ) {
	if( !Hangenabled || Hangport < 0 )
		return;
	static short darab[HANG_GRAIN];
	while( Irpos - Olvpos < HANG_CEL ) {
		int getstatesoundon( void );
		if( Mute || !getstatesoundon() )
			memset( darab, 0, sizeof( darab ) );
		else
			callbackhang( darab, HANG_GRAIN );
		for( int i = 0; i < HANG_GRAIN; i++ )
			Gyuru[(Irpos+i) & (HANG_GYURU-1)] = darab[i];
		__sync_synchronize(); // publish the samples before Irpos
		Irpos += HANG_GRAIN;
	}
}

void nullazhangot( void ) {
	// Drop what is queued so sound stops at once (done by the audio thread,
	// which is the only one that moves the read position):
	Hangtorles = 1;
}

// ---------------------------------------------------------------------------
// Entry point (replaces WinMain)
// ---------------------------------------------------------------------------

int main( int argc, char* argv[] ) {
	argc = argc;
	argv = argv;

	// The game renders in software; give it the full CPU/GPU clocks:
	scePowerSetArmClockFrequency( 444 );
	scePowerSetBusClockFrequency( 222 );
	scePowerSetGpuClockFrequency( 222 );
	scePowerSetGpuXbarClockFrequency( 166 );

	sceCtrlSetSamplingMode( SCE_CTRL_MODE_ANALOG );
	sceTouchSetSamplingState( SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START );

	SceAppUtilInitParam initparam;
	SceAppUtilBootParam bootparam;
	memset( &initparam, 0, sizeof( initparam ) );
	memset( &bootparam, 0, sizeof( bootparam ) );
	sceAppUtilInit( &initparam, &bootparam );
	SceCommonDialogConfigParam dialogconf;
	sceCommonDialogConfigParamInit( &dialogconf );
	sceAppUtilSystemParamGetInt( SCE_SYSTEM_PARAM_ID_LANG, (int*)&dialogconf.language );
	sceAppUtilSystemParamGetInt( SCE_SYSTEM_PARAM_ID_ENTER_BUTTON,
								 (int*)&dialogconf.enterButtonAssign );
	sceCommonDialogSetConfigParam( &dialogconf );

	vita2d_init();
	vita2d_set_clear_color( RGBA8( 0, 0, 0, 255 ) );
	vita2d_set_vblank_wait( 1 );
	Vitainit = 1;

	if( chdir( PORT_ADATKONYVTAR ) != 0 ) {
		szovegesuzenet( "Game data not found. Copy the game files to " PORT_ADATKONYVTAR );
		mv_exit();
	}

	void upgradehakell( void );
	upgradehakell();

	void initqopen( void );
	initqopen();

	mk_init();

	Mute = 1;

	void initmenukep1( void );
	void initmenukep2( void );
	initmenukep1();

	mv_check();

	if( access( "noretrac.inf", 0 ) == 0 )
		Waitforretrace = 0;

	ddinit();

	initmenukep2();

	mv_main();

	hiba( (char*)"main vegere erkezett!" );
	return 0;
}
