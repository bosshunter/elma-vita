#include	"all.h"

int Aafogo = 0, Kozelvannak = 0;
int Aleszfogo = 1; // Legkozelebbi jateknal
double Afogoido = 0.0, Bfogoido = 0.0;

// Ez nem publikus:
double Ucsoido = -1.0;

void fogocskareset( void ) {
	Aafogo = Aleszfogo;
	Aleszfogo = !Aleszfogo;
	Kozelvannak = 1;
	Afogoido = 0.0, Bfogoido = 0.0;
	Ucsoido = 0.0;
}

static int kozelvan( vekt2* pa, vekt2* pb, double tavnegyzet ) {
	double dx = pa->x-pb->x;
	double dy = pa->y-pb->y;
	if( dx*dx + dy*dy < tavnegyzet )
		return 1;
	else
		return 0;
}

static double Fogokistavnegyzet = 0.64; // 0.8 a negyzeten
//static double Fogonagytavnegyzet = 3.2; // 1.8 a negyzeten
static double Fogonagytavnegyzet = 1.44; // 1.2 a negyzeten

void fogocska( double t ) {
	if( Kozelvannak ) {
		// Eddig kozel voltak:
		if( !kozelvan( &Pmot1->kor2.r, &Pmot2->kor2.r, Fogonagytavnegyzet ) &&
			!kozelvan( &Pmot1->kor2.r, &Pmot2->kor4.r, Fogonagytavnegyzet ) &&
			!kozelvan( &Pmot1->kor4.r, &Pmot2->kor2.r, Fogonagytavnegyzet ) &&
			!kozelvan( &Pmot1->kor4.r, &Pmot2->kor4.r, Fogonagytavnegyzet ) )
			// Most mar elegendoen tavol vannak:
			Kozelvannak = 0;
	}
	else {
		// Eddig tavol voltak:
		if( kozelvan( &Pmot1->kor2.r, &Pmot2->kor2.r, Fogokistavnegyzet ) ||
			kozelvan( &Pmot1->kor2.r, &Pmot2->kor4.r, Fogokistavnegyzet ) ||
			kozelvan( &Pmot1->kor4.r, &Pmot2->kor2.r, Fogokistavnegyzet ) ||
			kozelvan( &Pmot1->kor4.r, &Pmot2->kor4.r, Fogokistavnegyzet ) ) {
			// Megerintettek egymast:
			Kozelvannak = 1;
			Aafogo = !Aafogo;
		}
	}

	// Idot noveljuk:
	if( Aafogo )
		Afogoido += t-Ucsoido;
	else
		Bfogoido += t-Ucsoido;

	Ucsoido = t;
}

// Aafogo alapjan csak idoket allitja:
void fogocska_r( double t ) {
	// Idot noveljuk:
	if( Aafogo )
		Afogoido += t-Ucsoido;
	else
		Bfogoido += t-Ucsoido;

	Ucsoido = t;
}


// ---------------------------------------------------------------------------
// (port) Flag Tag for 3 and 4 players
// ---------------------------------------------------------------------------

int Fogo = 0;
int Kozel[4] = { 0, 0, 0, 0 };
double Fogoido[4] = { 0.0, 0.0, 0.0, 0.0 };
int Kovfogo = 0;
int Fogohalott[4] = { 0, 0, 0, 0 };

void fogo_ketbol( void ) {
	Fogo = Aafogo ? 0 : 1;
	Kozel[0] = Kozel[1] = 0;
	Kozel[1-Fogo] = Kozelvannak;
	Fogoido[0] = Afogoido;
	Fogoido[1] = Bfogoido;
	Fogoido[2] = Fogoido[3] = 0.0;
}

// Any wheel of bike a near any wheel of bike b:
static int motorokkozel( motorst* a, motorst* b, double tavnegyzet ) {
	return kozelvan( &a->kor2.r, &b->kor2.r, tavnegyzet ) ||
		   kozelvan( &a->kor2.r, &b->kor4.r, tavnegyzet ) ||
		   kozelvan( &a->kor4.r, &b->kor2.r, tavnegyzet ) ||
		   kozelvan( &a->kor4.r, &b->kor4.r, tavnegyzet );
}

static void kozelfrissit( int n ) {
	for( int i = 0; i < 4; i++ )
		Kozel[i] = i < n && i != Fogo &&
				   motorokkozel( Pmotok[i], Pmotok[Fogo], Fogonagytavnegyzet );
}

void fogocskareset_n( int n ) {
	if( n < 1 )
		n = 1;
	Fogo = Kovfogo % n;
	Kovfogo = (Fogo+1) % n; // everyone gets to start in turn
	for( int i = 0; i < 4; i++ )
		Fogoido[i] = 0.0;
	Ucsoido = 0.0;
	// Everyone starts at the same place: nobody can tag until they separate
	for( int i = 0; i < 4; i++ )
		Kozel[i] = i < n && i != Fogo;
}

void fogocska_n( double t, int n ) {
	// The holder is out of the game for good: the flag goes on
	if( Fogohalott[Fogo] )
		fogo_meghalt_n( Fogo, n, Pmotok[Fogo]->kor1.r );

	for( int i = 0; i < n; i++ ) {
		if( i == Fogo || Fogohalott[i] )
			continue;
		if( Kozel[i] ) {
			// They were touching: must get apart before the flag can pass again
			if( !motorokkozel( Pmotok[i], Pmotok[Fogo], Fogonagytavnegyzet ) )
				Kozel[i] = 0;
		}
		else if( motorokkozel( Pmotok[i], Pmotok[Fogo], Fogokistavnegyzet ) ) {
			// Touched the holder: the flag passes
			int regi = Fogo;
			Fogo = i;
			kozelfrissit( n );
			Kozel[regi] = 1;
			break;
		}
	}

	// Idot noveljuk:
	Fogoido[Fogo] += t-Ucsoido;
	Ucsoido = t;
}

void fogocska_r_n( double t ) {
	Fogoido[Fogo & 3] += t-Ucsoido;
	Ucsoido = t;
}

void fogo_meghalt_n( int ki, int n, vekt2 hely ) {
	if( ki != Fogo || n < 2 )
		return;
	// The flag goes to the other player nearest to where the holder died:
	int legjobb = -1;
	double legjobbtav = 0.0;
	for( int i = 0; i < n; i++ ) {
		if( i == ki || Fogohalott[i] )
			continue;
		vekt2 d = Pmotok[i]->kor1.r - hely;
		double tav = d.x*d.x + d.y*d.y;
		if( legjobb < 0 || tav < legjobbtav ) {
			legjobb = i;
			legjobbtav = tav;
		}
	}
	if( legjobb >= 0 ) {
		Fogo = legjobb;
		kozelfrissit( n );
	}
}
