// io.c

#include <io.h>
#include <windows.h>

DWORD _io_error;

DWORD open( const char *filename, int oflag , int pmode)
{
	DWORD dwDesiredAccess, dwCreationDisposition, dwFlagsAndAttributes ;
	HANDLE h;
	if (oflag & _O_RDONLY)  
		dwDesiredAccess  =  GENERIC_READ;
	else {
		if (oflag & _O_WRONLY) dwDesiredAccess = GENERIC_WRITE;
		else if (oflag & _O_RDWR) dwDesiredAccess = GENERIC_WRITE | GENERIC_READ; 
	};
	if (oflag & _O_CREAT)	 
		dwCreationDisposition = (oflag & _O_TRUNC) ? CREATE_ALWAYS : OPEN_ALWAYS;
	else dwCreationDisposition = OPEN_EXISTING;
	
	dwFlagsAndAttributes = (oflag & _O_TEMPORARY) ? FILE_ATTRIBUTE_TEMPORARY :
		 FILE_ATTRIBUTE_NORMAL;
		
	h = apicall(&CreateFileA,28,filename,dwDesiredAccess,((pmode & 1) ?
	FILE_SHARE_READ : 0) | ((pmode & 2) ? FILE_SHARE_WRITE : 0),
		NULL, dwCreationDisposition,dwFlagsAndAttributes,0);
	
	if (h == INVALID_HANDLE_VALUE) {
		_io_error = apicall(&GetLastError,0);
		h = 0;
	} else _io_error = 0; 
	
	return (DWORD) h;
}

int read( DWORD handle, void *buffer, unsigned int count )
{
	DWORD done;
	if (!apicall(&ReadFile,20,(HANDLE)handle,buffer,count,&done,NULL)){
		_io_error = apicall(&GetLastError,0);
		done = 0xffffffff;
	} else _io_error = 0;
	return done;	
}

int write( DWORD handle, const void *buffer, unsigned int count )
{
	DWORD written;
	if ( !apicall(&WriteFile,20,(HANDLE)handle,buffer,count,&written,NULL)){
		_io_error = apicall(&GetLastError,0);
		written = 0;
	} else _io_error = 0;
	return written;
}

int close( DWORD handle )
{
	if (!apicall(&CloseHandle,4,(HANDLE)handle)) {
		_io_error = apicall(&GetLastError,0);
		return -1;
	};
	_io_error=0;
	return 0;
}

long tell( DWORD handle )
{
	return (long)apicall(&SetFilePointer,16,(HANDLE)handle,0,NULL,FILE_CURRENT);
}

long lseek( DWORD handle, long offset, int origin )
{
	DWORD p;
	p = (origin & SEEK_SET) ? FILE_BEGIN : ((origin & SEEK_CUR) ? FILE_CURRENT : FILE_END);
	p = apicall(&SetFilePointer,16,(HANDLE)handle,offset,NULL,p);
	if( p == 0xFFFFFFFF){
		_io_error = apicall(&GetLastError,0);
		p = (unsigned int)NULL;
	}
	return p;
}


DWORD _get_std_handle(DWORD hType)
{
	if (hType == 0) return (DWORD)apicall(&GetStdHandle,4,STD_INPUT_HANDLE);
	if (hType == 1) return (DWORD)apicall(&GetStdHandle,4,STD_OUTPUT_HANDLE);
	if (hType == 2) return (DWORD)apicall(&GetStdHandle,4,STD_ERROR_HANDLE);     
	return 0;
}

int _delete_file(const char * path) 
{
	return apicall(&DeleteFileA,4,path);
}

int _move_file(const char * lpExistingFileName, const char * lpNewFileName)
{
	return apicall(&MoveFileA,8,lpExistingFileName,lpNewFileName);
}


DWORD _get_last_error(void) 
{
	return apicall(&GetLastError,0);
}

DWORD _get_temp_file_name(char * lpPathName,  char * lpPrefixString,
		unsigned int uUnique,char * lpTempFileName) 
{                                                       
	return apicall(&GetTempFileNameA,16,lpPathName,lpPrefixString,uUnique,lpTempFileName);                   
}

DWORD _create_tmp_file(char * name)
{
	return (DWORD)apicall(&CreateFileA,28,name,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_NEW,
		FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE,NULL);
}

