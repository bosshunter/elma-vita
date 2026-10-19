/* compat.h - small shims so the original Watcom/Win32-era sources build with GCC/clang
 * (VitaSDK arm-vita-eabi-gcc).  Part of the PS Vita port of the Elasto Mania source
 * (see LICENSE.md: CC BY-NC-SA 4.0 + additional terms). */
#ifndef ELMA_COMPAT_H
#define ELMA_COMPAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Watcom / MSVC runtime names used by the game */
#define stricmp   strcasecmp
#define strcmpi   strcasecmp
#define _stricmp  strcasecmp
#define strnicmp  strncasecmp
#define _strnicmp strncasecmp
#define _fstrnicmp strncasecmp

char* itoa( int value, char* str, int radix );
char* strupr( char* s );
char* strlwr( char* s );
long  filelength( int fd );

/* The game builds DOS paths such as "lev\\name.lev"; these accept both
 * separators (the Vita filesystem needs '/'): */
FILE* port_fopen( const char* nev, const char* mod );
int   port_access( const char* nev, int mod );
/* Text files come from Windows (CR LF); Windows' text mode dropped the CR: */
char* port_fgets( char* s, int n, FILE* h );

#ifdef __cplusplus
}
#endif

/* 'access' mode constants are the same (0 == F_OK) */
#define fopen  port_fopen
#define access port_access
#define fgets  port_fgets

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#endif
