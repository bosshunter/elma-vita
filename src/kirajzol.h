
extern int Voltkilogas; // Csak tesztelesre

// Annak eldontesere, hogy kirajzolando doboz nem log-e ki:
// peldaul: Hatarx1 = 2.0, Hatarx2 = 318.0 (resetkirajzol tolti ki):
extern double   Hatarx1,
				Hatarx2,
				Hatary1,
				Hatary2;

extern int	Cxsize, Cysize; // Belso kep meretei

extern int Kitoltestmegrak;
// Es ennyire toltjuk:
extern int Kitoltestmegrakkezd; 

void novelkepmeret( void );
void csokkentkepmeret( void );
// (port) In-game zoom, called every frame with the Zoom In / Zoom Out keys:
void zoomkezel( int be, int ki );
void zoomalap( void ); // (port) back to 1.0 (Start in a level)

void kirajzol320(
		double t,
		valtozok* pvalt1, valtozok* pvalt2,
		int viewki1, int timeki1,
		int viewki2, int timeki2 );

// (port) 1..4 players. pvalt/viewki/timeki are indexed by player (0..3).
// 1-2 players: as the original (split screen top/bottom). 3 players: A on top,
// B and C below side by side. 4 players: A B on top, C D below.
void kirajzol320n( double t, int n, valtozok** pvalt, int* viewki, int* timeki );

extern int Jatekosszam; // (port) players in the current play / replay (lejatszo.cpp)

void forditkepet( pic8* ppic );


