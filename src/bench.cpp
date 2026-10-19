#include	"all.h"

struct ketdupla {
	double a, b;
};

static ketdupla Duplak1[110];
static ketdupla Duplak2[110];


// WATCOM: 39sec
void bench_double( void ) {
	for( int i = 0; i < 100; i++ ) {
		Duplak1[i].a = i*2;
		Duplak1[i].b = i*3;
	}
	for( int j = 0; j < 2000000; j++ ) {
		for( int i = 0; i < 99; i++ ) {
			Duplak2[i].a = Duplak1[i].a*Duplak1[99-i].a +
						   Duplak1[i].b*Duplak1[99-i].b;
		}
	}
}

struct utasit {
	unsigned char* mut;
	int xsize;
};

static unsigned char Source[10000], Dest[10000];

static utasit Utasitasok[100] = {
	{ &Source[100], 10 },
	{ NULL,         22 },
	{ &Source[200], 10 },
	{ NULL,         10 },
	{ &Source[300], 10 },
	{ &Source[400], 4 },
	{ &Source[300], 50 } // ez mar nem fut le
	};

static void egysor( utasit* put, unsigned char* dest ) {
	int xpos = 0;
	while( xpos < 64 ) {
		int xsize = put->xsize;
		if( put->mut )
			memcpy( dest, put->mut, xsize );
		dest += xsize;
		xpos += xsize;
		put++;
	}
}

// WATCOM: 28.5sec
void bench_int( void ) {
	for( int i = 0; i < 12000000; i++ )
		egysor( Utasitasok, &Dest[20] );
}


