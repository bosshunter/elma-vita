
#ifdef __cplusplus
#define CFUGGVENY extern "C"
#else
#define CFUGGVENY
#endif

// (port) Registered version, not shareware (needs the registered elma.res):
//#define SARVARI  (port) not used any more: shareware or registered is decided
//                 at start-up from elma.res (see initqopen in qopen.cpp)
extern int Sarvari;

//#define BUTITOTT

extern int Billint; // Billentyuzet interraptos!

extern int Sajatugras; // Ha 1, akkor left->pozitiv forgas
extern int Otszoros; // Ha igaz, mindig otszor rakja ki palyat
extern int Dupladuplabuffer; // Ha igaz duplan huzza az idot (Win95)

// Ha nem igaz, csak memoriaba ir es nem flip-pel:
extern int Waitforretrace; // Csak Windows-os w_v.cpp-ben allitodik

#define BYTECOPY(a,b,c) memcpy( a, b, c )

//void BYTECOPY( void* dest, void* source, int size );

#define BYTESET(a,b,c) memset( a, b, c )

void semmi( void );

void inkremental( void );
extern long	Inkremens;

