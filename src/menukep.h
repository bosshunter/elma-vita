
// Ezt mv_main elott meghivja es grafikus hibakiirashoz
// kell beolvasni peldaul abc-t es hatteret (teljes.cpp-ben):
//void initmenukep1( void );
//void initmenukep2( void );

void grafikushiba( char* text1, char* text2 = NULL, char* text3 = NULL );

extern pic8* Buffsima;
extern pic8* Buffgolyo;

extern abc8* Pmenuabc;

extern ddpal* Pmenupal;
extern unsigned char Menupaltomb[768];

// (port) The menus are drawn at the Vita's native 960x544. Menu code still
// uses the classic 640x480 coordinates, which szoveglista moves to the centre;
// the *_szeles functions take coordinates on the whole wide screen:
#define MENUSZEL (PORT_FIZX)
#define MENUMAG  (PORT_FIZY)
#define MENUOFSX ((MENUSZEL-640)/2)
#define MENUOFSY ((MENUMAG-480)/2)

struct egyszovsor {
	char text[102];
	int x, y;
};

#define MAXSZOVEGSOR (30)

class szoveglista {
	int sisakx, sisaky;

	egyszovsor* sorok;
	int elemszam;
	int ervenyes;
public:
	szoveglista( void );
	~szoveglista( void );
	void addszoveg( char* text, int x, int y );
	void addszoveg_kozep( char* text, int x, int y );
	void setsisak( int x, int y );
	void addszoveg_szeles( char* text, int x, int y );        // (port)
	void addszoveg_szeles_kozep( char* text, int x, int y );  // (port)
	void setsisak_szeles( int x, int y );                     // (port)
	void clear( void );
	void kirajzol( int nemkellgolyo = 0,
				   pic8* rarakkep1 = NULL, pic8* rarakkep2 = NULL,
				   pic8* rarakkep3 = NULL, pic8* rarakkep4 = NULL );
	int  kirajzolanim( double ido );
	void kirajzolanimkocka( double ido, int elt ); // (port)
};

// Letorli Elasto Mania feliratot hatterkeprol:
void tilevaltas( void );

// Teljes olvassa be es elso kiirloading szabaditja fel:
extern pic8* Pintro;

