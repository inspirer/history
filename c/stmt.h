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
 */

#ifndef stmt_h_included
#define stmt_h_included

#include "types.h"

/*   
 *	Expr structure describes an expression. It can be a leaf or a node.
 *	While parsing an input, first, we build Expr tree, then if the tree
 *	needs code generation we generate a code for it (Node list).
 */

enum {
	e_leaf,
	e_direct,
	e_unary,
	e_binary,
	e_tripl,
};


/*	(about next_free)
 * 
 *	Sometimes, during optimizations, Expr and Node structures can be freed, therefore 
 *	we cache such structures in Compiler::free_expr/free_node lists and can use them 
 *	again later. next_free points to the next structure in the list.
 *
 *	For example 1*2+3 expression will be simplified and four structures
 *	will be freed. 
 */

struct Expr {

	short type, sub;
	union {
		Expr  *op[3];		// e_unary, e_binary, e_tripl
		vlong value;		// e_direct
		Sym   *base;		// e_leaf
		Expr  *next_free;	// (see comment above about next_free)
	};

	// methods
	vlong calculate( int dest_type, Compiler *cc );

	// constructors
	static Expr *create( int type, int subtype, Compiler *cc );
	static Expr *create_icon_from_string( const char *s, Compiler *cc );
	static Expr *create_ccon_from_string( const char *s, Compiler *cc );
	              
	// destructors
	void free( Compiler *cc );
};

/*
 *    Node represents an intermediate language construction. Each node
 *    is described by quadruple:  op, res, arg1, arg2
 *    arg1 and arg2 can be of the following types: reg/imm/mem/addr
 *    res is always reg
 */

enum {
	argt_reg = 0,
	argt_imm = 1,
	argt_mem = 2,
	argt_codeaddr = 3
};

enum {
	cl_binop  = 1,          // res = arg1(reg,imm) op arg2(reg,imm)
	cl_unop   = 2,			// res = op arg1(reg,imm)
	cl_assign = 3,			// arg1(mem,reg) = arg2(mem,reg,imm,addr)
	cl_if_x_rel_y = 4,		// if( arg1(reg,imm) rel arg2(reg,imm) ) goto res(addr)
	cl_param  = 5,			// param arg1(reg,imm)
	cl_call   = 6,			// call arg1(addr,reg,imm)
	cl_deref  = 7,			// res = * arg1(reg,imm)
};

enum {
	op_add = 1,
	op_sub = 2,
	op_mul = 3,
	op_div = 4,
	op_shl = 5,

	un_min = 100,
};

#define OP(class,op,arg1,arg2) ( ((class)<<22) + ((arg1)<<19) + ((arg2)<<16) + (op) )
#define OPARG1(op) ( ((op)>>19) & 7 )
#define OPARG2(op) ( ((op)>>16) & 7 )
#define OPCLASS(op) ( (op) >> 22 )
#define OPSUB(op) ( (op) & ((1<<16)-1) )


struct Node {

	// quadruple (op, res, arg1, arg2) (16 bytes)
	unsigned long op, res, arg1, arg2;

	// Node list organistation (8 bytes)
	Node *next;
	union {
		Node *next_tail;	// during syntax analysis
		Node *next_free;    // (see comment above about next_free)
	};

	// methods
	static Node *combine( Node *left, Node *right );

	// constructor
	static Node *create( int type, Compiler *cc );

	// destructor (moves it to free list)
	void free( Compiler *cc );
};


/*
 *   Initializer
 */

struct Init {

	enum { init_expr, init_sublist, init_des_array, init_des_member };
	
	int type;
	Init *next;

	union {
		Expr *expr;     	// init_expr
		Init *child;		// init_designator, init_sublist
	};
	union {
		char *member;
		Expr *number;
	};
};

#endif
