// io.h

#ifndef IO_H_INCLUDED
#define IO_H_INCLUDED

#include <windows.h>

#define _O_RDONLY       0x0010 
#define _O_WRONLY       0x0001 
#define _O_RDWR         0x0002 
#define _O_APPEND       0x0008 

#define _O_CREAT        0x0100 
#define _O_TRUNC        0x0200 
#define _O_EXCL         0x0400 
#define _O_TEMPORARY	0x0800

#define _O_TEXT         0x4000 
#define _O_BINARY       0x8000 

#define _S_IREAD        0x400  
#define _S_IWRITE       0x200  

#ifndef SEEK_CUR
#define SEEK_CUR 		0x10
#define SEEK_END 		0x20
#define SEEK_SET 		0x40
#endif

DWORD   open(  const char *filename, int oflag , int pmode);
int     close( DWORD handle );
int     read(  DWORD handle, void *buffer, unsigned int count );
int     write( DWORD handle, const void *buffer, unsigned int count );
long    lseek( DWORD handle, long offset, int origin );
long    tell(  DWORD handle );

DWORD   _get_std_handle( DWORD th );
DWORD   _get_last_error( void ); 
DWORD   _create_tmp_file( char * name ); 
DWORD   _get_temp_file_name( char * lpPathName,  char * lpPrefixString,
								 unsigned int uUnique,char * lpTempFileName );
int     _delete_file( const char * path );
int     _move_file( const char * lpExistingFileName,const char * lpNewFileName );

#endif
