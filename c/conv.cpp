/*   conv.cpp
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

//
// returns 1 if type if convertible to the target type
// 
int Type::can_convert_to( Type *t, Compiler *cc )
{
	// TODO
	return 0;
}

/*
	6.2.7 Compatible type and composite type

	1. Two types have compatible type if their types are the same. Additional rules for
	determining whether two types are compatible are described in 6.7.2 for type specifiers,
	in 6.7.3 for type qualifiers, and in 6.7.5 for declarators.46) Moreover, two structure,
	union, or enumerated types declared in separate translation units are compatible if their
	tags and members satisfy the following requirements: If one is declared with a tag, the
	other shall be declared with the same tag. If both are completed types, then the following
	additional requirements apply: there shall be a one-to-one correspondence between their
	members such that each pair of corresponding members are declared with compatible
	types, and such that if one member of a corresponding pair is declared with a name, the
	other member is declared with the same name. For two structures, corresponding
	members shall be declared in the same order. For two structures or unions, corresponding
	bit-fields shall have the same widths. For two enumerations, corresponding members
	shall have the same values.

*/

Type *Type::compatible( Type *t1, Type *t2, Compiler *cc )
{
	// TODO
	return NULL;
}


Type *Type::integer_promotion( Compiler *cc )
{
	// TODO bittype

	if( INTTYPE(this) && RANK(this) < INTRANK )
		return Type::get_basic_type( t_int, cc );

	return this;
}


/*
   Returns the common type for two arithmetic subexpressions and applies
   cast operator to each of them if needed.
*/
const static int dom_to_type[3][3] = { 
	{ t_float, t_double, t_ldouble }, { t_fcompl, t_dcompl, t_lcompl }, { t_fimg, t_dimg, t_limg } };

const static int dom_x_dom[3][3] = {
	{ 0, 1, 2 }, { 1, 0, 2 }, { 2, 2, 2 } };


Type *Expr::usual_conversions( Expr **x1, Expr **x2, Compiler *cc )
{
	Type *p1, *p2, *res;
	Expr *e1 = *x1, *e2 = *x2;

	ASSERT( ARITHMETIC(e1->restype) && ARITHMETIC(e1->restype) );

	// float types
	if( FLOATTYPE(e1->restype) || FLOATTYPE(e2->restype) ) {
		int d1 = tdescr[T(e1->restype)].domain, d2 = tdescr[T(e2->restype)].domain;
		int type = F_TYPE(d1) > F_TYPE(d2) ? F_TYPE(d1) : F_TYPE(d2);
		int domain = dom_x_dom[F_DOMAIN(d1)][F_DOMAIN(d2)];

		ASSERT( type < 3 && domain < 3 );

		res = Type::get_basic_type( dom_to_type[domain][type], cc );
        p1 = Type::get_basic_type( dom_to_type[F_DOMAIN(d1)][type], cc );
        p2 = Type::get_basic_type( dom_to_type[F_DOMAIN(d2)][type], cc );
		goto exit;
	}	

	// integer types
	p1 = e1->restype->integer_promotion(cc);
	p2 = e2->restype->integer_promotion(cc);

	// 6.3.8.1 If both operands have the same type, then no further conversion is needed.
	if( T(p1) == T(p2) ) {
		res = p1;
		goto exit;
	}

	// If both operands have signed integer types or both have unsigned
	// integer types, the operand with the type of lesser integer conversion rank is
	// converted to the type of the operand with greater rank.
	if( SIGNED(p1) == SIGNED(p2) ) {
		res = RANK(p1) >= RANK(p2) ? p1 : p2;
		goto exit;
	}

	// if the operand that has unsigned integer type has rank greater or
	// equal to the rank of the type of the other operand, then the operand with
	// signed integer type is converted to the type of the operand with unsigned
	// integer type.
	if( !SIGNED(p1) && SIGNED(p2) && RANK(p1) >= RANK(p2) ) {
		res = p1;
		goto exit;
	}

	if( SIGNED(p1) && !SIGNED(p2) && RANK(p1) <= RANK(p2) ) {
		res = p2;
		goto exit;
	}

	// If the type of the operand with signed integer type can represent
	// all of the values of the type of the operand with unsigned integer type, then
	// the operand with unsigned integer type is converted to the type of the
	// operand with signed integer type.
	res = RANK(p1) > RANK(p2) ? p1 : p2;

exit:
	if( T(e1->restype) != T(p1) ) {
		Expr *e = create( e_cast, p1, cc );
		e->op[0] = e1;
        *x1 = e;
	}

	if( T(e2->restype) != T(p2) ) {
		Expr *e = create( e_cast, p2, cc );
		e->op[0] = e2;
        *x2 = e;
	}

	return res;
}
