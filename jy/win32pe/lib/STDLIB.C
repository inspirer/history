// stdlib.c

#include <stdlib.h>
#include <windows.h>

int errno;

char * _sys_errlist[10] = {
	"No error has occured\n",	// 0 
	"Bad file number\n",     	// 1
	"Not enough memory\n",   	// 2
	"Permission denied\n",   	// 3
	"Undefined fault\n",	 	// 4
	"File exist\n",			 	// 5
	"Invalid argument\n",	    // 6
	"No such file\n",           // 7
	"Too many open files\n",    // 8
	"No such file or directory\n" // 9
};

#define MEM_BASE 0x10000000

static DWORD _allocated_size, _alloc_size, _last_block, _allocated_base;

int _heap_change_size(int size)
{
	DWORD e = _allocated_base + _allocated_size;
	size = ((size+_alloc_size-1)/_alloc_size)*_alloc_size;
    while( size > _allocated_size ){
    	if( !apicall(&VirtualAlloc,16,e,_alloc_size,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE))
        	break;
        e += _alloc_size; _allocated_size += _alloc_size;
    }
    while( size < _allocated_size ){
    	e -= _alloc_size; _allocated_size -= _alloc_size;
    	if( !apicall(&VirtualFree,12,e,_alloc_size,MEM_RELEASE,PAGE_READWRITE))
        	break;
    }
    return (size == _allocated_size);
}

void * malloc(size_t size)
{
	DWORD i, c, d, b = _last_block;

    if( !size ) return NULL;

    size = ((size + 7) >> 3) << 3;
	restart: while( b ){
    	i = *(DWORD *)b;
    	if( ( i & 1) == 0 && i >= size ){
        	if( i-size < 16 ) size = i;
			if( i == size )
				*(DWORD *)b |= 1;
			else {
            	d = b+i+8;
				c = b+size+8;
            	i -= 8 + size;
				*(DWORD *)b = size + 1;
                *(DWORD *)c = i;
                *((DWORD *)c+1) = b;
                if( b == _last_block)
                	_last_block = c;
                else
                	*((DWORD *)d+1) = c;
            }
            return (PVOID)(b+8);
        }
		b = *((DWORD *)b + 1);
    }

    if( _last_block ){
	    i = *(DWORD *)_last_block;
	    if( i & 1){
	    	if( !_heap_change_size(_allocated_size+size)) return NULL;
	        i -= 1;
			b = _last_block+i+8;
	    	*((DWORD *)b+1) = _last_block;
	    	*(DWORD *)b = _allocated_size + _allocated_base - b - 8;
	        _last_block = b;
	    } else {
	    	if( !_heap_change_size(_allocated_size+size-i)) return NULL;
	    	*(DWORD *)_last_block = _allocated_size + _allocated_base - _last_block - 8;
		}
    } else {
    	_allocated_base = apicall(&VirtualAlloc,16,(PVOID)MEM_BASE,_alloc_size,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    	if( !_allocated_base ){
    		apicall(&MessageBoxA,16,0,"memory allocation failed","fatal",0);
    		apicall(&ExitProcess,4,0);
    	}
    	_allocated_size = _alloc_size;
    	*((DWORD *)_allocated_base+1) = 0;
    	*(DWORD *)_allocated_base = _alloc_size-8;
    	_last_block = _allocated_base;
    }
    b = _last_block;
    goto restart;
}

void * realloc(void *block, size_t size)
{
	DWORD p, q, i, e;
    char *s,*d;

    size = ((size + 7) >> 3) << 3;
	if( size == 0 ){
		if( block ) free(block);
    	return NULL;
    }
    if( !block) return malloc(size);

    p = (DWORD)block - 8;
    if( p >= _allocated_base && p < _allocated_base + _allocated_size && *(DWORD*)p & 1 ){
    	i = *(DWORD*)p - 1;

        if( size > i ){
	        if( p == _last_block ){
		    	if( !_heap_change_size(_allocated_size+size-i)) return NULL;
				q = p+i+8;
		    	*((DWORD *)q+1) = _last_block;
		    	*(DWORD *)q = _allocated_size + _allocated_base - q - 8;
		        _last_block = q;
            }
            q = p+i+8;
            if( (*(DWORD*)q & 1) == 0 && (*(DWORD*)q+8 >= size-i || q == _last_block) ){
            	i = size-i;
            	if( q == _last_block && *(DWORD*)q+8 < i ){
			    	if( !_heap_change_size(_allocated_size+i-*(DWORD*)q-8)) return NULL;
		    		*(DWORD *)q = _allocated_size + _allocated_base - q - 8;
                }
                if( *(DWORD*)q-8<i ){
                    *(DWORD*)p += *(DWORD*)q+8;
                    if( q == _last_block )
                    	_last_block = p;
                    else {
                    	q += *(DWORD*)q+8;
                        *((DWORD*)q+1) = p;
                    }
                } else {
                	*(DWORD*)p += i;
                    e = *(DWORD*)q-i;
                    if( q == _last_block ){
                    	q += i;
                        _last_block = q;
						*((DWORD*)q+1) = p;
                        *(DWORD*)q = e;
                    } else {
                    	q += i;
						*((DWORD*)q+1) = p;
                        *(DWORD*)q = e;
                    	p = q + *(DWORD*)q+8;
                        *((DWORD*)p+1) = q;
                    }
            	}
				return block;
            }
			d = (char *)malloc(size);
            if( !d ) return NULL;
            s = block; p = (DWORD)d;
            for( i=size; i; i--) *d++=*s++;
            free(block);
            return (PVOID)p;

        } else if( i-size >= 16 ){
        	q = p+i+8;
            if( p == _last_block ) q = 0;
            *(DWORD*)p = size+1;
            e = p+size+8;
            *((DWORD*)e+1) = p;
            p = e;
            i = i-size-8;
            if( q && (*(DWORD*)q & 1) == 0 ){
            	i += *(DWORD*)q+8;
            	*(DWORD*)p = i;
                if( q == _last_block )
                	_last_block = p;
                else {
                	e = q+*(DWORD*)q+8;
                    *((DWORD*)e+1)=p;
                }
            } else {
            	*(DWORD*)p = i;
                if( q ) *((DWORD*)q+1) = p;
                	else _last_block = p;
            }
        	
            return block;
        }
        return block;
    }
    return NULL;
}

void free(void *block)
{
	DWORD p = (DWORD)block - 8, q, i;
    if( p >= _allocated_base && p < _allocated_base + _allocated_size && *(DWORD*)p & 1 ){
    	*(DWORD*)p &= ~1;
        if( p != _last_block){
        	i = *(DWORD*)p + 8;
        	q = p + i;
            if( (*(DWORD*)q & 1) == 0 ){
            	i += *(DWORD*)q;
				*(DWORD*)p = i;
                if( q == _last_block)
                	_last_block = p;
                else {
                	q = p + i + 8;
                	*((DWORD *)q+1) = p;
                }
			}
        }
        q = p;
        if( (p=*((DWORD*)q+1)) ){
        	i = *(DWORD*)p;
            if( (i&1) == 0 ){
            	i += *(DWORD*)q + 8;
				*(DWORD*)p = i;
                if( q == _last_block)
                	_last_block = p;
                else {
                	q = p + i + 8;
                	*((DWORD *)q+1) = p;
                }
        	}
        }
        if( (*(DWORD*)_last_block & 1) == 0){
        	i = *(DWORD*)_last_block;
            if( i>=_alloc_size-8 ){
            	p = *((DWORD*)_last_block+1);
            	_heap_change_size(_last_block-_allocated_base);
                q = _allocated_base + _allocated_size;
                if( q-_last_block < 16){
					_last_block = p;
                }
	   			if( _last_block ) *(DWORD*)_last_block = q - _last_block - 8;
            }
        }
    }
}

void _init_memory_allocation()
{
	SYSTEM_INFO inf;
	apicall(&GetSystemInfo,4,&inf);
    _alloc_size = inf.dwAllocationGranularity;
    _last_block = 0;
    _allocated_size = 0;
  	asm "section .systeminit { call __init_memory_allocation; }";
}

#ifdef _DEBUG_memoryleaks

#include <io.h>
#include <string.h>

static char hex[]="0123456789abcdef";
void _print_hex_number(int handle,DWORD n)
{
	char s[11];
    int i = 9;
    s[10]=0;
    while(i>1){ s[i--]=hex[n%16];n>>=4; }
    if( i == 9 ) s[i--]='0';
    i++;write(handle, s+i,10-i);
}
void _print_string(int handle,char * string)
{
	write(handle,string,strlen(string));
}

void _ShowMemoryLeaks()
{
	int i;
    DWORD q,e,old;

    i = open("leaks",_O_CREAT|_O_TRUNC|_O_WRONLY,0);
    
    _print_string(i,"Memory size: ");
    _print_hex_number(i,_allocated_size);
    _print_string(i,"\n\n");
    q = _allocated_base;
    while(q < _allocated_base + _allocated_size){
    	_print_hex_number(i,q);_print_string(i,":    ");
		e = *(DWORD*)q;
        if( e&1 )
			_print_string(i,"busy");
        else 
        	_print_string(i,"free");
        e &= ~1;

        _print_string(i,"    ");
        _print_hex_number(i,e);
        if( q == _last_block )
        	_print_string(i,"  last");
        _print_string(i,"\n");
        old = q;
    	q += e+8;
        if( q < _allocated_base + _allocated_size && *((DWORD*)q+1) != old ) _print_string(i,"error ");
    }
    close( i );
}

#endif

double _pow_ten(int k);

long strtol( const char *nptr, const char **endptr, int base )
{
	int i,j;
	long a = 0;
	char c = 0;

	for (; (*nptr == ' ' || *nptr == '\t'); nptr++) ;
	if (*nptr == '-') {
		i = -1;
		nptr++;
	} else {
		i = 1;
		if (*nptr == '+') nptr++;
	}
	
	if ( base < 2 || base > 36 ) {
	if (*nptr == '0') {
			nptr++;
			if (*nptr == 'x' || *nptr == 'X') {				
				nptr++;
				base = 16;
			} else base = 8;
		} else base = 10;
	}

	if (base <= 10) {
		for (; (*nptr >= '0' && *nptr <= '0' + base - 1); nptr++) 
		   a = a*base + *nptr - '0'; 
	} else {
		for (; ( (*nptr >= '0' && *nptr <= '9') || 
				 ((j = (c == 'a' || c == 'A')) && *nptr >= c && *nptr <= (c + base - 11)) ||
				 (!j && ((*nptr >= 'a' && *nptr <= 'z' + base - 11 && (c = 'a')) || 
							(*nptr >= 'A' && *nptr <= 'Z' + base - 11 && (c = 'A')))));	nptr++) 
		   if (*nptr >= '0' && *nptr <= '9') a = a*base + *nptr - '0';
		   else a = a*base + *nptr - c + 11;
	}
	if (endptr) *endptr = nptr;
	a *= i;
	return a;					
}

double strtod( const char *nptr,const char **endptr )
{
	int i,k;
	double d;

	for (; (*nptr == ' ' || *nptr == '\t'); nptr++) ;	   	   	   	
	if (*nptr == '-') {
		i = -1;
		nptr++;
	} else {
		if (*nptr == '+') nptr++;
		i = 1;
	}
	
	for (d = 0; (*nptr >= '0' && *nptr <= '9'); nptr++) {
		d = d*10 + *nptr - '0';
	}
	
	if (*nptr == '.') {
		nptr++;
		for (k = -1; (*nptr >= '0' && *nptr <= '9'); nptr++, k--)
			d += (*nptr - '0') * _pow_ten(k);
	}
	d *= i;
	if (*nptr == 'e' || *nptr == 'E' || *nptr == 'd' || *nptr == 'D') {
		nptr++;
		if (*nptr == '-') {
			i = -1;
			nptr++;
		} else {
			if (*nptr == '+') nptr++;
			i = 1;
		}
		for (k = 0; (*nptr >= '0' && *nptr <= '9'); nptr++) 
			k = k*10 + *nptr - '0';	
		k *= i;
		d = d * _pow_ten(k);
	}
	if (endptr) *endptr = nptr;
	return d;	
}

double atof( const char *string )
{
	return strtod(string,NULL);
}

int atol(const char *string)
{
	return strtol(string,NULL,10);
}

int atoi( const char *string )
{
	return atol(string);
}

int abs( int a )
{
	return (a < 0) ? -a : a;
}
