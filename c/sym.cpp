/*   sym.cpp
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


Sym *Sym::create( char *s, Compiler *cc )
{
	Sym *m = (Sym *)cc->sym_sl.allocate();
	m->name = s;

	m->offset = -1;
	m->ns_modifier = 0;
	m->type = m->type_tail = NULL;
	m->next = m->hashed = NULL;
	m->next_tail = m;
	m->init = NULL;
	return m;
}


void Sym::fixtype( Type *t, Compiler *cc )
{
	if( type )
		type_tail->parent = t;
	else
		type = t;
}


void Sym::addtype( Type *t )
{
	if( !type_tail ) {
		type = type_tail = t;
	} else {
		type_tail->parent = t;
		type_tail = t;
	}

	while( type_tail->parent )
		type_tail = type_tail->parent;
}


void Sym::addnext( Sym *s )
{
	if( s ) {
		next_tail->next = s;
		next_tail = s->next_tail;
	}
}


/*
	registers variable declaration (global, in structure or in function)
	!: if variable is function declaration, redirect to declare_function
*/
Node *Sym::register_self( Compiler *cc )
{
	ASSERT( type );

	if( type->specifier == t_func ) {
		declare_function( NULL, cc->current, cc );
		return NULL;
	}


	if( cc->func ) {

		// in function declaration

	} else {

		// global declaration

	}

	// TODO check for redefinition
	Sym *s = cc->current->search_id( name, ns_modifier, 0 );

	cc->current->add_item( this );

	// TODO process initializers
	return NULL;
}


/*
	This function is called for each function declarator. On function with
	body it is called twice, the second one is for statement.
*/
void Sym::declare_function( Node *statement, Namespace *outer, Compiler *cc )
{
	ASSERT( type );

	// do not process bad symbols
	if( BAD(this) )
		return;

	if( type->specifier != t_func ) {

		cc->error( "only function declarator can have a body\n" );
		makebad( this );
		return;
	}


	// register function in a namespace
	if( outer ) {

		ASSERT( statement == NULL );

		// 1. A function declarator shall not specify a return type that 
		//	is a function type or an array type.
		if( type->return_value->specifier == t_array || type->return_value->specifier == t_func ) {
			cc->error( "function cannot return function or array type\n" );
			makebad( this );
			return;
		}

		// 2. The only storage-class specifier that shall occur in a 
		//	parameter declaration is register.
		for( Sym *param = type->params; param; param = param->next )
			if( param->type->storage != scs_register && param->type->storage != scs_none ) {
				cc->error( "parameter `%s' has invalid storage-class specifier\n", param->name );
				makebad( this );
				return;
			}

		// 3. An identifier list in a function declarator that is not part 
		//	of a definition of that function shall be empty.

		// TODO

		// 4. After adjustment, the parameters in a parameter type list in 
		//	a function declarator that is part of a definition of that
		//	function shall not have incomplete type.

		// TODO

		// check for redefinition
		Sym *s = cc->current->search_id( name, ns_modifier, 0 );
		outer->add_item( this );

	// add a body
	} else {

		ASSERT( outer == NULL );
		type->body = statement;
	}
}
