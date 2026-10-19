#include	"all.h"

// Ha ez define-olva van, akkor titkositani fog.
// (Igy kell eloallitani titkosito exe-t.)
// (Billint 0 kell hogy legyen.)
//#define TITKOSITREADME

static unsigned char Readmesor[21] = "87oq3tfy378gfh783sdj";

// Ez mindig meghivodik teljesbol, csak ki van define-olva:
#ifndef W_95
void titkositreadme( void ) {
#ifdef TITKOSITREADME
	#ifdef W_95
		Windows-os verzioba nem szabad define-olni
	#endif
	if( Billint )
		hiba( "titkositreadme eseten nem lehet Billint!" );
	FILE* h = fopen( "readme.txt", "rb" );
	if( !h )
		hiba( "Nincsen readme.txt!" );

	FILE* hout = fopen( "kiment.leb", "wb" );
	if( !hout )
		hiba( "9gp89hg4tvupg" );

	int hossz = 0;
	if( fwrite( &hossz, 1, 4, hout ) != 4 )
		hiba( "678tf87tr" );

	int szamlalo = 0;
	while( 1 ) {
		unsigned char c;
		if( fread( &c, 1, 1, h ) != 1 )
			break; // befejezodott:
		// Kodol:
		c = c ^ Readmesor[szamlalo];
		szamlalo++;
		if( szamlalo >= 20 )
			szamlalo = 0;
		if( fwrite( &c, 1, 1, hout ) != 1 )
			hiba( "uiwfy78ytf8" );
		hossz++;
	}

	fseek( hout, 0, SEEK_SET );
	if( fwrite( &hossz, 1, 4, hout ) != 4 )
		hiba( "978gfy78" );

	fclose( h );
	fclose( hout );

	hiba( "Kiirta kiment.leb-et" );
#endif
}
#endif

static void beirnevet( FILE* h ) {
	char nev[18];

	nev[2] = 'a';
	nev[4] = 'a';
	nev[0] = 'G';
	nev[7] = 'M';
	nev[13] = 'l';
	nev[14] = 'l';
	nev[1] = 'r';
	nev[5] = 'm';
	nev[6] = ' ';
	nev[3] = 'h';
	nev[8] = 'i';
	nev[11] = 'h';
	nev[12] = 'e';
	nev[9] = 't';
	nev[10] = 'c';

	char c = ',';
	fwrite( &c, 1, 1, h );
	c = ' ';
	fwrite( &c, 1, 1, h );
	for( int i = 0; i < 15; i++ )
		fwrite( nev+i, 1, 1, h );

	// Toroljuk nevet:
	strcpy( nev, "                " );
}

static int Readmementve = 0;

void kimentreadme( void ) {
	if( Readmementve )
		return;
	Readmementve = 1;

	// Itt h es hout felcserelodik:

	// Megnyitjuk file-okat:
	char innev[12];
	innev[2] = 'm';
	innev[4] = 'n';
	innev[6] = '.';
	innev[0] = 'k';
	innev[10] = 0;
	innev[3] = 'e';
	innev[8] = 'e';
	innev[1] = 'i';
	innev[7] = 'l';
	innev[5] = 't';
	innev[9] = 'b';
	FILE* h = qopen( innev, "rb" );
	strcpy( innev, "aaaaaaaaaa" );
	if( !h )
		hiba( "Hiba 20!" );

	char outnev[12];
	outnev[2] = 'a';
	outnev[4] = 'm';
	outnev[3] = 'd';
	outnev[6] = '.';
	outnev[8] = 'x';
	outnev[7] = 't';
	outnev[9] = 't';
	outnev[10] = 0;
	outnev[5] = 'e';
	outnev[0] = 'r';
	outnev[1] = 'e';
	FILE* hout = fopen( outnev, "wb" );
	strcpy( outnev, "aaaaaaaaaa" );
	if( !hout )
		uzenet( "Maybe writeprotected files 1!" );

	// Atkonvertalas:
	int hossz = 0;
	if( fread( &hossz, 1, 4, h ) != 4 )
		hiba( "Hiba 21" );

	int szamlalo = 0;
	int igaziszamlalo = 0;
	for( int i = 0; i < hossz; i++ ) {
		unsigned char c;

		int szamzero = 14624;
		if( Sarvari )
			szamzero = 16198;
		if( igaziszamlalo == szamzero ) {
			c = 'a'; // beirjuk uj nevet
			beirnevet( hout );
		}
		if( igaziszamlalo == szamzero+33 ) {
			c = 0xd; // sort emelunk
			fwrite( &c, 1, 1, hout );
			c = 0xa;
			fwrite( &c, 1, 1, hout );
		}

		if( fread( &c, 1, 1, h ) != 1 )
			hiba( "Hiba 22" );
		// Kodol:
		c = c ^ Readmesor[szamlalo];

		int verziotjavit = 0;
		if( igaziszamlalo == 228 ) {
			c = '1'; // beirjuk verzio a betujet
			verziotjavit = 1;
		}

		szamlalo++;
		if( szamlalo >= 20 )
			szamlalo = 0;
		igaziszamlalo++;
		if( fwrite( &c, 1, 1, hout ) != 1 )
			uzenet( "Maybe writeprotected files 2!" );
		if( verziotjavit )
			fwrite( &c, 1, 1, hout );
	}

	qclose( h );
	fclose( hout );
}

