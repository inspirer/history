/*   types.cpp
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


Type *Type::create( int ts, Compiler *cc )
{
	Type *t = (Type *)cc->type_sl.allocate();
	t->specifier = ts;
	t->storage = scs_none;
	t->qualifier = tq_none;
	t->parent = NULL;
	t->size = 0;
	return t;
}


//
//  clones type structure
// 
Type *Type::clone( Compiler *cc )
{
	Type *t = (Type *)cc->type_sl.allocate();
	*t = *this;
	return t;
}


//
//  pushes string to buffer with defined size at pos (helper routine)
//  buffer grows down
//
static inline int _insert( char **buffer, int *left, char *str, int pos )
{
	int len = strlen( str );

	if( len > *left ) {
		str += len - *left;
		len = *left;
	}
	*left -= len;
	*buffer -= len;
	if( pos )
		memmove( *buffer, *buffer + len, pos );
	memmove( *buffer + pos, str, len );

	return len;
}

#define BEFORE(str) size += _insert( b, left, str, pos )
#define AFTER(str)  size += _insert( b, left, str, pos+size )

//
//  processes one sequence of types, calls itself on function types
//
static int toString_helper( Type *root, char **b, int *left, int pos ) {

	int size = 0;
	Type *t = root;

	for( ; t; t = t->parent ) {

		switch( t->specifier ) {
		case t_struct:
		case t_union:
		case t_enum:
			if( t->tagname )
				BEFORE( t->tagname );
			else 
				BEFORE( "__noname" );
			BEFORE( t->specifier == t_struct ? "struct " : t->specifier == t_union ? "union " : "enum " );
			break;
		case t_array: 
			if( t == root )
				BEFORE( "[]");
			else {
				BEFORE( "(" );
				AFTER( ")[]" );
			}
			break;
		case t_func:
			if( t == root )
				BEFORE( "()");
			else {
				BEFORE( "(" );
				AFTER( ")()" );
			}
			if( t->params )
				for( Sym *s = t->params; s; s = s->next ) {
					size += toString_helper( s->type, b, left, pos + size - 1 );
					if( s->next )
						size += _insert( b, left, ", ", pos+size-1 );
				}
			else
				size += _insert( b, left, "void", pos+size-1 );
			break;			
		case t_pointer:
			break; // pointer handling differs

		case t_void: 
			BEFORE( "void" ); break;
		case t_char: 
			BEFORE( "char" ); break;
		case t_schar: 
			BEFORE( "signed char" ); break;
		case t_uchar: 
			BEFORE( "unsigned char" ); break;
		case t_short: 
			BEFORE( "short" ); break;
		case t_ushort: 
			BEFORE( "unsigned short" ); break;
		case t_int: 
			BEFORE( "int" ); break;
		case t_uint: 
			BEFORE( "unsigned int" ); break;
		case t_long: 
			BEFORE( "long" ); break;
		case t_ulong: 
			BEFORE( "unsigned long" ); break;
		case t_llong: 
			BEFORE( "long long" ); break;
		case t_ullong: 
			BEFORE( "unsigned long long" ); break;
		case t_float: 
			BEFORE( "float" ); break;
		case t_double: 
			BEFORE( "double" ); break;
		case t_ldouble: 
			BEFORE( "long double" ); break;
		case t_bool: 
			BEFORE( "_Bool" ); break;
		case t_fcompl: 
			BEFORE( "float _Complex" ); break;
		case t_dcompl: 
			BEFORE( "double _Complex" ); break;
		case t_lcompl: 
			BEFORE( "long double _Complex" ); break;
		case t_fimg: 
			BEFORE( "float _Imaginary" ); break;
		case t_dimg: 
			BEFORE( "double _Imaginary" ); break;
		case t_limg: 
			BEFORE( "long double _Imaginary" ); break;
		default:
			BEFORE( "<unknown>" );
		}

		if( t->qualifier & tq_const )
			BEFORE( t->specifier == t_pointer ? " const" : "const " );
		if( t->qualifier & tq_volatile )
			BEFORE( t->specifier == t_pointer ? " volatile" : "volatile " );
		if( t->qualifier & tq_restrict )
			BEFORE( t->specifier == t_pointer ? " restrict" : "restrict " );

		if( t->specifier == t_pointer )
			BEFORE("*");

		// space is not set between child and parent types in the following cases
		//  - child is pointer/array/func and parent is a pointer without qualifiers
		//  - parent is either array or function
		if( t->parent && 
		!((t->specifier == t_pointer || t->specifier == t_array || t->specifier == t_func) 
			&& t->parent->specifier == t_pointer && t->parent->qualifier == 0 ) 
			&& t->parent->specifier != t_func && t->parent->specifier != t_array )
				BEFORE( " " );
	}

	return size;
}

//
//   This function uses two buffers, and is not re-entrant.
//
const char *Type::toString()
{
	enum { SIZE = 2048 };
	static int calltimes = 0;
	static char buff[2][SIZE];
	
	char *buffer; 
	int left;

	calltimes++;
	buffer = buff[calltimes&1]+SIZE-1;
	*buffer = 0;
	left = SIZE-1;

	toString_helper( this, &buffer, &left, 0 );
	return buffer;
}

// definition of basic type information array

#define FD(i,e) ((i<<4) + e)

const struct basic_type_descr tdescr[t_basiccnt] = {
 {	0,	0,      	0,       },	
 {	0,	0,			0,		 },	//1 void
 {	1,	2,			0,		 },	//2 char
 {	1,	2 | SBIT,	0,       },	//3 signed char
 {	1,	2,		 	0,		 },	//4 unsigned char 
 {	2,	3 | SBIT,	0,       },	//5 short, signed short, short int, or signed short int
 {	2,	3,		 	0,		 },	//6 unsigned short, or unsigned short int
 {	4,	4 | SBIT,	0,       },	//7 int, signed, or signed int
 {	4,	4,		 	0,		 },	//8 unsigned, or unsigned int
 {	4,	5 | SBIT,	0,       },	//9 long, signed long, long int, or signed long int
 {	4,	5,		 	0,		 },	//0 unsigned long, or unsigned long int
 {	8,	6 | SBIT,	0,       },	//1 long long, signed long long, long long int, or signed long long int
 {	8,	6,		 	0,		 },	//2 unsigned long long, or unsigned long long int
 {	4,	0,		 	FD(0,0), },	//3 float
 {	8,	0,		 	FD(0,1), },	//4 double
 {	8,	0,		 	FD(0,2), },	//5 long double
 {	1,	1,		 	0,		 },	//6 _Bool
 {	8,	0,		 	FD(2,0), },	//7 float _Complex
 {	16,	0,		 	FD(2,1), },	//8 double _Complex
 {	16,	0,		 	FD(2,2), },	//9 long double _Complex
 {	4,	0,		 	FD(1,0), },	//0 float _Imaginary
 {	8,	0,		 	FD(1,1), },	//1 double _Imaginary
 {	8,	0,		 	FD(1,2), },	//2 long double _Imaginary
};


                    
static void print_type( int ts, Compiler *cc )
{
	int wr = 1;
	
	if( ts & ts_void ) cc->error( " void" + wr ), wr = 0;
	if( ts & ts_signed ) cc->error( " signed" + wr ), wr = 0;
	if( ts & ts_unsigned ) cc->error( " unsigned" + wr ), wr = 0;
	if( ts & ts_char ) cc->error( " char" + wr ), wr = 0;
	if( ts & ts_short ) cc->error( " short" + wr ), wr = 0;
	if( ts & ts_long ) cc->error( " long" + wr ), wr = 0;
	if( ts & ts_llong ) cc->error( " long" + wr ), wr = 0;
	if( ts & ts_int ) cc->error( " int" + wr ), wr = 0;
	if( ts & ts_float ) cc->error( " float" + wr ), wr = 0;
	if( ts & ts_double ) cc->error( " double" + wr ), wr = 0;
	if( ts & ts__Bool ) cc->error( " _Bool" + wr ), wr = 0;
	if( ts & ts__Complex ) cc->error( " _Complex" + wr ), wr = 0;
	if( ts & ts__Imaginary ) cc->error( " _Imaginary" + wr ), wr = 0;
}


static int validate_type( int ts )
{
	switch( ts ) {
		case ts_void:
			return t_void;
		case ts_char:
			return t_char;
		case ts_signed|ts_char:
			return t_schar;
		case ts_unsigned|ts_char:
			return t_uchar;
		case ts_short:
		case ts_signed|ts_short:
		case ts_short|ts_int:
		case ts_signed|ts_short|ts_int:
			return t_short;
		case ts_unsigned|ts_short:
		case ts_unsigned|ts_short|ts_int:
			return t_ushort;
		case ts_int:
		case ts_signed:
		case ts_signed|ts_int:
			return t_int;
		case ts_unsigned:
		case ts_unsigned|ts_int:
			return t_uint;
		case ts_long:
		case ts_signed|ts_long:
		case ts_long|ts_int:
		case ts_signed|ts_long|ts_int:
			return t_long;
		case ts_unsigned|ts_long:
		case ts_unsigned|ts_long|ts_int:
			return t_ulong;
		case ts_long|ts_llong:
		case ts_signed|ts_long|ts_llong:
		case ts_long|ts_llong|ts_int:
		case ts_signed|ts_long|ts_llong|ts_int:
			return t_llong;
		case ts_unsigned|ts_long|ts_llong:
		case ts_unsigned|ts_long|ts_llong|ts_int:
			return t_ullong;
		case ts_float:
			return t_float;
		case ts_double:
			return t_double;
		case ts_long|ts_double:
			return t_ldouble;
		case ts__Bool:
			return t_bool;
		case ts_float|ts__Complex:
			return t_fcompl;
		case ts_double|ts__Complex:
			return t_dcompl;
		case ts_long|ts__Complex:
		case ts_long|ts_double|ts__Complex:
			return t_lcompl;
		case ts_float|ts__Imaginary:
			return t_fimg;
		case ts_double|ts__Imaginary:
			return t_dimg;
		case ts_long|ts__Imaginary:
		case ts_long|ts_double|ts__Imaginary:
			return t_limg;
	}
	return 0;
}


void Type::add( int ts, Place loc, Compiler *cc )
{
	if( ts == ts_long && (specifier & ts_long ) )
		ts = ts_llong;

	if( t_mask & specifier ) {
		cc->error( LOC "'", loc );
		switch( t_mask & specifier ) {
			case t_typename:
			case t_struct:
			case t_union:
			case t_enum:
				cc->error("type/struct/union/enum");
		}
		cc->error( "' followed by '" );
		print_type( ts, cc );
		cc->error( "' is illegal\n" );
		return;
	}

	if( (ts & specifier) || !validate_type(specifier|ts) ) {
		cc->error( LOC "'", loc );
		print_type( specifier, cc );
		cc->error( "' followed by '" );
		print_type( ts, cc );
		cc->error( "' is illegal\n" );
		return;
	}

	specifier |= ts;
}


void Type::add_modifier( int modifier, Place loc, Compiler *cc )
{
	switch( modifier & ~0xff ) {
		case mod_storage:
			if( storage == scs_none )
				storage = modifier & 0xff;
			else
				cc->error( LOC "more than one storage class specified\n", loc );
			break;
		case mod_func:
			cc->error( "not implemented yet: 'inline'\n" ); // TODO
 			break;
		case mod_qual:
			qualifier = addqualifier( qualifier, modifier & 0xff, loc, cc );
	}
}


void Type::fixup( Compiler *cc )
{
	if( (t_mask & specifier) == 0 ) {
		specifier = validate_type( specifier );
		ASSERT( BASICTYPE(this) );
		size = tdescr[specifier].size;
	}
}


int Type::addqualifier( int list, int one, Place loc, Compiler *cc )
{
	if( list & one )
		cc->error( LOC "warn: same type qualifier used more than once\n", loc );

	return list|one;
}


Type *Type::create_function( Sym *paramlist, Compiler *cc )
{
	Type *t = create( t_func, cc );
	t->params = paramlist;
	t->body = NULL;
	return t;
}

/*  
   DESC: creates Sym and Type. Registers structure in the outer namespace.
   type can be t_union or t_struct. Either name, or members parameter
   must not be NULL.

   mm is namespace of the structure, members contains ordered list of members
   (mm contains only members). Namespace is used for fast access, members to
   compare structures.

   EG: members == NULL means that structure has no definition

*/
Type *Type::create_struct( char *name, int type, Sym *members, Namespace *mm, Compiler *cc )
{
	Type *t = NULL;
	Sym  *s = NULL;

	// try to find out current structure declaration
	if( name ) {
		s = cc->current->search_id( name, type, 0 );
		if( s )
			t = s->type;
	}

	// create if not exist
	if( !t ) {
		t = Type::create( type, cc );
		t->params = NULL;
		t->members = NULL;
		t->tagname = name;
	}

	// add structure to the outer namespace
	if( !s && name ) {
		s = Sym::create( name, cc );
		s->type = t;
		s->ns_modifier = type;
		cc->current->add_item( s );
	}

	// TODO
	if( t->params && members ) {
	  	cc->error( " : error: comparing of two structures is not implemented\n" );

	// add member declarations
	} else if( members ) {
		t->params = members;
		t->members = mm;

		word size = 0;

		for( Sym *s = members; s; s = s->next ) {
			s->offset = type == t_struct ? size : 0;
			size += s->type->size;
		}

		t->size = size;
	}

	return t->clone( cc );
}


Type *Type::create_array( int tqualifier, Expr *expr, Compiler *cc ) 
{
	Type *t = create( t_array, cc );

	if( expr ) {
		t->ar_size = expr;
		t->qualifier = tqualifier;

	} else {
		t->ar_size = NULL;
	}

	return t;
}

/*
	DESC: returns the pointer to "unsigned int" imm type
	
	EG: the type is cached in the Compiler::basic[0] variable
*/
Type *Type::get_enum_type( Compiler *cc )
{
	if( cc->basic[0] == NULL ) {
		cc->basic[0] = create( t_uint, cc );
		cc->basic[0]->storage = scs_imm;
	}

	return cc->basic[0];
}


/*
	DESC: returns the pointer to basic type
	
	EG: the type is cached in the Compiler::basic[1..t_basiccnt] variable
*/
Type *Type::get_basic_type( int spec, Compiler *cc )
{
	ASSERT( spec > 0 && spec < t_basiccnt );

	if( cc->basic[spec] == NULL )
		cc->basic[spec] = create( spec, cc );

	return cc->basic[spec];
}


/*  
   DESC: creates Sym and Type. Registers enumeration in the outer namespace.
   'name' must not be NULL.

   EG: ns == NULL means that enumeration has no definition
*/
Type *Type::create_enum( char *name, Namespace *ns, Compiler *cc )
{
	Type *t = NULL;
	Sym  *s = NULL;

	// try to find out current enum declaration
	s = cc->current->search_id( name, t_enum, 0 );
	if( s )
		t = s->type;

	// create if not exist
	if( !t && ns ) {
		t = Type::create( t_enum, cc );
		t->enum_members = ns;
		t->tagname = name;

		if( s )
			s->type = t;

	} else if( t && ns ) {
		cc->error( " : error: 'enum %s' type redifinition\n", name );
	}

	// add structure to the outer namespace
	if( !s ) {
		s = Sym::create( name, cc );
		s->type = t;
		s->ns_modifier = t_enum;
		cc->current->add_item( s );
	}

	return t->clone( cc );
}
