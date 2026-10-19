#include	"all.h"

static int Maxnevhossz = 8; // Ez kisebb kell hogy legyen JATEKOSNEVHOSSZ-nal!

// Siker eseten igazzal ter vissza:
static int bevesznevet( char* nev, int visszater ) {
	szoveglista szovlist;

	int i = 0;
	mk_emptychar();
	int valtozott = 1;
	nev[0] = 0;
	while( 1 ) {
		// (port) Vita on-screen keyboard; typed text arrives as key presses:
		if( !mk_kbhit() )
			port_ime_bevitel( "Please enter your name:", "", Maxnevhossz, 0, 0, 0 );
		while( mk_kbhit() ) {
			int c = mk_getextchar();
			if( c == MK_ESC ) {
				if( visszater )
					return 0;
				kilep();
			}
			if( c == MK_ENTER ) {
				// ENTER
				if( i > 0 )
					return 1;
			}
			if( ( c >= 'A' && c <= 'Z' ) ||
				( c >= 'a' && c <= 'z' ) ||
				( c >= '0' && c <= '9' ) ) {
				if( i >= Maxnevhossz )
					continue;
				nev[i] = (char)c;
				nev[i+1] = 0;
				i++;
				valtozott = 1;
			}
			if( c == MK_BACKSPACE ) {
				// <- (torles):
				if( i > 0 ) {
					i--;
					nev[i] = 0;
					valtozott = 1;
				}
			}
		}
		if( valtozott ) {
			valtozott = 0;

			szovlist.clear();

			nev[i] = '_';
			nev[i+1] = 0;
			szovlist.addszoveg_kozep( nev, 320, 240 );
			nev[i] = 0;

			szovlist.addszoveg_kozep( "Please enter your name:", 320, 180 );
			//szovlist.addszoveg_kozep( "Press ESC to exit", 320, 300 );
		}
		szovlist.kirajzol();
	}
}

// (port) Player index: 0..3 = A..D. The original A/B entry points are kept.
void newjatekos_n( int index, int visszater );
void jatekosvalasztas_n( int index, int visszater );

void newjatekos( int A, int visszater ) {
	newjatekos_n( A ? 0 : 1, visszater );
}

void jatekosvalasztas( int A, int visszater ) {
	jatekosvalasztas_n( A ? 0 : 1, visszater );
}

void newjatekos_n( int index, int visszater ) {
	// Ellenorzes:
	if( State->jatekosokszama >= MAXJATEKOS-1 )
		uzenet( "Sorry, no more players can get onto the list!" );
	int maxrub = Maxrubrikaszam;
	if( MAXJATEKOS > maxrub )
		hiba( "MAXJATEKOS > Maxrubrikaszam!" );

	if( bevesznevet( State->jatekosok[int(State->jatekosokszama)].nev, visszater ) ) {
		// (port) any of the four players:
		strcpy( State->jatekosnev( index ),
					State->jatekosok[int(State->jatekosokszama)].nev );
		State->jatekosokszama++;
		if( State->jatekosokszama == 1 ) {
			// The very first profile is used by every player:
			for( int k = 0; k < 4; k++ )
				strcpy( State->jatekosnev( k ), State->jatekosnev( index ) );
		}
	}
	else {
		visszater = 1; // Ha elescapelte, akkor visszamegyunk menube
	}

	if( State->jatekosokszama == 0 )
		kilep();

	if( visszater )
		return;

	mainmenu();
}

void jatekosvalasztas_n( int index, int visszater ) {
	// Valasztas:
	valaszt2* pval = NULL;
	pval = new valaszt2;
	char* eddigjatekos = State->jatekosnev( index ); // (port)
	pval->egykepen = LISTegykepen;
	pval->x0 = LISTx0;
	pval->y0 = LISTy0;
	pval->dy = LISTdy;
	pval->escelheto = 1;
	strcpy( pval->cim, "Choose Player" );

	strcpy( Rubrikak[0], "Create New Player" );
	pval->kur = 0;
	for( int i = 0; i < State->jatekosokszama; i++ ) {
		if( strlen( State->jatekosok[i].nev ) > MAXRUBRIKAHOSSZ )
			hiba( "Tul hosszu nev jatekosvalasztas-ban!" );
		strcpy( Rubrikak[i+1], State->jatekosok[i].nev );

		if( strcmp( eddigjatekos, State->jatekosok[i].nev ) == 0 )
			pval->kur = i+1;
	}

	pval->bead( State->jatekosokszama + 1 );

	RECidokellhet = 1;
	RECidokellis = 0;
	int eredmeny = pval->valassz();
	RECidokellhet = 0;

	delete pval; pval = NULL;

	if( eredmeny < 0 ) {
		if( visszater )
			return;
		kilep();
	}
	if( eredmeny == 0 ) {
		newjatekos_n( index, visszater );
		return;
	}

	if( RECidokellis && visszater ) {
		// Ezt a jatekost akarja torolni (opciokbol hivva):
		if( State->jatekosokszama <= 1 )
			return;

		int torlendo = eredmeny-1;

		// Megjegyezzuk hogy A, B, C vagy D nem torolt jatekos-e veletlenul:
		int torlodott[4];
		for( int k = 0; k < 4; k++ ) // (port) all four players
			torlodott[k] = strcmp( State->jatekosnev( k ),
								   State->jatekosok[torlendo].nev ) == 0;

		// Toroljuk jatekost, tobbit elore hozzuk eggyel:
		for( int i = torlendo; i < State->jatekosokszama-1; i++ ) {
			// i-edik helyre elore hozzuk i+1-edik helyrol:
			State->jatekosok[i] = State->jatekosok[i+1];
		}
		State->jatekosokszama--;

		for( int k = 0; k < 4; k++ )
			if( torlodott[k] )
				strcpy( State->jatekosnev( k ), State->jatekosok[0].nev );

		return;
	}

	strcpy( State->jatekosnev( index ), State->jatekosok[eredmeny-1].nev ); // (port)

	if( visszater )
		return;

	mainmenu();
}


