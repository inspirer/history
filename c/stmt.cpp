/*   stmt.cpp
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
 *	Constructors
 */
Node *Node::create( int op, Compiler *cc )
{
	Node *n;

	// get from pool
	if( cc->free_node ) {
		n = cc->free_node;
		cc->free_node = n->next_free;

	} else 
		n = (Node *)cc->node_sl.allocate();

	n->op = op;
	n->res = n->arg1 = n->arg2 = 0;
	n->next = NULL;
	n->next_tail = n;
	return n;
}

/*
 *	Combine to Nodelists, fixes node_tail of the first element
 */
Node *Node::combine( Node *left, Node *right )
{
	if( left && right ) {
		left->next_tail->next = right;
		left->next_tail = right->next_tail;
        return left;

	} else if( left )
		return left;
	else 
		return right;
}

/*
 *	DESC: moves the Node to free_expr list
 */
void Node::free( Compiler *cc )
{
	next_free = cc->free_node;
    cc->free_node = this;
}
