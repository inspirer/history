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


Type *Type::clone( Compiler *cc )
{
    Type *t = (Type *)cc->type_sl.allocate();
	*t = *this;
	return t;
}


static const int type_size[t_basiccnt] = {
    0,
    0,  //1 void
    1,  //2 char
    1,  //3 signed char
    1,  //4 unsigned char 
    2,  //5 short, signed short, short int, or signed short int
    2,  //6 unsigned short, or unsigned short int
    4,  //7 int, signed, or signed int
    4,  //8 unsigned, or unsigned int
    4,  //9 long, signed long, long int, or signed long int
    4,  //0 unsigned long, or unsigned long int
    8,  //1 long long, signed long long, long long int, or signed long long int
    8,  //2 unsigned long long, or unsigned long long int
    4,  //3 float
    8,  //4 double
    8,  //5 long double
    1,  //6 _Bool
    8,  //7 float _Complex
    16, //8 double _Complex
    16, //9 long double _Complex
    4,  //0 float _Imaginary
    8,  //1 double _Imaginary
    8,  //2 long double _Imaginary
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
		ASSERT( specifier > 0 && specifier < t_basiccnt );
		size = type_size[specifier];
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
