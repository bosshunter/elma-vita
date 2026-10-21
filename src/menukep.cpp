#include	"all.h"

pic8* Buffsima = NULL;
pic8* Buffgolyo = NULL;

// (port) The menus are drawn natively at the Vita's 960x544 (MENUSZEL x
// MENUMAG). Buffsima/Buffgolyo belong to the level editor, so the menus have
// their own copies, and szoveglista moves the classic 640x480 layout to the
// centre (MENUOFSX, MENUOFSY):
static pic8* Menusima = NULL;
static pic8* Menugolyo = NULL;

static pic8 *Kisnevsima = NULL, *Kisnevgolyo = NULL;
abc8* Pmenuabc = NULL;
anim* Psisak = NULL;

static pic8 *Vegetile1 = NULL, *Vegetile2 = NULL;

ddpal* Pmenupal = NULL;
unsigned char Menupaltomb[768];

/*void retusal( pic8* ppic ) {
	int xsize = ppic->getxsize();
	int ysize = ppic->getysize();
	for( int y = 0; y < ysize; y++ ) {
		for( int x = 0; x < xsize; x++ ) {
			unsigned szin = ppic->gpixel( x, y );
			if( szin == 2 )
				ppic->ppixel( x, y, 30 );
		}
	}
} */

void initmenukep1( void ) {
	if( Buffsima )
		hiba( "initmenukep1-ben Buffsima!" );
	// (port) The level editor's screen, 960x544 like everything else:
	Buffsima = new pic8( PORT_FIZX, PORT_FIZY );
	Buffgolyo = new pic8( PORT_FIZX, PORT_FIZY );
	Menusima = new pic8( MENUSZEL, MENUMAG ); // (port)
	Menugolyo = new pic8( MENUSZEL, MENUMAG );
	Kisnevsima = new pic8( "szoveg1.pcx" ); //retusal( Kisnevsima );
	Kisnevgolyo = new pic8( "szoveg2.pcx" ); //retusal( Kisnevgolyo );
	Pmenuabc = new abc8( "menu.abc" );
	Pmenuabc->settav( 2 );

	pic8* sisaktmp = new pic8( "sisak.pcx" );
	forditkepet( sisaktmp );
	Psisak = new anim( sisaktmp, "sisak.pcx" );
	delete sisaktmp; sisaktmp = NULL;
	Psisak->korrigal();

	Vegetile1 = new pic8( "ures1b.pcx" );
	Vegetile2 = new pic8( "ures2b.pcx" );

	initgolyo();
}

void initmenukep2( void ) {
	if( Pmenupal )
		hiba( "initmenukep2-ben Pmenupal!" );
	pcxtopal( "intro.pcx", &Pmenupal ); // O new-ik egy ddpal-t
	pcxtopal( "intro.pcx", Menupaltomb );
}

void tilevaltas( void ) {
	for( int y = 0; y < Kisnevsima->getysize(); y += Vegetile1->getysize() ) {
		for( int x = 0; x < Kisnevsima->getxsize(); x += Vegetile1->getxsize() ) {
			blt8( Kisnevsima, Vegetile1, x, y );
			blt8( Kisnevgolyo, Vegetile2, x, y );
		}
	}
}

szoveglista::szoveglista( void ) {
	sisakx = sisaky = -100;

	elemszam = 0;
	ervenyes = 0;
	sorok = new egyszovsor[MAXSZOVEGSOR];
	if( !sorok )
		hiba( "Memoria!" );
}

szoveglista::~szoveglista( void ) {
	if( !sorok )
		hiba( "szoveglista::~szoveglista !sorok!" );
	delete sorok;
	sorok = NULL;
}

void szoveglista::addszoveg( char* text, int x, int y ) {
	if( strlen( text ) > 100 )
		hiba( "89ygui" );
	if( elemszam >= MAXSZOVEGSOR-1 )
		hiba( "849yptgp8" );
	strcpy( sorok[elemszam].text, text );
	sorok[elemszam].x = x + MENUOFSX; // (port) centre the 640x480 layout
	sorok[elemszam].y = y + MENUOFSY;
	elemszam++;
	ervenyes = 0;
}

void szoveglista::addszoveg_kozep( char* text, int x, int y ) {
	addszoveg( text, x-Pmenuabc->len( text )/2, y );
}

// (port) Screen coordinates on the whole 960x544 menu screen:
void szoveglista::addszoveg_szeles( char* text, int x, int y ) {
	addszoveg( text, x - MENUOFSX, y - MENUOFSY );
}

void szoveglista::addszoveg_szeles_kozep( char* text, int x, int y ) {
	addszoveg_szeles( text, x-Pmenuabc->len( text )/2, y );
}

void szoveglista::setsisak( int x, int y ) {
	sisakx = x + MENUOFSX; // (port)
	sisaky = y + MENUOFSY;
}

void szoveglista::setsisak_szeles( int x, int y ) {
	sisakx = x;
	sisaky = y;
}

void szoveglista::clear( void ) {
	elemszam = 0;
	ervenyes = 0;
}


static pic8* Tmpbuffer = NULL; // Ideiglenes

static int Scrolly1 = 0; // Ennel kiseb x-ek helyere sotet zoldet rak kirazgolyo
static unsigned char Introhatter = 254;
static unsigned char* Zoldsor = NULL;
static unsigned char* Hattersor = NULL;

static void kirajzgolyo( vekt2 g, double r, pic8* forras ) {
	if( !Zoldsor ) {
		Zoldsor = new unsigned char[MENUSZEL]; // (port)
		Hattersor = new unsigned char[MENUSZEL];
		for( int i = 0; i < MENUSZEL; i++ ) {
			Zoldsor[i] = 248;
			Hattersor[i] = Introhatter;
		}
	}

	if( Nincsfal ) {
		if( g.x < -500.0 || g.x > 1500.0 ||
			g.y < -500.0 || g.y > 1500.0 )
			return;
	}

	double xo = g.x;
	double yo = g.y;
	int y = yo - r;
	while( y < yo+r ) {
		if( y < 0 || y >= MENUMAG ) { // (port)
			y++;
			continue;
		}

		double dx = r*r - (y-yo)*(y-yo);
		if( dx < 0.5 ) {
			y++;
			continue;
		}
		dx = sqrt( dx );
		int x1 = xo-dx;
		int x2 = xo+dx;
		if( x1 < 0 )
			x1 = 0;
		if( x1 > MENUSZEL-1 ) // (port)
			x1 = MENUSZEL-1;
		if( x2 < 0 )
			x2 = 0;
		if( x2 > MENUSZEL-1 )
			x2 = MENUSZEL-1;
		if( x1 >= x2 ) {
			y++;
			continue;
		}
		unsigned char* dest = Tmpbuffer->getptr( y ) + x1;
		unsigned char* source;
		if( y < Scrolly1 ) {
			if( forras == Menugolyo ) // (port)
				source = Zoldsor;
			else
				source = Hattersor;
		}
		else
			source = forras->getptr( y ) + x1;
		BYTECOPY( dest, source, x2-x1+1 );

		y++;
	}
}

static double Regiido = 0.0;
static double Elsostabilido = 0.0; // Scroll befejezesekor
static int Kezdoanimjon = 1;

// (port) Tiles the menu background over the whole 960x544 menu screen. The
// pattern is anchored so that the centred 640x480 part looks exactly like the
// original screen:
static int Hatterverzio = 0; // (port) changes whenever Menusima / Menugolyo are redrawn (GPU)

static void hatterrajzol( void ) {
	Hatterverzio++;
	int tx = Kisnevsima->getxsize();
	int ty = Kisnevsima->getysize();
	// The original rows started at y = -47 (row 0), going down by ty:
	int sor = 0;
	int y = -47 + MENUOFSY;
	while( y > -ty ) {
		y -= ty;
		sor--;
	}
	while( y < MENUMAG ) {
		// The original row 'sor' started at x = 110*sor shifted left until <= 0:
		int x = 110*sor;
		x %= tx;
		if( x > 0 )
			x -= tx;
		x += MENUOFSX;
		while( x > 0 )
			x -= tx;
		while( x < MENUSZEL ) {
			blt8( Menusima, Kisnevsima, x, y );
			blt8( Menugolyo, Kisnevgolyo, x, y );
			x += tx;
		}
		y += ty;
		sor++;
	}
}

void szoveglista::kirajzol( int nemkellgolyo,
							pic8* rarakkep1, pic8* rarakkep2,
							pic8* rarakkep3, pic8* rarakkep4 ) {
	// Kirakjuk hatteret:
	if( !Tmpbuffer ) // Ideiglenes
		Tmpbuffer = new pic8( MENUSZEL, MENUMAG ); // (port)

	if( Kezdoanimjon && !Pintro )
		Kezdoanimjon = 0;

	if( !ervenyes && !Kezdoanimjon ) {
		// Ujrarajzoljuk hatterkepeket:
		ervenyes = 1;
		hatterrajzol(); // (port)

		// Rairjuk szovegeket vagy kepet:
		if( rarakkep4 ) {
			// (port) The four 640x120 strips go in the middle:
			pic8* kepek[4] = { rarakkep1, rarakkep2, rarakkep3, rarakkep4 };
			for( int k = 0; k < 4; k++ ) {
				blt8( Menusima, kepek[k], MENUOFSX, MENUOFSY + k*120 );
				blt8( Menugolyo, kepek[k], MENUOFSX, MENUOFSY + k*120 );
			}
		}
		else {
			for( int i = 0; i < elemszam; i++ ) {
				Pmenuabc->write( Menusima, sorok[i].x, sorok[i].y,
									sorok[i].text );
				Pmenuabc->write( Menugolyo, sorok[i].x, sorok[i].y,
									sorok[i].text );
			}
		}
	}

	double ujido = mv_stopperido();
	if( Kezdoanimjon ) {
		Pmenupal->set();
		while( 1 ) {
			if( !kirajzolanim( ujido ) ) {
				Kezdoanimjon = 0;
				Regiido = ujido;
				return;
			}
			
			mv_check( 1 ); // (port) present this frame; buttons pressed during the intro are not queued

			ujido = mv_stopperido();
			Elsostabilido = ujido;
			Kezdoanimjon = 0;
		}
	}

	double dt;
	if( ujido < Elsostabilido + 10.0 ) 
		dt = 0.0000000000001;
	else {
		dt = ujido-Regiido;
		Elsostabilido = -100.0;
	}

	Regiido = ujido;
	if( dt < 0.0 )
		dt = 0.001;

	dt *= 3.6;
	if( dt > 100.0 )
		dt = 100.0;

	leptetgolyo( dt ); // 12.0 volt regebben

	if( port_gpu() ) {
		// (port) The GPU draws the frame: the two pictures are uploaded when
		// they change, the balls are discs showing the same place of them.
		port_gm_hatter( Menusima, Menugolyo, Hatterverzio );
		port_korong k[64];
		int db = 0;
		for( int i = 0; i < Ngolyok && db+3 <= 64; i++ ) {
			if( nemkellgolyo || !State->animatedmenus )
				break;
			vekt2 r = Golyok[i].rmostani;
			double sug = Golyok[i].sug;
			vekt2 diff( sug*0.5*cos( Golyok[i].alfamostani ),
						sug*0.5*sin( Golyok[i].alfamostani ) );
			vekt2 r1 = r + diff, r2 = r - diff;
			k[db].x = (float)r.x;  k[db].y = (float)r.y;  k[db].r = (float)sug;      k[db].golyobol = 1; db++;
			k[db].x = (float)r1.x; k[db].y = (float)r1.y; k[db].r = (float)(sug*0.25); k[db].golyobol = 0; db++;
			k[db].x = (float)r2.x; k[db].y = (float)r2.y; k[db].r = (float)(sug*0.25); k[db].golyobol = 0; db++;
		}
		pic8* sisak = NULL;
		if( !nemkellgolyo ) {
			if( State->animatedmenus )
				sisak = Psisak->getframe( ujido*0.0024 );
			else
				sisak = Psisak->getframebyindex( 25 );
		}
		port_gm_rajz( db, k, sisak, sisakx-20, sisaky-7 );
		return;
	}

	blt8( Tmpbuffer, Menusima );

	for( int i = 0; i < Ngolyok; i++ ) {
		if( nemkellgolyo || !State->animatedmenus )
			break;

		kirajzgolyo( Golyok[i].rmostani, Golyok[i].sug, Menugolyo );

		vekt2 diff( Golyok[i].sug*0.5*cos( Golyok[i].alfamostani ),
					Golyok[i].sug*0.5*sin( Golyok[i].alfamostani ) );
		kirajzgolyo( Golyok[i].rmostani + diff,
					Golyok[i].sug*0.25, Menusima );
		kirajzgolyo( Golyok[i].rmostani - diff,
					Golyok[i].sug*0.25, Menusima );
	}
	if( !nemkellgolyo ) {
		pic8* sisakframe = NULL;
		if( State->animatedmenus )
			sisakframe = Psisak->getframe( ujido*0.0024 );
		else
			sisakframe = Psisak->getframebyindex( 25 );

		blt8( Tmpbuffer, sisakframe, sisakx-20, sisaky-7 );
	}

	bltfront( Tmpbuffer );
}

static int Elsokiranim = 1;
// Teljes olvassa be es elso kiirloading szabaditja fel:
extern pic8* Pintro = NULL;

static double Animkezdete = 0.0;

// (port) The intro: the menu slides up from the bottom of the 960x544 screen
// while the title picture (centred, like the loading screen) slides down.
// 'elt' is how far the menu still has to go (MENUMAG at the start, 0 at the end).
void szoveglista::kirajzolanimkocka( double ido, int elt ) {
	int i;
	Tmpbuffer->fillbox( Introhatter );
	blt8( Tmpbuffer, Menusima, 0, elt );

	// As in the original, the balls, the text and the helmet come down from
	// the top while the background comes up from the bottom:
	Scrolly1 = elt;
	double yeltolas = -elt;
	for( i = 0; i < Ngolyok; i++ ) {
		if( !State->animatedmenus )
			break;

		Golyok[i].rmostani.y += yeltolas;

		kirajzgolyo( Golyok[i].rmostani, Golyok[i].sug, Menugolyo );

		vekt2 diff( Golyok[i].sug*0.5*cos( Golyok[i].alfamostani ),
					Golyok[i].sug*0.5*sin( Golyok[i].alfamostani ) );
		kirajzgolyo( Golyok[i].rmostani + diff,
					Golyok[i].sug*0.25, Menusima );
		kirajzgolyo( Golyok[i].rmostani - diff,
					Golyok[i].sug*0.25, Menusima );

		Golyok[i].rmostani.y -= yeltolas;
	}

	// Rairjuk szovegeket:
	for( i = 0; i < elemszam; i++ )
		Pmenuabc->write( Tmpbuffer, sorok[i].x, sorok[i].y-elt,
							sorok[i].text );

	// Rarakjuk sisakot:
	pic8* sisakframe = NULL;
	if( State->animatedmenus && ido >= 0.0 )
		sisakframe = Psisak->getframe( ido*0.0024 );
	else
		sisakframe = Psisak->getframebyindex( 25 );

	blt8( Tmpbuffer, sisakframe, sisakx-20, sisaky-7-elt );

	blt8( Tmpbuffer, Pintro, MENUOFSX, MENUOFSY + (MENUMAG-elt) );
}

// Hamis ha animnak mar vege:
int szoveglista::kirajzolanim( double ido ) {
	if( Elsokiranim ) {
		leptetgolyo( 0.000000001 );
		Elsokiranim = 0;
		Animkezdete = ido;

		// Berajzoljuk hatterkepeket:
		hatterrajzol(); // (port)
		// Most egyszer kirakunk minden elemet kepre hogy
		// ne akadjon meg elso lejatszas kozben:
		kirajzolanimkocka( -1.0, 0 );
		// Idaig tartott minden elem kirakasa egyszer
	}

	ido -= Animkezdete;

	// (port) Same speed as the original 480-line scroll, over 544 lines:
	int frame = ido*2.3;

	if( frame >= MENUMAG ) {
		Scrolly1 = 0;
		return 0;
	}

	kirajzolanimkocka( ido, MENUMAG-frame );
	bltfront( Tmpbuffer );
	return 1;
}

#define MAXHIBASORHOSSZ (34)
typedef char hibasor[MAXHIBASORHOSSZ+4];
#define MAXHIBASOR (10)
static hibasor Hibasortomb[MAXHIBASOR];

void grafikushiba( char* text1, char* text2, char* text3 ) {
	int i;
	// (port) Drawn at the size of the current screen: 960x544, or 640x480 in
	// the level editor. The classic layout is centred on it:
	static pic8* Hibakep = NULL;
	port_kepmeretfrissit();
	pic8* cel = Buffsima;
	if( Port_kepx != Buffsima->getxsize() || Port_kepy != Buffsima->getysize() ) {
		if( Hibakep && (Hibakep->getxsize() != Port_kepx ||
						Hibakep->getysize() != Port_kepy) ) {
			delete Hibakep;
			Hibakep = NULL;
		}
		if( !Hibakep )
			Hibakep = new pic8( Port_kepx, Port_kepy );
		cel = Hibakep;
	}
	int ofsx = (cel->getxsize()-640)/2;
	int ofsy = (cel->getysize()-480)/2;
	// Kirajzoljuk hatteret Buffsima-ba:
	int tx = Kisnevsima->getxsize();
	int ty = Kisnevsima->getysize();
	int sor = 0;
	int y = ofsy;
	while( y > 0 ) {
		y -= ty;
		sor++;
	}
	int paros = sor & 1;
	while( y < cel->getysize() ) {
		int x = ofsx;
		if( paros )
			x -= tx/2;
		while( x > 0 )
			x -= tx;
		paros = !paros;
		while( x < cel->getxsize() ) {
			blt8( cel, Kisnevsima, x, y );
			x += tx;
		}
		y += ty;
	}

	int sorszam = 0;
	for( i = 0; i < 3; i++ ) {
		char* eredeti = text1;
		if( i == 1 )
			eredeti = text2;
		if( i == 2 )
			eredeti = text3;
		if( !eredeti )
			break;
		// (port) The lines are cut by writing into the text, which is often a
		// string literal: read-only on the Vita (this crashed the game when an
		// error message was too long for one line). Work on a copy:
		static char Masolat[600];
		strncpy( Masolat, eredeti, sizeof( Masolat )-1 );
		Masolat[sizeof( Masolat )-1] = 0;
		char* text = Masolat;

		while( 1 ) {
			if( sorszam >= MAXHIBASOR )
				break;
			if( strlen( text ) <= MAXHIBASORHOSSZ ) {
				// Siman belefer:
				strcpy( Hibasortomb[sorszam], text );
				sorszam++;
				break;
			}
			// Tul hosszu sor, le kell vagni belole:
			int j = MAXHIBASORHOSSZ;
			while( j > 0 ) {
				if( text[j] == ' ' )
					break;

				j--;
			}
			if( j <= 0 ) {
				// Nem talalt szovegben space-t:
				char c = text[MAXHIBASORHOSSZ];
				text[MAXHIBASORHOSSZ] = 0;	
				strcpy( Hibasortomb[sorszam], text );
				sorszam++;
				text[MAXHIBASORHOSSZ] = c;
				text += MAXHIBASORHOSSZ;	
			}
			else {
				char c = text[j];
				text[j] = 0;	
				strcpy( Hibasortomb[sorszam], text );
				sorszam++;
				text[j] = c;
				text += j;
				while( text[0] == ' ' ) // (port) was text[j]
					text++;
			}
		}
	}

	// Rairjuk szovegeket:
	for( i = 0; i < sorszam; i++ ) {
		int y = 240-20-sorszam*(40/2)+i*40;
		Pmenuabc->writekozep( cel, 320+ofsx, y+ofsy, Hibasortomb[i] );
	}
	// Kirakjuk:
	bltfront( cel );
	if( Pmenupal )
		Pmenupal->set();
}

void menuproba( void ) {
	szoveglista szl;
	szl.addszoveg( "ABCDEFGH", 100, 100 );
	szl.addszoveg( "abcdefgh", 100, 200 );
	Pmenupal->set();
	while( 1 ) {
		if( mk_kbhit() )
			hiba( "Vege!" );
		szl.kirajzol();
	}
}




/*void szoveglista::kirajzol( int nemkellgolyo ) {
	if( !ervenyes ) {
		// Ujrarajzoljuk hatterkepeket:
		ervenyes = 1;
		int y = 0;
		int paros = 0;
		while( y < 480 ) {
			int x = 0;
			if( paros )
				x -= Kisnevsima->getxsize()/2;
			paros = !paros;
			while( x < 640 ) {
				blt8( Buffsima, Kisnevsima, x, y );
				blt8( Buffgolyo, Kisnevgolyo, x, y );
				x += Kisnevsima->getxsize();
			}
			y += Kisnevsima->getysize();
		}
		// Rairjuk szovegeket:
		for( int i = 0; i < elemszam; i++ ) {
			Pmenuabc->write( Buffsima, sorok[i].x, sorok[i].y,
								sorok[i].text );
			Pmenuabc->write( Buffgolyo, sorok[i].x, sorok[i].y,
								sorok[i].text );
		}
	}

	double ujido = mv_stopperido();
	double dt = ujido-Regiido;
	Regiido = ujido;
	if( dt < 0.0 )
		dt = 0.001;

	dt *= 3.6;
	if( dt > 100.0 )
		dt = 100.0;

	leptetgolyo( dt ); // 12.0 volt regebben

	// Kirakjuk hatteret:
	if( !Tmpbuffer ) // Ideiglenes
		Tmpbuffer = new pic8( 640, 480 );

	blt8( Tmpbuffer, Buffsima );

	for( int i = 0; i < Ngolyok; i++ ) {
		if( nemkellgolyo || !State->animatedmenus )
			break;

		kirajzgolyo( Golyok[i].rmostani, Golyok[i].sug, Buffgolyo );

		vekt2 diff( Golyok[i].sug*0.5*cos( Golyok[i].alfamostani ),
					Golyok[i].sug*0.5*sin( Golyok[i].alfamostani ) );
		kirajzgolyo( Golyok[i].rmostani + diff,
					Golyok[i].sug*0.25, Buffsima );
		kirajzgolyo( Golyok[i].rmostani - diff,
					Golyok[i].sug*0.25, Buffsima );
	}
	if( !nemkellgolyo ) {
		pic8* sisakframe = NULL;
		if( State->animatedmenus )
			sisakframe = Psisak->getframe( ujido*0.0024 );
		else
			sisakframe = Psisak->getframebyindex( 25 );

		blt8( Tmpbuffer, sisakframe, sisakx-20, sisaky-7 );
	}

	bltfront( Tmpbuffer );
} */