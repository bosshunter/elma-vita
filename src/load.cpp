#include	"all.h"
void betoltecseteket( void );

int Volttopsave = 0;

static char Bentlevelnev[20] = "";

// Beolvas egy level-t ha kell (nev max 12 karakter).
// Torli es kitolti Ptop, Plgr, Pszak, ecset mutatokat.
// Ha belso file-ban is megvan palya onnan veszi:
// Hamissal ter vissza, ha hibas topologiaju level:
int floadlevel_p( char* nev ) {
	if( !nev )
		hiba( "floadlevel_p-ben !nev!" );
	if( strlen( nev ) > 12 || !nev[0] )
		hiba( "floadlevel_p-ben strlen( nev ) > 12 || !nev[0]!: ", nev );
	if( Volttopsave || !Ptop || !Pszak || strcmpi( nev, Bentlevelnev ) != 0 ) {
		// (port) The shareware version cannot load a registered level: say so
		// and return 0 (the caller goes back), instead of stopping the game.
		// The level loaded before stays loaded:
		if( csakregisztralt_lev( nev ) ) {
			csakregisztralt_uzenet( nev );
			return 0;
		}
		Volttopsave = 0;
		// Ujra be kell olvasni palyat:
		strcpy( Bentlevelnev, nev );
		if( Ptop )
			delete Ptop;
		Ptop = new topol( nev );
		if( Ptop->hibas ) {
			// Kiir hiba uzenetet:
			szoveglista szl;
			szl.addszoveg_kozep( "Level file has some topology errors!", 320, 190 );
			szl.addszoveg_kozep( "Use the editor to fix them!", 		320, 240 );
			mk_emptychar();
			while( 1 ) {
				if( mk_kbhit() ) {
					int c = mk_getextchar();
					if( c == MK_ESC || c == MK_ENTER )
						break;
				}
				szl.kirajzol();
			}

			delete Ptop;
			Ptop = NULL;
			return 0;
		}
		loadlgrfile( Ptop->lgrnev );
		Ptop->pic_selejtez( Plgr );

		if( Pszak )
			delete Pszak;
		Pszak = new szakaszok( Ptop );
		if( Fejsugar > Pmot1->kor2.sugar )
			Pszak->rendez( Fejsugar );
		else
			Pszak->rendez( Pmot1->kor2.sugar );

		betoltecseteket();
	}
	return 1;
}


// Torli es kitolti Ptop, Plgr, mutatokat.
// Pszak, ecset mutatokat torli:
// Ha belso file-ban is megvan palya onnan veszi:
// Igaz ha file nem volt lezarva, kulonben new palya hamissal:
int floadlevel_e( char* nev, int forceload ) {
	int levoltzarva = 0;

	if( !nev )
		hiba( "floadlevel_e-ben !nev!" );
	if( strlen( nev ) > 12 || !nev[0] )
		hiba( "floadlevel_e-ben strlen( nev ) > 12 || !nev[0]!: ", nev );
	if( forceload || Volttopsave || !Ptop || Ptop->lezart ||
							strcmpi( nev, Bentlevelnev ) != 0 ) {
		Volttopsave = 0;
		// Ujra be kell olvasni palyat:
		strcpy( Bentlevelnev, nev );
		if( Ptop )
			delete Ptop;
		if( strcmpi( nev, "_uj_topol_" ) == 0 ) {
			Ptop = new topol;
			Bentlevelnev[0] = 0;
		}
		else if( csakregisztralt_lev( nev ) ) {
			// (port) Shareware: a registered level cannot be opened. Say so and
			// start an empty level instead (as for a locked level below):
			char szoveg[80];
			sprintf( szoveg, "'%s' can only be loaded into the registered version!", nev );
			dialog( "Sorry, this level file", szoveg );
			levoltzarva = 1;
			Ptop = new topol;
			Bentlevelnev[0] = 0;
		}
		else
			Ptop = new topol( nev );

			if( Ptop->lezart && Sarvari ) {
				// (port) The shareware version keeps the original rule: a
				// locked level cannot be edited, an empty level is loaded:
				levoltzarva = 1;
				char szoveg[80];
				sprintf( szoveg, "The file '%s' is locked, you cannot edit it!", nev );
				dialog( szoveg );
				delete Ptop;
				Ptop = new topol;
				Bentlevelnev[0] = 0;
			}

			// (port) A locked level can be edited after a confirmation. On Yes the
			// lock is dropped, so a plain Save writes a normal (editable) file; SAVE
			// LOCKED still locks it again. (The registered version only; the
			// original did as the shareware version above for every locked level):
			if( Ptop->lezart ) {
					char szoveg[80];
					sprintf( szoveg, "The level '%s' is marked as locked.", nev );
					dialog( szoveg, "If you save it normally, it will be saved unlocked." );
					Ptop->lezart = 0;
			}

		loadlgrfile( Ptop->lgrnev );
		if( Ptop->pic_selejtez( Plgr ) )
			dialogselejtez();
	}
	Ptop->checklevelforsarvari();

	if( Pszak ) // ervenytelenites
		delete Pszak;
	Pszak = NULL;

	// Ha vannak benne idok, figyelmeztetes:
	if( Ptop->idok.singleidok.idokszama > 0 ||
		Ptop->idok.multiidok.idokszama > 0 )
		dialog( "Warning!",
				"The level file you are opening has some best times.",
				"If you save this level file, these times will be erased!" );

	return !levoltzarva;
}


// Opciok minibike-ja hasznalja:
void invalidate_ptop( void ) {
	if( Ptop ) {
		delete Ptop;
		Ptop = 0;
	}
}

void dialogselejtez( void ) {
dialog( "The LGR file has changed since the last edition of this level and",
		"some parts (pictures, textures or border polygons) of the level",
		"must have been deleted!",
		"",
		"!!!!IMPORTANT!!!!",
		"If you do not want to loose these parts, do not save this level",
		"on its original name!" );
}

