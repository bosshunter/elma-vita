#include    "all.h"

static pic8 *Lockbuff = NULL;

static int Backpiclocked = 0, Frontpiclocked = 0;

//int Mindigfront = 0;

static pic8* Pmindigfpic = NULL;

// (port) Called by the error handler, which draws on the screen even if the
// error was raised while a buffer was locked:
void port_picfeloldas( void ) {
	Backpiclocked = Frontpiclocked = 0;
}

pic8* lockbackbuffer_pic( int fizikai ) {
	if( Backpiclocked || Frontpiclocked )
		hiba( "lbb_p lock!" );
	Backpiclocked = 1;
	port_kepmeretfrissit(); // (port)

	if( !Lockbuff ) {
		Lockbuff = new pic8( 10, 10 );
		Lockbuff->sormuttomb = NULL;
	}
	// (port) The screen is 960x544 during gameplay, 640x480 otherwise:
	Lockbuff->xsize = Port_kepx;
	Lockbuff->ysize = Port_kepy;

	if( Waitforretrace ) {
		Lockbuff->sormuttomb = lockbackbuffer( fizikai );
		return Lockbuff;
	}
	else {
		if( Pmindigfpic && (Pmindigfpic->getxsize() != Port_kepx ||
							Pmindigfpic->getysize() != Port_kepy) ) {
			delete Pmindigfpic;
			Pmindigfpic = NULL;
		}
		if( !Pmindigfpic )
			Pmindigfpic = new pic8( Port_kepx, Port_kepy );
		return Pmindigfpic;
	}
}

void  unlockbackbuffer_pic( void ) {
	if( !Backpiclocked || Frontpiclocked )
		hiba( "ulbb_p lock!" );
	Backpiclocked = 0;

	if( Waitforretrace ) {
		unlockbackbuffer();
		Lockbuff->sormuttomb = NULL;
	}
	else {
		bltfront( Pmindigfpic );	
	}
}

#ifdef W_95

// WINDOWSOS:
void  lockfrontbuffer_pic( void ) {
	if( Frontpiclocked || Backpiclocked )
		hiba( "lfb_pw lock!" );
	Frontpiclocked = 1;
	port_kepmeretfrissit(); // (port)

	if( !Lockbuff ) {
		Lockbuff = new pic8( 10, 10 );
		Lockbuff->sormuttomb = NULL;
	}
	Lockbuff->xsize = Port_kepx; // (port)
	Lockbuff->ysize = Port_kepy;

	Lockbuff->sormuttomb = lockfrontbuffer();
}

// WINDOWSOS:
void  unlockfrontbuffer_pic( void ) {
	if( !Frontpiclocked || Backpiclocked )
		hiba( "ulfb_pw lock!" );
	Frontpiclocked = 0;
	unlockfrontbuffer();
}

// WINDOWSOS:
void  ppixelfront( int x, int y, unsigned char szin ) {
	if( !Frontpiclocked )
		hiba( "u53e983w" );
	extern int Port_dialofsx, Port_dialofsy; // (port) centred editor dialogs
	Lockbuff->ppixel( x+Port_dialofsx, y+Port_dialofsy, szin );
}

// (port) A picture of another size than the screen is centred: a classic
// 640x480 one on the 960x544 screen (the border is filled with the colour of
// its top left pixel, which is the background colour of those screens), or
// the middle of a 960x544 menu picture on the editor's 640x480 screen:
static int kozepx( pic8* ppic ) {
	return (Lockbuff->getxsize() - ppic->getxsize())/2;
}
static int kozepy( pic8* ppic ) {
	return (Lockbuff->getysize() - ppic->getysize())/2;
}

// WINDOWSOS:
void  bltfront( pic8* ppic ) {
	lockfrontbuffer_pic();
	int ox = kozepx( ppic ), oy = kozepy( ppic );
	if( (ox > 0 || oy > 0) && !Port_dialogban ) {
		// (a centred editor dialog keeps the editor around it)
		Lockbuff->fillbox( ppic->gpixel( 0, 0 ) );
		blt8( Lockbuff, ppic, ox, oy );
	}
	else if( ox || oy )
		blt8( Lockbuff, ppic, ox, oy );
	else
		blt8( Lockbuff, ppic );
	unlockfrontbuffer_pic();
}
// WINDOWSOS:
void  bltfront( pic8* ppic, int x1, int y1, int x2, int y2 ) {
	lockfrontbuffer_pic();
	int ox = kozepx( ppic ), oy = kozepy( ppic );
	blt8( Lockbuff, ppic, x1+ox, y1+oy, x1, y1, x2, y2 );
	unlockfrontbuffer_pic();
}

#else

extern pic8* Pscr8;

// DOSOS:
void  lockfrontbuffer_pic( void ) {
	if( Frontpiclocked || Backpiclocked )
		hiba( "lfb_p lock!" );
	Frontpiclocked = 1;
}

// DOSOS:
void  unlockfrontbuffer_pic( void ) {
	if( !Frontpiclocked || Backpiclocked )
		hiba( "ulfb_p lock!" );
	Frontpiclocked = 0;
}

// DOSOS:
void  ppixelfront( int x, int y, unsigned char szin ) {
	if( !Frontpiclocked )
		hiba( "53e983" );
	Pscr8->ppixel( x, y, szin );
}

// DOSOS:
void  bltfront( pic8* ppic ) {
	if( Frontpiclocked )
		hiba( "5398765" );
	extern int Locky0_alul;
	if( Locky0_alul ) {
		if( ppic->getxsize() != 640 || ppic->getysize() != 480 )
			hiba( "5646879" );
		for( int y = 0; y < 480; y++ )
			putline8_l( 0, y, ppic->sormuttomb[479-y], 640 );
	}
	else {
		blt8( Pscr8, ppic );
	}
}

// DOSOS:
void  bltfront( pic8* ppic, int x1, int y1, int x2, int y2 ) {
	if( Frontpiclocked )
		hiba( "879666" );
	blt8( Pscr8, ppic, x1, y1, x1, y1, x2, y2 );
}

#endif






