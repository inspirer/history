// stdio.h

#ifndef STDIO_H_INCLUDED
#define STDIO_H_INCLUDED

#include <stddef.h>		
#include <stdarg.h> 
#include <windows.h>

#define _F_READ		 0x0001 
#define _F_WRITE	 0x0002
#define _F_BINARY	 0x0004
#define _F_MOVEP	 0x0008
#define _F_BUF		 0x0010
#define _F_BUFSYS	 0x0020
#define _F_B_READ    0x0040
#define _F_B_WRITE   0x0080
#define _F_B_MUSTNEX 0x0100
#define _F_APPEND	 0x0200 
#define _F_B_NOTDEF  0x0400
#define _F_TMPFILE   0x0800
#define _F_ERROR	 0x1000
#define _F_EOF		 0x2000

#define BUFSIZ 1024
#define FILENAME_MAX 1024
#define FOPEN_MAX 100

#ifndef _IOFBF
#define _IOFBF 1 
#define _IOLBF 2
#define _IONBF 4
#endif


#ifndef SEEK_CUR

#define SEEK_CUR 0x10 
#define SEEK_END 0x20
#define SEEK_SET 0x40

#endif
 
#ifndef EOF
#define EOF (-1)
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef fpos_t
#define fpos_t long
#endif

typedef struct {
    DWORD file_descr;
    int flag;
    int b_first;
    int b_last;
    int bufsiz;
    char *buf;
} FILE;

#ifndef fpos_t
#define fpos_t long
#endif

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

extern FILE *files[FOPEN_MAX];
extern int FILE_num;   
    
FILE *fopen ( const char *name,const char *mode );

int fprintf(FILE  *stream,const char *format,... );
int printf( const char *format,... );
int sprintf( char *buffer, const char *format ,... );
int vfprintf( FILE *stream, const char *format, va_list mark );

int setvbuf(FILE *stream,char * buf,int mode,size_t size);
void setbuf( FILE *stream, char *buffer );
int fflush (FILE *stream) ;
int feof( FILE *stream );


int fgetpos( FILE *stream, fpos_t *pos );
int fsetpos( FILE *stream, fpos_t *pos );
int fseek( FILE *stream, long offset, int origin );
long ftell( FILE *stream );

size_t fread( void *buffer, size_t size, size_t count, FILE *stream );
size_t fwrite( const void *buffer, size_t size, size_t count, FILE *stream ); 


int fputs( const char *string, FILE *stream );
int puts(const char *s);
char *fgets( char *string, int n, FILE *stream );
int fgetc(FILE *f);
int fputc(int ch,FILE *stream);
int putc(int ch);

int  fclose(FILE *);
int fflush (FILE *stream);
 
int scanf( const char *format ,... );
int fscanf( FILE *stream, const char *format , ... );
int vsprintf(const char *buffer, const char *format, va_list argptr );
int sscanf( const char *buffer, const char *format, ... );

int feof( FILE *stream );
void clearerr( FILE *stream );
void rewind( FILE *stream );
int remove( const char *path );
int rename( const char *oldname, const char *newname );

char *tmpnam( char *string );
FILE *tmpfile( void );

void perror( const char *string );
int ferror( FILE *stream );

int ungetc( int c, FILE *stream );
 
#define getc(f) \
	(fgetc((f)))
#define getchar() \
	(fgetc(stdin))
#define putchar(ch) \
	(putc((ch)))

#endif
