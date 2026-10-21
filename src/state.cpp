#include	"all.h"
#include	"dik.h" // (port) key codes of the old L / R defaults

#ifdef BUTITOTT
	int Palyaszam = 2;
#endif

#ifndef BUTITOTT
	// (port) 19 in the shareware version (End level included), 55 in the
	// registered one (Visit included). Set by initqopen from elma.res:
	int Palyaszam = 55;
#endif

int getstatesoundon( void ) {
	return State->soundon;
}

// Van ugyanilyen neven topol.cpp-ben is ket fv.:
static void titkosread( void* mut, int hossz, FILE* h, char* nev ) {
	if( fread( mut, 1, hossz, h ) != hossz )
		uzenet( "Corrupt file, please delete it!", nev );
	unsigned char* pc = (unsigned char*)mut;
	short a = 23, b = 9782, c = 3391;

	for( int i = 0; i < hossz; i++ ) {
		pc[i] ^= a;
		a %= c;
		b += a*c;
		a = 31*b+c;
	}
}

// (port) A file being built in memory (state.dat, state34.dat):
struct mentpuffer {
	unsigned char* p;
	int n, kap;
	mentpuffer( void ) { p = NULL; n = kap = 0; }
	~mentpuffer( void ) { free( p ); }
};

static void pirs( mentpuffer* m, const void* adat, int hossz ) {
	if( m->n + hossz > m->kap ) {
		int uj = m->kap ? m->kap*2 : 64*1024;
		while( uj < m->n + hossz )
			uj *= 2;
		unsigned char* p = (unsigned char*)realloc( m->p, uj );
		if( !p )
			hiba( "memory pirs!" );
		m->p = p;
		m->kap = uj;
	}
	memcpy( m->p + m->n, adat, hossz );
	m->n += hossz;
}

static void titkoswrite( void* mut, int hossz, mentpuffer* h ) {
	int i;
	unsigned char* pc = (unsigned char*)mut;
	short a = 23, b = 9782, c = 3391;

	for( i = 0; i < hossz; i++ ) {
		pc[i] ^= a;
		a %= c;
		b += a*c;
		a = 31*b+c;
	}
	pirs( h, mut, hossz );
	a = 23; b = 9782; c = 3391;

	for( i = 0; i < hossz; i++ ) {
		pc[i] ^= a;
		a %= c;
		b += a*c;
		a = 31*b+c;
	}
}

static int Checknumber_state_s = 123432112;
static int Checknumber_state_r = 123432221;

static char Statenev[] = "state.dat";

state_s::state_s( char* nev ) {
	// Beallitjuk default ertekeket:
	memset( palyakidejei, 0, sizeof( palyakidejei ) );
	memset( jatekosok, 0, sizeof( jatekosok ) );
	jatekosokszama = 0;
	jatekosa[0] = 0;
	jatekosb[0] = 0;
	soundon = 1;
	secondarybuffer = 0; // ha igaz, kompatibilis mod
	single = 1;
	tag = 0;
	minibike = 0;
	aplayerabike = 1;
	highquality = 1;

	animatedobjects = 1;
	animatedmenus = 1;

	editfilenev[0] = 0;
	playextfilenev[0] = 0;

	// (port) 3/4 players (state34.dat):
	jatekosszam = 1;
	jatekosc[0] = 0;
	jatekosd[0] = 0;
	memset( palyak3, 0, sizeof( palyak3 ) );
	memset( palyak4, 0, sizeof( palyak4 ) );
	kulso34 = NULL;
	kulso34szam = 0;
	kulso34hely = 0;
	zoom = 1.0;
	kepmeret = 1.0;
	reszint = 1;

	resetcontrols();

	// Beolvasas ha kell:
	if( !nev )
		nev = Statenev;

	if( access( nev, 0 ) != 0 ) {
		// Nem letezik state.dat:
		load34(); // (port)
		save();
		return;
	}

	// Itt hagyni kell fopen-t!!!!!!!!!!!!!!!!!!!!:
	FILE* h = fopen( nev, "rb" );
	if( !h )
		hiba( "Nem nyilik state file!: ", nev );

	int verzio = 0;
	titkosread( &verzio, 4, h, nev );
	if( verzio != 200 )
		uzenet( "File version is incorrect!",
				"Please rename it!",
				nev );

	titkosread( palyakidejei, sizeof( palyakidejei ), h, nev );
	titkosread( jatekosok, sizeof( jatekosok ), h, nev );
	titkosread( &jatekosokszama, sizeof( int ), h, nev );
	titkosread( jatekosa, sizeof( jatekosa ), h, nev );
	titkosread( jatekosb, sizeof( jatekosb ), h, nev );

	titkosread( &soundon, 4, h, nev );
	titkosread( &secondarybuffer, 4, h, nev );

	titkosread( &single, sizeof( int ), h, nev );
	titkosread( &tag, sizeof( int ), h, nev );
	titkosread( &aplayerabike, sizeof( int ), h, nev );
	titkosread( &highquality, sizeof( int ), h, nev );

	titkosread( &animatedobjects, sizeof( int ), h, nev );
	titkosread( &animatedmenus, sizeof( int ), h, nev );

	titkosread( &opciok1, sizeof( opciok1 ), h, nev );
	titkosread( &opciok2, sizeof( opciok2 ), h, nev );
	titkosread( &billplussz, sizeof( billplussz ), h, nev );
	titkosread( &billminusz, sizeof( billminusz ), h, nev );
	titkosread( &billsnap, sizeof( billminusz ), h, nev );

	titkosread( &editfilenev[0], 20, h, nev );
	titkosread( &playextfilenev[0], 20, h, nev );

	// Magic number:
	long checknumber = 0;
	if( fread( &checknumber, 1, sizeof( long ), h ) !=
					sizeof( long ) )
		uzenet( "Corrupt file, please rename it!", nev );
	if( Sarvari ) {
		if( checknumber != Checknumber_state_s )
			uzenet( "Corrupt file or registered version's file, please rename it!", nev );
	}
	else {
		if( checknumber != Checknumber_state_s &&
			checknumber != Checknumber_state_r )
			uzenet( "Corrupt file, please rename it!", nev );
	}

	fclose( h );

	load34(); // (port)
}

void idothuz( int* pi ) {
	*pi++;
}

void state_s::reloadidok( void ) {
	char* nev = Statenev;

	//if( access( nev, 0 ) != 0 )
	//	return;

	// Itt hagyni kell fopen-t!!!!!!!!!!!!!!!!!!!!:
	FILE* h = fopen( nev, "rb" );
	while( !h ) {
		// Var egy kicsit:
		int a = 0;
		for( int i = 0; i < 10000; i++ )
			idothuz( &a );

		h = fopen( nev, "rb" ); // (port) was a new variable, so it never ended
	}

	int verzio = 0;
	titkosread( &verzio, 4, h, nev );
	if( verzio != 200 )
		uzenet( "File version is incorrect!",
				"Please rename it!",
				nev );

	titkosread( palyakidejei, sizeof( palyakidejei ), h, nev );

	fclose( h );
}

state_s::~state_s( void ) {
	free( kulso34 ); // (port)
	kulso34 = NULL;
}

void state_s::save( void ) {
	// (port) The file is built in memory and written by a background thread
	// (only when it changed), so the menus don't stop while it is written:
	mentpuffer puffer;
	mentpuffer* h = &puffer;

	int verzio = 200;
	titkoswrite( &verzio, 4, h );
	titkoswrite( palyakidejei, sizeof( palyakidejei ), h );
	titkoswrite( jatekosok, sizeof( jatekosok ), h );
	titkoswrite( &jatekosokszama, sizeof( int ), h );
	titkoswrite( jatekosa, sizeof( jatekosa ), h );
	titkoswrite( jatekosb, sizeof( jatekosb ), h );

	titkoswrite( &soundon, 4, h );
	titkoswrite( &secondarybuffer, 4, h );

	titkoswrite( &single, sizeof( int ), h );
	titkoswrite( &tag, sizeof( int ), h );
	titkoswrite( &aplayerabike, sizeof( int ), h );
	titkoswrite( &highquality, sizeof( int ), h );

	titkoswrite( &animatedobjects, sizeof( int ), h );
	titkoswrite( &animatedmenus, sizeof( int ), h );

	titkoswrite( &opciok1, sizeof( opciok1 ), h );
	titkoswrite( &opciok2, sizeof( opciok2 ), h );
	titkoswrite( &billplussz, sizeof( billplussz ), h );
	titkoswrite( &billminusz, sizeof( billminusz ), h );
	titkoswrite( &billsnap, sizeof( billminusz ), h );

	titkoswrite( &editfilenev[0], 20, h );
	titkoswrite( &playextfilenev[0], 20, h );

	long checknumber = Checknumber_state_r;
	if( Sarvari )
		checknumber = Checknumber_state_s;
	pirs( h, &checknumber, sizeof( long ) );
	port_fajlmentes( Statenev, puffer.p, puffer.n );

	save34(); // (port)
}

// ---------------------------------------------------------------------------
// (port) state34.dat: players C and D, their controls and the number of
// players. Plain binary, little endian:
//   "EM34", int version, int jatekosszam, jatekosc, jatekosd, opciok3, opciok4
// Version 2 adds the 3/4 player best times:
//   int MAXPALYASZAM, palyak3[], palyak4[]   (internal levels)
//   int count, kulso34idok[count]           (external levels)
// Version 3 adds the zoom keys:
//   int billzoomin, int billzoomout
// Version 4 adds the current zoom:
//   double zoom
// Version 5 adds the screen size and the split screen gap:
//   double kepmeret, int reszint
// ---------------------------------------------------------------------------

static char State34nev[] = "state34.dat";
static const int State34verzio = 5;

palya34idok* state_s::idok34( int level, int n ) {
	if( level < 0 || level >= MAXPALYASZAM )
		hiba( "idok34 level!" );
	return n >= 4 ? &palyak4[level] : &palyak3[level];
}

palya34idok* state_s::kulsoidok34( const char* filenev, long belyeg, int n,
								   int letrehoz ) {
	char nev[24];
	strncpy( nev, filenev, sizeof( nev )-1 );
	nev[sizeof( nev )-1] = 0;
	strupr( nev );
	for( int i = 0; i < kulso34szam; i++ ) {
		if( kulso34[i].belyeg == belyeg && strcmp( kulso34[i].nev, nev ) == 0 )
			return n >= 4 ? &kulso34[i].idok4 : &kulso34[i].idok3;
	}
	if( !letrehoz )
		return NULL;
	if( kulso34szam >= kulso34hely ) {
		int ujhely = kulso34hely ? kulso34hely*2 : 32;
		kulso34idok* uj = (kulso34idok*)realloc( kulso34, ujhely*sizeof( kulso34idok ) );
		if( !uj )
			hiba( "Out of memory (kulsoidok34)!" );
		kulso34 = uj;
		kulso34hely = ujhely;
	}
	kulso34idok* k = &kulso34[kulso34szam++];
	memset( k, 0, sizeof( *k ) );
	strcpy( k->nev, nev );
	k->belyeg = belyeg;
	return n >= 4 ? &k->idok4 : &k->idok3;
}

// A table read from the file must not break anything:
static void javit34( palya34idok* p ) {
	if( p->idokszama < 0 || p->idokszama > MAXIDOK )
		p->idokszama = 0;
	for( int j = 0; j < 4; j++ )
		for( int i = 0; i < MAXIDOK; i++ )
			p->nevek[j][i][sizeof( jatekosnev_type )-1] = 0;
	for( int i = 0; i < p->idokszama; i++ )
		if( p->idok[i] < 0 )
			p->idok[i] = 0;
}

char* state_s::jatekosnev( int index ) {
	switch( index ) {
		case 0: return jatekosa;
		case 1: return jatekosb;
		case 2: return jatekosc;
		default: return jatekosd;
	}
}

jatekosopciok* state_s::opciok( int index ) {
	switch( index ) {
		case 0: return &opciok1;
		case 1: return &opciok2;
		case 2: return &opciok3;
		default: return &opciok4;
	}
}

// After loading: keep the player count consistent with 'single' (which is
// what the original game and its state.dat use) and fill in empty profiles.
static void ellenoriz34( state_s* s ) {
	if( s->single )
		s->jatekosszam = 1;
	else if( s->jatekosszam < 2 || s->jatekosszam > 4 )
		s->jatekosszam = 2;
	if( !s->jatekosc[0] )
		strcpy( s->jatekosc, s->jatekosa );
	if( !s->jatekosd[0] )
		strcpy( s->jatekosd, s->jatekosa );
	// A deleted profile falls back to player A's:
	for( int i = 2; i < 4; i++ ) {
		char* nev = s->jatekosnev( i );
		int van = 0;
		for( int j = 0; j < s->jatekosokszama; j++ )
			if( strcmp( nev, s->jatekosok[j].nev ) == 0 )
				van = 1;
		if( !van )
			strcpy( nev, s->jatekosa );
	}
}

void state_s::load34( void ) {
	int zoomvan = 0; // the zoom keys were in the file
	FILE* h = fopen( State34nev, "rb" );
	if( h ) {
		char magic[4];
		int verzio = 0;
		int ok = fread( magic, 1, 4, h ) == 4 && memcmp( magic, "EM34", 4 ) == 0 &&
				 fread( &verzio, 1, 4, h ) == 4 && verzio >= 1;
		int n = 0;
		jatekosnev_type c, d;
		jatekosopciok o3, o4;
		if( ok )
			ok = fread( &n, 1, 4, h ) == 4 &&
				 fread( c, 1, sizeof( c ), h ) == sizeof( c ) &&
				 fread( d, 1, sizeof( d ), h ) == sizeof( d ) &&
				 fread( &o3, 1, sizeof( o3 ), h ) == sizeof( o3 ) &&
				 fread( &o4, 1, sizeof( o4 ), h ) == sizeof( o4 );
		if( ok ) {
			c[sizeof( c )-1] = 0;
			d[sizeof( d )-1] = 0;
			if( !single )
				jatekosszam = n;
			strcpy( jatekosc, c );
			strcpy( jatekosd, d );
			opciok3 = o3;
			opciok4 = o4;
		}
		if( ok && verzio >= 2 ) {
			// Best times:
			int palyak = 0;
			if( fread( &palyak, 1, 4, h ) == 4 && palyak == MAXPALYASZAM &&
				fread( palyak3, 1, sizeof( palyak3 ), h ) == sizeof( palyak3 ) &&
				fread( palyak4, 1, sizeof( palyak4 ), h ) == sizeof( palyak4 ) ) {
				for( int i = 0; i < MAXPALYASZAM; i++ ) {
					javit34( &palyak3[i] );
					javit34( &palyak4[i] );
				}
				int szam = 0;
				if( fread( &szam, 1, 4, h ) == 4 && szam > 0 && szam < 1000000 ) {
					free( kulso34 );
					kulso34szam = kulso34hely = 0;
					kulso34 = (kulso34idok*)malloc( szam*sizeof( kulso34idok ) );
					if( kulso34 ) {
						kulso34hely = szam;
						kulso34szam = int(fread( kulso34, sizeof( kulso34idok ), szam, h ));
						for( int i = 0; i < kulso34szam; i++ ) {
							kulso34[i].nev[sizeof( kulso34[i].nev )-1] = 0;
							javit34( &kulso34[i].idok3 );
							javit34( &kulso34[i].idok4 );
						}
					}
				}
			}
			else {
				memset( palyak3, 0, sizeof( palyak3 ) );
				memset( palyak4, 0, sizeof( palyak4 ) );
				ok = 0;
			}
		}
		if( ok && verzio >= 3 ) {
			int be = 0, ki = 0;
			if( fread( &be, 1, 4, h ) == 4 && fread( &ki, 1, 4, h ) == 4 ) {
				billzoomin = be;
				billzoomout = ki;
				zoomvan = 1;
				double z = 1.0;
				if( verzio >= 4 && fread( &z, 1, sizeof( z ), h ) == sizeof( z ) &&
					z >= PORT_ZOOMMIN && z <= PORT_ZOOMMAX )
					zoom = z;
				double km = 1.0;
				int rsz = 1;
				if( verzio >= 5 && fread( &km, 1, sizeof( km ), h ) == sizeof( km ) &&
					fread( &rsz, 1, 4, h ) == 4 ) {
					if( km >= 0.69 && km <= 1.0 )
						kepmeret = km;
					if( rsz >= 0 && rsz <= 2 )
						reszint = rsz;
				}
				if( verzio < 5 && billsnap == DIK_I )
					billsnap = 0; // v5: Make a Screenshot is unassigned by default
			}
		}
		fclose( h );
	}
	if( !zoomvan ) {
		// (port) Make a Screenshot is unassigned by default since v1.1 (it was
		// I / Triangle, also the PC game's default):
		if( billsnap == DIK_I )
			billsnap = 0;
		// First start with zoom: L / R used to change the screen size (the
		// old defaults, also the PC game's - and = keys). They zoom now, and
		// the screen size keys become unassigned, as for a new player:
		if( billplussz == DIK_EQUALS && billminusz == DIK_MINUS ) {
			billplussz = 0;
			billminusz = 0;
		}
		if( billplussz == billzoomin || billplussz == billzoomout )
			billplussz = 0;
		if( billminusz == billzoomin || billminusz == billzoomout )
			billminusz = 0;
	}
	ellenoriz34( this );
}

void state_s::save34( void ) {
	mentpuffer puffer; // (port) written in the background, see save()
	mentpuffer* h = &puffer;
	int verzio = State34verzio;
	pirs( h, "EM34", 4 );
	pirs( h, &verzio, 4 );
	pirs( h, &jatekosszam, 4 );
	pirs( h, jatekosc, sizeof( jatekosc ) );
	pirs( h, jatekosd, sizeof( jatekosd ) );
	pirs( h, &opciok3, sizeof( opciok3 ) );
	pirs( h, &opciok4, sizeof( opciok4 ) );
	// Version 2: best times
	int palyak = MAXPALYASZAM;
	pirs( h, &palyak, 4 );
	pirs( h, palyak3, sizeof( palyak3 ) );
	pirs( h, palyak4, sizeof( palyak4 ) );
	pirs( h, &kulso34szam, 4 );
	if( kulso34szam )
		pirs( h, kulso34, sizeof( kulso34idok )*kulso34szam );
	// Version 3: zoom keys
	pirs( h, &billzoomin, 4 );
	pirs( h, &billzoomout, 4 );
	// Version 4: current zoom
	pirs( h, &zoom, sizeof( zoom ) );
	// Version 5: screen size and split screen gap
	pirs( h, &kepmeret, sizeof( kepmeret ) );
	pirs( h, &reszint, 4 );
	port_fajlmentes( State34nev, puffer.p, puffer.n );
}

static void exportegylevel( FILE* h, palyaegyfeleidok* pidok, int single ) {
	for( int i = 0; i < pidok->idokszama; i++ ) {
		char idostr[40];
		ido2string( pidok->idok[i], idostr, 1 );
		fprintf( h, "    " );
		fprintf( h, idostr );
		for( int k = 0; k < (12 - strlen( idostr)); k++ )
			fprintf( h, " " );
		fprintf( h, pidok->nevek1[i] );
		if( !single )
			fprintf( h, ", %s", pidok->nevek2[i] );
		fprintf( h, "\n" );
	}
}

// Ha nincs egy palyan ido, tiz percet szamol helyette:
// Ez itt szazadmasodpercben van megadva:
static int const Nincsmegpalyaido = 100*60*10;

// Anonymous total times:
void state_s::exportanonim( FILE* h, int single, char* text1, char* text2,
														char* text3 ) {
	int i;
	int sum = 0;
	for( i = 0; i < Palyaszam-1; i++ ) { // ucso palyat nem szamitjuk
		int levelsum = 100000000;
		// Single:
		palyaegyfeleidok* pidok = &palyakidejei[i].singleidok;
		if( pidok->idokszama > 0 )
			levelsum = pidok->idok[0];
		if( !single ) {
			// Multiplayer:
			palyaegyfeleidok* pidok = &palyakidejei[i].multiidok;
			if( pidok->idokszama > 0 && levelsum > pidok->idok[0] )
				levelsum = pidok->idok[0];
		}
		if( levelsum >= Nincsmegpalyaido )
			sum += Nincsmegpalyaido;
		else
			sum += levelsum;
	}

	fprintf( h, "%s\n%s\n%s\n", text1, text2, text3 );

	char idostr[40];
	ido2string( sum, idostr, 1 );
	fprintf( h, idostr );
	fprintf( h, "\n\n" );
}

void state_s::exportjatekossum( FILE* h, char* nev, int single ) {
	int i;
	int sum = 0;
	for( i = 0; i < Palyaszam-1; i++ ) { // ucso palyat nem szamitjuk
		int levelsum = 100000000;

		// Single times:
		palyaegyfeleidok* pidok = &palyakidejei[i].singleidok;
		for( int j = 0; j < pidok->idokszama; j++ ) {
			if( strcmp( nev, pidok->nevek1[j] ) == 0 /*||
				(!single && strcmp( nev, pidok->nevek2[j] ) == 0)*/ ) {
				// Megvan nev:
				levelsum = pidok->idok[j];
				break;
			}
		}

		if( !single ) {
			// Multi player times:
			palyaegyfeleidok* pidok = &palyakidejei[i].multiidok;
			for( int j = 0; j < pidok->idokszama; j++ ) {
				if( strcmp( nev, pidok->nevek1[j] ) == 0 ||
					strcmp( nev, pidok->nevek2[j] ) == 0 ) {
					// Megvan nev:
					if( levelsum > pidok->idok[j] )
						levelsum = pidok->idok[j];
					break;
				}
			}
		}

		if( levelsum >= Nincsmegpalyaido )
			sum += Nincsmegpalyaido;
		else
			sum += levelsum;
	}
	char idostr[40];
	ido2string( sum, idostr, 1 );
	fprintf( h, idostr );
	for( int k = 0; k < (12 - strlen( idostr)); k++ )
		fprintf( h, " " );
	fprintf( h, "%s\n", nev );
}

void state_s::exportstate( void ) {
	int i;
	FILE* h = fopen( "stats.txt", "wt" );
	if( !h )
		return; // (port) only an output file: never stop the game for it

	fprintf( h,
"This text file is generated automatically each time you quit the\n" );
	fprintf( h,
"ELMA.EXE program. If you modify this file, you will loose the\n" );
	fprintf( h,
"changes next time you run the game. This is only an output file, the\n" );
	fprintf( h,
"best times are stored in the STATE.DAT binary file.\n" );
	if( Sarvari )
	fprintf( h,
"Shareware version 1.0\n" );
	else
	fprintf( h,
"Registered version 1.0\n" );

	fprintf( h, "\n" );

	// Single palyak:
	fprintf( h, "Single player times:\n" );
	fprintf( h, "\n" );
	for( i = 0; i < Palyaszam-1; i++ ) { // utolsot nem irjuk ki
		fprintf( h, "Level %d, %s:\n", i+1, getleveldescription( i ) );
		palyaegyfeleidok* pidok = &palyakidejei[i].singleidok;
		exportegylevel( h, pidok, 1 );
		fprintf( h, "\n" );
	}

	fprintf( h, "\n" );
	// Multiplayer palyak:
	fprintf( h, "Multiplayer times:\n" );
	fprintf( h, "\n" );
	for( i = 0; i < Palyaszam-1; i++ ) { // utolsot nem irjuk ki
		fprintf( h, "Level %d, %s:\n", i+1, getleveldescription( i ) );
		palyaegyfeleidok* pidok = &palyakidejei[i].multiidok;
		exportegylevel( h, pidok, 0 );
		fprintf( h, "\n" );
	}

	// Single player total times:
	fprintf( h,
"The following are the single player total times for individual players.\n" );
	fprintf( h,
"If a player doesn't have a time in the top ten for a level, this\n" );
	fprintf( h,
"will add ten minutes to the total time.\n" );

	for( i = 0; i < jatekosokszama; i++ )
		exportjatekossum( h, State->jatekosok[i].nev, 1 );
	fprintf( h, "\n" );

	// Multi player total times:
	fprintf( h,
"The following are the combined total times for individual players. For each\n" );
	fprintf( h,
"level the best time is choosen of either the player's single player best\n" );
	fprintf( h,
"time, or the best multiplayer time where the player was one of the two\n" );
	fprintf( h,
"players.\n" );
	fprintf( h,
"If a player doesn't have such a time for a level, this will add ten\n" );
	fprintf( h,
"minutes to the total time.\n" );

	for( i = 0; i < jatekosokszama; i++ )
		exportjatekossum( h, State->jatekosok[i].nev, 0 );
	fprintf( h, "\n" );

	// Anonymous total times:
	exportanonim( h, 1,
		"The following is the anonymous total time of the best single player",
		"times. If there is no single player time for a level, this will",
		"add ten minutes to the total time." );

	exportanonim( h, 0,
		"The following is the anonymous combined total time of the best",
		"single or multiplayer times. If there is no single or multiplayer",
		"time for a level, this will add ten minutes to the total time." );

	fclose( h );
}

void state_s::resetcontrols( void ) {
#ifdef W_95
	void di_reset_state( state_s* mut );
	di_reset_state( this );
#else
	opciok1.billgaz    = 22;
	opciok1.billfek    = 40;
	opciok1.billugras1 = 37;
	opciok1.billugras2 = 35;
	opciok1.billfordul = ' ';
	opciok1.billview   = 'v';
	opciok1.billtime   = 't';
	opciok1.billshowkep= 59; // F1

	opciok2.billgaz = 71;    // Home
	opciok2.billfek = 79;
	opciok2.billugras1 = 81;
	opciok2.billugras2 = 83;
	opciok2.billfordul = 29; // CTRL
	opciok2.billview   = 'b';
	opciok2.billtime   = 'y';
	opciok2.billshowkep= 60;

	// Kozos:
	billplussz = '+';
	billminusz = '-';
	billsnap = 'i';
#endif
}

int getjatekosindex( char* nev ) {
	for( int i = 0; i < MAXJATEKOS; i++ ) {
		jatekos* pjatekos = &State->jatekosok[i];
		if( strcmp( pjatekos->nev, nev ) == 0 )
			return i;
	}
	hiba( "getjatekosindex-ben nem talalja nevet!" );
	return 0;
}



