// stdlib.h

#ifndef STDLIB_H_INCLUDED
#define STDLIB_H_INCLUDED

#include <stddef.h>

#define _DEBUG_memoryleaks

extern int errno;
extern char * _sys_errlist[10];

void * malloc(size_t);
void * realloc(void *, size_t);
void free(void *);

long strtol( const char *nptr, const char **endptr, int base );
double strtod( const char *nptr, const char **endptr );
double atof( const char *string );
int atol(const char *string);
int atoi( const char *string );
int abs( int a );

#ifdef _DEBUG_memoryleaks
void _ShowMemoryLeaks();
#endif

#endif
