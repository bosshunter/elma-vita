#include	"all.h"

int Hangmarmegy = 0;

// (port) Name of a key binding for the Help screen: the Vita button name,
// "-" if nothing is assigned, or "Key code: N" for a key without a button.
char* kodtobillnev( int billkod ); // (port) vita_platform.cpp
static char* billnev_help( int billkod, char* puffer ) {
        char* nev = kodtobillnev( billkod );
        if( nev )
                strcpy( puffer, nev );
        else if( billkod == 0 )
                strcpy( puffer, "???" );
        else
                sprintf( puffer, "Key code: %d", billkod );
        return puffer;
}

void help( void ) {
        szoveglista szl;

        int x1 = 40;
        int x2 = 250; // (port) wider first column: the button names are longer
        int yo = 80;
        int dy = 32;
        szl.addszoveg_kozep( "Current controls:", 320, 20 );

        // (port) Names are copied into the list, so a local buffer is enough:
        char nev[16][30];
        jatekosopciok* o = &State->opciok1;

        szl.addszoveg( billnev_help( o->billgaz, nev[0] ), x1, yo );
        szl.addszoveg( "- Accelerate", x2, yo );

        szl.addszoveg( billnev_help( o->billfek, nev[1] ), x1, yo+dy );
        szl.addszoveg( "- Block Wheels", x2, yo+dy );

        szl.addszoveg( billnev_help( o->billugras2, nev[2] ), x1, yo+dy*2 );
        szl.addszoveg( "- Rotate AntiClockwise", x2, yo+dy*2 );

        szl.addszoveg( billnev_help( o->billugras1, nev[3] ), x1, yo+dy*3 );
        szl.addszoveg( "- Rotate Clockwise", x2, yo+dy*3 );

        szl.addszoveg( billnev_help( o->billfordul, nev[4] ), x1, yo+dy*4 );
        szl.addszoveg( "- Turn Around", x2, yo+dy*4 );

        szl.addszoveg( billnev_help( o->billview, nev[5] ), x1, yo+dy*5 );
        szl.addszoveg( "- View Box Toggle", x2, yo+dy*5 );

        szl.addszoveg( billnev_help( o->billtime, nev[6] ), x1, yo+dy*6 );
        szl.addszoveg( "- Time Display Toggle", x2, yo+dy*6 );

        szl.addszoveg_kozep( "After you have eaten all the fruits,", 320, yo+dy*9 );
        szl.addszoveg_kozep( "touch the flower!", 320, yo+dy*10 );

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

// Egyben allithato:
// (port) The entries are built from a list of IDs, so rows can come and go
// (Flag Tag in multiplayer, Players C/D in 3 and 4 player modes).
enum { OPC_MOD, OPC_TAG, OPC_JATEKOS, OPC_HANG, OPC_SECONDARY, OPC_ANIMMENU,
	   OPC_DETAIL, OPC_ANIMOBJ, OPC_SWAP, OPC_CUSTOM };

static const char* Jatekmodnev[5] = { "", "Single Player", "2P Multiplayer",
									  "3P Multiplayer", "4P Multiplayer" };

void options( void ) {
	int origsecondary = State->secondarybuffer;

	int kurrens = 0;
	while( 1 ) {
		valaszt2 val;
		val.kur = kurrens;
		val.egykepen = 11;
		val.x0 = 72;
		val.x0_tab = 390;
		val.y0 = 77;
		val.dy = 36;
		val.escelheto = 1;
		strcpy( val.cim, "Options" );

		int azon[20];    // entry -> OPC_...
		int jatekosi[20]; // entry -> player index, for OPC_JATEKOS
		int n = 0;

		int jsz = State->jatekosszam;
		if( jsz < 1 || jsz > 4 )
			jsz = State->single ? 1 : 2;

		strcpy( Rubrikak[n], "Play mode:" );
		strcpy( Rubrikak_tab[n], Jatekmodnev[jsz] );
		azon[n++] = OPC_MOD;

		if( jsz > 1 ) {
			strcpy( Rubrikak[n], "Flag Tag:" );
			strcpy( Rubrikak_tab[n], State->tag ? "On" : "Off" );
			azon[n++] = OPC_TAG;
		}

		// Jatekos valasztas (A and B always, C and D in 3/4 player modes):
		int latszik = jsz < 2 ? 2 : jsz;
		for( int j = 0; j < latszik; j++ ) {
			sprintf( Rubrikak[n], "Player %c:", 'A'+j );
			strcpy( Rubrikak_tab[n], State->jatekosnev( j ) );
			jatekosi[n] = j;
			azon[n++] = OPC_JATEKOS;
		}

		strcpy( Rubrikak[n], "Sound:" );
		strcpy( Rubrikak_tab[n], State->soundon ? "Enabled" : "Disabled" );
		azon[n++] = OPC_HANG;

		strcpy( Rubrikak[n], "Sound Optimization:" );
		strcpy( Rubrikak_tab[n], State->secondarybuffer ? "Compatibility" : "Best Quality" );
		azon[n++] = OPC_SECONDARY;

		strcpy( Rubrikak[n], "Animated Menus:" );
		strcpy( Rubrikak_tab[n], State->animatedmenus ? "Yes" : "No" );
		azon[n++] = OPC_ANIMMENU;

		strcpy( Rubrikak[n], "Video Detail:" );
		strcpy( Rubrikak_tab[n], State->highquality ? "High" : "Low" );
		azon[n++] = OPC_DETAIL;

		strcpy( Rubrikak[n], "Animated Objects:" );
		strcpy( Rubrikak_tab[n], State->animatedobjects ? "Yes" : "No" );
		azon[n++] = OPC_ANIMOBJ;

		strcpy( Rubrikak[n], "Swap Bikes:" );
		strcpy( Rubrikak_tab[n], State->aplayerabike ? "No" : "Yes" );
		azon[n++] = OPC_SWAP;

		strcpy( Rubrikak[n], "Customize Controls ..." );
		strcpy( Rubrikak_tab[n], "" );
		azon[n++] = OPC_CUSTOM;

		val.bead( n, 1 );

		kurrens = val.valassz();

		if( kurrens < 0 ) {
			//State->save_aaa(); majd programbol valo kilepeskor kiirjuk
			if( origsecondary != State->secondarybuffer && Hangmarmegy ) {
				// Kiirunk egy dialogust, hogy ujra kell inditani ehhez:
				szoveglista szl;
				szl.addszoveg_kozep( "You modified the sound buffer", 320, 140 );
				szl.addszoveg_kozep( "optimization! As you already", 320, 190 );
				szl.addszoveg_kozep( "used the sound in this session,", 320, 240 );
				szl.addszoveg_kozep( "this will only take effect", 320, 290 );
				szl.addszoveg_kozep( "if you restart the game.", 320, 340 );
				mk_emptychar();
				while( 1 ) {
					if( mk_kbhit() ) {
						int c = mk_getextchar();
						if( c == MK_ESC || c == MK_ENTER )
							break;
					}
					szl.kirajzol();
				}
			}
			return;
		}
		if( kurrens >= n )
			continue;

		switch( azon[kurrens] ) {
			case OPC_MOD: {
				// Single -> 2P -> 3P -> 4P -> Single:
				int uj = jsz % 4 + 1;
				int regi = kurrens;
				State->jatekosszam = uj;
				State->single = (uj == 1);
				// Keep the cursor on "Play mode" even though rows change:
				kurrens = regi;
				break;
			}
			case OPC_TAG:
				State->tag = !State->tag;
				break;
			case OPC_JATEKOS:
				jatekosvalasztas_n( jatekosi[kurrens], 1 );
				break;
			case OPC_HANG:
				State->soundon = !State->soundon;
				break;
			case OPC_SECONDARY:
				State->secondarybuffer = !State->secondarybuffer;
				break;
			case OPC_ANIMMENU:
				State->animatedmenus = !State->animatedmenus;
				break;
			case OPC_DETAIL:
				State->highquality = !State->highquality;
				invalidate_ptop();
				break;
			case OPC_ANIMOBJ:
				State->animatedobjects = !State->animatedobjects;
				break;
			case OPC_SWAP:
				State->aplayerabike = !State->aplayerabike;
				break;
			case OPC_CUSTOM:
				customize();
				break;
		}
	}
}
