#include	"all.h"
#include	"dik.h" // (port) DIK_RETURN

// Jatek modja ket fele lehet:
int Single = 1; 
// Van-e fogocska:
int Tag = 0;
//int Multi = 0;
//int Match = 0;

int Legkozment = 0;

// (port) Number of players in the current play / replay (1..4):
int Jatekosszam = 1;
// (port) Who touched the flower (0..3), or -1:
int Viragjatekos = -1;

// (port) Player index of a bike:
static int motorindex( motorst* pmot ) {
	for( int i = 0; i < 4; i++ )
		if( Pmotok[i] == pmot )
			return i;
	return 0;
}

static int Kajakell;

struct viewtimest {
	int viewkinplay, viewkinreplay;
	int timekinplay, timekinreplay;
};

static void initsoundlejatszoban( void ) {
	if( State->soundon && !Hangmarmegy ) {
		Hangmarmegy = 1;
		Hangenabled = 1;

		Mute = 1;
		#ifdef W_95
			starthanghigh();
			void initdsound( int );
			initdsound( State->secondarybuffer );
		#endif
	}
}

// Player 1 es 2-re (port: and 3, 4):
static viewtimest Viewtime[4] = { { 1, 0, 1, 0 }, { 1, 0, 1, 0 },
								  { 1, 0, 1, 0 }, { 1, 0, 1, 0 } };

// 0-meghalt, 1-megnyerte, 2-semmi kulonos:
static int spritefeldolgoz( int sorszam, int* pkajaszam, motorst* pmot ) {
	if( sorszam < 0 || sorszam >= MAXKEREK )
		hiba( "spritefeldolgoz-ban sorszam < 0 || sorszam >= MAXKEREK!" );
	if( !Ptop->kerektomb[sorszam] )
		hiba( "spritefeldolgoz-ban !Ptop->kerektomb[sorszam]!" );

	int tipus = Ptop->kerektomb[sorszam]->tipus;

	if( tipus == T_HALALOS )
		return 0;
	if( tipus == T_KAJA ) {
		Ptop->kerektomb[sorszam]->aktiv = 0;
		(*pkajaszam)++;
		startwavegyujto( WAV_EVES, 0.99, -1 );
		switch( Ptop->kerektomb[sorszam]->kajatipus ) {
			case KT_UP: pmot->gravirany = 0; break;
			case KT_DOWN: pmot->gravirany = 1; break;
			case KT_LEFT: pmot->gravirany = 2; break;
			case KT_RIGHT: pmot->gravirany = 3; break;
		}
		return 2;
	}
	if( tipus == T_CEL ) {
		// (port) apples eaten by all players count together:
		int kajak = 0;
		for( int i = 0; i < 4; i++ )
			kajak += Pmotok[i]->kajaszam;
		if( kajak >= Kajakell )
			return 1;
	}
	return 2;
}

static double Fordido = 0.5;
static double baljobbszamol( double eltelt, int hatra ) {
	#ifdef TEST
		if( eltelt < 0 )
			hiba( "baljobbszamol-ban eltelt < 0!" );
	#endif
	if( hatra ) {
		if( eltelt > Fordido )
			return 0.0;
		return 1.0-eltelt/Fordido;
	}
	else {
		if( eltelt > Fordido )
			return 1.0;
		return eltelt/Fordido;
	}
}

static double Forgasido = 0.35;

//static int Marvoltgaz = 0;

// Ide csak *pmeghalt == 0-val erkezhet
static void belsoresz( motorst* pmot, jatekosopciok* popciok,
					   valtozok* pvalt, recorder* prec,
					   long* pmegvanido, int* pmeghalt,
					   double eddig, double dt ) {
	// Ugras elintezese:
	int ugrik1 = 0, ugrik2 = 0;
	if( eddig > pvalt->utolsougras + Ugroturelem ) {
		if( (mk_getstate( popciok->billugras1 ) && Sajatugras) ||
			(mk_getstate( popciok->billugras2 ) && !Sajatugras) ) {
			ugrik1 = 1;
			pvalt->utolsougras = eddig;
			pvalt->ugras1volt = 1;
			startwavegyujto( WAV_UGRAS1, 0.99, -1 );
		}
		if( (mk_getstate( popciok->billugras2 ) && Sajatugras) ||
			(mk_getstate( popciok->billugras1 ) && !Sajatugras) ) {
			ugrik2 = 1;
			pvalt->utolsougras = eddig;
			pvalt->ugras1volt = 0;
			startwavegyujto( WAV_UGRAS2, 0.99, -1 );
		}
	}

	// LEPTET!!!!!!!:
	// 0-meghalt, 1-megnyerte, 2-semmi kulonos
	leptet(
		pmot,
		eddig,
		dt,
		mk_getstate( popciok->billgaz ),
		mk_getstate( popciok->billfek ),
		ugrik1,
		ugrik2 );

	int eredmeny = vizsgalat( pmot );
	if( eredmeny == 0 ) {
		// Beteszunk egy 0 hangot:
		pvalt->inf.surlero = 0;
		pvalt->inf.frekvencia = -1;
		pvalt->inf.gaz = 0;
		prec->store( pmot, eddig, &pvalt->inf );
		if( !Single && Tag ) {
			vekt2 halalhely = pmot->kor1.r; // (port) where the holder died
			// Flag Tag modban nem allnak meg, hanem visszamennek starthelyre:
			void initmotor( motorst* pmot ); // adatok.cpp-ben
			initmotor( pmot );
			// Kezdetihelydiff-et setallaktiv allitja be:
			pmot->kor1.r = pmot->kor1.r + Kezdetihelydiff;
			pmot->kor2.r = pmot->kor2.r + Kezdetihelydiff;
			pmot->kor4.r = pmot->kor4.r + Kezdetihelydiff;
			pmot->vezetor = pmot->vezetor + Kezdetihelydiff;

			// Valtozok inicializalasa:
			int showkep = pvalt->showkep;
			BYTESET( pvalt, 0, sizeof( valtozok ) );
			pvalt->baljobbv_f.ucsoford    = -1000.0;
			pvalt->baljobbv_f.ucsoforgas  = -1000.0;
			pvalt->baljobbv_h.ucsoford    = -1000.0;
			pvalt->baljobbv_h.ucsoforgas  = -1000.0;
			pvalt->utolsougras = -100.0;
			pvalt->showkep = showkep;

			if( Jatekosszam >= 3 ) {
				// (port) 3/4 players: the flag goes to the nearest player
				fogo_meghalt_n( motorindex( pmot ), Jatekosszam, halalhely );
			}
			else if( (pmot == Pmot1 && Aafogo) ||
				(pmot == Pmot2 && !Aafogo) ) {
				// Nala volt zaszlo
				Aafogo = !Aafogo;
			}
		}
		else {
			*pmeghalt = 1;
			return;
		}
	}

	//eddig += dt; // Ezt meg kell ismetelni eggyel feljebb is
	// Berreges es nyikorgas:
	pvalt->inf.surlero = kiszamolsurlodast();

	if( pmot->hatra_f )
		pvalt->inf.frekvencia = fabs(pmot->kor2.omega)*0.025;
	else
		pvalt->inf.frekvencia = fabs(pmot->kor4.omega)*0.025;
	// Frekvenciat betesszuk 1.0-2.0 tartomanyba:
	if( pvalt->inf.frekvencia > 30.0 )
		pvalt->inf.frekvencia = 30.0;
	pvalt->inf.frekvencia = 2.0 - exp(-pvalt->inf.frekvencia);

	pvalt->inf.gaz = (char)mk_getstate( popciok->billgaz );
	prec->store( pmot, eddig, &pvalt->inf );

	// Egyedi hangok:
	int wavazonosito;
	double hangero;
	int objszam;
	while( getwavegyujto( &wavazonosito, &hangero, &objszam ) ) {
		if( objszam >= 0 ) {
			int eredmeny = spritefeldolgoz( objszam, &pmot->kajaszam, pmot );
			if( eredmeny == 0 || eredmeny == 1 ) {
				if( eredmeny == 0 )
					*pmeghalt = 1;
				else
					*pmegvanido = eddig*100.0/(182.0*0.0024);
			}
		}
		else {
			startwave( wavazonosito, hangero );
		}
		prec->addhang( eddig, (char)wavazonosito, hangero, objszam );
	}
}

// (port) 3/4 players: if every view has been hidden, all of them are shown
// again (with 2 players the original does the same in toggleresz):
static void mindetmutat( valtozok* valt, int n ) {
	if( n < 3 )
		return;
	for( int j = 0; j < n; j++ )
		if( valt[j].showkep )
			return;
	for( int j = 0; j < n; j++ )
		valt[j].showkep = 1;
	Kitoltestmegrak = Kitoltestmegrakkezd;
}

static void toggleresz( jatekosopciok* popciok, valtozok* pvalt,
						int* pviewkin, int* ptimekin, int* pmasikshow ) {
	// Showkep:
	if( !pvalt->showkepnyomva && mk_getstate( popciok->billshowkep ) ) {
		Kitoltestmegrak = Kitoltestmegrakkezd;
		if( !pmasikshow ) {
			// (port) 3/4 players: each player hides or shows their own view;
			// lejatszo shows them all again if every view is hidden:
			pvalt->showkep = !pvalt->showkep;
		}
		else if( !*pmasikshow ) {
			// Ha masik jatekos ablaka nem latszott, mindegyik fog latszani:
			*pmasikshow = 1;
			pvalt->showkep = 1;
		}
		else {
			pvalt->showkep = !pvalt->showkep;
		}
	}
	pvalt->showkepnyomva = mk_getstate( popciok->billshowkep );
	// View ablak:
	if( !pvalt->viewnyomva && mk_getstate( popciok->billview ) )
		 *pviewkin = !*pviewkin;
	pvalt->viewnyomva = mk_getstate( popciok->billview );
	// Ido kint legyen vagy ne:
	if( !pvalt->timenyomva && mk_getstate( popciok->billtime ) )
		*ptimekin = !*ptimekin;
	pvalt->timenyomva = mk_getstate( popciok->billtime );
}

static void baljobbelintez( baljobbvaltozok* pvalt, recorder* prec,
							double eddig, int hatra ) {
	if( pvalt->eddighatra != hatra ) {
		pvalt->ucsoforgas = eddig;
		double eltelt = eddig-pvalt->ucsoford;
		if( eltelt < Fordido ) {
			pvalt->ucsoford = eddig+eltelt-Fordido;
		}
		else {
			pvalt->ucsoford = eddig;
		}
		pvalt->eddighatra = hatra;
		// Hang adas:
		if( prec ) {
			// Most kihagyjuk wavgyujtot:
			startwave( WAV_FORDULAS, 0.99 );
			prec->addhang( eddig, WAV_FORDULAS, 0.99, -1 );
		}
	}

	// Kirajzolashoz kellenek:
	pvalt->forgas = (eddig-pvalt->ucsoforgas)/Forgasido;
	#ifdef TEST
		if( pvalt->forgas < 0.0 )
			hiba( "lejatszo-ban forgas < 0.0!" );
	#endif
	if( pvalt->forgas > 1.0 )
		pvalt->forgas = 1.0;

	pvalt->baljobb = baljobbszamol( eddig-pvalt->ucsoford, hatra );
}

static void kulsoresz( motorst* pmot, jatekosopciok* popciok,
					   valtozok* pvalt, recorder* prec,
					   viewtimest* pvt, double eddig,
					   int* pmasikshow, int meghalt ) {
	toggleresz( popciok, pvalt, &pvt->viewkinplay, &pvt->timekinplay,
		pmasikshow );

	// Hatra fordulas elintezes:
	if( !meghalt ) {
		if( !pvalt->hatranyomva && mk_getstate( popciok->billfordul ) ) {
			pmot->hatra_f = !pmot->hatra_f;
			szamitfejr( pmot );
		}
		pvalt->hatranyomva = mk_getstate( popciok->billfordul );
	}
	baljobbelintez( &pvalt->baljobbv_f, prec, eddig, pmot->hatra_f );

	// Hatra elintezes:
	if( pmot->gravirany == 0 ) // UP
		pmot->hatra_h = !pmot->hatra_f;
	else
		pmot->hatra_h = pmot->hatra_f;
	baljobbelintez( &pvalt->baljobbv_h, NULL, eddig, pmot->hatra_h );

	// Kirajzolashoz kell:
	pvalt->ugrasnagysag = 1.0-(eddig-pvalt->utolsougras)/Ugroturelem;
	if( pvalt->ugrasnagysag < 0 )
		pvalt->ugrasnagysag = 0;
	#ifdef TEST
		if( pvalt->ugrasnagysag > 1.00001 )
			hiba( "pvalt->ugrasnagysag > 1.00001!" );
	#endif
}

// lejatszo ezt beallitja: 0 -> nem kell kiirni, 1 -> A eloszor, 2 -> B
// csak akkor A vagy B eloszor, ha mindketten meghaltak, tehat
// afterplay menu-nek ezt mar feldolgozta elore:
int MeghalteloszorAB = 0;

// Ha sikeres szint, ide teszi be hogy ki ment ki:
int Aerintetteviragot = 0;

int Masodikmenet = 0;

// Idot adja vissza szazadmasodpercben:
// (port) Generalised to 1..4 players. With 1 or 2 players everything happens
// in the same order as in the original, so play and .rec replays are unchanged.
long lejatszo( char* filenev ) {
	// (port) Full 960x544 view and gameplay buttons while this function runs:
	port_jatek_guard portguard;

	// Azert allitom 1-be, hogy mindenkeppen eszrevegyem, ha nem allitom
	// meg kilepes elott be rendesen:
	MeghalteloszorAB = 1;
	Aerintetteviragot = 0;
	Viragjatekos = -1;

	// Jatekmod eldontese:
	Single = State->single;
	Tag = State->tag;
	int n = 1;
	if( !Single ) {
		n = State->jatekosszam;
		if( n < 2 )
			n = 2;
		if( n > 4 )
			n = 4;
	}
	Jatekosszam = n;
	Prec1->settag( Tag );
	recek234settag( Tag );

	beallitlegjobbidot( filenev, Single );
	if( Jatekosszam >= 3 )
		beallitlegjobbidot34( filenev, Jatekosszam ); // (port) 3/4 player best time

	if( Single )
		Multirec = 0;
	else if( n >= 3 )
		Multirec = n; // (port) .rc4
	else
		Multirec = 1;

	int palmegnincs = 1;
	initadatok();
	if( !Ptop )
		hiba( "lejatszo-ban !Ptop!" );
	Ptop->kereklefejjel();
	Ptop->kitoltiszineket();
	Ptop->killerekelore();
	// setallaktiv allitja be motor kezdeti helyzetet es fazisokat is!:
	Kajakell = Ptop->setallaktiv( Pmot1 );
	for( int j = 1; j < 4; j++ )
		Ptop->setallaktiv( Pmotok[j] );

	// Eloszor keretet tobbszor kirakja kirajzol320:
	Kitoltestmegrak = Kitoltestmegrakkezd; 

	double eddig = 0.0;
	resetwavegyujto();

	int plussznyomva = 0;
	int minusznyomva = 0;
	int snapnyomva = 0;
	mv_startstopper();

	// Valtozok inicializalasa:
	valtozok valt[4]; // Inicializalni kell !!!!!!!!!!!!
	valtozok* pvaltok[4];
	int meghalt[4];
	long megvanido[4];
	for( int j = 0; j < 4; j++ ) {
		BYTESET( &valt[j], 0, sizeof( valtozok ) );
		valt[j].baljobbv_f.ucsoford    = -1000.0;
		valt[j].baljobbv_f.ucsoforgas  = -1000.0;
		valt[j].baljobbv_h.ucsoford    = -1000.0;
		valt[j].baljobbv_h.ucsoforgas  = -1000.0;
		valt[j].utolsougras = -100.0;
		valt[j].showkep     = 1;
		pvaltok[j] = &valt[j];
		meghalt[j] = 0;
		megvanido[j] = 0;
		resetleptet( Pmotok[j] );
	}
	int elsohalott = 0; // (port) who died first (1..4), 0 = nobody yet
	int motorjar[4] = { 1, 1, 1, 1 }; // engine sounds of players A..D

	initsoundlejatszoban();
	Mute = 0;
	if( Single ) {
		startmotor( 1 );
	}
	else {
		startmotor( 1 );
		startmotor( 0 );
		for( int j = 2; j < n; j++ )
			startmotor_n( j ); // (port) players C and D
	}

	if( n >= 3 )
		fogocskareset_n( n );
	else {
		fogocskareset();
		fogo_ketbol();
	}
	for( int j = 0; j < 4; j++ )
		Fogohalott[j] = j >= n; // (port)

	for( int j = 0; j < 4; j++ )
		Pmotok[j]->kajaszam = 0;
	long l = 0;
	while( 1 ) {
		// Kiszamolja mennyit kell leptetni:
		double cel = mv_stopperido()*0.0024;
		if( cel < 0.000001 )
			cel = 0.000001;

		mv_check( /*"Lej 112"*/ );  // Billentyut itt olvassuk be
		int hangfuto = 1;
		while( eddig <= cel-0.000001 ) {
			if( (hangfuto++)%10 == 0 )
				hang( /*"hangfuto 10-nel tart"*/ );

			// Ez ELASTO MANIA 1.0 lepeskoze:
			double dt = 0.0055; // 0.0065

			if( eddig+dt > cel )
				dt = cel-eddig;

			for( int j = 0; j < n; j++ )
				if( !meghalt[j] )
					belsoresz( Pmotok[j], State->opciok( j ),
							   &valt[j], Precek[j],
							   &megvanido[j], &meghalt[j],
							   eddig, dt );

			int elok = 0;
			for( int j = 0; j < n; j++ )
				if( !meghalt[j] )
					elok++;
			if( elok > 0 && !elsohalott ) {
				// Ha meg nem halt meg mindenki:
				for( int j = 0; j < n && !elsohalott; j++ )
					if( meghalt[j] )
						elsohalott = j+1;
			}
			if( elok > 0 && elsohalott )
				MeghalteloszorAB = elsohalott; // A=1 .. D=4 (as the original for A/B)

			if( n == 2 ) {
				// As the original: the engine of whoever dies first stops
				// (the other one keeps idling under the crash sound):
				if( motorjar[0] && motorjar[1] && (meghalt[0] || meghalt[1]) ) {
					if( meghalt[0] )
						stopmotor( 1 );
					else
						stopmotor( 0 );
					motorjar[0] = motorjar[1] = 0;
				}
			}
			else if( n >= 3 && elok > 0 ) {
				// (port) A player's engine stops when they die while others
				// still ride (the last one keeps idling, like in 2P):
				for( int j = 0; j < n; j++ ) {
					if( motorjar[j] && meghalt[j] ) {
						stopmotor_n( j );
						motorjar[j] = 0;
					}
				}
			}
			for( int j = 0; j < 4; j++ )
				Fogohalott[j] = j < n ? meghalt[j] : 1; // (port) for Flag Tag

			int celba = -1;
			for( int j = 0; j < n; j++ )
				if( megvanido[j] && celba < 0 )
					celba = j;

			if( celba >= 0 || elok == 0 ) {
				// Vege jateknak:
				long megvan = 0;
				if( celba >= 0 )
					megvan = megvanido[celba];

				if( elok > 0 || Single ) {
					// Ha nem halt meg mindenki, akkor ne irjon semmit:
					MeghalteloszorAB = 0;
				}
				Viragjatekos = celba;
				if( celba == 0 )
					Aerintetteviragot = 1;
				else
					Aerintetteviragot = 0;

				setmotor( 1, 1.0, 0 );
				setmotor( 0, 1.0, 0 );
				for( int j = 2; j < n; j++ )
					setmotor_n( j, 1.0, 0 ); // (port)
				if( megvan )
					startwave( WAV_SIKER, 0.999 );
				else
					startwave( WAV_TORES, 0.999 );
				hangosdelay( Vegenvaras*1000.0 );

				stopmotor( 1 );
				stopmotor( 0 );
				stopmotor_n( 2 ); // (port) players C and D
				stopmotor_n( 3 );

				Mute = 1;
				nullazhangot();

				Ptop->kerekjolalljon();
				for( int j = 0; j < 4; j++ )
					berakrecbehosszat( Precek[j] );
				if( megvan )
					return megvan;
				else
					return -1;
			}

			eddig += dt;
		}

		// Fogocska elintezes:
		if( !Single && Tag ) {
			if( n >= 3 )
				fogocska_n( eddig, n );
			else
				fogocska( eddig );
		}
		if( n <= 2 )
			fogo_ketbol();

		if( Single ) {
			setsurlodas( valt[0].inf.surlero );
			setmotor( 1, valt[0].inf.frekvencia, valt[0].inf.gaz );
		}
		else {
			double surl = 0.0;
			for( int j = 0; j < n; j++ )
				surl += valt[j].inf.surlero;
			setsurlodas( surl );
			setmotor( 1, valt[0].inf.frekvencia, valt[0].inf.gaz );
			setmotor( 0, valt[1].inf.frekvencia, valt[1].inf.gaz );
			for( int j = 2; j < n; j++ ) // (port) players C and D
				setmotor_n( j, valt[j].inf.frekvencia, valt[j].inf.gaz );
		}

		int viewki[4], timeki[4];
		for( int j = 0; j < n; j++ ) {
			// With 2 players each one's Show/Hide affects the other's view;
			// with 3/4 players (NULL) each one hides their own view:
			int dummyshow = 1;
			int* pmasikshow = &dummyshow;
			if( n == 2 )
				pmasikshow = &valt[1-j].showkep;
			if( n >= 3 )
				pmasikshow = NULL;
			kulsoresz( Pmotok[j], State->opciok( j ), &valt[j], Precek[j],
					   &Viewtime[j], eddig, pmasikshow, meghalt[j] );
			viewki[j] = Viewtime[j].viewkinplay;
			timeki[j] = Viewtime[j].timekinplay;
		}
		mindetmutat( valt, n ); // (port)

		kirajzol320n( eddig, n, pvaltok, viewki, timeki );

		if( palmegnincs ) {
			palmegnincs = 0;
			Plgr->pal->set();
		}

		// Egy par kozos toggle:
		// Plusz elintezes:
		if( !plussznyomva && mk_getstate( State->billplussz ) ) {
			novelkepmeret();
		}
		plussznyomva = mk_getstate( State->billplussz );
		// Minusz elintezes:
		if( !minusznyomva && mk_getstate( State->billminusz ) ) 
			csokkentkepmeret();
		minusznyomva = mk_getstate( State->billminusz );
		// (port) Zoom (held keys); Start (Enter) goes back to the original scale:
		zoomkezel( mk_getstate( State->billzoomin ), mk_getstate( State->billzoomout ) );
		if( mk_getstate( DIK_RETURN ) )
			zoomalap();
		// Snap nyomas elintezese:
		if( !snapnyomva && mk_getstate( State->billsnap ) )
			Legkozment = 1;
		snapnyomva = mk_getstate( State->billsnap );

		// Vizsgalat kilepesre:
		if( mk_getstate( MK_ESC ) ) {
			MeghalteloszorAB = 0;
			double eltelido = mv_stopperido();
			// Motor hangjat kikapcsolja:
			stopmotor( 1 );
			stopmotor( 0 );
			stopmotor_n( 2 ); // (port) players C and D
			stopmotor_n( 3 );

			Mute = 1;

			nullazhangot();

			// Kiirja framerate-et:
			double tpl = double(l)/eltelido;
			tpl *= 182;
			int framerate = tpl;
			if( access( "f_rate.inf", 0 ) == 0 ) {
				// igyhagyni fopen-t!:
				FILE* h = fopen( "f_rate.inf", "wt" );
				if( !h )
					hiba( "framerate nem nyilik!" );
				fprintf( h, "%d", framerate );
				fclose( h );
			}
			Ptop->kerekjolalljon();
			for( int j = 0; j < 4; j++ )
				berakrecbehosszat( Precek[j] );
			Masodikmenet = 1;
			return -1;
		}
		l++;
	}
}

// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY
// REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY REPLAY

//char Ccc[] = "Cs1_2";

// Hamissal ter vissza, ha nincsen mar tobb:
static int replaymag( motorst* pmot, jatekosopciok* popciok,
					  valtozok* pvalt, recorder* prec,
					  double eddig, viewtimest* pvt,
					  int* pmasikshow ) {

	toggleresz( popciok, pvalt, &pvt->viewkinreplay, &pvt->timekinreplay,
							pmasikshow );

	// recall koordokat es Hatra-t is beallitja:
	int visszaad = prec->recall( pmot, eddig, &pvalt->inf );
	szamitfejr( pmot ); // mivel nincs leptet ami kiszamolna
	// HANG ELINTEZES:
	char wavindex;
	double hangero;
	int objszam;
	while( prec->gethang( eddig, &wavindex, &hangero, &objszam ) ) {
		if( objszam >= 0 ) {
			int tmpi = 0;
			spritefeldolgoz( objszam, &tmpi, pmot );
		}
		else {
			startwave( wavindex, hangero );
			if( wavindex == WAV_UGRAS1 ) {
				pvalt->ugras1volt = 1;
				pvalt->utolsougras = eddig;
			}
			if( wavindex == WAV_UGRAS2 ) {
				pvalt->ugras1volt = 0;
				pvalt->utolsougras = eddig;
			}
		}
	}

	// Hatra elintezes:
	baljobbelintez( &pvalt->baljobbv_f, NULL, eddig, pmot->hatra_f );

	if( pmot->gravirany == 0 ) // UP
		pmot->hatra_h = !pmot->hatra_f;
	else
		pmot->hatra_h = pmot->hatra_f;
	baljobbelintez( &pvalt->baljobbv_h, NULL, eddig, pmot->hatra_h );

	// Kirajzolashoz kellenek:
	pvalt->ugrasnagysag = 1.0-(eddig-pvalt->utolsougras)/Ugroturelem;
	if( pvalt->ugrasnagysag < 0 )
		pvalt->ugrasnagysag = 0;
	#ifdef TEST
		if( pvalt->ugrasnagysag > 1.00001 )
			hiba( "pvalt->ugrasnagysag > 1.00001!" );
	#endif

	return visszaad;
}

static int Elozoshowkep[4] = { 1, 1, 1, 1 };

// Termeszetes befejezes eseten 0-t ad vissza:
// (port) Generalised to 1..4 players (see lejatszo).
long lejatszo_r( char* filenev, int showkepmarad ) {
	// (port) Full 960x544 view and gameplay buttons while this function runs:
	port_jatek_guard portguard;
	Kitoltestmegrak = 3; // kirajz meg 3x kirakja keretet

	// Jatekmod eldontese:
	Single = !Multirec;
	int n = recjatekosszam();
	Jatekosszam = n;
	Tag = Prec1->gettag();

	beallitlegjobbidot( filenev, Single );
	if( Jatekosszam >= 3 )
		beallitlegjobbidot34( filenev, Jatekosszam ); // (port) 3/4 player best time

	int palmegnincs = 1;
	initadatok();
	if( !Ptop )
		hiba( "lejatszo-ban !Ptop!" );
	Ptop->kereklefejjel();
	Ptop->kitoltiszineket();
	Ptop->killerekelore();
	// setallaktiv allitja be motor kezdeti helyzetet is!:
	Kajakell = Ptop->setallaktiv( Pmot1 );
	for( int j = 1; j < 4; j++ )
		Ptop->setallaktiv( Pmotok[j] );

	// Eloszor keretet tobbszor is kirakja kirajzol320:
	Kitoltestmegrak = Kitoltestmegrakkezd; 

	resetwavegyujto();
	mv_startstopper();

	int plussznyomva = 0;
	int minusznyomva = 0;
	int snapnyomva = 0;

	// Valtozok inicializalasa:
	valtozok valt[4]; // Inicializalni kell !!!!!!!!!!!!
	valtozok* pvaltok[4];
	for( int j = 0; j < 4; j++ ) {
		BYTESET( &valt[j], 0, sizeof( valtozok ) );
		valt[j].baljobbv_f.ucsoford    = -1000.0;
		valt[j].baljobbv_f.ucsoforgas  = -1000.0;
		valt[j].baljobbv_h.ucsoford    = -1000.0;
		valt[j].baljobbv_h.ucsoforgas  = -1000.0;
		valt[j].utolsougras = -100.0;
		valt[j].showkep = showkepmarad ? Elozoshowkep[j] : 1;
		pvaltok[j] = &valt[j];
		resetleptet( Pmotok[j] );
	}

	initsoundlejatszoban();
	Mute = 0;
	if( Single ) {
		startmotor( 1 );
	}
	else {
		startmotor( 1 );
		startmotor( 0 );
		for( int j = 2; j < n; j++ )
			startmotor_n( j ); // (port) players C and D
	}

	if( n >= 3 )
		fogocskareset_n( n );
	else {
		fogocskareset();
		fogo_ketbol();
	}

	long l = 0;
	int jar[4] = { 1, 1, 1, 1 }; // (port)
	int befejezte[4] = { 0, 0, 0, 0 };
	while( 1 ) {
		mv_check( /*"Lej 534"*/ );  // Billentyut itt olvassuk be
		// Kiszamolja mennyit kell leptetni:
		double eddig = mv_stopperido()*0.0024;
		// recall koordokat es Hatra-t is beallitja:
		int mindkesz = 1;
		for( int j = 0; j < n; j++ ) {
			int dummyshow = 1;
			int* pmasikshow = &dummyshow;
			if( n == 2 )
				pmasikshow = &valt[1-j].showkep;
			if( n >= 3 )
				pmasikshow = NULL; // (port) see lejatszo
			befejezte[j] = !replaymag( Pmotok[j], State->opciok( j ),
									   &valt[j], Precek[j], eddig, &Viewtime[j],
									   pmasikshow );
			if( !befejezte[j] )
				mindkesz = 0;
		}
		mindetmutat( valt, n ); // (port)

		if( mindkesz ) {
			// Motor hangjat kikapcsolja:
			setmotor( 1, 1.0, 0.0001 );
			setmotor( 0, 1.0, 0.0001 );
			for( int j = 2; j < n; j++ )
				setmotor_n( j, 1.0, 0.0001 ); // (port)
			stopmotor( 1 );
			stopmotor( 0 );
			stopmotor_n( 2 ); // (port) players C and D
			stopmotor_n( 3 );
			setsurlodas( 0 );
			hangosdelay( Vegenvaras*400.0 );

			Mute = 1;
			nullazhangot();
			Ptop->kerekjolalljon();

			for( int j = 0; j < 4; j++ )
				Elozoshowkep[j] = valt[j].showkep;
			return 0;
		}

		if( !Single ) {
			// (port) each player's engine stops when their replay ends:
			for( int j = 0; j < n; j++ ) {
				if( jar[j] && befejezte[j] ) {
					stopmotor_n( j );
					jar[j] = 0;
				}
			}

			// Fogocska:
			if( n >= 3 )
				fogocska_r_n( eddig );
			else
				fogocska_r( eddig );
		}
		if( n <= 2 )
			fogo_ketbol();

		// HANG ELINTEZES:
		if( Single ) {
			setmotor( 1, valt[0].inf.frekvencia, valt[0].inf.gaz );
			setsurlodas( valt[0].inf.surlero );
		}
		else {
			setmotor( 1, valt[0].inf.frekvencia, valt[0].inf.gaz );
			setmotor( 0, valt[1].inf.frekvencia, valt[1].inf.gaz );
			for( int j = 2; j < n; j++ ) // (port) players C and D
				setmotor_n( j, valt[j].inf.frekvencia, valt[j].inf.gaz );
			double surl = 0.0;
			for( int j = 0; j < n; j++ )
				surl += valt[j].inf.surlero;
			setsurlodas( surl );
		}

		// KIRAJZOLAS:
		int viewki[4], timeki[4];
		for( int j = 0; j < n; j++ ) {
			viewki[j] = Viewtime[j].viewkinreplay;
			timeki[j] = Viewtime[j].timekinreplay;
		}
		kirajzol320n( eddig, n, pvaltok, viewki, timeki );

		if( palmegnincs ) {
			palmegnincs = 0;
			Plgr->pal->set();
		}

		// Egy par kozos toggle:
		// Plusz elintezes:
		if( !plussznyomva && mk_getstate( State->billplussz ) ) {
			novelkepmeret();
		}
		plussznyomva = mk_getstate( State->billplussz );
		// Minusz elintezes:
		if( !minusznyomva && mk_getstate( State->billminusz ) )
			csokkentkepmeret();
		minusznyomva = mk_getstate( State->billminusz );
		// (port) Zoom (held keys); Start (Enter) goes back to the original scale:
		zoomkezel( mk_getstate( State->billzoomin ), mk_getstate( State->billzoomout ) );
		if( mk_getstate( DIK_RETURN ) )
			zoomalap();
		// Snap nyomas elintezese:
		if( !snapnyomva && mk_getstate( State->billsnap ) )
			Legkozment = 1;
		snapnyomva = mk_getstate( State->billsnap );

		// Vizsgalat kilepesre:
		if( mk_getstate( MK_ESC ) ) {
			double eltelido = mv_stopperido();
			// Kiirja framerate-et:
			double tpl = double(l)/eltelido;
			tpl *= 182;
			int framerate = tpl;
			if( access( "f_rate.inf", 0 ) == 0 ) {
				// igyhagyni fopen-t!:
				FILE* h = fopen( "f_rate.inf", "wt" );
				if( !h )
					hiba( "framerate nem nyilik!" );
				fprintf( h, "%d", framerate );
				fclose( h );
			}
			// Hang kikapcsolas:
			setmotor( 1, 1.0, 0.0001 );
			setmotor( 0, 1.0, 0.0001 );
			for( int j = 2; j < n; j++ )
				setmotor_n( j, 1.0, 0.0001 ); // (port)
			stopmotor( 1 );
			stopmotor( 0 );
			stopmotor_n( 2 ); // (port) players C and D
			stopmotor_n( 3 );
			setsurlodas( 0 );
			hangosdelay( Vegenvaras*400.0 );

			Mute = 1;
			nullazhangot();

			Ptop->kerekjolalljon();

			for( int j = 0; j < 4; j++ )
				Elozoshowkep[j] = valt[j].showkep;
			return -1;
		}
		l++;
	}
}
 


			/*if( Match ) {
				if( !meghalt1 )
					meghalt1 = elsomegoltemasodikat( Pmot2, Pmot1 );
				if( !meghalt2 )
					meghalt2 = elsomegoltemasodikat( Pmot1, Pmot2 );
				if( meghalt1 && meghalt2 )
					if( int(eddig*100.0) % 2 )
						meghalt1 = 0;
					else
						meghalt2 = 0;
				if( meghalt1 || meghalt2 ) {
					if( meghalt1 )
						dialog320( "ESC", "B jatekos gyozott!" );
					else
						dialog320( "ESC", "A jatekos gyozott!" );
					// Kilep:
					// Motor hangjat kikapcsolja:
					Mute = 1;
					nullazhangot();
					Ptop->kerekjolalljon();
					return -1;
				}
			}*/

