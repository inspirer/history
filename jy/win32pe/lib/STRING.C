// string.c

#include <string.h>
#include <stddef.h>
#include <stdlib.h>

size_t strlen( const char *string );

void *memcpy(void *s1, const void *s2, size_t n) 
{
	unsigned int i=0;
	char *p1=(char *)s1;
	const char *p2=(char *)s2;
	for (;i<n; i++,p1++,p2++ ) *p1=*p2;
	return s1;
}

char *strcpy(char *s1, const char *s2)
{
	char *p1=s1;
	for (; *s2!=0; p1++,s2++) *p1=*s2;	  
	*p1=0;
	return s1;
}

char *strncpy( char *s1, const char *s2, size_t count )
{
	char *p1=s1;
	unsigned int i;
	for ( i=0; i<count && *s2!=0; i++, p1++, s2++) *p1=*s2;
	for (; i<count; i++, p1++) *p1=0;
	return s1;
}

char *strcat( char *s_destin, const char *s_source )
{
	char *p1=s_destin;
	p1+=strlen(p1);
	for (; *s_source!=0; p1++, s_source++) *p1=*s_source;
	*p1=0;
	return s_destin;
}

int memcmp( const void *buf1, const void *buf2, size_t count )
{
	unsigned int i;
	const char *b1=(char *)buf1,*b2=(char *) buf2;
    for (i=0; *b1==*b2 && i<count; b1++,b2++,i++ ) ;
	if (count==i) return 0;
	if (*b1>*b2) return 1;
	return -1;
}

int strcmp( const char *s1, const char *s2 )
{
	for (; *s1!=0 && *s1==*s2; s1++, s2++);
	if (*s1>*s2) return 1;
	if (*s2>*s1) return -1;
	return 0;
}

int strncmp( const char *s1, const char *s2, size_t count )
{
	unsigned int i;
	for (i=0; i<count&& *s1==*s2 && *s1!=0; s1++, s2++, i++) ;
	if ((count == i)|| ((*s1==*s1) &&(*s1==0))) return 0;
	if (*s1 > *s2) return 1;
	return -1;
}


void *memchr( const void *buf, int c, size_t count )
{
	unsigned int i;
	const char *b=(char *)buf;
	for (i=0; i<count && *b!=c; b++) ;
	if (count == i) return NULL;
	return (void *)b;
}

char *strchr( const char *s, int c )
{
	for (; *s!=0 && *s!=c; s++) ;
	if (*s == 0) if (c==0) return (char *)s; else return NULL;
	return (char *)s;
}

void *memset( void *dest, int c, size_t count )
{
	char *p=(char*) dest;
	char *end=(char*)dest+count;
	for (; p<end;p++) *p=(char)c;
	return dest;
}

char *_strset( char *string, int c )
{
	char *p=string;
	for (; *p!=0; p++) *p=(char)c;
	return string;
}

size_t strlen( const char *string )
{
	int l=0;
	if (!string) return 0;
	for (;*string!=0; string++,l++);
	return l;
}

void *_memccpy( void *dest, const void *src, int c, unsigned int count )
{
	char *p1=(char*)dest,*p2=(char*)src,*end=(char*)dest+count;	
	for (; *p2!=c && p2<end; p1++,p2++) *p1=*p2;
	if (p2==end) return NULL;
	*p1=(char)c;
	p1++;
	return (void *)p1;
}

int _memicmp( const void *buf1, const void *buf2, unsigned int count )
{
	const char *p1=(char *)buf1,*p2=(char *)buf2,*end=(char*)buf1+count;
	for (; p1<end;p1++) 
		if (*p1!=*p2){
			if (((*p1>'a') && (*p1<'z')) ||
				((*p1>'A') && (*p1<'Z'))){
				if (*p1>*p2) {
					if ((*p1-'a'+'A')>*p2) return 1;
					if ((*p1-'a'+'A')<*p2) return -1;
				} else {
					if (*p1>(*p2-'a'+'A')) return 1;
					if (*p1<(*p2-'a'+'A')) return -1;
				};
			} else return (*p1>*p2) ? 1: -1;
		};
   return 0;
}

void *memmove( void *dest, const void *src, size_t count )
{
	char  *p,*p1,*p2;
	const char  *p3,*end;
	if (((char*)dest+count<(char*)src) || ((char*)src+count<(char*)dest)) 
		return memcpy(dest,src,count);
	p1=p=(char *)malloc(count);	
	p3=(char *)src;
	end=p1+count;
	if (!p) return dest;
	for (; p1<end;p1++,p3++ ) *p1=*p3;
	p1=p;
	p2=(char *)dest;
	for (; p1<end; p1++,p2++) *p2=*p1;
	free(p);
	return dest;
}

char *strncat( char *strDest, const char *strSource, size_t count )
{
	char *p=strDest;
	const char *p1=strSource;
	const char*end=p1+count;
	for (; *p!=0;p++);
	for (;p1<end && *p1!=0; p1++,p++) *p=*p1;
	*p=0;
	return strDest;
}

size_t strcspn( const char *string, const char *strCharSet )
{
	const char *p;
	int i=0;
	for (; *string!=0; string++,i++) 
		for (p=strCharSet; *p!=0; p++) 
			if (*p==*string) return i;
	return i;
}

char *strpbrk( const char *string, const char *strCharSet )
{
	const char *p;
	for (; *string!=0; string++) 
		for (p=strCharSet; *p!=0; p++) 
			if (*p==*string) return (char *)string;
	return NULL;
}

char *strrchr( const char *string, int c )
{
	const char *p=string+strlen(string);
	string--;
	for (; p!=string;p--) if (*p==c) return (char *)p;
	return NULL;
}

size_t strspn( const char *string, const char *strCharSet )
{
	int i=0;
	for (; *string!=0 && (strchr(strCharSet,*string)); string++,i++);
	return i;
}

char *strstr( const char *string, const char *strCharSet )
{
	const char *p=string;
	if (strlen(strCharSet)==0) return (char *)string;
	while (p=strchr(p,*strCharSet))	
		if (!(strncmp(p,strCharSet,strlen(strCharSet))))
			return (char*)p;
    return NULL;
}

char *strtok( char *strToken, const char *strDelimit )
{
	int k;
	static char *p;
	if (strToken) p=strToken;
	while (*p!=0) {
		if (k=strcspn(p,strDelimit)) {
			*(p+k)=0;
			return p;
		};
		p++;
	};
	return 0;			
}

char *strerror( int errnum )
{
	return _sys_errlist[errnum];
}

char *strrev( char *string )
{
	int i,len,k;

	len = strlen(string);
	i = len/2;
	for (; i > 0; i--) {
		k = string[i];
		string[i] = string[len - i];
		string[len - i] = k;
	}
	return string;
}
