// stddef.h

#ifndef STDDEF_H_INCLUDED
#define STDDEF_H_INCLUDED

#define offsetof(s,m) (size_t)&(((s *)0)->m)

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef _SIZE_T
typedef unsigned long size_t;
#define _SIZE_T
#endif 						  

#ifndef _PTRDIFF_T
typedef int ptrdiff_t;
#define _PTRDIFF_T
#endif


#ifndef _WCHAR_T
typedef unsigned short wchar_t;
#define _WCHAR_T
#endif 

#endif
