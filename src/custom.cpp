#include "all.h"

#ifdef W_95
// W_DI.CPP-ben van definialva:
char* kodtobillnev( int billkod );
#else
static char* kodtobillnev( int billkod ) {
	static char egybetu[] = "A";
	if( billkod >= 'a' && billkod <= 'z' ) {
		egybetu[0] = (char)billkod;
		return egybetu;
	}
	switch( billkod ) {
		case MK_UP: 		return "Up";
		case MK_DOWN: 		return "Down";
		case MK_LEFT: 		return "Left";
		case MK_RIGHT: 		return "Right";
		case MK_PGUP:       return "PageUp";
		case MK_PGDOWN:     return "PageDown";
		case MK_BACKSPACE:  return "Backspace";

		default: return NULL;
	}
}
#endif

typedef int* billmuttomb[MAXRUBRIKASZAM_TAB+1];

static billmuttomb Pbilltomb0;
// (port) One table per player (A..D):
static billmuttomb Pbilltombj[4];
#define Pbilltomb1 (Pbilltombj[0])
#define Pbilltomb2 (Pbilltombj[1])

// (port) The main list: reset, Customize Player A..(B/C/D), then the common
// keys. How many players are listed depends on the play mode:
static int Testrjatekos = 2;      // players listed in the main list
static int Kozoskezd = 3;         // first common-key row (1 + Testrjatekos)
static int Rubrikaszam0 = 6;      // rows in the main list
static int Rubrikaszam12 = 8;

static void betolt( billmuttomb bmuttomb, int sorszam,
					char* szoveg, int* pbillkod ) {
	strcpy( Rubrikak[sorszam], szoveg );

	char* billnev = kodtobillnev( *pbillkod );
	char tmp[20] = "";
	if( !billnev ) {
		if( *pbillkod == 0 )
			sprintf( tmp, "???" );
		else
			sprintf( tmp, "Key code: %d", *pbillkod );
		billnev = tmp;
	}

	strcpy( Rubrikak_tab[sorszam], billnev );
	bmuttomb[sorszam] = pbillkod;
}

static void gombotvalaszt( int rubrikaszam, billmuttomb bmuttomb, int sorszam ) {
	int j;
	strcpy( Rubrikak_tab[sorszam], "_" );
	valaszt2 val;
	val.kur = sorszam;
	val.egykepen = 10;
	val.x0 = 60;
	val.x0_tab = 400;
	val.y0 = 86;
	val.dy = 40;
	val.escelheto = 1;

	strcpy( val.cim, "Customize controls" );
	strcpy( Rubrikak_tab[sorszam], "_" );

	val.bead( rubrikaszam, 1 ); // tab-os kiiras

	val.valassz( NULL, 0, 1 ); // Csak kirajzolja kepet

	// (port) Read the button with the gameplay mapping, so the binding stored
	// is the one used in game:
	port_gombfelvetel_guard portguard;

	// Megnezzuk milyen gombot nyomott le:
	while( 1 ) {
		mv_check( /*"cust.cpp"*/ );	
		for( int i = 1; i < PORT_KODSZAM; i++ ) { // (port) incl. controller buttons
			if( mk_getstate( MK_ESC ) )
				return;

			#ifdef W_95
				if( i == 28 || i == MK_ESC )
			#else
				if( i == MK_ENTER || i == MK_ESC )
			#endif
				continue;

			if( !mk_getstate( i ) )
				continue;

			// Ez a billentyu le van nyomva:

			//char tmp[10];
			//sprintf( tmp, "Bill: %d", i );
			//hiba( tmp );

			// Most meg van nyomva egy gomb:
			// Kiszedjuk mindenhonnan mashonnan ezt a billt:
			for( j = Kozoskezd; j < Rubrikaszam0; j++ ) { // (port)
				if( *Pbilltomb0[j] == i )
					*Pbilltomb0[j] = 0;
			}
			for( j = 0; j < Rubrikaszam12; j++ ) {    // 0-tol megy
				for( int p = 0; p < 4; p++ ) // (port) all four players
					if( *Pbilltombj[p][j] == i )
						*Pbilltombj[p][j] = 0;
			}
			*bmuttomb[sorszam] = i;
			return;
		}
		// Golyokat leptetjuk:
		val.kirajzol();
	}
}

static void bejegyez0( void ) {
	// (port) Players C and D only in 3 and 4 player modes:
	Testrjatekos = State->jatekosszam > 2 ? State->jatekosszam : 2;
	Kozoskezd = 1 + Testrjatekos;
	Rubrikaszam0 = Kozoskezd + 5; // (port) + Zoom In / Zoom Out
	strcpy( Rubrikak[0], "Reset all controls to default" );
	Rubrikak_tab[0][0] = 0;
	for( int p = 0; p < Testrjatekos; p++ ) {
		sprintf( Rubrikak[1+p], "Customize Player %c", 'A'+p );
		Rubrikak_tab[1+p][0] = 0;
	}
	betolt( Pbilltomb0, Kozoskezd,   "Inc. Screen Size", &State->billplussz );
	betolt( Pbilltomb0, Kozoskezd+1, "Dec. Screen Size", &State->billminusz );
	betolt( Pbilltomb0, Kozoskezd+2, "Zoom In",  &State->billzoomin );  // (port)
	betolt( Pbilltomb0, Kozoskezd+3, "Zoom Out", &State->billzoomout ); // (port)
	betolt( Pbilltomb0, Kozoskezd+4, "Make a Screenshot", &State->billsnap );
}

static void bejegyez12( billmuttomb bmuttomb, jatekosopciok* pjatopc ) {
	betolt( bmuttomb, 0, "Throttle", 			&pjatopc->billgaz );
	betolt( bmuttomb, 1, "Brake",    			&pjatopc->billfek );
	betolt( bmuttomb, 2, "Rotate left",    		&pjatopc->billugras2 );
	betolt( bmuttomb, 3, "Rotate right",   		&pjatopc->billugras1 );
	betolt( bmuttomb, 4, "Change direction",   	&pjatopc->billfordul );
	betolt( bmuttomb, 5, "Toggle Navigator",  	&pjatopc->billview );
	betolt( bmuttomb, 6, "Toggle Time",   		&pjatopc->billtime );
	betolt( bmuttomb, 7, "Toggle Show/Hide",   	&pjatopc->billshowkep );
}

static void customizeplayer( billmuttomb bmuttomb, 
				jatekosopciok* pjatopc, char* ABjel ) {
	int kurrens = 0;
	while( 1 ) {
		valaszt2 val;
		val.kur = kurrens;
		val.egykepen = 10;
		val.x0 = 60;
		val.x0_tab = 400;
		val.y0 = 86;
		val.dy = 40;
		val.escelheto = 1;
		val.torolheto = 1; // (port)

		strcpy( val.cim, "Customize Player " );
		strcat( val.cim, ABjel );

		bejegyez12( bmuttomb, pjatopc );

		val.bead( Rubrikaszam12, 1 ); // tab-os kiiras

		kurrens = val.valassz();

		if( kurrens < 0 )
				return;

		if( val.torolt ) { // (port) Square: unassign the highlighted control
				if( kurrens >= 0 && kurrens < Rubrikaszam12 )
						*bmuttomb[kurrens] = 0;
				continue; // redraws the list, which now shows "???"
		}

		if( kurrens >= 0 && kurrens < Rubrikaszam12 )
				gombotvalaszt( Rubrikaszam12, bmuttomb, kurrens );
	}
}

// (port) Loads every player's table (needed for removing duplicate bindings):
static void bejegyezjatekosok( void ) {
	for( int p = 0; p < 4; p++ )
		bejegyez12( Pbilltombj[p], State->opciok( p ) );
}

void customize( void ) {
	bejegyezjatekosok(); // (port)

	int kurrens = 0;
	while( 1 ) {
		valaszt2 val;
		val.kur = kurrens;
		val.egykepen = 10;
		val.x0 = 60;
		val.x0_tab = 400;
		val.y0 = 86;
		val.dy = 40;
		val.escelheto = 1;
		val.torolheto = 1; // (port)

		strcpy( val.cim, "Customize controls" );

		bejegyez0();

		val.bead( Rubrikaszam0, 1 ); // tab-os kiiras

		kurrens = val.valassz();

		if( kurrens < 0 )
				return;

		if( val.torolt ) { // (port) Square: unassign the highlighted control
				if( kurrens >= Kozoskezd && kurrens < Rubrikaszam0 )
						*Pbilltomb0[kurrens] = 0;
				continue;
		}

		if( kurrens == 0 ) {
			State->resetcontrols();
			bejegyezjatekosok(); // (port)
			bejegyez0();
		}

		// (port) Customize Player A..D:
		if( kurrens >= 1 && kurrens <= Testrjatekos ) {
			char jel[2] = { (char)('A'+kurrens-1), 0 };
			customizeplayer( Pbilltombj[kurrens-1], State->opciok( kurrens-1 ), jel );
			bejegyezjatekosok(); // the player screen reuses the row tables
		}

		if( kurrens >= Kozoskezd && kurrens < Rubrikaszam0 )
			gombotvalaszt( Rubrikaszam0, Pbilltomb0, kurrens );
	}
}


