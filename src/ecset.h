
class ecset;
struct mdarab;

// Egy iterator class (inkabb cache) ecsethez (foltoz hasznalja oket):
class mdbiter {
	ecset* pecset;
	int xposbent, ybent;
	mdarab* pmd;
public:
	mdbiter( ecset* pecset );
	mdarab* getpmd( int x, int y );
};


// Elso par strukturat be lehetne majd vinni cpp-be:

#define PX_FOLD (1)
#define PX_EG   (NULL)
#define PX_URES (2)

struct mdarab {
	mdarab *pkov;
	int xsize;
	int tavolsag;
	unsigned char* pixelek;
};

#define TOMBBENMDARAB (10000)

struct mdarabtomb {
	mdarab 		tomb[TOMBBENMDARAB];
	mdarabtomb* kovtomb;
};

#define MAXECSETSOR (12500) // (port) was 12000: more room above/below for zooming out

struct darab {
	int xsize;
	unsigned char* pixelek;
};

class ecset {
	friend void betoltecseteket( void );
	friend void segedfv( void );
	friend mdarab* mdbiter::getpmd( int x, int y );

	// madarab allokalas:
	mdarabtomb* elsotomb;
	mdarabtomb* kurtomb;
	int tombbenkov;
	mdarab* newmdarab( void );
	void deletemdarabok( void );

	darab* nagydarabtomb;

	int view;
	vekt2 origo;
	double xsizedb, ysizedb;
	int maxx, sorszam;
	mdarab* msorok[MAXECSETSOR];
	darab*  sorok[MAXECSETSOR];
	// Kulon van ket jatekosnak:
	// (port) Four players: one cursor slot each. _A always points at slot 0;
	// _B points at slot 1, or at slot 2/3 while player C/D is being drawn.
	int		kurxposok_tar[4][MAXECSETSOR];
	darab*  curdarabok_tar[4][MAXECSETSOR];
	int*	kurxposok_A;  // Eppen hol all mutatohoz tartozo x
	darab** curdarabok_A; // Melyik darab kurrens
	int*	kurxposok_B;
	darab** curdarabok_B;
	int		generacio;    // changes whenever the rows (and so the cursors) are rebuilt
	int		slotgen[4];   // generation each slot's cursors belong to
	void	kurzorinit( void );
	void	slotvalaszt( int jatekos ); // _B -> this player's slot (1..3)

	void getorigoandsize( void );
	void addszakasz( vonal* psz );
	// Rovid folddarabokat atalakitja konkret mutatova:
	void foldmutatocsere( void );
	// Egymas utani azonos maszkokat es ureseket lecsereli egy hosszabbra:
	void duplaeliminacio( void );
	void textura2mutato( void );
	void mutatotlanit( void );

	// Ptop kerek-jeinek egesz koordinatait kitolti:
	void kitoltfoodkoordokat( void );

	void levon10000t( void );
	mdarab* beepitegymadarabnyit( mdarab* pmd, unsigned char* ujpixelek,
				  int x1, int ujx1, int ujx2,
				  int ujtavolsag, mdarab* vege, int* pujavege, int fazis );
	void addbytesor( unsigned char* ujpixelek, int ujtavolsag,
								int ujx1, int ujx2, int y, int fazis );
	void addegymaszk( sprite* psp, int index_t, int index_m, int fazis );
	void addspriteok( int fazis );
	void addobjektumok( void ); // Ez most nem view eseten visszter

	void kovetotextura( koveto* pkov, int index, int xo, int yo, int fazis );
	void kiegykovetokep( koveto* pkov, int index, int xo, int yo, int fazis );
	void kikovetokepek( koveto* pkov, int* ytomb, int hossz, int x0, int fazis );
	void addkovetok( int fazis );

	void kiegysor_A( unsigned char* pc, int ye );
	void kiegysor_B( unsigned char* pc, int ye );
	void kiegysor_FF( unsigned char* pc, int ye ); // fekete-feher

	void foltoz( void );

	ecset( int view ); // Ptop es Pszak alapjan
	ecset( ecset* pold ); // Ures sortomboket hoz benne letre
	~ecset( void );
public:
	void getbalalso_int( vekt2 balalso, int* px, int* py );
	// Beallitja Kisordarabszam-ot is ha define-olt (escet.cpp eleje):
	// (port) jatekos: player index 0..3 (was: 1 = player A, 0 = player B)
	void kitesz(     int jatekos, pic8* ppic, vekt2 balalso, int x1, int y1, int x2, int y2 );
	void kiteszview( int jatekos, pic8* ppic, vekt2 balalso, int x1, int y1, int x2, int y2 );
	void kitesz_ab(     int ajatekos, pic8* ppic, vekt2 balalso, int x1, int y1, int x2, int y2 );
	void kiteszview_ab( int ajatekos, pic8* ppic, vekt2 balalso, int x1, int y1, int x2, int y2 );

	//void egetkeszit( void );

	// (port) GPU drawing: the level as square pieces ("csempe") for textures.
	// A rectangle of the level picture, written to dest (stride bytes per
	// row). Row 0 of dest is ecset row y0 (rows go up, like the game's
	// screen buffer). Sky, empty and outside pixels get 'atlatszo'; ground is
	// the LGR ground picture; atkodol (if not NULL) changes the colours of
	// the pictures. kurzor/kurzorx: the caller's own row cursors (sorszam
	// entries each, set up by csempekurzor), so two threads can do this at
	// the same time:
	void csempe( unsigned char* dest, int stride, int x0, int y0, int w, int h,
				 unsigned char atlatszo, const unsigned char* atkodol,
				 darab** kurzor, int* kurzorx );
	void csempekurzor( darab** kurzor, int* kurzorx ); // sets cursors to the row starts
	int getsorszam( void ) { return sorszam; }
	int getmaxx( void ) { return maxx; }
	// Adds 1 to hasznalt[c] for every picture pixel of colour c:
	void szinhasznalat( unsigned int* hasznalt );
private:
	int*  sorkezdx;      // (port) x of each row's first piece
	darab* nagydarabvege; // (port) end of nagydarabtomb
public:
};

extern ecset* Pecsetalso;
extern ecset* Pecsetfelso;
extern ecset* Pecsetview;

extern int Kisordarabszam;

// Ket kis utility fuggveny (hasznalja oket LGR.CPP):
// Visszaadja hany pixel lyuk szinu x-tol kezdve:
int uresszam( int x, int xsize, unsigned char* sor, unsigned char lyuk );
// Visszaadja hany pixel nem lyuk szinu x-tol kezdve:
int teliszam( int x, int xsize, unsigned char* sor, unsigned char lyuk );

// betoltecseteket ide osszegzi osszes lefoglalt szakaszt:
extern int Osszegszam;


