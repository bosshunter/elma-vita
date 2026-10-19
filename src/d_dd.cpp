#include	"all.h"

// Ezt csak windows-os DirectDraw hasznalja, de itt is kell definialni:
int Locky0_alul = 0;

//static pic8* Pures = NULL;

static void setpal( unsigned char* pal ) {
	outp( 0x3c8, 0 );

	for( int i = 0; i < 768; i++ )
		outp( 0x3c9, pal[i] );
}

ddpal::ddpal( unsigned char* tombp ) {
	{ tomb = NEW(unsigned char*) unsigned char[768]; }
	if( !tomb )
		hiba( "memory ddpal-ban!" );
	memcpy( tomb, tombp, 768 );
}

ddpal::~ddpal( void ) {
	if( tomb )
		DELETE tomb; }
}

void ddpal::set( void ) {
	setpal( tomb );
}

static pic8* Ppicbuff = NULL;

void init_d_dd( void ) {
	if( Ppicbuff ) // Mire kell Ppicbuff?????????????????????????????
		hiba( "init_d_dd ketszer hivva!" );
	{ Ppicbuff = NEW(pic8*) pic8( 640, 480 ); }
}

unsigned char** lockbackbuffer( int fizikai ) {
	fizikai++;

	if( !Ppicbuff )
		hiba( "lockbackbuffer init_d_dd hivasa nelkul!" );

	return Ppicbuff->sormuttomb;
}

// Ki is teszi
void unlockbackbuffer( void ) {
	if( !Ppicbuff )
		hiba( "unlockbackbuffer init_d_dd hivasa nelkul!" );

	for( int y = 0; y < 480; y++ ) {
		putline8_l( 0, y, Ppicbuff->sormuttomb[479-y], 640 );
		if( y == 220 )
			hang( /*"D_DD: unlockbackbuffer"*/ );
	}
}

