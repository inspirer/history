// string.h

#ifndef STRING_H_INCLUDED
#define STRING_H_INCLUDED

#include <stddef.h>

char * strrev( char *string );

void * memcpy(void *s1, const void *s2, size_t n) ;
char * strcpy(char *s1, const char *s2) ;
char * strncpy( char *s1, const char *s2, size_t count ) ;
void * _memccpy( void *dest, const void *src, int c, unsigned int count );

char * strcat( char *s_destin, const char *s_source ) ;
char * strncat( char *strDest, const char *strSource, size_t count );

int    memcmp( const void *buf1, const void *buf2, size_t count )	;
int    strcmp( const char *s1, const char *s2 ) ;
int    strncmp( const char *s1, const char *s2, size_t count ) ;

void * memchr( const void *buf, int c, size_t count );
char * strchr( const char *s, int c );
char * strrchr( const char *string, int c );

void * memset( void *dest, int c, size_t count );
char * _strset( char *string, int c );

size_t strlen( const char *string );
int    _memicmp( const void *buf1, const void *buf2, unsigned int count )	;
void * memmove( void *dest, const void *src, size_t count );

size_t strcspn( const char *string, const char *strCharSet ) ;
char * strpbrk( const char *string, const char *strCharSet )  ;
size_t strspn( const char *string, const char *strCharSet );
char * strstr( const char *string, const char *strCharSet );
char * strtok( char *strToken, const char *strDelimit );

char * strerror( int errnum );

#endif
