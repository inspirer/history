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
	t->st1.storage = scs_none;
	t->qualifier = tq_none;
	t->parent = NULL;
	t->size = ts == t_pointer ? POINTER_SIZE : 0;
	t->cloned = NULL;
	return t;
}


//
//  clones type structure, used to modify qualifiers
// 
Type *Type::clone( Compiler *cc ) const
{
	Type *t = (Type *)cc->type_sl.allocate();
	*t = *this;
	TYPE(this)->cloned = t;
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
static int toString_helper( const PType root, char **b, int *left, int pos ) {

	int size = 0;
	PType t = root;

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
				for( PSym s = t->params; s; s = s->next ) {
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
const char *Type::toString() const
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


/*
	DESC: returns the pointer to basic type
	
	EG: the type is cached in the Compiler::basic[1..t_basiccnt] variable
*/
PType Type::get_basic_type( int spec, Compiler *cc )
{
	ASSERT( spec > 0 && spec < t_basiccnt );

	if( cc->basic[spec] == NULL )
		cc->basic[spec] = create( spec, cc );

	return cc->basic[spec];
}

// //////////////////////////////////////////////////////////////////////////////////// //

                    
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
		cc->error( LOC "'%s' followed by '", loc, toString() );
		print_type( (ts==ts_llong)?ts_long:ts, cc );
		cc->error( "' is illegal\n" );
		return;
	}

	if( (ts & specifier) || !validate_type(specifier|ts) ) {
		cc->error( LOC "'", loc );
		print_type( specifier, cc );
		cc->error( "' followed by '" );
		print_type( (ts==ts_llong)?ts_long:ts, cc );
		cc->error( "' is illegal\n" );
		return;
	}

	specifier |= ts;
}


void Type::add_modifier( int modifier, Place loc, Compiler *cc )
{
	switch( modifier & ~0xff ) {
		case mod_storage:
			if( st1.storage == scs_none )
				st1.storage = modifier & 0xff;
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


// //////////////////////////////////////////////////////////////////////////////////// //


// creates function type

PType Type::create_function( PSym paramlist, Compiler *cc )
{
	Type *t = create( t_func, cc );
	t->params = paramlist;
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
PType Type::create_struct( char *name, int agg_type, Sym *members, Namespace *mm, Place loc, Compiler *cc )
{
	Type *t = NULL;
	Sym  *s = NULL;

	ASSERT( name || members && mm );

	// try to find out current structure declaration
	// if we search for 'struct ID' => search in the outer
	if( name ) {
		s = SYM(cc->current->search_id( name, agg_type, members == NULL ? 1 : 0 ));
		if( s )
			t = TYPE(s->type);
	}

	// create if not exist
	if( !t ) {
		t = Type::create( agg_type, cc );
		t->params = NULL;
		t->members = NULL;
		t->tagname = name;
	}

	// add structure to the outer namespace
	if( !s && name ) {
		s = Sym::create( name, cc );
		s->type = t;
		s->ns_modifier = agg_type;
		cc->current->add_item( s );
	}

	// 6.7.2.3.1 A specific type shall have its content defined at most once.
	if( t->params && members ) {
		cc->error( LOC "'%s %s' type redefinition\n", loc, agg_type==t_struct?"struct":"union", t->tagname );

	// add member declarations
	} else if( members ) {

		word size = 0;

		for( PSym q = members; q; q = q->next ) {
			if( GOOD(q) ) {
				if( agg_type == t_struct ) {
					SYM(q)->loc.member.offset = size;
					size += q->type->size;
				} else {
					SYM(q)->loc.member.offset = 0;
					size = (size > q->type->size) ? size : q->type->size;
				}

				// 6.7.2.1.2 A structure or union shall not contain a member with incomplete
				// or function type (hence, a structure shall not contain an instance of itself,
				// but may contain a pointer to an instance of itself), except that the last member
				// of a structure with more than one named member may have incomplete array type;
				// such a structure (and any union containing, possibly recursively, a member that
				// is such a structure) shall not be a member of a structure or an element of an array.
				if( INCOMPLETE(q->type) ) {
					if( q->next ) {
						cc->error( LOC "'%s %s' member cannot have incomplete array type\n", loc, agg_type==t_struct?"struct":"union", t->tagname );
						makebad(SYM(q));
						T(t) = t_bad;
					} else {
						// check varsizearray
						TODO();
					}
				}
			} else {
				T(t) = t_bad;
			}
		}

		// do not register bad types
		if( T(t) != t_bad )
			for( Type *cl = t; cl ; cl = TYPE(cl->cloned) ) {
				cl->params = members;
				cl->members = mm;
				cl->size = size;
			}
	}

	// do not clone unnamed structures
	return name ? t->clone( cc ) : t;
}

/*  
   DESC: creates Sym and Type. Registers enumeration in the outer namespace.
   'name' must not be NULL.

   EG: ns == NULL means that enumeration has no definition
*/
PType Type::create_enum( char *name, Namespace *ns, Place loc, Compiler *cc )
{
	Type *t = NULL;
	Sym  *s = NULL;

	ASSERT( name || ns );

	// try to find out current enum declaration
	if( name ) {
		s = SYM(cc->current->search_id( name, t_enum, 0 ));
		if( s )
			t = TYPE(s->type);
	}

	// create if not exist
	if( !t && ns && name ) {
		t = Type::create( t_enum, cc );
		t->enum_members = ns;
		t->tagname = name;

	} else if( t && ns ) {
		cc->error( LOC "'enum %s' type redifinition\n", loc, name );

	} else if( !t /* && name */ ) {
		// 6.7.2.3.2 A type specifier of the form 'enum identifier' without an enumerator
		// list shall only appear after the type it specifies is completed.
		cc->error( LOC "'enum %s': unknown type\n", loc, name );
		t = Type::create( t_bad, cc );
	}

	// add structure to the outer namespace
	if( !s && name ) {
		s = Sym::create( name, cc );
		s->type = t;
		M(s) = (T(t) == t_bad) ? t_bad : t_enum;
		cc->current->add_item( s );

	// export enumerators of unnamed enumeration
	} else if( !name /* && ns */ ) {
		ns->export_outer();
	}

	return t->clone( cc );
}


PType Type::create_array( int tqualifier, Expr *expr, Compiler *cc ) 
{
	Type *t = create( t_array, cc );

	if( expr ) {
		t->ar_size_val = -1;
		t->ar_size = expr;
		t->qualifier = tqualifier;

	} else {
		t->ar_size = NULL;
		t->ar_size_val = 0;
	}

	return t;
}
