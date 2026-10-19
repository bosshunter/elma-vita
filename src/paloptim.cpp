#include	"all.h"

#ifdef W_95
ezt nem szabad vegleges verzioba beforditani
#endif

static pic8* newkulsopcx( char* nev ) {
	FILE* h = fopen( nev, "rb" );
	if( !h )
		uzenet( "Could not open PCX file!:", nev );

	pic8* ppic = new pic8( nev, h );

	fclose( h );

	return ppic;
}

static void kulsopcxtopal( char* nev, unsigned char* pal ) {
	// Paletta beolvasasa:
	FILE* h = fopen( nev, "rb" );
	if( !h )
		uzenet( "Could not open PCX file!:", nev );
	long l = -769;
	if( fseek( h, l, SEEK_END ) != 0 )
		uzenet( "Error 769 in PCX file!: ", nev );
	char c;
	l = fread( &c, 1, 1, h );
	if( l != 1 )
		uzenet( "Cannot read from file!:", nev );
	if( c != 0x0c )
		uzenet( "Palette error 0x0C in PCX file!: ", nev );
	if( fread( pal, 768, 1, h ) != 1 )
		uzenet( "Cannot read from file!: ", nev );
	for( int i = 0; i < 768; i++ )
		pal[i] = (unsigned char)(pal[i] >> 2);
	fclose( h );
}

static void gettgasize( char* nev, int* pxsize, int* pysize ) {
	FILE* h = fopen( nev, "rb" );
	if( !h )
		uzenet( "Cannot open TGA file!", nev );

	fseek( h, 12, SEEK_SET );
	unsigned short size = 0;
	if( fread( &size, 1, 2, h ) != 2 )
		uzenet( "Could not read from TGA file!", nev );
	*pxsize = size;
	if( fread( &size, 1, 2, h ) != 2 )
		uzenet( "Could not read from TGA file!", nev );
	*pysize = size;

	fclose( h );
}

void kulonexe_szetpcx( void ) {
	if( Billint ) {
		printf( "Billint-et 0-ra kell allitani!\n" );
		exit( 0 );
	}

	unsigned char* pal = new unsigned char[1000];
	kulsopcxtopal( "tmp.pcx", pal );

	pic8* pnagy = newkulsopcx( "tmp.pcx" );
	int xnsize = pnagy->getxsize();
	int ynsize = pnagy->getysize();

	// Beolvassa szovegesen pictures.lst file-t,
	piclist* ppiclist = new piclist;
	if( !ppiclist )
		uzenet( "Not enough memory!" );

	int szamuk = ppiclist->kepszam + Allandokepszam;
	if( szamuk >= MAXPICLISTKEPSZAM )
		uzenet( "Too many pcx files in PICTURES.LST file!" );
	int futoxn = 0, futoyn = 0;
	for( int i = 0; i < szamuk; i++ ) {
		char nev[20];
		if( i < Allandokepszam )
			strcpy( nev, Allandotomb[i] );
		else {
			int index = i-Allandokepszam;
			if( ppiclist->tipusok[index] == PLTIP_MINTA )
				continue;
			strcpy( nev, &ppiclist->nevek[index*10] );
		}

		strcat( nev, ".tga" );
		int xsize, ysize;
		gettgasize( nev, &xsize, &ysize );
		pic8* ppic = new pic8( xsize, ysize );
		for( int y = 0; y < ysize; y++ ) {
			for( int x = 0; x < xsize; x++ ) {
				unsigned char szin = pnagy->gpixel( futoxn, futoyn );
				ppic->ppixel( x, y, szin );

				futoxn++;
				if( futoxn >= xnsize ) {
					futoxn = 0;
					futoyn++;
				}
			}
		}
		*strchr( nev, '.' ) = 0;
		strcat( nev, ".pcx" );
		ppic->save( nev, pal );
		delete ppic;
	}

	char tmp[80];
	sprintf( tmp, "%d PCX files are written!\n", szamuk );
	uzenet( tmp );
}

