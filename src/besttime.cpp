#include	"all.h"

void ido2string( long l, char* text, int hosszu ) {
	if( l < 0 )
		hiba( "ido2string-ben ido < 0!" );
	int szazad = int(l%100);
	l /= 100;
	int masodperc = int(l%60);
	l /= 60;
	int perc = int(l%60);
	l /= 60;
	int ora = 0;
	if( l ) {
		if( hosszu ) {
			ora = l;
		}
		else {
			szazad = 99;
			masodperc = 59;
			perc = 59;
		}
	}
	text[0] = 0;
	char tmp[30];
	if( ora ) {
		// ora:
		itoa( ora, tmp, 10 );
		if( !tmp[1] )
			strcat( text, "0" );
		strcat( text, tmp );
		strcat( text, ":" );
	}
	// perc:
	itoa( perc, tmp, 10 );
	if( !tmp[1] )
		strcat( text, "0" );
	strcat( text, tmp );
	strcat( text, ":" );
	// masodperc:
	itoa( masodperc, tmp, 10 );
	if( !tmp[1] )
		strcat( text, "0" );
	strcat( text, tmp );
	strcat( text, ":" );
	// szazad:
	itoa( szazad, tmp, 10 );
	if( !tmp[1] )
		strcat( text, "0" );
	strcat( text, tmp );
}

void elemibesttimes( palyaegyfeleidok* pidok, char* fejlec, int single ) {
	if( pidok->idokszama == 0 )
		return;

	szoveglista szl;

	szl.addszoveg_kozep( fejlec, 320, 37 );

	int x1, x2;
	if( single ) {
		x1 = 120;
		x2 = 360;
	}
	else {
		x1 = 80;
		x2 = 400;
	}
	for( int i = 0; i < pidok->idokszama; i++ ) {
		char szoveg[50];
		strcpy( szoveg, pidok->nevek1[i] );
		if( !single ) {
			strcat( szoveg, "  " );
			strcat( szoveg, pidok->nevek2[i] );
		}
		// Levagjuk hogy ne loghasson ki:
		while( Pmenuabc->len( szoveg ) > x2-x1-4 )
			szoveg[strlen( szoveg )-1] = 0;

		szl.addszoveg( szoveg, x1, 110+i*(SM+19) );
		char tmp[30];
		ido2string( pidok->idok[i], tmp );
		szl.addszoveg( tmp, x2, 110+i*(SM+19) );
	}

	mk_emptychar();
	while( 1 ) {
		if( mk_kbhit() ) {
			int c = mk_getextchar();
			if( c == MK_ESC || c == MK_ENTER )
				return;
		}
		szl.kirajzol();
	}
}

// (port) Cuts 'szoveg' until it is at most 'szel' pixels wide:
static void levag( char* szoveg, int szel ) {
	while( szoveg[0] && Pmenuabc->len( szoveg ) > szel )
		szoveg[strlen( szoveg )-1] = 0;
}

// (port) The players of one 3/4 player entry, "Ann  Bob  Cecil":
static void nevsor34( palya34idok* pidok, int i, int n, char* szoveg ) {
	szoveg[0] = 0;
	for( int j = 0; j < n; j++ ) {
		if( j )
			strcat( szoveg, "  " );
		strcat( szoveg, pidok->nevek[j][i] );
	}
}

// (port) Best time table of a 3 or 4 player game, using the whole width of the
// 960x544 screen (the names of up to 4 players fit on one line):
static void elemibesttimes34( palya34idok* pidok, char* fejlec, int n ) {
	if( !pidok || pidok->idokszama == 0 )
		return;

	szoveglista szl;
	szl.addszoveg_szeles_kozep( fejlec, MENUSZEL/2, 37+MENUOFSY );

	char tmp[40];
	sprintf( tmp, "%d Players", n );
	szl.addszoveg_szeles_kozep( tmp, MENUSZEL/2, 37+MENUOFSY+38 );

	int x1 = 40, x2 = MENUSZEL-190;
	for( int i = 0; i < pidok->idokszama; i++ ) {
		int y = 110+MENUOFSY+20+i*(SM+19);
		char szoveg[100];
		sprintf( szoveg, "%d.", i+1 );
		szl.addszoveg_szeles( szoveg, x1, y );
		nevsor34( pidok, i, n, szoveg );
		levag( szoveg, x2-(x1+44)-10 );
		szl.addszoveg_szeles( szoveg, x1+44, y );
		ido2string( pidok->idok[i], tmp );
		szl.addszoveg_szeles( tmp, x2, y );
	}

	mk_emptychar();
	while( 1 ) {
		if( mk_kbhit() ) {
			int c = mk_getextchar();
			if( c == MK_ESC || c == MK_ENTER )
				return;
		}
		szl.kirajzol();
	}
}

// (port)
void levelbesttimes34( int level, int n ) {
	char fejlec[100];
	sprintf( fejlec, "%d: %s", level+1, getleveldescription( level ) );
	elemibesttimes34( State->idok34( level, n ), fejlec, n );
}

// (port) External level (the one in Ptop):
void besttimes34( char* filenev, int n ) {
	elemibesttimes34( idok34keres( filenev, n, 0 ), Ptop->levelname, n );
}

void levelbesttimes( int level, int single ) {
	char fejlec[100];
	itoa( level+1, fejlec, 10 );
	strcat( fejlec, ": " );
	strcat( fejlec, getleveldescription( level ) );

	palyaegyfeleidok* pidok = &State->palyakidejei[level].singleidok;
	if( !single )
		pidok = &State->palyakidejei[level].multiidok;

	elemibesttimes( pidok, fejlec, single );
}

void besttimes( topol* ptop, int single ) {
	if( single )
		elemibesttimes( &ptop->idok.singleidok, ptop->levelname, single );
	else
		elemibesttimes( &ptop->idok.multiidok, ptop->levelname, single );
}

// (port) single: 1 = single player, 0 = 2 players, 3 or 4 = 3 or 4 players
void besttimes_egytipus( int single ) {
	int i;
	int n34 = 0;
	if( single >= 3 ) {
		n34 = single;
		single = 0;
	}
	int palyaszam = 0;
	for( i = 0; i < State->jatekosokszama; i++ ) {
		if( State->jatekosok[i].sikerespalyakszama > palyaszam )
			palyaszam = int(State->jatekosok[i].sikerespalyakszama);
	}
	/*if( palyaszam == 0 )
		return;
	if( palyaszam == 1 ) {
		levelbesttimes( 0, single );
		return;
	} */

	palyaszam++; // Mivel utolso nem sikeres palya is szamit
	if( palyaszam >= Palyaszam )
		palyaszam = Palyaszam-1; // mivel utolso palya nem rendes palya

	valaszt2 val;
	val.kur = 0;
	val.egykepen = LISTegykepen;
	val.x0 = 61;
	val.x0_tab = 380;
	val.y0 = LISTy0;
	val.dy = LISTdy;
	val.escelheto = 1;
	if( single )
		strcpy( val.cim, "Single Player Best Times" );
	else if( n34 )
		sprintf( val.cim, "%d Player Best Times", n34 ); // (port)
	else
		strcpy( val.cim, "Multi Player Best Times" );

	for( i = 0; i < palyaszam && n34; i++ ) {
		// (port) 3/4 players: level names on the left, the best entry's
		// players in the right column, which reaches the right edge of the
		// wide screen:
		palya34idok* p34 = State->idok34( i, n34 );
		itoa( i+1, Rubrikak[i], 10 );
		strcat( Rubrikak[i], " " );
		strcat( Rubrikak[i], getleveldescription( i ) );
		strcat( Rubrikak[i], " " );
		while( Pmenuabc->len( Rubrikak[i] ) < 184 )
			strcat( Rubrikak[i], " " );
		if( p34->idokszama > 0 ) {
			char nevek[100];
			nevsor34( p34, 0, n34, nevek );
			nevek[MAXRUBRIKAHOSSZ] = 0;
			levag( nevek, MENUSZEL-(val.x0_tab+MENUOFSX)-10 );
			strcpy( Rubrikak_tab[i], nevek );
		}
		else
			strcpy( Rubrikak_tab[i], "-" );
	}

	for( i = 0; i < palyaszam && !n34; i++ ) {
		palyaegyfeleidok* pidok = &State->palyakidejei[i].singleidok;
		if( !single )
			pidok = &State->palyakidejei[i].multiidok;
		int vanido = 1;
		if( pidok->idokszama == 0 )
			vanido = 0;
		char jatekosnev[50];
		strcpy( jatekosnev, pidok->nevek1[0] );
		if( !single ) {
			strcat( jatekosnev, "  " );
			strcat( jatekosnev, pidok->nevek2[0] );
		}
		// Sorszam 1-tol:
		itoa( i+1, Rubrikak[i], 10 );
		strcat( Rubrikak[i], " " );
		// Level neve:
		strcat( Rubrikak[i], getleveldescription( i ) );
		//strcat( Rubrikak[i], "Nincs meg nev." );
		strcat( Rubrikak[i], " " );
		while( Pmenuabc->len( Rubrikak[i] ) < 184 )
			strcat( Rubrikak[i], " " );
		// Jatekos neve:
		if( vanido )
			strcpy( Rubrikak_tab[i], jatekosnev );
		else
			strcpy( Rubrikak_tab[i], "-" );

		/*strcat( Korny->rubrikak[i], " " );
		// Legjobb idok kiirasa: Sajnos most nem fer ki:
		char tmp[30];
		ido2string( pidok->idok[0], tmp );
		strcat( Korny->rubrikak[i], tmp );
		*/
	}

	val.bead( palyaszam, 1 );
	while( 1 ) {
		int eredmeny = val.valassz();

		if( eredmeny < 0 )
			return;

		if( n34 )
			levelbesttimes34( eredmeny, n34 ); // (port)
		else
			levelbesttimes( eredmeny, single );
	}
}

void besttimes( void ) {
	valaszt2 val;
	if( State->single )
		val.kur = 0;
	else
		val.kur = 1;
	// (port) Two more tables: 3 and 4 players
	if( State->jatekosszam >= 3 )
		val.kur = State->jatekosszam-1;
	val.egykepen = 6;
	val.x0 = 170;
	val.y0 = 150;
	val.dy = 50;
	val.escelheto = 1;
	strcpy( val.cim, "Best Times" );

	strcpy( Rubrikak[0], "Single Player Times" );
	strcpy( Rubrikak[1], "Multi Player Times" );
	strcpy( Rubrikak[2], "3 Player Times" );
	strcpy( Rubrikak[3], "4 Player Times" );

	val.bead( 4 );
	while( 1 ) {
		int eredmeny = val.valassz();

		if( eredmeny < 0 )
			return;
		if( eredmeny == 0 )
			besttimes_egytipus( 1 );
		else if( eredmeny == 1 )
			besttimes_egytipus( 0 );
		else
			besttimes_egytipus( eredmeny+1 ); // (port) 3 or 4 players
	}
}

