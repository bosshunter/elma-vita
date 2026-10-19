#include	"all.h"

double K_pi = 3.141592;
double K_pip2 = K_pi*0.5;
double K_2pi = K_pi*2.0;

void k_memcpy( void *dest, const void *src, size_t n ) {
	//inkremental();
	memcpy( dest, src, n );
}

static int  Fiben = 0;

char Levalmappa[100] = ""; // (port)

void levutvonal( char* dest, int meret, const char* nev ) { // (port)
    snprintf( dest, meret, "lev\\%s%s", Levalmappa, nev );
}

#if 1 /* (port) POSIX implementation: wildcard directory listing with 8.3-style name filter */

#include <dirent.h>
#include <sys/stat.h>

static DIR* Fidir = NULL;
static char Fipattern[100];

// Case-insensitive DOS-style wildcard match ('*' and '?'):
static int wildmatch( const char* p, const char* s ) {
	while( *p ) {
		if( *p == '*' ) {
			while( *p == '*' )
				p++;
			if( !*p )
				return 1;
			while( *s ) {
				if( wildmatch( p, s ) )
					return 1;
				s++;
			}
			return 0;
		}
		if( !*s )
			return 0;
		if( *p != '?' && tolower( (unsigned char)*p ) != tolower( (unsigned char)*s ) )
			return 0;
		p++;
		s++;
	}
	return *s == 0;
}

// Next entry that matches the pattern and has a DOS-style (8.3) name; 0 = found
static int finextmatch( char* filenev ) {
	filenev[0] = 0;
	struct dirent* de;
	while( (de = readdir( Fidir )) != NULL ) {
		const char* name = de->d_name;
		if( strlen( name ) > 12 )
			continue; // Tul hosszu nev
		char tmpnev[20];
		strcpy( tmpnev, name );
		if( strstr( tmpnev, "." ) )
			*strstr( tmpnev, "." ) = 0;
		if( strlen( tmpnev ) > 8 )
			continue; // Tul hosszu nev
		if( name[0] == '.' )
			continue;
		if( !wildmatch( Fipattern, name ) )
			continue;
		strcpy( filenev, name );
		return 0;
	}
	return 1;
}

int fifirst( char* minta, char* filenev ) {
	if( Fiben || Fidir )
		hiba( "fifirst" );
	Fiben = 1;
	filenev[0] = 0;

	// Split "dir/pattern" (both '/' and '\\' accepted):
	char dir[200];
	strncpy( dir, minta, 199 );
	dir[199] = 0;
	char* slash = NULL;
	for( char* p = dir; *p; p++ ) {
		if( *p == '\\' )
			*p = '/';
		if( *p == '/' )
			slash = p;
	}
	if( slash ) {
		*slash = 0;
		strncpy( Fipattern, slash+1, 99 );
		Fipattern[99] = 0;
		if( !dir[0] )
			strcpy( dir, "/" );
	}
	else {
		strncpy( Fipattern, dir, 99 );
		Fipattern[99] = 0;
		strcpy( dir, "." );
	}
	Fidir = opendir( dir );
	if( !Fidir )
		return 1;
	return finextmatch( filenev );
}

int finext( char* filenev ) {
	if( !Fiben || !Fidir )
		hiba( "8794t" );
	return finextmatch( filenev );
}

void ficlose( void ) {
	if( !Fiben )
		hiba( "908uy6" );
	Fiben = 0;
	if( Fidir ) {
		closedir( Fidir );
		Fidir = NULL;
	}
}

// (port) Lists the subfolders of a directory: names of at most 12 characters,
// no "." entries. Returns how many names were written to nevek[0..maxdb-1].
int fialmappak( const char* konyvtar, char nevek[][16], int maxdb ) {
	char dir[200];
	strncpy( dir, konyvtar, 199 );
	dir[199] = 0;
	for( char* p = dir; *p; p++ )
		if( *p == '\\' )
			*p = '/';
	size_t l = strlen( dir );
	while( l > 1 && dir[l-1] == '/' )
		dir[--l] = 0;

	DIR* d = opendir( dir );
	if( !d )
		return 0;
	int db = 0;
	struct dirent* de;
	while( db < maxdb && (de = readdir( d )) != NULL ) {
		const char* name = de->d_name;
		if( name[0] == '.' )
			continue;
		if( strlen( name ) > 12 )
			continue; // too long: the path buffers could not hold it
		char full[300];
		snprintf( full, sizeof( full ), "%s/%s", dir, name );
		struct stat st;
		if( stat( full, &st ) != 0 || !S_ISDIR( st.st_mode ) )
			continue;
		strcpy( nevek[db++], name );
	}
	closedir( d );
	return db;
}



#endif /* (port) POSIX implementation */
