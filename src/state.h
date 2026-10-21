
#define MAXJATEKOS (50)
#define JATEKOSNEVHOSSZ (14)

#define MAXPALYASZAM (90) // Ennyit soha nem er el

extern int Palyaszam; // Ez fugg attol, hogy shareware, vagy regisztralt

struct jatekos {
	char nev[JATEKOSNEVHOSSZ+2];
	char skippedtomb[((MAXPALYASZAM/4)+1)*4];
	long sikerespalyakszama;
	long jelenlegipalya; // -1 eseten external files
};

int getjatekosindex( char* nev );

#define MAXIDOK (10)

typedef char jatekosnev_type[JATEKOSNEVHOSSZ+1];

struct palyaegyfeleidok {
	long idokszama;
	long idok[MAXIDOK];
	jatekosnev_type nevek1[MAXIDOK];
	jatekosnev_type nevek2[MAXIDOK];
};

struct palyaidok {
	palyaegyfeleidok singleidok, multiidok;
};

// (port) Best times of a 3 or 4 player game (stored in state34.dat):
struct palya34idok {
	long idokszama;
	long idok[MAXIDOK];
	jatekosnev_type nevek[4][MAXIDOK]; // players A..D of each entry
};

// (port) 3/4 player best times of an external level, found by its file name
// and its checksum (like the times stored in the .lev file, they belong to
// that exact version of the level):
struct kulso34idok {
	char nev[24];
	long belyeg;
	palya34idok idok3, idok4;
};

struct jatekosopciok {
	int billgaz, billfek, billugras1, billugras2, billfordul;
	int billview, billtime, billshowkep;
};

class state_s {
public:
	palyaidok palyakidejei[MAXPALYASZAM];
	jatekos jatekosok[MAXJATEKOS];
	long jatekosokszama;
	jatekosnev_type jatekosa, jatekosb;

	// Opciok:
	int soundon;
	int secondarybuffer;
	// Ehelyett Single valtozot kell figyelni, mivel rec visszajatszasnal
	// lehet hogy mas kell legyen mint state-ben levo:
	int    single;
	// Itt pedig Tag valtozot kell figyelni:
	int	   tag;
	int    minibike; // Ezt most meg csak nem is mentjuk
	int    aplayerabike;
	int	   highquality;

	int	   animatedobjects;
	int	   animatedmenus;

	// Billentyuk:
	jatekosopciok opciok1;
	jatekosopciok opciok2;
	int billplussz, billminusz;
	int billsnap;

	char editfilenev[20];
	char playextfilenev[20];

	// (port) 3 and 4 player support. Stored in state34.dat, so state.dat keeps
	// the original format:
	int jatekosszam;                     // players: 1..4 (single == (jatekosszam == 1))
	jatekosnev_type jatekosc, jatekosd;  // profiles of players C and D
	int billzoomin, billzoomout;         // zoom while playing (state34.dat v3)
	double zoom;                         // current zoom, 1.0 = original (v4)
	double kepmeret;                     // screen size (Inc. / Dec. Screen Size), 1.0 = full (v5)
	int reszint;                         // split screen gap: 0 none, 1 thin, 2 original (v5)
	jatekosopciok opciok3;               // controls of players C and D
	jatekosopciok opciok4;
	palya34idok palyak3[MAXPALYASZAM];   // internal levels, 3 players
	palya34idok palyak4[MAXPALYASZAM];   // internal levels, 4 players
	kulso34idok* kulso34;                // external levels (kulso34szam of them)
	int kulso34szam, kulso34hely;
	// Internal level 0.., n = 3 or 4:
	palya34idok* idok34( int level, int n );
	// External level; NULL if it has no times yet and letrehoz is 0:
	palya34idok* kulsoidok34( const char* filenev, long belyeg, int n, int letrehoz );
	char* jatekosnev( int index );       // 0..3 -> jatekosa..jatekosd
	jatekosopciok* opciok( int index );  // 0..3 -> opciok1..opciok4
	void load34( void );
	void save34( void );

	state_s( char* nev = NULL );
	~state_s( void );

	void reloadidok( void );

	void save( void );
	void exportjatekossum( FILE* h, char* nev, int single );
	void exportanonim( FILE* h, int single, char* text1, char* text2,
															char* text3 );
	void exportstate( void );
	void resetcontrols( void );
};

extern state_s* State; // teljes.cpp-ben van inicializalva!


