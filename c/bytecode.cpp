/*   bytecode.cpp
 *   Default backend
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
 */

#include "cc.h"


class bytecode_backend : public Backend { 

	void defaddress( Sym );
	void defconst( int suffix, int size, Value v );
	void defstring( int n, char *s );
	void defsymbol( Sym );

	void global( Sym );
	void segment( char* );
	void space( int );

	void emit( Node );
};

char *stringf(const char *fmt, ...) 
{
	char buf[1024];
	va_list ap;

	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	return _strdup(buf);
}


// /////////////////////////////////////

void bytecode_backend::segment( char *name ) {
	print( "segment %s\n", name );
}

void bytecode_backend::global( Sym p ) {
	print( "align %d\n", p->type->align );
	print( "%s:\n", p->x.name );
}

void bytecode_backend::space( int n ) {
	print("skip %d\n", n);
}

void bytecode_backend::defaddress( Sym p ) {
	print("dd %s\n", p->x.name);
}

void bytecode_backend::defconst( int suffix, int size, Value v ) {
	switch (suffix) {
	case I:
		/*if (size > sizeof (int))
			print("byte %d %D\n", size, v.i);
		else
			print("byte %d %d\n", size, v.i);
		return;*/
		TODO();
	case U:
		/*if (size > sizeof (unsigned))
			print("byte %d %U\n", size, v.u);
		else
			print("byte %d %u\n", size, v.u);
		return;*/
		TODO();
	case P: 
		print( "dd 0x%08x\n", (unsigned long)v.p ); 
		return;
	case F:
		if (size == 4) {
			float f = v.d;
			print("dd %u\n", *(unsigned *)&f);
		} else {
			double d = v.d;
			unsigned *p = (unsigned *)&d;
			//print("dd %u, %u\n", p[swap], p[1 - swap]);
		}
		return;
	}
	ASSERT(0);
}

void bytecode_backend::defstring( int len, char *str ) {
	char *s;

	for (s = str; s < str + len; s++)
		print("db %d\n", (*s)&0377);
}

void bytecode_backend::defsymbol( Sym p ) {
	if( p->storage == scs_imm )
		switch( optype( p->type->asOP() )) {
		case I: SYM(p)->x.name = stringf("%D", p->loc.val.i); break;
		case U: SYM(p)->x.name = stringf("%U", p->loc.val.u); break;
		case P: SYM(p)->x.name = stringf("%U", p->loc.val.p); break;
		default: assert(0);
		}
	/*else if( p->storage == LOCAL && p->storage == STATIC)
		p->x.name = stringf( "$%d", genlabel(1) );
	else if( p->scope == LABELS || p->generated )
		p->x.name = stringf( "$%s", p->name );*/
	else
		SYM(p)->x.name = p->name;
}

static void dumptree( Node p ) {
/*	switch (specific(p->op)) {
	case ASGN+B:
		assert(p->kids[0]);
		assert(p->kids[1]);
		assert(p->syms[0]);
		dumptree(p->kids[0]);
		dumptree(p->kids[1]);
		print("%s %d\n", opname(p->op), p->syms[0]->u.c.v.u);
		return;
	case RET+V:
		assert(!p->kids[0]);
		assert(!p->kids[1]);
		print("%s\n", opname(p->op));
		return;
	}
	switch (generic(p->op)) {
	case CNST: case ADDRG: case ADDRF: case ADDRL: case LABEL:
		assert(!p->kids[0]);
		assert(!p->kids[1]);
		assert(p->syms[0] && p->syms[0]->x.name);
		print("%s %s\n", opname(p->op), p->syms[0]->x.name);
		return;
	case CVF: case CVI: case CVP: case CVU:
		assert(p->kids[0]);
		assert(!p->kids[1]);
		assert(p->syms[0]);
		dumptree(p->kids[0]);
		print("%s %d\n", opname(p->op), p->syms[0]->u.c.v.i);
		return;
	case ARG: case BCOM: case NEG: case INDIR: case JUMP: case RET:
		assert(p->kids[0]);
		assert(!p->kids[1]);
		dumptree(p->kids[0]);
		print("%s\n", opname(p->op));
		return;
	case CALL:
		assert(p->kids[0]);
		assert(!p->kids[1]);
		assert(optype(p->op) != B);
		dumptree(p->kids[0]);
		print("%s\n", opname(p->op));
		return;
	case ASGN: case BOR: case BAND: case BXOR: case RSH: case LSH:
	case ADD: case SUB: case DIV: case MUL: case MOD:
		assert(p->kids[0]);
		assert(p->kids[1]);
		dumptree(p->kids[0]);
		dumptree(p->kids[1]);
		print("%s\n", opname(p->op));
		return;
	case EQ: case NE: case GT: case GE: case LE: case LT:
		assert(p->kids[0]);
		assert(p->kids[1]);
		assert(p->syms[0]);
		assert(p->syms[0]->x.name);
		dumptree(p->kids[0]);
		dumptree(p->kids[1]);
		print("%s %s\n", opname(p->op), p->syms[0]->x.name);
		return;
	}*/
	assert(0);
}

void bytecode_backend::emit( Node p ) {
	for (; p; p = p->link)
		dumptree(p);
}


BackendOptions bytecode_opts = {

   {{ 0, 0, 0 },
	{ 0, 0, 0 },	//1 void
	{ 1, 1, 0 },	//2 char
	{ 1, 1, 0 },	//3 signed char
	{ 1, 1, 0 },	//4 unsigned char 
	{ 2, 2, 0 },	//5 short, signed short, short int, or signed short int
	{ 2, 2, 0 },	//6 unsigned short, or unsigned short int
	{ 4, 4, 0 },	//7 int, signed, or signed int
	{ 4, 4, 0 },	//8 unsigned, or unsigned int
	{ 4, 4, 0 },	//9 long, signed long, long int, or signed long int
	{ 4, 4, 0 },	//0 unsigned long, or unsigned long int
	{ 8, 8, 0 },	//1 long long, signed long long, long long int, or signed long long int
	{ 8, 8, 0 },	//2 unsigned long long, or unsigned long long int
	{ 4, 4, 1 },	//3 float
	{ 8, 8, 1 },	//4 double
	{ 8, 8, 1 },	//5 long double
	{ 1, 1, 0 },	//6 _Bool
	{ 8, 8, 1 },	//7 float _Complex
	{16,16, 1 },	//8 double _Complex
	{16,16, 1 },	//9 long double _Complex
	{ 8, 8, 1 },	//0 float _Imaginary
	{16,16, 1 },	//1 double _Imaginary
	{16,16, 1 }},	//2 long double _Imaginary

	{ 4, 4, 0 },	/* T* */
	{ 0, 4, 0 },	/* struct */
	0,		/* little_endian */
	0,		/* mulops_calls */
	0,		/* wants_callb */
	0,		/* wants_argb */
	1,		/* left_to_right */
	0,		/* wants_dag */
	0		/* unsigned_char */
};


Backend *get_bytecode_be()
{
	bytecode_backend *be = new bytecode_backend();
	be->o = bytecode_opts;
	return be;
}
