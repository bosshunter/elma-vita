#include "compat.h"
#include <unistd.h>
char* itoa( int value, char* str, int radix ) {
	if( radix == 16 ) sprintf( str, "%x", (unsigned)value );
	else if( radix == 8 ) sprintf( str, "%o", (unsigned)value );
	else sprintf( str, "%d", value );
	return str;
}
char* strupr( char* s ) { char* p = s; for( ; *p; p++ ) *p = (char)toupper( (unsigned char)*p ); return s; }
char* strlwr( char* s ) { char* p = s; for( ; *p; p++ ) *p = (char)tolower( (unsigned char)*p ); return s; }
long filelength( int fd ) {
	struct stat st;
	if( fstat( fd, &st ) != 0 ) return -1;
	return (long)st.st_size;
}

/* ---- DOS-style path separators ---- */
#undef fopen
#undef access
static void utvonal( char* dest, const char* sour, size_t meret ) {
	size_t i = 0;
	for( ; sour[i] && i < meret-1; i++ )
		dest[i] = (sour[i] == '\\') ? '/' : sour[i];
	dest[i] = 0;
}
FILE* port_fopen( const char* nev, const char* mod ) {
	char tmp[512];
	utvonal( tmp, nev, sizeof( tmp ) );
	// (port) The Windows text mode flag 't' ("wt", "rt") is not accepted by
	// the Vita's C library: fopen( "stats.txt", "wt" ) failed, which is what
	// showed the error that crashed the game on exit. Drop it:
	char m[8];
	size_t k = 0;
	for( size_t i = 0; mod[i] && k < sizeof( m )-1; i++ )
		if( mod[i] != 't' )
			m[k++] = mod[i];
	m[k] = 0;
	return fopen( tmp, m );
}
int port_access( const char* nev, int mod ) {
	char tmp[512];
	utvonal( tmp, nev, sizeof( tmp ) );
	return access( tmp, mod );
}
#undef fgets
char* port_fgets( char* s, int n, FILE* h ) {
	char* r = fgets( s, n, h );
	if( r ) {
		size_t l = strlen( s );
		if( l >= 2 && s[l-2] == '\r' && s[l-1] == '\n' ) {
			s[l-2] = '\n';
			s[l-1] = 0;
		}
		else if( l >= 1 && s[l-1] == '\r' )
			s[l-1] = 0;
	}
	return r;
}
