#include    "all.h"


static void szogigazit( double* pd ) {
	if( *pd < -K_pi )
		*pd += 2*K_pi;
	if( *pd > K_pi )
		*pd -= 2*K_pi;
}

void leptet( motorst* pmot, double most, double dt, int gaz, int fek,
									int ugrik1, int ugrik2 ) {

	if( fek )
		pmot->kor1.alfa += dt*2.0;
	pmot->kor2.alfa = pmot->kor4.alfa = pmot->kor1.alfa;

	vekt2 i1( cos( pmot->kor1.alfa ), sin( pmot->kor1.alfa ) );
	i1 = i1*dt*(-2.0);

	if( gaz ) {
		pmot->kor1.r = pmot->kor1.r + i1;
		pmot->kor2.r = pmot->kor2.r + i1;
		pmot->kor4.r = pmot->kor4.r + i1;
	}

	pmot->vezetor = pmot->kor1.r;
	szamitfejr( pmot );
}

// 0-meghalt, 2-semmi kulonos
int vizsgalat( motorst* pmot ) {
	// Objektumokkal utkozesvizsgalat:
	int voltkaja = 1;
	while( voltkaja ) { // Ha nem volt kaja kilepunk, kulonben vegtelen ciklus
		voltkaja = 0;
		int sorszamtomb[3];
		sorszamtomb[0] = utkozikesprite( pmot->kor2.r, pmot->kor2.sugar );
		sorszamtomb[1] = utkozikesprite( pmot->kor4.r, pmot->kor4.sugar );
		for( int i = 0; i < 2; i++ ) {
			if( sorszamtomb[i] >= 0 ) {
				startwavegyujto( 0, 0.0, sorszamtomb[i] );
				kerek* pker = Ptop->getptrkerek( sorszamtomb[i] );
				if( pker->tipus == T_KAJA ) {
					pker->aktiv = 0;
					voltkaja = 1; // Hatha van meg tobb kaja is
				}
			}
		}
	}

	return 2;
}

void resetleptet( motorst* pmot ) {
	pmot->voltfek = 0;
	pmot->dfek2 = pmot->dfek4 = 0;
	pmot->ugrasban1 = pmot->ugrasban2 = 0;
	pmot->ugras1kezd = pmot->ugras2kezd = -1.0;
	pmot->kezdoomega1 = pmot->kezdoomega2 = -1.0;
}

// Kor1-bol eloallitja Fejr-et!:
void szamitfejr( motorst* pmot ) {
	pmot->fejr = pmot->kor1.r;
}

double kiszamolsurlodast( void ) {
	return 0.01;
}