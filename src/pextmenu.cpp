#include	"all.h"


/*static egyszovsor Idoszovegek[4];

// Igaz, ha ujra jatszik, hamis ha listat ker:
static int afterplayext( char* filenev, long ido ) {
	int kurrens = 0;
	while( 1 ) {
		// Ido feliratot keszitunk:
		char idoszov1[30] = "", idoszov2[30] = "";
		if( ido < 0 ) {
			strcpy( idoszov1, "You Failed" );
			strcpy( idoszov2, " to Finish!" );
		}
		else {
			strcpy( idoszov1, "Your time:" );
			ido2string( ido, idoszov2 );
		}

		//Korny->pabc_afterplaymenu->write( Korny->picbuffer, 200, 104, idoszov1 );
		strcpy( Idoszovegek[0].text, idoszov1 );
		Idoszovegek[0].x = 200;
		Idoszovegek[0].y = 104;
		//Korny->pabc_afterplaymenu->write( Korny->picbuffer, 200, 134, idoszov2 );
		strcpy( Idoszovegek[1].text, idoszov2 );
		Idoszovegek[1].x = 200;
		Idoszovegek[1].y = 134;

		// Kiirjuk ha lockolt:
		if( Ptop->lezart ) {
			//Korny->pabc_afterplaymenu->write( Korny->picbuffer,
			//		9, 173, "Locked" );
			strcpy( Idoszovegek[2].text, "Locked" );
			Idoszovegek[2].x = 9;
			Idoszovegek[2].y = 173;
		}

		//Korny->pal_afterplaymenu->set();
		// Valasztas:
		valaszt2 val;
		val.kur = kurrens;
		val.egykepen = 6;
		val.x0 = 50;
		val.y0 = 40;
		val.dy = 25;
		val.escelheto = 1;
		char* levdesc = getlevelname( filenev );
		if( levdesc )
			strcpy( val.cim, levdesc );
		else
			strcpy( val.cim, "External level" );

		strcpy( Rubrikak[0], "Play again" );
		strcpy( Rubrikak[1], "Replay" );
		strcpy( Rubrikak[2], "Save play" );
		strcpy( Rubrikak[3], "File list" );

		// Idoszoveg helyere beirjuk ecset-ben levok lefoglaltak szamat:
		//sprintf( Idoszovegek[0].text, "%d", Osszegszam );

		val.bead( 4 );
		int eredmeny = 0;
		if( Ptop->lezart )
			eredmeny = val.valassz( Idoszovegek, 3 );
		else
			eredmeny = val.valassz( Idoszovegek, 2 );
		kurrens = eredmeny;

		if( eredmeny < 0 || eredmeny == 3 )
			return 0; // Lista
		if( eredmeny == 0 )
			return 1; // Play again
		if( eredmeny == 1 ) {
			replayjelenlegi(); // Replay
			Pmenupal->set();
			continue;
		}
		if( eredmeny == 2 ) {
			unsigned long belyeg = getbelyeg( filenev );
			saveplay( belyeg ); // Save play
			continue;
		}
		hiba( "Itt nem szabadna lennie (afterplayext)!" );
	}
} */

int abcbenelobb( char* text1, char* text2 );

// (port) qsort instead of bubble sort: the list is no longer capped and can
// hold tens of thousands of files.
static int abchasonlit( const void* a, const void* b ) {
	char* nev1 = (char*)a;
	char* nev2 = (char*)b;
	if( abcbenelobb( nev1, nev2 ) )
		return -1;
	if( abcbenelobb( nev2, nev1 ) )
		return 1;
	return 0;
}

static void abcberendez( int szam ) {
	if( szam > 1 )
		qsort( Rubrikak, szam, sizeof( rubrika ), abchasonlit );
}

// (port) Folder navigation helpers (Levalmappa is the current subfolder)
static void mappafel( char* nev ) { // leaves the last folder; nev = its name
	nev[0] = 0;
	size_t l = strlen( Levalmappa );
	if( l == 0 )
		return;
	if( Levalmappa[l-1] == '\\' )
			Levalmappa[--l] = 0;
	char* p = strrchr( Levalmappa, '\\' );
	if( p ) {
		strcpy( nev, p+1 );
		p[1] = 0;
	}
	else {
		strcpy( nev, Levalmappa );
		Levalmappa[0] = 0;
	}
}

static void mappale( const char* nev ) {
	strcat( Levalmappa, nev );
	strcat( Levalmappa, "\\" );
}

// Nincs h-ja, hanem prototype ki van irva play.cpp-ben:
void playextmenu( void ) {
	Levalmappa[0] = 0; // (port) always start in lev itself
	char kezdonev[20] = ""; // folder we just left: the cursor starts on it

	while( 1 ) { // one pass per folder shown
		int szam = 0;

		// "[..]" (when inside a folder), then the subfolders:
		if( Levalmappa[0] )
				strcpy( Rubrikak[szam++], "..\\" );
		char levdir[200];
		snprintf( levdir, sizeof( levdir ), "lev\\%s", Levalmappa );
		char mappak[60][16];
		int mappadb = fialmappak( levdir, mappak, 60 );
		if( mappadb > 1 )
				qsort( mappak, mappadb, 16, abchasonlit );
		for( int i = 0; i < mappadb && szam < Maxrubrikaszam-4; i++ ) {
				if( strlen( Levalmappa ) + strlen( mappak[i] ) + 1 >= 80 )
						continue; // path would get too long
				sprintf( Rubrikak[szam++], "%s\\", mappak[i] );
		}
		int mappaszam = szam;

		// The levels of this folder:
		char minta[200];
		snprintf( minta, sizeof( minta ), "lev\\%s*.lev", Levalmappa );
		finame fname;
		int done = fifirst( minta, fname );
		while( !done ) {
				char* filenev = fname;
				strcpy( Rubrikak[szam], filenev );
				done = finext( fname );
				szam++;
				if( szam >= Maxrubrikaszam-4 )
						done = 1;
		}
		ficlose();

		if( szam <= 0 )
				return;

		if( szam-mappaszam > 1 )
				qsort( Rubrikak+mappaszam, szam-mappaszam, sizeof( rubrika ), abchasonlit );

		int kezdo = 0; // Ha megtalalja eddigit, akkor kur-t raallitja
		if( kezdonev[0] ) {
				char tmp[50];
				sprintf( tmp, "%s\\", kezdonev );
				for( int i = 0; i < mappaszam; i++ )
						if( strcmp( Rubrikak[i], tmp ) == 0 )
								kezdo = i;
				kezdonev[0] = 0;
		}
		else {
				for( int i = mappaszam; i < szam; i++ )
						if( strcmpi( Rubrikak[i], State->playextfilenev ) == 0 )
								kezdo = i;
		}

		// Feltoltottuk neveket, jon valasztas:
		valaszt2 val;
		val.kur = kezdo;
		val.egykepen = LISTegykepen;
		val.x0 = LISTx0;
		val.y0 = LISTy0;
		val.dy = LISTdy;
		val.escelheto = 1;
		val.korbe = 1; // (port)
		strcpy( val.cim, "Select External File!" );

		val.bead( szam );

		int ujlista = 0;
		while( !ujlista ) {
			int eredmeny = val.valassz();
			if( eredmeny < 0 ) {
				if( !Levalmappa[0] )
					return;
				mappafel( kezdonev ); // Esc inside a folder = one level up
				ujlista = 1;
				continue;
			}

			if( eredmeny < mappaszam ) { // a folder entry
				if( Levalmappa[0] && eredmeny == 0 )
					mappafel( kezdonev );
				else {
					char nev[20];
					strcpy( nev, val.rubrikak[eredmeny] ); // skip '['
					nev[strlen( nev )-1] = 0;                // cut ']'
					mappale( nev );
				}
				ujlista = 1;
				continue;
			}

			char filenev[20];
			if( strlen( val.rubrikak[eredmeny] ) > 15 )
				hiba( "playextmenu-ben val.rubrikak[eredmeny] ) > 15!" );
			strcpy( filenev, val.rubrikak[eredmeny] );

			strcpy( State->playextfilenev, filenev );

			while( 1 ) {
				kiirloading();
				// Ha topologiailag hibas lev file (uzenetet load adja):
				if( !floadlevel_p( filenev ) )
					break;
				Prec1->erase( filenev );
				recek234erase( filenev ); // (port) players B..D
				int ido = lejatszo( filenev );
				Pmenupal->set();
				char valasz[100] = "";
				idoelintezes( ido, valasz, 0, filenev );
				// Igaz, ha ujra jatszik, hamis ha listat ker:
				if( !afterplay( 0, 0, valasz, filenev ) ) {
					Prec1->erase( filenev );
					recek234erase( filenev ); // (port) players B..D
					break;
				}
			}
		}
	}
}






