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
	ASSERT( SCALAR(this) && ( SCALAR(t) || VOIDTYPE(t) ) );

	//TODO();

	return 1;
}

/*
	6.2.7 Compatible type and composite type

	qualif - 1 means to process qualification
*/

Type *Type::compatible( Type *t1, Type *t2, int qualif, Compiler *cc )
{
	// 6.7.3.9 For two qualified types to be compatible, both shall have the
	// identically qualified version of a compatible type; the order of type 
	// qualifiers within a list of specifiers or qualifiers does not affect the
	// specified type.
	if( T(t1) != T(t2) || qualif == 0 && Q(t1) != Q(t2) )
		return NULL;

	Type *t, *result = t1;

	switch( T(t1) ) {

	// 6.7.5.1.2 For two pointer types to be compatible, both shall be identically
	//  qualified and both shall be pointers to compatible types.
	case t_pointer:
		t = Type::compatible( t1->parent, t2->parent, qualif, cc );
		result = t == t1->parent ? t1 : t == t2->parent ? t2 : NULL;
		if( t && (!result || Q(t1) != Q(t2) ) ) {
			result = t1->clone(cc);
			result->parent = t;
			Q(result) |= Q(t2);
		}
		break;

	// 6.7.5.2.6
	// For two array types to be compatible, both shall have compatible element types, and if
	// both size specifiers are present, and are integer constant expressions, then both size
	// specifiers shall have the same constant value. If the two array types are used in a context
	// which requires them to be compatible, it is undefined behavior if the two size specifiers
	// evaluate to unequal values.
	case t_array:
		TODO();

	// 6.7.5.3.15
	// For two function types to be compatible, both shall specify compatible return types.125)
	// Moreover, the parameter type lists, if both are present, shall agree in the number of
	// parameters and in use of the ellipsis terminator; corresponding parameters shall have
	// compatible types. If one type has a parameter type list and the other type is specified by a
	// function declarator that is not part of a function definition and that contains an empty
	// identifier list, the parameter list shall not have an ellipsis terminator and the type of each
	// parameter shall be compatible with the type that results from the application of the
	// default argument promotions. If one type has a parameter type list and the other type is
	// specified by a function definition that contains a (possibly empty) identifier list, both shall
	// agree in the number of parameters, and the type of each prototype parameter shall be
	// compatible with the type that results from the application of the default argument
	// promotions to the type of the corresponding identifier. (In the determination of type
	// compatibility and of a composite type, each parameter declared with function or array
	// type is taken as having the adjusted type and each parameter declared with qualified type
	// is taken as having the unqualified version of its declared type.)
	case t_func:
		TODO();

    // 6.2.7 two structure, union, or enumerated types declared in separate translation units are 
    // compatible if their tags and members satisfy the following requirements: If one is 
    // declared with a tag, the other shall be declared with the same tag. If both are completed 
    // types, then the following additional requirements apply: there shall be a one-to-one 
    // correspondence between their members such that each pair of corresponding members are 
    // declared with compatible types, and such that if one member of a corresponding pair is 
    // declared with a name, the other member is declared with the same name. For two
    // structures, corresponding members shall be declared in the same order. For two structures 
    // or unions, corresponding bit-fields shall have the same widths.
    case t_struct: case t_union:
		TODO();

    // 6.2.7 For two enumerations, corresponding members shall have the same values.
    case t_enum:
		TODO();

	// check qualification
	default: 
		if( Q(t1) != Q(t2) ) {
			result = t1->clone(cc);
			Q(result) |= Q(t2);
		}
	}

	return result;
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
	{ t_float, t_double, t_ldouble }, { t_fimg, t_dimg, t_limg }, { t_fcompl, t_dcompl, t_lcompl } };

const static int dom_x_dom_mul[3][3] = {
	{ 0, 1, 2 }, { 1, 0, 2 }, { 2, 2, 2 } };


Type *Expr::usual_conversions( Expr **x1, Expr **x2, int action, Compiler *cc )
{
	Type *p1, *p2, *res;
	Expr *e1 = *x1, *e2 = *x2;

	ASSERT( ARITHMETIC(e1->restype) && ARITHMETIC(e1->restype) );

	// float types
	if( FLOATTYPE(e1->restype) || FLOATTYPE(e2->restype) ) {
		int d1 = tdescr[T(e1->restype)].domain, d2 = tdescr[T(e2->restype)].domain;
		int type = F_TYPE(d1) > F_TYPE(d2) ? F_TYPE(d1) : F_TYPE(d2);
		int domain = 2, dom1 = F_DOMAIN(d1), dom2 = F_DOMAIN(d2);
		
		switch( action ) {
		case e_mult:
			domain = dom_x_dom_mul[dom1][dom2];
			break;
		case e_div:
			TODO();
		case e_tripl: case e_plus: case e_minus:
			if( dom1 == dom2 )
				domain = dom1;
			else 
				dom1 = dom2 = domain;
		default:
			TODO();
		}

		ASSERT( type < 3 && domain < 3 );

		res = Type::get_basic_type( dom_to_type[domain][type], cc );
        p1 = Type::get_basic_type( dom_to_type[dom1][type], cc );
        p2 = Type::get_basic_type( dom_to_type[dom2][type], cc );
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
