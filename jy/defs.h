// defs.h

//  This file is part of the Linker project.
//  AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

//#define DEBUG_expr
//#define DEBUG_parse
//#define DEBUG_object

#define BUFSIZE 4096
#define MAXTOKEN 1024

#define E_WARN  1
#define E_ERROR 2
#define E_FATAL 3

typedef unsigned char uchar;
typedef unsigned long ulong;

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define CLASS_SEQ(name) struct name##Seq { name *first,*last; };

#endif
