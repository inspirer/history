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

// (see 6.3.1.1 Boolean, characters, and integers)

// TODO improve enum_type

#define SBIT 0x1000
#define RANK(type) (type == t_enum ? 4 : type_rank[type]&0xff)
#define SIGNED(type) (type_rank[type]&SBIT)

static const int type_rank[t_basiccnt] = {
	0,
	0,		  //1 void
	2,		  //2 char
	2 | SBIT, //3 signed char
	2,		  //4 unsigned char 
	3 | SBIT, //5 short, signed short, short int, or signed short int
	3,		  //6 unsigned short, or unsigned short int
	4 | SBIT, //7 int, signed, or signed int
	4,		  //8 unsigned, or unsigned int
	5 | SBIT, //9 long, signed long, long int, or signed long int
	5,		  //0 unsigned long, or unsigned long int
    6 | SBIT, //1 long long, signed long long, long long int, or signed long long int
    6,		  //2 unsigned long long, or unsigned long long int
    0,		  //3 float
    0,		  //4 double
    0,		  //5 long double
    1,		  //6 _Bool
    0,		  //7 float _Complex
    0,		  //8 double _Complex
    0,		  //9 long double _Complex
    0,		  //0 float _Imaginary
    0,		  //1 double _Imaginary
    0,		  //2 long double _Imaginary
};


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
