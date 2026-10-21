#include    "all.h"

// Ezek mindig beallitodnak Kepmeret-bol kirajzol320-ban:

int Kitoltestmegrak = 0;
int Kitoltestmegrakkezd = 10;

// (port) The screen size (Inc. / Dec. Screen Size) and the split screen gap
// are kept in State, so they are saved in state34.dat and come back at start:
#define Kepmeret (State->kepmeret)

// (port) Gap between the split screen views. Above the biggest view size,
// two more steps: 2 = the original gap, 1 = a thin gap (default), 0 = none.
// R (bigger) at full size makes the gap thinner, L (smaller) wider, and only
// then do the views shrink. With one player there is no gap to change.
#define Reszint (State->reszint)

static int resz( int eredeti ) {
	if( Reszint >= 2 )
		return eredeti;
	if( Reszint == 1 )
		return eredeti/2;
	return 0;
}

static double Szorzo = 1.1;
void novelkepmeret( void ) {
	if( !Single && Kepmeret >= 0.999 ) {
		// (port) full size already: thinner gap
		if( Reszint > 0 )
			Reszint--;
		Kitoltestmegrak = Kitoltestmegrakkezd;
		return;
	}
    Kepmeret *= Szorzo;
    if( Kepmeret > 1.0 )
        Kepmeret = 1.0;
    Kitoltestmegrak = Kitoltestmegrakkezd;
}

void csokkentkepmeret( void ) {
	if( !Single && Kepmeret >= 0.999 && Reszint < 2 ) {
		// (port) wider gap first, then smaller views
		Reszint++;
		Kitoltestmegrak = Kitoltestmegrakkezd;
		return;
	}
    Kepmeret /= Szorzo;
    if( Kepmeret < 0.7 )
        Kepmeret = 0.7;
    Kitoltestmegrak = Kitoltestmegrakkezd;
}

// (port) In-game zoom: 1.0 = the original scale. Changed smoothly while the
// Zoom In / Zoom Out keys are held (lejatszo calls zoomkezel every frame),
// between PORT_ZOOMMIN and PORT_ZOOMMAX. It applies to every view. The value
// is State->zoom, so it stays the same from level to level and is saved in
// state34.dat (Start in a level sets it back to 1.0):
#define Zoom (State->zoom)
static double Zoomido = -1.0;

// Back to the original scale (Start in a level):
void zoomalap( void ) {
	Zoom = 1.0;
}

void zoomkezel( int be, int ki ) {
	double most = mv_stopperido()/182.0; // seconds
	double dt = 0.0;
	if( Zoomido >= 0.0 && most-Zoomido < 0.2 )
		dt = most-Zoomido;
	Zoomido = most;
	const double sebesseg = 1.8; // times per second
	if( be && !ki )
		Zoom *= pow( sebesseg, dt );
	if( ki && !be )
		Zoom /= pow( sebesseg, dt );
	if( !(Zoom >= PORT_ZOOMMIN) ) // also a bad value from the file
		Zoom = PORT_ZOOMMIN;
	if( Zoom > PORT_ZOOMMAX )
		Zoom = PORT_ZOOMMAX;
	// When the keys are let go close to the original scale, snap to it
	// (it is drawn directly, without scaling):
	if( !be && !ki && fabs( Zoom-1.0 ) < 0.03 )
		Zoom = 1.0;
}

// Kirajzolando terulet nagysaga:
static int Cx1 = 0, Cy1 = 0, Cx2 = PORT_FIZX-1, Cy2 = PORT_FIZY-1, // (port)
           Cy1Bplayer = 0, Cy2Bplayer = 0;
int Cxsize = PORT_FIZX, Cysize = PORT_FIZY; // (port)

// Motoros kepernyo mely reszen helyezkedik el:
static double Mo_y = 0.0, Mo_bal = 0.0, Mo_dx = 0.0;

double  Hatarx1,
        Hatarx2,
        Hatary1,
        Hatary2;

// View ablak meretei:
static int Viewxsize, Viewysize;
static int Viewxorig, Viewxtolas;

void beallitmereteket( int splitscreen ) {
    Cy1Bplayer = 100, Cy2Bplayer = 200;

    // (port) Uses the current screen size (960x544 on the Vita):
    Cxsize = Port_kepx*(Kepmeret*1.001);
    Cysize = Port_kepy*(Kepmeret*1.001);
    Cx1 = (Port_kepx-Cxsize)/2;
    Cy1 = (Port_kepy-Cysize)/2;
    Cx2 = Cx1 + Cxsize - 1;
    Cy2 = Cy1 + Cysize - 1;
    if( splitscreen ) {
        // (port) 480: 234 + 12 + 234 -> scaled to the current height:
        Cysize = (Port_kepy-resz( 12 ))/2; // (port) gap: see Reszint
		Cy1 = Port_kepy-Cysize; // Mivel kep fejjel lefele van, forditva vannak koordok
        Cy2 = Port_kepy-1; // Vagyis A player van felul, B pedig lent
        Cy1Bplayer = 0;
        Cy2Bplayer = Cysize-1;
    }
    #ifdef TEST
        if( Cx2 >= Port_kepx )
            hiba( "beallitmereteket Cx2 >= Port_kepx!" );
        if( Cy2 >= Port_kepy )
            hiba( "beallitmereteket Cy2 >= Port_kepy!" );
    #endif

    // Ezek csak doboz kirajzolashoz kellenek:
    Hatarx1 = 2.0;
    Hatary1 = 2.0;
    Hatarx2 = Cxsize-3.0;
    Hatary2 = Cysize-3.0;

    // Motoros kepernyo mely reszen helyezkedik el:
    Mo_bal = (Port_kepx/Arany)*0.15; // (port)
    Mo_dx  = Cxsize/Arany - 2.0*Mo_bal;
    Mo_y   = Cysize/Arany/2.0;

    Viewxsize = 140*sqrt(double(Cysize)/480.0); // 200
    Viewysize = 70*sqrt(double(Cysize)/480.0);  // 80
    Viewxorig = 40*(Kepmeret-0.6)/0.4;
    Viewxtolas = Cxsize-2*Viewxorig-Viewxsize;
}

// (port) Sets the drawing area for one view of a 3/4 player split screen:
// the rectangle (x, y, w, h) of the whole slot, shrunk by Kepmeret (L / R).
static void nezetbeallit( int x, int y, int w, int h ) {
	Cxsize = w*(Kepmeret*1.001);
	Cysize = h*(Kepmeret*1.001);
	if( Cxsize > w )
		Cxsize = w;
	if( Cysize > h )
		Cysize = h;
	Cx1 = x + (w-Cxsize)/2;
	Cy1 = y + (h-Cysize)/2;
	Cx2 = Cx1 + Cxsize - 1;
	Cy2 = Cy1 + Cysize - 1;

	// Ezek csak doboz kirajzolashoz kellenek:
	Hatarx1 = 2.0;
	Hatary1 = 2.0;
	Hatarx2 = Cxsize-3.0;
	Hatary2 = Cysize-3.0;

	// Motoros kepernyo mely reszen helyezkedik el (relative to this view):
	Mo_bal = (w/Arany)*0.15;
	Mo_dx  = Cxsize/Arany - 2.0*Mo_bal;
	Mo_y   = Cysize/Arany/2.0;

	Viewxsize = 140*sqrt(double(Cysize)/480.0);
	Viewysize = 70*sqrt(double(Cysize)/480.0);
	Viewxorig = 40*(Kepmeret-0.6)/0.4;
	Viewxtolas = Cxsize-2*Viewxorig-Viewxsize;
}

// (port) Player index (0..3) -> bike pictures. A and C ride the first bike,
// B and D the second ("Swap Bikes" swaps them, as in the original):
static motkepek* motorkepe( int ji ) {
	int elso = (ji % 2) == 0;
	if( !State->aplayerabike )
		elso = !elso;
	return elso ? &Plgr->mkepek1 : &Plgr->mkepek2;
}

// (port) ... and the colour of its dot on the navigator (minimap):
static unsigned char viewszine( int ji ) {
	int elso = (ji % 2) == 0;
	if( !State->aplayerabike )
		elso = !elso;
	return elso ? Plgr->viewmotorindex1 : Plgr->viewmotorindex2;
}

// (port) The valtozok of every player, set by kirajzol320n for drawing others:
static valtozok** Aktvaltok = NULL;

void forditkepet( pic8* ppic ) {
    for( int y1 = 0; y1 < ppic->getysize()/2; y1++ ) {
        int y2 = ppic->getysize()-1-y1;
        unsigned char* sora = ppic->getptr( y1 );
        unsigned char* sorb = ppic->getptr( y2 );
        int xsize = ppic->getxsize();
        for( int x = 0; x < xsize; x++ ) {
            unsigned char tmp = sora[x];
            sora[x] = sorb[x];
            sorb[x] = tmp;
        }
    }
}

// v1 iranyabol v2 fele interpolal:
/*static vekt2 interpolal( vekt2 v1, vekt2 v2, double x ) {
	vekt2 vissza = v1+(v2-v1)*x;
	return vissza;
} */

// Meg nem affin koordok:
static void kidoboz( vekt2 a, vekt2 b, pic8* ppic8, double szelesseg,
        kiskep* pk, double tulloga, double tullogb, int tukor = 0 ) {
    vekt2 vegys = egys( b-a );
    b = b + vegys*tullogb;
    a = a - vegys*tulloga;
    vekt2 v = b-a;
    vekt2 v2fel;
    if( tukor )
        v2fel = forgatas90fokkal( vegys ) * szelesseg;
    else
		v2fel = forgatasminusz90fokkal( vegys ) * szelesseg;
    a = a-v2fel;

    a.x *= Arany;
    a.y *= Arany;
    v.x *= Arany;
    v.y *= Arany;
    v2fel.x *= Arany;
    v2fel.y *= Arany;
    dobozki( ppic8, pk, v, v2fel*2.0, a );
}

// sugar meg nincs megszorozva:
// Meg nem affin koordok:
static void kidobozkerek( vekt2 r, double sugar, double alfa,
        pic8* ppicbuffer, kiskep* pk, int tukor = 0 ) {
    vekt2 vfel( cos( alfa ) * sugar, sin( alfa ) * sugar );
    if( tukor )
        kidoboz( r+vfel, r-vfel, ppicbuffer, sugar, pk, 0.0, 0.0, tukor );
    else
        kidoboz( r-vfel, r+vfel, ppicbuffer, sugar, pk, 0.0, 0.0, tukor );
}


static double Mx = -1, My = -1;
static vekt2 Mi, Mj, Mr;
static vekt2 Miar, Mjar, Mrar; // Ezek meg vannak szorozva Aranyossaggal

static void kitag( pic8* ppic8, kiskep* pk, kisbox* pb ) {
	vekt2 r = Miar*(pb->x1+260-Mx) + Mjar*(My-(pb->y1+260)) + Mrar;
	vekt2 u = Miar*(pb->x2-pb->x1);
	vekt2 v = Mjar*(pb->y1-pb->y2);

    dobozki( ppic8, pk, u, v, r );
}

static void kigyuru( pic8* ppic8, int x, int y, unsigned char szin ) {
    #ifdef TEST
        if( x < 1 || x >= ppic8->getxsize()-1 ||
            y < 1 || y >= ppic8->getysize()-1 )
            hiba( "kigyuru-ben x, vagy y lelog!" );
    #endif
    unsigned char *sor = ppic8->getptr( y-1 );
    sor[x-1] = szin; sor[x] = szin; sor[x+1] = szin;
    sor = ppic8->getptr( y+1 );
    sor[x-1] = szin; sor[x] = szin; sor[x+1] = szin;
    ppic8->ppixel( x-1, y, szin );
    ppic8->ppixel( x+1, y, szin );
}


/* Most nincsen felho:
static void felhokfel( int x, int y, pic8* ppicbuffer, pic8* pfelho,
                            double lassit ) {
    int kerx = -x*lassit;
    int kery = -y*lassit;
    // Vegigmegy kepernyon:
    int kerxsize = pfelho->getxsize();
    kerx %= kerxsize;
    kerx -= kerxsize; // Lehet, hogy enelkul is negativ?
    while( kerx < Displaysizex ) {
        int kerysize = pfelho->getysize();
        kery %= kerysize;
        kery -= kerysize; // Lehet, hogy enelkul is negativ?
        while( kery < Displaysizey ) {
            blt8( ppicbuffer, pfelho, kerx, kery );

            kery += kerysize;
        }
        kerx += kerxsize;
    }
} */

// (port) GPU drawing (vita_gpu.cpp): set for every frame by kirajzol320n.
// The level, the objects, the bikes and the timer are then sent to the GPU;
// the navigator is drawn into a work picture (Gpunav) and sent as a picture.
static int Gpujatek = 0;
static pic8* Gpunav = NULL;
static int Gx1 = 0, Gy1 = 0; // the bottom left corner of the current view in the screen buffer

// The current view (in screen buffer coordinates, y up) to the GPU:
static void gpunezet( int x, int y, int w, int h, int vw, int vh ) {
	int ox = 0, oy = 0;
	if( Port_kepx != PORT_FIZX ) {
		ox = PORT_KLASSZ_OFSX;
		oy = PORT_KLASSZ_OFSY;
	}
	port_gj_nezet( ox + x, oy + Port_kepy - y - h, w, h, vw, vh );
}

// Starts a GPU frame; returns the picture the views are cut out of:
static pic8* gpukezd( void ) {
	extern int Port_atlatszo, Port_szintgen; // ecset.cpp
	port_kepmeretfrissit();
	// The level's colours: the first frame of a level is drawn before the
	// game switches the screen to them (Plgr->pal->set() after it)
	port_gj_kezd( Plgr->pal->pddpal, Port_atlatszo, Port_szintgen );
	int ox = 0, oy = 0;
	if( Port_kepx != PORT_FIZX ) {
		ox = PORT_KLASSZ_OFSX;
		oy = PORT_KLASSZ_OFSY;
	}
	// The gaps between the views (the CPU drawing only fills them now and
	// then, the GPU draws every frame from scratch):
	port_gj_hatter( Plgr->pkimarad, ox, oy, Port_kepx, Port_kepy );
	if( !Gpunav )
		Gpunav = new pic8( PORT_FIZX, PORT_FIZY );
	return Gpunav;
}

// Ends the GPU frame (with a screenshot if one was asked for):
static void gpuvege( void ) {
	int ment = Legkozment;
	Legkozment = 0;
	port_gj_vege( ment );
}

// (port) ji: player index 0..3. All other players are shown as dots.
static void kiview( int ji, pic8* ppic, double baljobb,
				vekt2 motorkozep ) {

    double Viewxsize_d = Viewxsize*Viewzoom*Aranyrec;
    double Viewysize_d = Viewysize*Viewzoom*Aranyrec;

    double szeltoltav = 0.2; // 20%
    double atfuthossz = 1.0-2.0*szeltoltav;
    vekt2 sarokbolkozepre( Viewxsize_d*(szeltoltav + baljobb*atfuthossz),
                           Viewysize_d/2 );
	vekt2 sarok = motorkozep - sarokbolkozepre;

    int vx1 = Viewxorig + baljobb*Viewxtolas;
    int vx2 = vx1 + Viewxsize-1;
    int vy1 = 1;
    int vy2 = vy1+Viewysize-1;

    Pecsetview->kiteszview( ji, ppic, sarok, vx1, vy1, vx2, vy2 );

    // Most kirajzoljuk food-okat:
    int balalsox, balalsoy;
    Pecsetview->getbalalso_int( sarok, &balalsox, &balalsoy );
    int objminx = balalsox;
    int objminy = balalsoy;
    int objmaxx = balalsox + Viewxsize-1;
    int objmaxy = balalsoy + Viewysize-1;
    for( int i = 0; i < MAXKEREK; i++ ) {
        kerek* pker = Ptop->kerektomb[i];
        if( !pker )
            break;

		int kell1 = pker->tipus == T_KAJA && pker->aktiv;
		int kell2 = (Single || !Tag) && pker->tipus == T_CEL;

		if( !kell1 && !kell2 )
            continue;

        if( pker->rxint_v < objminx || pker->ryint_v < objminy ||
            pker->rxint_v > objmaxx || pker->ryint_v > objmaxy )
            continue;

		// Potty kirajzolasa View ablakba:
		if( pker->tipus == T_KAJA )
			kigyuru( ppic, pker->rxint_v-balalsox+vx1,
								 pker->ryint_v-balalsoy+vy1, Plgr->viewfood );
		else
			kigyuru( ppic, pker->rxint_v-balalsox+vx1,
							 pker->ryint_v-balalsoy+vy1, Plgr->viewexit[0] );
	}

	// Most kirajzoljuk masik (hatso) motorosokat (port: every other player):
	for( int j = 0; j < Jatekosszam; j++ ) {
		if( j == ji )
			continue;
		vekt2 sarokrel = Pmotok[j]->kor1.r - sarok;
		int mx = sarokrel.x * Arany/Viewzoom + vx1;
		int my = sarokrel.y * Arany/Viewzoom + vy1;
		if( mx >= vx1 && mx <= vx2 && my >= vy1 && my <= vy2 )
			kigyuru( ppic, mx, my, viewszine( j ) );
	}

	// Most kirajzoljuk elso motorost:
	int mx = sarokbolkozepre.x*Arany/Viewzoom;
	int my = sarokbolkozepre.y*Arany/Viewzoom;
	kigyuru( ppic, vx1+mx, vy1+my, viewszine( ji ) );

	// Kirajzoljuk keretet:
	vx1--; vy1--;
    vx2++; vy2++;
    for( int y = vy1; y <= vy2; y++ ) {
		ppic->ppixel( vx1, y, Plgr->keretindex );
		ppic->ppixel( vx2, y, Plgr->keretindex );
	}
	unsigned char* sor = ppic->getptr( vy1 ) + vx1 + 1;
	BYTESET( sor, Plgr->keretindex, Viewxsize );
	sor = ppic->getptr( vy2 ) + vx1 +1;
	BYTESET( sor, Plgr->keretindex, Viewxsize );
	if( Gpujatek ) // (port) drawn into a work picture, now onto the screen
		port_gj_resz( ppic, vx1, vy1, vx2-vx1+1, vy2-vy1+1 );
}

// Mentes lemezre:
static void menteshakell( pic8* ppic ) {
    if( Legkozment ) {
        Legkozment = 0;
        int i = 0;
        while( 1 ) {
            char filenev[20];
            if( i < 10 )
                sprintf( filenev, "snap0%d.pcx", i );
            else
                sprintf( filenev, "snap%d.pcx", i );
            if( access( filenev, 0 ) != 0 ) {
                forditkepet( ppic );
				ppic->save( filenev, Plgr->paltomb );
                forditkepet( ppic );
                break;
            }
            i++;
        }
    }
}

static void kitolthatteret( pic8* ppic ) {
    int y = 0;
    while( y < ppic->getysize() ) { // (port)
        int x = 0;
        while( x < ppic->getxsize() ) {
			// hiba( "Itt van 14!" ); idejott
			// hiba( tmp ); (16) kiirja koordokat szepen
			
			//extern int Debugblt;
			//Debugblt = 1;
			blt8( ppic, Plgr->pkimarad, x, y );
			//Debugblt = 0;
			// hiba( "Itt van 20!" ); idejott
			x += Plgr->pkimarad->getxsize();

			// hiba( tmp ); (21) kiirja x-et 207-nek
		}
		//hiba( "Itt van 15!" );
		y += Plgr->pkimarad->getysize();
    }
}

#ifdef TEST
int Voltkilogas = 0;
#endif

// (port) ji: player index 0..3
static void kibike( int ji, pic8* ppic, double t, vekt2 sarok,
					motorst* pmot, valtozok* pvalt, motkepek* pmk ) {

    // Mivel ezeket fv megvaltoztatja:
    double ugrasnagysag = pvalt->ugrasnagysag;
	double forgas = pvalt->baljobbv_f.forgas;

    //if( pvalt->baljobb < 0 || pvalt->baljobb > 1.0 )
    //    hiba( "pvalt->baljobb < 0 || pvalt->baljobb > 1.0!" );
    //if( forgas < 0.0 || forgas > 1.0 )
    //    hiba( "forgas < 0.0 || forgas > 1.0!" );

	// Ha kell affinitassal torodni:
    int vanforgas = 0;
	Affinitas = 0;
	if( forgas < 0.999 ) {
        vanforgas = 1;
        // forgast 0, 1 intervallumbol atkonvertaljuk -1, 1 intervallumba:
        forgas = -cos( forgas*K_pi );
	}

	vekt2 p_k1 = (pmot->kor2.r-sarok);
	vekt2 p_k2 = (pmot->kor4.r-sarok);

	int elsokerekrogton = 1;
	int hatsokerekrogton = 1;

	if( vanforgas ) {
		if( (forgas > 0.0 && !pmot->hatra_f) || (forgas <= 0.0 && pmot->hatra_f) )
			elsokerekrogton = 0;
		else
            hatsokerekrogton = 0;
	}

	// Kerekek:
	#ifdef BUTITOTT
	if( elsokerekrogton )
		kidobozkerek( p_k1, Kerekrajzsugar, 0, ppic, pmk->pkiskerek );
	if( hatsokerekrogton )
		kidobozkerek( p_k2, Kerekrajzsugar, 0, ppic, pmk->pkiskerek );
	#else
	if( elsokerekrogton )
		kidobozkerek( p_k1, Kerekrajzsugar, pmot->kor2.alfa, ppic, pmk->pkiskerek );
	if( hatsokerekrogton )
		kidobozkerek( p_k2, Kerekrajzsugar, pmot->kor4.alfa, ppic, pmk->pkiskerek );
	#endif

	vekt2 kozep = pmot->kor1.r-sarok;
	vekt2 jobbra = vekt2( cos( pmot->kor1.alfa ), sin( pmot->kor1.alfa ) );
	vekt2 fel = forgatas90fokkal( jobbra );

	if( vanforgas ) {
		Affinitas = 1;
		setaffinitas( kozep, jobbra, forgas, Arany, Arany );
	}

	if( pmot->hatra_f ) {
		jobbra = Vekt2null - jobbra;
		vekt2 tmpv = p_k1;
		p_k1 = p_k2;
		p_k2 = tmpv;
	}

	Mx = 390.0, My = 420.0;
	double malfa = 0.62;
	double mmeret = 0.0045;
	Mi = jobbra*(mmeret*cos(malfa))+fel*(mmeret*sin(malfa));
	Mj = forgatas90fokkal( Mi );
	if( pmot->hatra_f )
        Mj = Vekt2null - Mj;
    Mr = kozep;
	// Most Arany-t betesszuk Mi, Mj, Mr-be:
    Miar = Mi*Arany;
    Mjar = Mj*Arany;
    Mrar = Mr*Arany;

	// Elso felfuggesztes:
	vekt2 p_fogantyu = Mi*(365.0-Mx) + Mj*(My-292.0) + Mr; // Uj
	kidoboz( p_k1, p_fogantyu, ppic, 0.06,
		pmk->pkiselsorud, 0.05, 0.03 );

	// Hatso felfuggesztes:
	vekt2 p_darab2 = Mi*(370.0-Mx) + Mj*(My-520.0) + Mr;
	kidoboz( p_darab2, p_k2, ppic, 0.06,
		pmk->pkishatsorud, 0.0, 0.1 );

	// Ha kell zaszlot rajzol:
	int flages = 0;
	if( !Single && Tag ) {
		// (port) the holder has the flag; a player touching the holder blinks
		if( Fogo == ji )
			flages = 1;

		if( !flages && Kozel[ji & 3] ) {
			int egeszido = t*30.0;
			if( egeszido%2 )
				flages = 1;
		}
	}
	vekt2 zaszlotalp = Mi*(500.0+107-Mx) + Mj*(My+114-600.0) + Mr;
	// 330 bal oldal (5)
	// 340 bal oldal (3)
	// 355 bal oldal (1.5)
	// 365 jobb oldal (1)
	// 361 jobb hajszalnyit
	vekt2 zaszloteto = zaszlotalp + (Mi*356.0 + Mj*500.0)*0.2;
	if( flages )
		kidoboz( zaszlotalp, zaszloteto, ppic, 0.2, Plgr->pkiszaszlo,
											0.0, 0.0, pmot->hatra_f );

	// Motor teste:
	kitag( ppic, pmk->pkisa, &KisboxA );
	kitag( ppic, pmk->pkisb, &KisboxB );
	kitag( ppic, pmk->pkisc, &KisboxC );
	kitag( ppic, pmk->pkisd, &KisboxD );



	// Mozgo test:
	vekt2 p_vezeto = (pmot->vezetor-sarok); // Vezeto
	vekt2 p_csipo = p_vezeto + Mi*75.0 + Mj*(-47.0);
	vekt2 p_vall = p_vezeto + Mi*47.0 + Mj*65.0;
	vekt2 p_valltest = p_vezeto + Mi*41.0 + Mj*70.0;
	vekt2 p_labfej = Mi*(346.0-Mx) + Mj*(My-514.0) + Mr;
	/* Bicigli:
	p_labfej = p_labfej + 0.14*
					(jobbra * sin(pmot->labalfa) +
					 fel * cos(pmot->labalfa) );
	*/
	vekt2 p_terd;

	double combhossz = 0.51;
	double labszarhossz = 0.51;
	/* Bicigli lab hosszak:
	double combhossz = 0.56;
	double labszarhossz = 0.56;
	*/

	if( pmot->hatra_f )
		p_terd = ketkormetszete( p_csipo, p_labfej,
										combhossz, labszarhossz );
	else
		p_terd = ketkormetszete( p_labfej, p_csipo,
										labszarhossz, combhossz );

	// Fej:
	//double fejalfa = 0.20;
	double fejalfa = 0.0;
	if( pmot->hatra_f )
		fejalfa = 0 - fejalfa;
	kidobozkerek( pmot->fejr-sarok, Fejsugar, pmot->kor1.alfa+fejalfa,
				  ppic, pmk->pkisfej, pmot->hatra_f );

	// Kar ket reszletben:
	vekt2 p_kezfej = p_fogantyu;

	// Ugras belekombinalasa kar kirajzolasaba:
	// p_kezfej-et mozditjuk el szukseges mertekben:
	if( ugrasnagysag > 0.0001 ) {
		ugrasnagysag = 1.0-ugrasnagysag;
		int fellendit = 1; // kar kiteresenek iranya
		if( (pvalt->ugras1volt && !pmot->hatra_f) ||
			(!pvalt->ugras1volt && pmot->hatra_f) )  // XOR
			fellendit = 0;
		// alfa es hosszit kiszamolasa:
		double alfa = 0.0;
		double hosszit = 0.1;
		if( fellendit ) {
			// KAR FELFELE LENDUL KI:
			double hatar = 0.25;
			double maxalfa = 2.7;
			double maxhosszit = -0.3;
			if( ugrasnagysag < hatar ) {
				// Kilendit:
				alfa = maxalfa*ugrasnagysag/hatar;
				hosszit = maxhosszit*ugrasnagysag/hatar+1.0;
			}
			else {
				// Visszaenged:
				double mertek = 1.0-(ugrasnagysag-hatar)/(1.0-hatar);
				alfa = maxalfa*mertek;
				hosszit = maxhosszit*mertek+1.0;
			}
		}
		else {
			// KAR LEFELE LENDUL KI:
			double hatar = 0.2;
			double maxalfa = -1.6;
			double maxhosszit = 0.15;
			if( ugrasnagysag < hatar ) {
				// Kilendit:
				alfa = maxalfa*ugrasnagysag/hatar;
				hosszit = maxhosszit*ugrasnagysag/hatar+1.0;
			}
			else {
				// Visszaenged:
				double mertek = 1.0-(ugrasnagysag-hatar)/(1.0-hatar);
				alfa = maxalfa*mertek;
				hosszit = maxhosszit*mertek+1.0;
			}
		}
		// alfa es hosszit alapjan p_kezfej forgatasa es nyujtasa vall korul:
		vekt2 kar = p_kezfej-p_vall;
		if( !pmot->hatra_f )
			kar.forgatas( -alfa );
		else
			kar.forgatas( alfa );
		kar = kar*hosszit;
		p_kezfej = p_vall + kar;
	}

	double alkarhossz = 0.308*1.05;
	double felkarhossz = 0.328*1.05;
	vekt2 p_konyok;
	if( pmot->hatra_f )
		p_konyok = ketkormetszete( p_kezfej, p_vall,
										alkarhossz, felkarhossz );
	else
		p_konyok = ketkormetszete( p_vall, p_kezfej,
										felkarhossz, alkarhossz );

	// Lab:
	kidoboz( p_terd, p_csipo, ppic, 0.14,
		pmk->pkiscomb, 0.03, 0.1, pmot->hatra_f );

	kidoboz( p_labfej, p_terd, ppic, 0.21,
		pmk->pkislabszar, 0.03, 0.03, pmot->hatra_f );

	// Vezeto:
	kidoboz( p_csipo, p_valltest, ppic, 0.2,
		pmk->pkisvezeto, 0.1, 0.05, pmot->hatra_f );

	// Kez:
	kidoboz( p_konyok, p_vall, ppic, 0.11,
		pmk->pkisfelkar, 0.08, 0.1, !pmot->hatra_f );

	kidoboz( p_kezfej, p_konyok, ppic, 0.076,
		pmk->pkisalkar, 0.08, 0.1, pmot->hatra_f );

	Affinitas = 0;

	// Kerekek:
	if( !elsokerekrogton || !hatsokerekrogton ) {
		if( pmot->hatra_f ) {
			// Visszacserelem kerekek koordjait:
			vekt2 tmpv = p_k1;
			p_k1 = p_k2;
			p_k2 = tmpv;
		}
		#ifdef BUTITOTT
		if( !elsokerekrogton )
			kidobozkerek( p_k1, pmot->kor2.sugar, 0, ppic, pmk->pkiskerek );
		if( !hatsokerekrogton )
			kidobozkerek( p_k2, pmot->kor4.sugar, 0, ppic, pmk->pkiskerek );
		#else
		if( !elsokerekrogton )
			kidobozkerek( p_k1, pmot->kor2.sugar, pmot->kor2.alfa, ppic, pmk->pkiskerek );
		if( !hatsokerekrogton )
			kidobozkerek( p_k2, pmot->kor4.sugar, pmot->kor4.alfa, ppic, pmk->pkiskerek );
		#endif
	}
}

// (port) ji: player index 0..3. Every other player's bike is drawn too.
// (port) Zoom: the level is drawn into Zoomnezet (a part of Zoomkep) at
// Cxsize/Zoom x Cysize/Zoom, then scaled into the view:
static pic8* Zoomkep = NULL;
static pic8* Zoomnezet = NULL;

static void zoomskalaz( pic8* cel, int w, int h, int vw, int vh ) {
	static int Xterkep[PORT_FIZX];
	for( int x = 0; x < w; x++ )
		Xterkep[x] = x*vw/w;
	for( int y = 0; y < h; y++ ) {
		unsigned char* d = cel->getptr( y );
		unsigned char* s = Zoomnezet->getptr( y*vh/h );
		for( int x = 0; x < w; x++ )
			d[x] = s[Xterkep[x]];
	}
}

static void kirakegyjatekost(
		int ji,
		pic8* ppic, double t, motorst* pmot,
		valtozok* pvalt,
		int viewki, int timeki ) {

	// (port) Zoom: draw the level at another size first. The navigator and
	// the timer are drawn afterwards at their normal size.
	pic8* celpic = ppic;
	int zoomol = Zoom != 1.0;
	int eCxsize = Cxsize, eCysize = Cysize;
	double eHatarx2 = Hatarx2, eHatary2 = Hatary2;
	double eMo_bal = Mo_bal, eMo_dx = Mo_dx, eMo_y = Mo_y;
	if( zoomol ) {
		if( !Zoomkep && !Gpujatek ) { // (port) the GPU scales it itself
			Zoomkep = new pic8( PORT_ZOOMSZEL, PORT_ZOOMMAG );
			Zoomnezet = new pic8( 10, PORT_ZOOMMAG ); // only the row table is used
		}
		int vw = (int)(Cxsize/Zoom + 0.5);
		int vh = (int)(Cysize/Zoom + 0.5);
		if( vw > PORT_ZOOMSZEL ) vw = PORT_ZOOMSZEL;
		if( vh > PORT_ZOOMMAG )  vh = PORT_ZOOMMAG;
		if( vw < 16 ) vw = 16;
		if( vh < 16 ) vh = 16;
		Mo_bal /= Zoom;
		Cxsize = vw;
		Cysize = vh;
		Mo_dx = Cxsize/Arany - 2.0*Mo_bal;
		Mo_y = Cysize/Arany/2.0;
		Hatarx2 = Cxsize-3.0;
		Hatary2 = Cysize-3.0;
		if( !Gpujatek ) {
			Zoomnezet->keszitbelsot( 0, 0, vw-1, vh-1, Zoomkep );
			ppic = Zoomnezet;
		}
	}

	// Kiszamoljuk kepernyo bal also sarkat:
	vekt2 motorkozep = pmot->kor1.r;

	vekt2 sarok( motorkozep.x-(Mo_bal+pvalt->baljobbv_h.baljobb*Mo_dx),
				 motorkozep.y-Mo_y );

	//ppic->fillbox( 100 );
	if( Gpujatek ) {
		// (port) the view (zoomed: a bigger picture in the same rectangle),
		// the sky, then the level behind the bikes:
		gpunezet( Gx1, Gy1, eCxsize, eCysize, Cxsize, Cysize );
		int xe1 = 0, ye1 = 0;
		Pecsetalso->getbalalso_int( sarok, &xe1, &ye1 );
		port_gj_eg( Plgr->peg, Plgr->egxmodulus, Plgr->peg->getysize(),
					(xe1/2) % Plgr->egxmodulus );
		port_gj_reteg( 0, xe1, ye1 );
	}
	else
		Pecsetalso->kitesz( ji, ppic, sarok, 0, 0, Cxsize-1, Cysize-1 );
	int sordarabszam = Kisordarabszam;

	hang( /*"Ecsetalso utan"*/ );


	// Objektumok kirajzolasa, de view-ba csak kesobb kerulnek:
	int balalsox, balalsoy;
	Pecsetalso->getbalalso_int( sarok, &balalsox, &balalsoy );
	int objminx = balalsox - 2*OBJEKTUMSUGAR_I - 2;
	int objminy = balalsoy - 2*OBJEKTUMSUGAR_I - 2;
	int objmaxx = balalsox + (Cxsize > Port_kepx ? Cxsize : Port_kepx); // (port)
	int objmaxy = balalsoy + (Cysize > Port_kepy ? Cysize : Port_kepy);
	for( int i = 0; i < MAXKEREK; i++ ) {
		kerek* pker = Ptop->kerektomb[i];
		if( !pker )
			break;

		if( pker->tipus == T_KEZDO ||
			(pker->tipus == T_KAJA && !pker->aktiv) ||
			(!Single && Tag && pker->tipus == T_CEL) )
			continue;

		if( pker->rxint < objminx || pker->ryint < objminy ||
			pker->rxint > objmaxx || pker->ryint > objmaxy )
			continue;


		pic8* pobjpic;
		int dy = 0;
		if( State->animatedobjects ) {
			switch( pker->tipus ) {
				case T_KAJA:
					pobjpic = Plgr->foodtomb[
							pker->foodsorszam%Plgr->foodszam ]->getframe( t );
					dy = 5*sin( t*15.5+pker->sinfazis );
					break;
				case T_CEL:
					pobjpic = Plgr->pexit->getframe( t );
					dy = 5*sin( t*15.5+pker->sinfazis );
					break;
				case T_HALALOS:
					pobjpic = Plgr->pkiller->getframe( t );
					break;
				default:
					hiba( "HIBA6751353" );
			}
		}
		else {
			// Elso frame-et adja vissza mindig:
			switch( pker->tipus ) {
				case T_KAJA:
					pobjpic = Plgr->foodtomb[
							pker->foodsorszam%Plgr->foodszam ]->
							getframebyindex( 0 );
					break;
				case T_CEL:
					pobjpic = Plgr->pexit->getframebyindex( 0 );
					break;
				case T_HALALOS:
					pobjpic = Plgr->pkiller->getframebyindex( 0 );
					break;
				default:
					hiba( "6754783" );
			}
		}

		if( Gpujatek )
			port_gj_kep( pobjpic, pker->rxint-balalsox, pker->ryint-balalsoy + dy );
		else
			blt8( ppic, pobjpic, pker->rxint-balalsox,
									   pker->ryint-balalsoy + dy );
	}

	// Motorosok kirajzolasa:
	if( !Single && Aktvaltok ) {
		// Hatso motorosok kirajzolasa (port: every other player):
		// (port) Centre and reach follow the view size (was 320,240 and 540 =
		// half diagonal of 640x480 + 140):
		vekt2 kozep( sarok.x + 0.5*Cxsize*Aranyrec, sarok.y + 0.5*Cysize*Aranyrec );
		double felatlo = 0.5*sqrt( double(Cxsize)*Cxsize + double(Cysize)*Cysize );
		if( Cxsize < Port_kepx || Cysize < Port_kepy ) {
			// keep at least the original reach
			double teljes = 0.5*sqrt( double(Port_kepx)*Port_kepx + double(Port_kepy)*Port_kepy );
			if( felatlo < teljes )
				felatlo = teljes;
		}
		for( int j = 0; j < Jatekosszam; j++ ) {
			if( j == ji )
				continue;
			double tav = abs( Pmotok[j]->kor1.r - kozep );
			if( tav < (felatlo+140.0)*Aranyrec )
				kibike( j, ppic, t, sarok, Pmotok[j], Aktvaltok[j], motorkepe( j ) );
		}
	}

	// Motoros kirajzolasa: 
	kibike( ji, ppic, t, sarok, pmot, pvalt, motorkepe( ji ) );

	// Felso ecset kitevese:
	if( Gpujatek ) {
		int xe1 = 0, ye1 = 0;
		Pecsetfelso->getbalalso_int( sarok, &xe1, &ye1 );
		port_gj_reteg( 1, xe1, ye1 );
	}
	else {
		Pecsetfelso->kitesz( ji, ppic, sarok, 0, 0, Cxsize-1, Cysize-1 );
		sordarabszam += Kisordarabszam;
	}

	if( zoomol ) {
		// (port) scale the zoomed picture into the view and restore the sizes
		int vw = Cxsize, vh = Cysize;
		Cxsize = eCxsize;
		Cysize = eCysize;
		Hatarx2 = eHatarx2;
		Hatary2 = eHatary2;
		Mo_bal = eMo_bal;
		Mo_dx = eMo_dx;
		Mo_y = eMo_y;
		ppic = celpic;
		if( Gpujatek )
			port_gj_skala( Cxsize, Cysize ); // the navigator and the timer at their normal size
		else
			zoomskalaz( ppic, Cxsize, Cysize, vw, vh );
	}

	if( viewki )
		kiview( ji, ppic, pvalt->baljobbv_h.baljobb, motorkozep );

	double fogoido = -1.0;
	if( !Single )
		fogoido = Fogoido[ji & 3]; // (port)

	// Idokiiras:
	if( timeki ) {
		// Korny->legjobbido - volt eredetileg "" helyett:
		/*if( sordarabszam > 0 ) {
			char darabok[10];
			sprintf( darabok, "%d", sordarabszam );
			strrev( darabok );
			while( strlen( darabok ) < 6 )
				strcat( darabok, "0" );
			strrev( darabok );
			darabok[7] = darabok[5];
			darabok[6] = darabok[4];
			darabok[4] = darabok[3];
			darabok[3] = darabok[2];
			kidigit( darabok, fogoido, t, ppic, Cxsize, Cysize );
		}
		else*/
			if( Tag )
				kidigit( Ezenlegjobbido, fogoido, t, ppic, Cxsize, Cysize );
			else
				kidigit( Ezenlegjobbido, -1.0, t, ppic, Cxsize, Cysize );
	}
	if( Gpujatek )
		port_gj_nezet_vege();
}

static pic8* Bpic = NULL;

// (port) Bpic->keszitbelsot, also remembering where the view is (GPU):
static void bpicbeallit( int x1, int y1, int x2, int y2, pic8* npic ) {
	Bpic->keszitbelsot( x1, y1, x2, y2, npic );
	Gx1 = x1;
	Gy1 = y1;
}

extern int Ucsosurlodas;

// (port) The original 2-player entry point, kept for compatibility:
void kirajzol320(
        double t,
        valtozok* pvalt1, valtozok* pvalt2,
        int viewki1, int timeki1,
        int viewki2, int timeki2 ) {
	valtozok* pv[4] = { pvalt1, pvalt2, pvalt2, pvalt2 };
	int vk[4] = { viewki1, viewki2, 0, 0 };
	int tk[4] = { timeki1, timeki2, 0, 0 };
	kirajzol320n( t, Single ? 1 : 2, pv, vk, tk );
}

// (port) 3 or 4 views:
static void kirajzol34( double t, int n, valtozok** pv, int* viewki, int* timeki ) {
	pic8* npic = NULL;
	if( Gpujatek )
		npic = gpukezd();
	else {
		Locky0_alul = 1;
		npic = lockbackbuffer_pic( 0 );
		Locky0_alul = 0;
	}

	if( !Bpic )
		Bpic = new pic8( 10, PORT_FIZY );

	if( Kitoltestmegrak > 0 ) {
		Kitoltestmegrak--;
		if( !Gpujatek )
			kitolthatteret( npic ); // background in the gaps between the views
	}

	// Slots in buffer coordinates. The picture is upside down (Locky0_alul),
	// so the top of the screen is the high y values:
	int W = Port_kepx, H = Port_kepy;
	int res = resz( 8 ); // gap between views (port: see Reszint)
	int fh = (H-res)/2;
	int fw = (W-res)/2;
	// Only the views that are shown (Show/Hide), in player order. The
	// remaining views share the screen:
	//   1: whole screen   2: top / bottom   3: one on top, two below
	//   4: two on top, two below
	int lat[4], k = 0;
	for( int j = 0; j < n; j++ )
		if( pv[j]->showkep )
			lat[k++] = j;
	if( k == 0 ) {
		// lejatszo shows everyone again before this can happen
		for( int j = 0; j < n; j++ )
			lat[j] = j;
		k = n;
	}
	int sx[4], sy[4], sw[4], sh[4];
	if( k == 1 ) {
		sx[0] = 0;    sy[0] = 0;    sw[0] = W;  sh[0] = H;
	}
	else if( k == 2 ) {
		sx[0] = 0;    sy[0] = H-fh; sw[0] = W;  sh[0] = fh;
		sx[1] = 0;    sy[1] = 0;    sw[1] = W;  sh[1] = fh;
	}
	else if( k == 3 ) {
		// A triangle of three equal views, e.g. A on top in the middle,
		// B bottom left, C bottom right (the top corners show the background):
		sx[0] = (W-fw)/2; sy[0] = H-fh; sw[0] = fw; sh[0] = fh;
		sx[1] = 0;    sy[1] = 0;    sw[1] = fw; sh[1] = fh;
		sx[2] = W-fw; sy[2] = 0;    sw[2] = fw; sh[2] = fh;
	}
	else {
		// A B on top, C D below:
		sx[0] = 0;    sy[0] = H-fh; sw[0] = fw; sh[0] = fh;
		sx[1] = W-fw; sy[1] = H-fh; sw[1] = fw; sh[1] = fh;
		sx[2] = 0;    sy[2] = 0;    sw[2] = fw; sh[2] = fh;
		sx[3] = W-fw; sy[3] = 0;    sw[3] = fw; sh[3] = fh;
	}

	for( int i = 0; i < k; i++ ) {
		int j = lat[i];
		nezetbeallit( sx[i], sy[i], sw[i], sh[i] );
		bpicbeallit( Cx1, Cy1, Cx2, Cy2, npic );
		kirakegyjatekost( j, Bpic, t, Pmotok[j], pv[j], viewki[j], timeki[j] );
	}

	if( Gpujatek ) {
		gpuvege();
		hang();
		return;
	}

	// Mentes lemezre:
	menteshakell( npic );

	hang( /*"unlock elott"*/ );

	Locky0_alul = 1;
	unlockbackbuffer_pic();
	Locky0_alul = 0;
}

void kirajzol320n( double t, int n, valtozok** pv, int* viewki, int* timeki ) {
	Aktvaltok = pv;
	Gpujatek = port_gpu(); // (port)
	if( n >= 3 ) {
		kirajzol34( t, n, pv, viewki, timeki );
		return;
	}
	valtozok* pvalt1 = pv[0];
	valtozok* pvalt2 = pv[1];
	int viewki1 = viewki[0], timeki1 = timeki[0];
	int viewki2 = n > 1 ? viewki[1] : 0, timeki2 = n > 1 ? timeki[1] : 0;

	int splitscreen = 0;
	int fulljatekosA = 1;

	if( !Single ) {
		// Multiplayer modban:
		if( !pvalt1->showkep && !pvalt2->showkep )
			hiba( "g45jnuhbvfr" );
		if( pvalt1->showkep && pvalt2->showkep ) {
			splitscreen = 1;
		}
		else {
			// Csak egyik latszik:
			if( pvalt1->showkep )
				fulljatekosA = 1;
			else
				fulljatekosA = 0;
		}
	}

	pic8* npic = NULL;
	if( Gpujatek )
		npic = gpukezd(); // (port)
	else {
		Locky0_alul = 1;
		npic = lockbackbuffer_pic( 0 );
		Locky0_alul = 0;
	}

    if( !Bpic )
        Bpic = new pic8( 10, PORT_FIZY ); // (port) row capacity for the tallest screen


	// hiba( "Itt van 11!" ); idejott

    if( Kitoltestmegrak > 0 ) {
        Kitoltestmegrak--;
        beallitmereteket( splitscreen );

		// hiba( "Itt van 13!" ); idejott

        if( !Gpujatek )
            kitolthatteret( npic );
    }

	// hiba( "Itt van 12!" ); ide nemjon

	if( splitscreen ) {
		// 1. Jatekos:
		bpicbeallit( Cx1, Cy1, Cx2, Cy2, npic );

		kirakegyjatekost(
			0,
			Bpic, t, Pmot1,
			pvalt1,
			viewki1, timeki1 );

		// 2. Jatekos:
		bpicbeallit( Cx1, Cy1Bplayer, Cx2, Cy2Bplayer, npic );

		kirakegyjatekost(
			1,
			Bpic, t, Pmot2,
			pvalt2,
			viewki2, timeki2 );
	}
	else {
		bpicbeallit( Cx1, Cy1, Cx2, Cy2, npic );
		if( fulljatekosA ) {
			kirakegyjatekost(
					0,
                    Bpic, t, Pmot1,
                    pvalt1,
                    viewki1, timeki1 );
        }
        else {
            kirakegyjatekost(
					1,
                    Bpic, t, Pmot2,
                    pvalt2,
                    viewki2, timeki2 );
        }
    }

	if( Gpujatek ) { // (port)
		gpuvege();
		hang();
		return;
	}

    // Mentes lemezre:
    menteshakell( npic );

	//for( int i = 0; i < Ucsosurlodas; i++ )
	//	npic->ppixel( i, 100, 0 );

	/*extern int Voltosszemosas;
	if( Voltosszemosas ) {
		for( int i = 0; i < 10; i++ )
			for( int j = 0; j < 10; j++ )
				npic->ppixel( 100+i, 100+j, 0 );
		Voltosszemosas--;
	}

	extern int Konvex;
	if( Konvex ) {
		for( int i = 0; i < 10; i++ )
			for( int j = 0; j < 10; j++ )
				npic->ppixel( 100+i, 400+j, 0 );
		Konvex--;
	}

	extern int Konkav;
	if( Konkav ) {
		for( int i = 0; i < 10; i++ )
			for( int j = 0; j < 10; j++ )
				npic->ppixel( 100+i, 300+j, 0 );
		Konkav--;
	}*/

	hang( /*"unlock elott"*/ );

	Locky0_alul = 1;
    unlockbackbuffer_pic();
    Locky0_alul = 0;
}



