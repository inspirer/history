/*   expr.cpp
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

/*
	Main constructor for expressions
*/
Expr *Expr::create( int type, Type *restype, Compiler *cc )
{
	Expr *e;
	
	// get from pool
	if( cc->free_expr ) {
		e = cc->free_expr;
		cc->free_expr = e->next_free;

	} else 
		e = (Expr *)cc->expr_sl.allocate();

	ASSERT(restype);

	e->type = type;
	e->restype = restype;

	return e;
}


/*
	parses string in the following regular expression and returns the 
	corresponding Expr structure: 

    ([1-9][0-9]*|0[0-7]*|0[xX][0-9a-fA-F]+)([uU](l|L|ll|LL)?|(l|L|ll|LL)[uU]?)?

    TODO: fix overflow
*/
Expr *Expr::create_icon_from_string( const char *s, Place loc, Compiler *cc )
{
	Expr *e = create( e_direct, NULL, cc );
	vlong value = 0;
	int type = t_int;

	// hexadecimal
	if( *s == '0' && ( s[1] == 'x' || s[1] =='X' ) ) {
		for( s += 2; *s >= '0' && *s <='9' || *s >= 'a' && *s <= 'f' || *s >= 'A' && *s <= 'F'; s++ ) {
			value <<= 4;
			value |= (*s <= '9') ? *s - '0' : *s - (*s <= 'F'?'A':'a') + 10;
		}

	// octal
	} else if( *s == '0' ) {
		s++;
		while( *s >= '0' && *s <= '7' ) {
			value <<= 3;
			value |= *s++ - '0';		
		}

	// decimal
	} else {
		while( *s >= '0' && *s <= '9' ) {
			value *= 10;
			value += *s++ - '0';
		}
	}

	// unsigned
	if( *s == 'u' || *s == 'U' ) {
		s++;
        if( *s == 'l' || *s == 'L' ) {
        	if( s[1] == s[0] )
        		type = t_ullong;
        	else
        		type = t_ulong;
        } else 
        	type = t_uint;

	// long
	} else if( *s == 'l' || *s == 'L' ) {
		if( s[0] == s[1] ) {
			if( s[2] == 'u' || s[2] == 'U' )
				type = t_ullong;
			else
				type = t_llong;
		} else {
			if( s[1] == 'u' || s[1] == 'U' )
				type = t_ulong;
			else
				type = t_long;
		}
	}

	e->value = value;
	e->restype = Type::get_basic_type( type, cc );

	return e;
}


/*
 *	process escape characters
 */
static int escape_seq( const char * &s, Place loc, Compiler *cc ) {

	int value;

	switch( *s ) {
		case '\'': case '\"': case '?': case '\\':
			return *s++;
		case '0':
			for( value = 0; *s >= '0' && *s < '8'; s++ ) {
				if( value & (7<<29) )
					cc->error( LOC "overflow in octal escape sequence\n", loc );
				value = ( value << 3 ) | ( *s - '0' );
			}
			return value;
		case 'a': return s++, '\a';
		case 'b': return s++, '\b';
		case 'f': return s++, '\f';
		case 'n': return s++, '\n';
		case 'r': return s++, '\r';
		case 't': return s++, '\t';
		case 'v': return s++, '\v';
		case 'x': 
			s++;
			for( value = 0; (*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F'); s++ ) {
				if( value & (15<<28) )
					cc->error( LOC "overflow in hexadimal escape sequence\n", loc );
				value = ( value << 4 ) | ( *s <= '9' ? *s - '0' : 10 + ((*s <= 'F') ? *s - 'A' : *s - 'a') );
			}
			return value;
		default:
			ASSERT(0);
			return 0;
	}
}


/*
	parses string in the following regular expression and returns the 
	corresponding Expr structure: 

    L?'([^\n\\']|\\(['"?\\abfnrtv]|x[0-9a-fA-F]+|[0-7]([0-7][0-7]?)?))+'
*/
Expr *Expr::create_ccon_from_string( const char *s, Place loc, Compiler *cc )
{
	Expr *e = create( e_direct, Type::get_basic_type( t_int, cc), cc );
	vlong value = 0;

	// wide character constant TODO
	if( *s == 'L' ) {
		s += 2;
		
		cc->error( LOC "wide character constants are not implemented\n", loc );

	// integer character constant
	} else {

		s++;
		while( *s != '\'' ) {

			if( value & (255<<24) )
				cc->error( LOC "overflow in character constant\n", loc );
			if( *s == '\\' ) {
				s++;
				value = (value << 8) | escape_seq( s, loc, cc );

			} else {
				value = (value << 8) | *s;
				s++;
			}
		}
	}

	e->value = value;

	return e;
}


Expr *Expr::create_from_id( const char *id, Place loc, Compiler *cc )
{
	// 6.5.1.2. An identifier is a primary expression, provided it has been declared
	// as designating an object (in which case it is an lvalue) or a function 
	// (in which case it is a function designator).

	Sym  *s = cc->current->search_id( id, 0, 1 /* search outer namespaces */ );

	if( !s ) {
		cc->error( LOC "unknown identifier: `%s`\n", loc, id );
		return NULL;
	}
	
	Expr *e = NULL;

	if( s->type && s->type->storage == scs_imm ) {
		e = create( e_direct, s->type, cc );
		e->value = s->value;

	} else {
		e = create( e_leaf, s->type, cc );
		e->base = s;
	}

	return e;
}


Expr *Expr::create_array_subscripting( Expr *arr, Expr *index, Place loc, Compiler *cc )
{
	// 6.5.2.1.1. One of the expressions shall have type "pointer to object type", the
	// other expression shall have integer type, and the result has type "type".

	if( arr && arr->restype->specifier != t_pointer && arr->restype->specifier != t_array ) {
		cc->error( LOC "subscript requires array or pointer type\n", loc );
		arr = NULL;
	}

	if( index && !INTTYPE(index->restype) ) {
		cc->error( LOC "subscript is not of integral type\n", loc );
		index = NULL;
	}

	// we return BAD expression if we have BAD subexpression
	if( !arr || !index )
		return NULL;

	Expr *e = create( e_array_subscripting, arr->restype->parent, cc );
	e->op[0] = arr;
	e->op[1] = index;

	return e;
}


Expr *Expr::create_struct_member( Expr *str_or_un, char *membername, Place loc, Compiler *cc )
{

	// 6.5.2.3.1 The first operand of the . operator shall have a qualified or unqualified
	// structure or union type, and the second operand shall name a member of that type.

	if( str_or_un && str_or_un->restype->specifier != t_struct && str_or_un->restype->specifier != t_union ) {
		cc->error( LOC "left of '.%s' must have struct/union type\n", loc, membername );
		str_or_un = NULL;
	}

	if( str_or_un && !str_or_un->restype->params ) {
		cc->error( LOC "left of '.%s' specifies undefined struct/union\n", loc, membername );
		str_or_un = NULL;
	}

	// we return BAD expression if we have BAD subexpression
	if( !str_or_un )
		return NULL;

	Sym *s = str_or_un->restype->members->search_id( membername, 0, 0 );
	
	if( !s ) {
		cc->error( LOC "'%s' is not a member of struct/union\n", loc, membername );
		return NULL;
	}

	Expr *e = create( e_member, s->type, cc );
	e->op[1] = str_or_un;
	e->base = s;

	// TODO real qualification
	
	return e;
}


Expr *Expr::create_struct_member_ptr( Expr *str_or_un, char *membername, Place loc, Compiler *cc )
{

	// 6.5.2.3.2 The first operand of the -> operator shall have type "pointer to qualified
	// or unqualified structure" or "pointer to qualified or unqualified union", and 
	// the second operand shall name a member of the type pointed to.

	if( str_or_un && ( str_or_un->restype->specifier != t_pointer || 
			(str_or_un->restype->parent->specifier != t_struct 
			&& str_or_un->restype->parent->specifier != t_union) ) ) {

		if( str_or_un->restype->specifier == t_struct || str_or_un->restype->specifier == t_union )
			cc->error( LOC "'->%s': left operand has struct/union type, use '.'\n", loc, membername );
		else 
			cc->error( LOC "left of '->%s' must point to struct/union\n", loc, membername );
		str_or_un = NULL;
	}

	if( str_or_un && !str_or_un->restype->parent->params ) {
		cc->error( LOC "left of '->%s' specifies undefined struct/union\n", loc, membername );
		str_or_un = NULL;
	}


	if( !str_or_un )
		return NULL;

	Sym *s = str_or_un->restype->parent->members->search_id( membername, 0, 0 );
	
	if( !s ) {
		cc->error( LOC "'%s' is not a member of struct/union\n", loc, membername );
		return NULL;
	}

	Expr *derefed = create( e_deref, str_or_un->restype->parent, cc );
	derefed->op[0] = str_or_un;

	Expr *e = create( e_member, s->type, cc );
	e->op[1] = derefed;
	e->base = s;

	// TODO real qualification

	return e;
}

//
//  The sizeof operator
//
Expr *Expr::get_type_size( Type *t, Place loc, Compiler *cc )
{
	// 6.5.3.4.1 The sizeof operator shall not be applied to an expression that has
	//  function type or an incomplete type, to the parenthesized name of such a type,
	//  or to an expression that designates a bit-field member.

	if( t->specifier == t_func ) {
		cc->error( LOC "expression has function type\n", loc );
		return NULL;  
	}

	if( INCOMPLETE(t) ) {
		cc->error( LOC "expression has an incomplete type\n", loc );
		return NULL;  
	}

	Expr *e = create( e_direct, Type::get_basic_type( t_int, cc ), cc );
	e->value = t->size;	

	printf( "yo%i\n", t->size );

	return e;
}

//
//  Cast operator
//
Expr *Expr::cast_to( Type *t, Place loc, Compiler *cc )
{
	// 6.5.4.2 Unless the type name specifies a void type, the type name shall
	// specify qualified or unqualified scalar type and the operand shall have scalar type.

	if( !SCALAR( this->restype ) ) {
		cc->error( LOC "cast operator can be applied only to scalar types\n", loc ); // TODO
		return NULL;
	}

	if( !VOIDTYPE( t ) && !SCALAR( t ) ) {
		cc->error( LOC "cast to non-scalar type is illegal\n", loc );  // TODO
		return NULL;
	}

	// 6.5.4.3 Conversions that involve pointers, other than where permitted by the
	// constraints of 6.5.16.1, shall be specified by means of an explicit cast.

	return NULL; // TODO
}


//
//	Binary operator
//
Expr *Expr::create_binary( Expr *e1, Expr *e2, int op, Place loc, Compiler *cc )
{

	// error checking switch, return NULL on error

	switch( op ) {

		// 6.5.5.2 Each of the operands shall have arithmetic type. The operands of
		// the % operator shall have integer type.
		case e_mult: case e_div: 
			if( !ARITHMETIC(e1->restype) || !ARITHMETIC(e2->restype) ) {
				cc->error( LOC "each of the operands shall have arithmetic type\n", loc );
				return NULL;
			}
			break;

		case e_perc:
			if( !INTTYPE(e1->restype) || !INTTYPE(e2->restype) ) {
				cc->error( LOC "each of the operands shall have integer type\n", loc );
				return NULL;
			}
			break;

		// 6.5.6.2 For addition, either both operands shall have arithmetic type, 
		// or one operand shall be a pointer to an object type and the other shall
		// have integer type. (Incrementing is equivalent to adding 1.)
		case e_plus:
			if(!( ARITHMETIC(e1->restype) && ARITHMETIC(e2->restype) || 
				  PTRTYPE(e1->restype) && INTTYPE(e2->restype) || 
				  INTTYPE(e1->restype) && PTRTYPE(e2->restype) )) {

				if( PTRTYPE(e1->restype) && PTRTYPE(e2->restype) )
					cc->error( LOC "cannot add two pointers\n", loc );
				else if( PTRTYPE(e1->restype) || PTRTYPE(e2->restype) )
					cc->error( LOC "pointer addition requires integral operand\n", loc );
				else 
					cc->error( LOC "addition can be applied only to arithmetic and pointer types\n", loc );
				return NULL;
			}
			break;

		// 6.5.6.3 For subtraction, one of the following shall hold:
		// - both operands have arithmetic type;
		// - both operands are pointers to qualified or unqualified versions of compatible object types;
		// - the left operand is a pointer to an object type and the right operand has integer type.
		case e_minus:
			if(!( ARITHMETIC(e1->restype) && ARITHMETIC(e2->restype) || 
					// TODO ? compatible types
				  PTRTYPE(e1->restype) && PTRTYPE(e2->restype) /* && compatible(e1->..,e2>..) */ ||
				  PTRTYPE(e1->restype) && INTTYPE(e2->restype) )) {

				if( PTRTYPE(e1->restype) )
					cc->error( LOC "pointer subtraction requires integral or pointer operand\n", loc );
				else if( PTRTYPE(e2->restype) )
					cc->error( LOC "pointer can only be subtracted from another pointer\n", loc );
				else
					cc->error( LOC "subtraction can be applied only to arithmetic and pointer types\n", loc );

				return NULL;
			}
			break;
	}
	return NULL;
}


//
//	DESC: moves the expression to free_expr list
//
void Expr::free( Compiler *cc )
{
	next_free = cc->free_expr;
    cc->free_expr = this;
}


vlong Expr::calculate( int dest_type, Compiler *cc )
{
	if( type == e_direct /* && sub == dest_type */ ) {
    	return value;
	}

	cc->error( "expression is not constant\n" ); // TODO
	return 0;
}
