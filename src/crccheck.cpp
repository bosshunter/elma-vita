#include	"all.h"

Ez a file most kimarad forditasbol

static int Beallitas = 0;
static unsigned long Crcszam = 0x05b839e45l;

static int Crchiba = 0; // 1 eseten elma.exe, 2 eseten readme.txt valtozott,
						// 3 eseten kommand line nem elma-val fejezodik
static int Crcellenorizve = 0;

enum { FILE_IS_MISSING = 10, FILE_HAS_CHANGED, FILE_NAME_CHANGED, ELMA_EXE, README_TXT };

void check_kommand( const char* komline_ ) {
	if( Beallitas )
		return;

	char sor[210];
	strncpy( sor, komline_, 200 );
	sor[200] = 0;
	if( strchr( sor, ' ' ) )
		*strchr( sor, ' ' ) = 0;

	// Ha a vegen idezojel van:
	while( strlen(sor) > 4 &&
		   sor[strlen(sor)-1] == '\"' )
		   sor[strlen(sor)-1] = 0;

	if( strlen( sor ) < 10 )
		strcpy( sor, "aaaaaaaaaaaaaaa" );

	const char* nevmutato = &sor[strlen( sor ) - 10];

	

	/*FILE* h = fopen( "nev.txt", "wt" );
	if( !h )
		hiba( "uigruovvruvrvriruvr" );
	fprintf( h, "Nev: <%s>", nevmutato );
	fclose( h ); */

	char tmp[12];
	tmp[6] = 'e';
	tmp[7] = 0;
	tmp[3] = '.';
	tmp[8] = 0;
	tmp[0] = 'a';
	tmp[2] = 'c'; //
	tmp[10] = 0;
	tmp[5] = 'x'; //
	tmp[1] = 's'; //
	tmp[9] = 0;
	tmp[4] = 'e'; //

	if( strcmpi( nevmutato, tmp ) == 0 ) {
		for( int i = 0; i < 11; i++ )
			tmp[i] = ' ';
		return;
	}
	for( int i = 0; i < 11; i++ )
		tmp[i] = ' ';
	// Nem stimmel kommand line nev:
	// Most nem azt irjuk, hogy nev valtozott,hanem hogy 
	// elma.exe file valtozott:
	Crchiba = 1; 
	Crcellenorizve = 1;
}

static char* titstring( int azonosito, int masodikbuff = 0 ) {
	static char titbuffer1[100];
	static char titbuffer2[100];
	static int titbufferi[100];
	titbufferi[99] = 0;
	switch( azonosito ) {
		case FILE_IS_MISSING:
			titbufferi[0] = 'F';
			titbufferi[1] = 'i';
			titbufferi[2] = 'l';
			titbufferi[3] = 'e';
			titbufferi[4] = ' ';
			titbufferi[5] = 'i';
			titbufferi[6] = 's';
			titbufferi[7] = ' ';
			titbufferi[8] = 'm';
			titbufferi[9] = 'i';
			titbufferi[10] = 's';
			titbufferi[11] = 's';
			titbufferi[12] = 'i';
			titbufferi[13] = 'n';
			titbufferi[14] = 'g';
			titbufferi[15] = '!';
			titbufferi[16] = ':';
			titbufferi[17] = ' ';
			titbufferi[18] = 0;
			break;
		case FILE_HAS_CHANGED:
			titbufferi[0] = 'F';
			titbufferi[1] = 'i';
			titbufferi[2] = 'l';
			titbufferi[3] = 'e';
			titbufferi[4] = ' ';
			titbufferi[5] = 'h';
			titbufferi[6] = 'a';
			titbufferi[7] = 's';
			titbufferi[8] = ' ';
			titbufferi[9] = 'c';
			titbufferi[10] = 'h';
			titbufferi[11] = 'a';
			titbufferi[12] = 'n';
			titbufferi[13] = 'g';
			titbufferi[14] = 'e';
			titbufferi[15] = 'd';
			titbufferi[16] = '!';
			titbufferi[17] = ':';
			titbufferi[18] = ' ';
			titbufferi[19] = 0;
			break;
		case FILE_NAME_CHANGED:
			titbufferi[0] = 'F';
			titbufferi[1] = 'i';
			titbufferi[2] = 'l';
			titbufferi[3] = 'e';
			titbufferi[4] = ' ';
			titbufferi[5] = 'n';
			titbufferi[6] = 'a';
			titbufferi[7] = 'm';
			titbufferi[8] = 'e';
			titbufferi[9] = ' ';
			titbufferi[10] = 'c';
			titbufferi[11] = 'h';
			titbufferi[12] = 'a';
			titbufferi[13] = 'n';
			titbufferi[14] = 'g';
			titbufferi[15] = 'e';
			titbufferi[16] = 'd';
			titbufferi[17] = '!';
			titbufferi[18] = ':';
			titbufferi[19] = ' ';
			titbufferi[20] = 0;
			break;
		case ELMA_EXE:
			titbufferi[0] = 'e';
			titbufferi[1] = 'l';
			titbufferi[2] = 'm';
			titbufferi[3] = 'a';
			titbufferi[4] = '.';
			titbufferi[5] = 'e';
			titbufferi[6] = 'x';
			titbufferi[7] = 'e';
			titbufferi[8] = '0';
			titbufferi[9] = '0';
			titbufferi[10] = 0;
			break;
		case README_TXT:
			titbufferi[0] = 'r';
			titbufferi[1] = 'e';
			titbufferi[2] = 'a';
			titbufferi[3] = 'd';
			titbufferi[4] = 'm';
			titbufferi[5] = 'e';
			titbufferi[6] = '.';
			titbufferi[7] = 't';
			titbufferi[8] = 'x';
			titbufferi[9] = 't';
			titbufferi[10] = 0;
			break;
		default:
			hiba( "titstring-ben azonosito default!" );
	}
	for( int i = 0; i < 100; i++ ) {
		if( masodikbuff ) {
			titbuffer1[i] = (char)titbufferi[i];
			if( !titbuffer1[i] )
				return titbuffer1;
		}
		else {
			titbuffer2[i] = (char)titbufferi[i];
			if( !titbuffer2[i] )
				return titbuffer2;
		}
	}
	hiba( "Bug475876!" );
	return titbuffer1;
}

// Valodi file-t nez:
static unsigned long getcrc( char* nev_p, long ellenorzohossz ) {
	char nev[40];
	strcpy( nev, nev_p );
	FILE* h = fopen( nev, "rb" );
	if( !h )
		return 0;
	long hossz = filelength( fileno( h ) );
	if( hossz != ellenorzohossz && !Beallitas ) {
		//uzenet( "File size has changed!: ", nev );
		Crchiba = 1;
		return 0;
	}

	long number = (hossz-3)/4;

	unsigned long sum = Crcszam;
	for( long l = 0; l < number; l++ ) {
		unsigned long ujszam;
		if( fread( &ujszam, 1, 4, h ) != 4 )
			return 0;
		sum ^= ujszam ^ Crcszam;
		if( sum > 0x7fffffffl )
			sum = (sum << 1) + 1;
		else
			sum = (sum << 1);
	}

	fclose( h );

	if( Beallitas ) {
		FILE* h2 = fopen( "biztlist.txt", "at" );
		if( !h2 )
			hiba( "!h2!" );
		fprintf( h2, "%ld\n%lu\n", hossz, sum );
		fclose( h2 );
	}

	return sum;
}

static unsigned long loadulong( FILE* h ) {
	unsigned long ertek = 0xfffffffful;
	char sor[200];
	if( fgets( sor, 190, h ) == NULL )
		hiba( "Nem tudott olvasni sort filebol!" );
	if( sscanf( sor, "%lu", &ertek ) != 1 )
		hiba( "loadulong-ban nem tudta feldolgozni sort!: ", sor );
	if( ertek == 0xfffffffful )
		hiba( "loadulong-ban ertek == 0xfffffffful!: ", sor );
	return ertek;
}

void crccheck2( void ) {
     if( Crchiba ) {
		if( Crchiba == 1 )
			uzenet( titstring( FILE_HAS_CHANGED ), titstring( ELMA_EXE, 1 ) );
		if( Crchiba == 2 )
			uzenet( titstring( FILE_HAS_CHANGED ), titstring( README_TXT, 1 ) );
		if( Crchiba == 3 )
			uzenet( titstring( FILE_NAME_CHANGED ), titstring( ELMA_EXE, 1 ) );
	}
}

void crccheck( void ) {
	return; // Most sosem ellenoriz!			
	// Csak neha megy bele fuggvenybe:
	if( !Beallitas ) {
		/*unsigned short egesz, tort;
		getbiostort( &egesz, &tort );
		if( tort%120 )
			return;*/
		int ido = (int)mv_stopperido();
		if( ido%120 )
			return;
	}

	if( Crcellenorizve )
		return;
	Crcellenorizve = 1;

	long hossz_elma_exe = 0, hossz_readme_txt = 0;
	unsigned long crc_elma_exe = 0, crc_readme_txt = 0;
	if( !Beallitas ) {
		// (port) shareware or registered, decided at start-up:
		FILE* h = qopen( Sarvari ? "bizton_s.txt" : "bizton_r.txt", "rt" );
		if( !h )
			hiba( "Nem tudta megnyitni bizton_?.txt file-t!" );
		hossz_elma_exe = loadulong( h );
		crc_elma_exe = loadulong( h );

		hossz_readme_txt = loadulong( h );
		crc_readme_txt = loadulong( h );

		qclose( h );
	}

	/* Most nem ellenorizzuk exe file-t:
	if( getcrc( titstring( ELMA_EXE ), hossz_elma_exe ) != crc_elma_exe &&
							!Beallitas ) {
		Crchiba = 1;
		return;
	}
	if( Crchiba ) {
		Crchiba = 1;
		return;
	} */

	if( getcrc( titstring( README_TXT ), hossz_readme_txt ) != crc_readme_txt &&
							!Beallitas ) {
		Crchiba = 2;
		return;
	}
	if( Crchiba ) {
		Crchiba = 2;
		return;
	}

	if( Beallitas )
		hiba( "Kiirta kodokat!" );
}


