/*   il.h
 *
 *   C Compiler project (cc)
 *   Copyright (C) 2002-03  Eugeniy Gryaznov (gryaznov@front.ru)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *   
 *   This file describes three global structures: expr, Node and Init
 */

#ifndef il_h_included
#define il_h_included

#define print printf

#define sizeop(n) ((n)<<10)
#define generic(op)  ((op)&0x3F0)
#define specific(op) ((op)&0x3FF)
#define opindex(op) (((op)>>4)&0x3F)
#define opkind(op)  ((op)&~0x3F0)
#define opsize(op)  ((op)>>10)
#define optype(op)  ((op)&0xF)

#include "il_ops.h"

struct TypeMetrics {
	unsigned char size, align, outofline;
};


struct BackendOptions {

	TypeMetrics _met[t_basiccnt];
	TypeMetrics _ptr, _struct;

	unsigned little_endian:1;
	unsigned mulops_calls:1;
	unsigned wants_callb:1;
	unsigned wants_argb:1;
	unsigned left_to_right:1;
	unsigned wants_dag:1;
	unsigned unsigned_char:1;
};


class Backend {
public:
	Compiler *cc;
	BackendOptions o;
	int need_swap;

	virtual void defaddress( Sym )=0;
	virtual void defconst( int suffix, int size, Value v )=0;
	virtual void defstring( int n, char *s )=0;
	virtual void defsymbol( Sym )=0;

	virtual void global( Sym )=0;
	virtual void segment( char* )=0;
	virtual void space( int )=0;

	virtual void emit( Node )=0;
};

#endif
