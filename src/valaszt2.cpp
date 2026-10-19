#include	"all.h"

const int LISTegykepen = 11;
const int LISTx0 = 200;
const int LISTy0 = 90;
const int LISTdy = 33;

int Maxrubrikaszam = 1;

rubrika* Rubrikak = NULL;
rubrika Rubrikak_tab[MAXRUBRIKASZAM_TAB+1];

void initrubrikak( void ) {
	if( Rubrikak )
		hiba( "98y5t4" );
	// Megszamolja maximalis lev es rec file-okat:
	int levszam = 0;
	finame fname;
	int done = fifirst( "lev\\*.lev", fname );
	while( !done ) {
		done = finext( fname );
		levszam++;
	}
	ficlose();

	int recszam = 0;
	done = fifirst( "rec\\*.rec", fname );
	while( !done ) {
		done = finext( fname );
		recszam++;
	}
	ficlose();
	// (port) 3-4 player replays are listed too:
	done = fifirst( "rec\\*.rc4", fname );
	while( !done ) {
		done = finext( fname );
		recszam++;
	}
	ficlose();

	int szam = levszam;
	if( szam < recszam )
		szam = recszam;

	//if( szam < MAXKEPSZAM )
	//	szam = MAXKEPSZAM;

	// Megvan max szam:
	szam += 220;

	// (port) No cap, so every file is listed (the list is allocated to fit):
	//if( szam > 40000 )
	//	szam = 40000;

	Rubrikak = new rubrika[szam+10];
	if( !Rubrikak )
		hiba( "Nincs eleg memoria Rubrikak-at lefoglalni!" );
	Maxrubrikaszam = szam;
	if( Maxrubrikaszam < 200 )
		hiba( "87iuo" );
}

valaszt2::valaszt2( void ) {
	rubrikak = NULL;
	rubrikak_tab = NULL;
	szam = 0;
	kur = x0 = y0 = dy = egykepen = escelheto = 0;
	korbe = 1; // (port)
	torolheto = 0; // (port)
	torolt = 0;    // (port)
	szelespre = 0; // (port)
	cim[0] = 0;
	// Ezt o maganak beallitja, de nehanyan atdefinialjak:
	cimy = 30;
	pszovlist = NULL;
}

valaszt2::~valaszt2( void ) {
	if( rubrikak )
		delete rubrikak;
	if( rubrikak_tab )
		delete rubrikak_tab;
	if( pszovlist )
		delete pszovlist;

	rubrikak = NULL;
	rubrikak_tab = NULL;
	pszovlist = NULL;
	szam = 0;
	kur = x0 = y0 = dy = egykepen = escelheto = 0;
}

// Rubrikakat beveszi:
void valaszt2::bead( int szam_p, int tabos_p ) {
	int i;
	if( rubrikak )
		hiba( "valaszt2::bead rubrikak!" );
	szam = szam_p;
	tabos = tabos_p;
	if( szam < 1 || szam > Maxrubrikaszam )
		hiba( "valaszt2::bead nem jo szam!" );
	if( tabos && szam > MAXRUBRIKASZAM_TAB )
		hiba( "valaszt2::bead nem jo szam (tabos)!" );
	rubrikak = new rubrika[szam];
	if( !rubrikak )
		hiba( "memoria" );
	rubrikak_tab = NULL;
	if( tabos ) {
		rubrikak_tab = new rubrika[szam];
		if( !rubrikak_tab )
			hiba( "memoria" );
	}
	for( i = 0; i < szam; i++ ) {
		if( strlen( Rubrikak[i] ) > MAXRUBRIKAHOSSZ )
			hiba( "valaszt2::bead tul hosszu string!:", Rubrikak[i] );
		strcpy( rubrikak[i], Rubrikak[i] );
		if( tabos ) {
			if( strlen( Rubrikak_tab[i] ) > MAXRUBRIKAHOSSZ )
				hiba( "valaszt2::bead tul hosszu string (tab)!:", Rubrikak_tab[i] );
			strcpy( rubrikak_tab[i], Rubrikak_tab[i] );
		}
	}
}

// RECidomenteshez kell (mainmenu.cpp-ben is):
#ifndef W_95
static int controlaltnyomva( void ) {
	// DOS-os verzioban v-t es t-t hasznalunk:
	if( mk_getstate( 'v' ) && mk_getstate( 't' ) )
		return 1;
	return 0;
}
#endif

int RECidokellhet = 0;
int RECidokellis = 0;

// Visszaadja valasztott szamat, ESC eseten -1-t:
// tobbrajz eseten tobbszor is kirajzolja kezdetben (Win95):
int valaszt2::valassz( egyszovsor* preszovegek, int preszovegszam,
					   int csakkirajzol ) {
	int i;
    int mk_utolsoismetelt( void ); // (port) from vita_platform.cpp
	if( szam < 1 || egykepen < 2 )
		hiba( "valaszt2::valassz nem jo elejen!" );

	if( kur > szam-1 )
		kur = szam-1;

	int felso = kur-egykepen/2;
	int maxfelso = szam-egykepen;
	if( felso > maxfelso )
		felso = maxfelso;
	if( felso < 0 )
		felso = 0;

	if( pszovlist )
		delete pszovlist;
	pszovlist = new szoveglista;

	mk_emptychar();
	int valtozott = 1;
	while( 1 ) {
		while( !csakkirajzol && mk_kbhit() ) {
			int c = mk_getextchar();
			if( c == MK_BACKSPACE && torolheto ) { // (port)
					torolt = 1;
					return kur;
			}
			if( c == MK_ESC && escelheto )
				return -1;
			if( c == MK_ENTER ) {
				// RECidomenteshez kell (mainmenu.cpp-ben is):
				int controlaltnyomva( void );
				if( RECidokellhet && controlaltnyomva() )
					RECidokellis = 1;
				return kur;
			}
			if( c == MK_UP ) {
				if( kur > 0 ) {
					kur--;
					if( kur < felso ) {
						felso--;
						valtozott = 1;
					}
				}
				else if( korbe && szam > 1 && !mk_utolsoismetelt() ) {
					// (port) Wrap around: first -> last
					kur = szam-1;
					felso = szam-egykepen;
					if( felso < 0 )
						felso = 0;
					valtozott = 1;
				}
			}
			if( c == MK_PGUP ) {
				if( kur > 0 ) {
					kur -= egykepen;
					if( kur < 0 )
						kur = 0;
					if( kur < felso ) {
						felso = kur;
						valtozott = 1;
					}
				}
			}
			if( c == MK_DOWN ) {
				if( kur < szam-1 ) {
					kur++;
					if( kur > felso+egykepen-1 ) {
						felso++;
						valtozott = 1;
					}
				}
				else if( korbe && szam > 1 && !mk_utolsoismetelt() ) {
					// (port) Wrap around: last -> first
					kur = 0;
					felso = 0;
					valtozott = 1;
				}
			}
			if( c == MK_PGDOWN ) {
				if( kur < szam-1 ) {
					kur += egykepen;
					if( kur >= szam )
						kur = szam-1;
					if( kur > felso+egykepen-1 ) {
						felso = kur-egykepen+1;
						valtozott = 1;
					}
				}
			}
			// Volume allitas itt specialisan van elintezve:
			/*if( (c == MK_LEFT || c == MK_RIGHT) &&
				strcmp( rubrikak[kur], "Sound Volume:" ) == 0 &&
				(kur == 3 || kur == 4) &&
				strcmp( rubrikak[kur-1], "Player B:" ) == 0 ) {
				if( c == MK_LEFT ) {
					State->soundvolume -= 0.05;
					if( State->soundvolume < 0.0 )
						State->soundvolume = 0.0;
				}
				if( c == MK_RIGHT ) {
					State->soundvolume += 0.05;
					if( State->soundvolume > 1.0 )
						State->soundvolume = 1.0;
				}
				return kur;
			} */
		}
		if( valtozott ) {
			valtozott = 0;
			// Betaplalja kirajzba uj kepszoveget:
			pszovlist->clear();
			for( i = 0; i < preszovegszam; i++ ) {
				int kozep = strncmp( preszovegek[i].text, "*$$^&|@", 7 ) == 0;
				char* text = kozep ? &preszovegek[i].text[7] : preszovegek[i].text;
				int x = preszovegek[i].x, y = preszovegek[i].y;
				if( szelespre ) {
					// (port) coordinates on the whole 960x544 screen
					if( kozep )
						pszovlist->addszoveg_szeles_kozep( text, x, y );
					else
						pszovlist->addszoveg_szeles( text, x, y );
				}
				else if( kozep ) {
					// Kozepre kell tenni:
					pszovlist->addszoveg_kozep( text, x, y );
				}
				else {
					// Nem kell kozepre tenni:
					pszovlist->addszoveg( text, x, y );
				}
			}

			pszovlist->addszoveg_kozep( cim, 320, cimy );
			for( i = 0; i < egykepen && i < szam-felso; i++ ) {
				pszovlist->addszoveg( rubrikak[felso+i], x0, y0+i*dy );
			}
			if( tabos ) {
				for( i = 0; i < egykepen && i < szam-felso; i++ ) {
					pszovlist->addszoveg( rubrikak_tab[felso+i], x0_tab, y0+i*dy );
				}
			}
		}
		pszovlist->setsisak( x0-30, y0+(kur-felso)*dy );
		pszovlist->kirajzol();
		if( csakkirajzol )
			return 0;
	}
}


void valaszt2::kirajzol( void ) {
	if( !pszovlist )
		hiba( "tegcdy" );
	pszovlist->kirajzol();
}



