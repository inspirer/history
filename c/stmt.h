/*   stmt.h
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

#ifndef stmt_h_included
#define stmt_h_included

#include "types.h"

/*   
 *	expr structure describes an expression. It can be a leaf or a node.
 *	While parsing an input, first, we build expr tree, then if the tree
 *	needs code generation we generate a code for it (Node list).
 *
 *  !! We handle bad subexpressions as NULLs, i.e. if there was some syntax
 *	 or semantic error in expression. All create_* functions which create a new
 *	 expression verify that subexpressions are not NULL.
 */

enum {

	//	I've put binary expressions at the top of list to help compiler's
	//	'switch optimizer'.
	e_mult = 0, e_div, e_perc,
	e_plus, e_minus,
	e_shl, e_shr,
	e_xor, e_or, e_and,
	e_gt, e_lt, e_le, e_ge,
	e_eq, e_noteq,
	e_AND, e_OR,

	e_leaf,						// base = symbol, restype = type of symbol
	e_direct,					// restype = integral or float type, (d)value = imm value
	e_uminus,
	e_array_subscripting,		// op0 = array, op1 = index, restype = element type
	e_member,					// op1 = structure, restype, base = structure member
	e_deref,					// op0 = pointer_to_deref (op0->restype->specifier == t_pointer)
	e_tripl,					// restype = newtype, op0 - expr, op1,2 - variants
	e_cast,						// restype = cast_type
	e_comma,					// op0 = left, op1 = right, restype = op1->restype
};


/*	(about next_free)
 * 
 *	Sometimes, during optimizations, expr and Node structures can be freed, therefore 
 *	we cache such structures in Compiler::free_expr/free_node lists and can use them 
 *	again later. next_free points to the next structure in the list.
 *
 *	For example 1*2+3 expression will be simplified and four structures
 *	will be freed. 
 */

struct expr {

	int type;
	Type restype;			// !! be carefull, it must not be NULL
	union {
		Expr  op[3];		// e_*
		Value val;			// e_direct
		double dvalue;		// e_direct
		Sym base;			// e_leaf, e_member
		Expr  next_free;	// (see comment above about next_free)
	};

	// methods
	ullong calculate( int dest_type, Compiler *cc );

	// constructors
	static Expr create( int type, Type restype, Compiler *cc );
	static Expr create_icon_from_string( const char *s, Place loc, Compiler *cc );
	static Expr create_ccon_from_string( const char *s, Place loc, Compiler *cc );
	static Expr create_from_id( const char *id, Place loc, Compiler *cc );
	static Expr create_array_subscripting( Expr arr, Expr index, Place loc, Compiler *cc );
	static Expr create_struct_member( Expr str_or_un, char *membername, Place loc, Compiler *cc );
	static Expr create_struct_member_ptr( Expr str_or_un, char *membername, Place loc, Compiler *cc );
	static Expr get_type_size( Type t, Place loc, Compiler *cc );
	static Expr create_binary( Expr e1, Expr e2, int op, Place loc, Compiler *cc );
	static Expr create_conditional( Expr e1, Expr e2, Expr e3, Place loc, Compiler *cc );
	static Expr create_constant_expr( Expr e, Place loc, Compiler *cc );
	static Expr create_comma( Expr e1, Expr e2, Place loc, Compiler *cc );

	// casting
	static Expr cast_to( Expr e, Type t, Place loc, Compiler *cc );
	static Type usual_conversions( Expr *e1, Expr *t2, int action, Compiler *cc );
};


// 6.3.2.3.3 An integer constant expression with the value 0, or such an expression cast to type
// void *, is called a null pointer constant.

#define NULLCONST(ex) ( (ex)->type == e_direct && (ex)->val.u == 0 || \
					(ex)->type == e_cast && (ex)->op[0]->type == e_direct && (ex)->op[0]->val.u == 0 \
						&& T((ex)->restype) == t_pointer && T((ex)->restype->parent) == t_void )

/*
 *	Node represents an intermediate language construction. Each node
 *	is described by quadruple:  op, res, arg1, arg2
 *	arg1 and arg2 can be of the following types: reg/imm/mem/addr
 *	res is always reg
 */

struct node {
	short op;
	short count;
 	Sym syms[3];
	Node kids[2];
	Node link;
	//XNode2 x;
};


/*
 *   Initializer
 */

struct Init {

	enum { in_byte, in_word, in_dword, in_long, in_float, in_double, in_ldouble,
			in_pcname, in_dataname, in_storage, in_label };

	int type, offset;
	Init *next;

	union {
		long l;
		double d;
		sym *s;
	} val;
};

#endif
