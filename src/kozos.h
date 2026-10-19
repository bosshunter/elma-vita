

extern double K_pi;
extern double K_pip2;
extern double K_2pi;

#ifdef W_95
#define _fstrnicmp _strnicmp
#endif

typedef char finame[20];
int fifirst( char* minta, char* filenev );
int finext( char* filenev );
void ficlose( void );
int fialmappak( const char* konyvtar, char nevek[][16], int maxdb ); // (port) subfolders
extern char Levalmappa[100]; // (port) current subfolder of lev: "" or "dir\" or "dir\sub\"
void levutvonal( char* dest, int meret, const char* nev ); // (port) builds lev\<subfolder><name>
