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

	m->storage = scs_none;
	m->ns_modifier = 0;
	m->type = m->st1.type_tail = NULL;
	m->next = m->hashed = NULL;
	m->st1.next_tail = m;
	m->init = NULL;
	return m;
}


void Sym::fixtype( PType t, Compiler *cc )
{
	if( type )
		TYPE(st1.type_tail)->parent = t;
	else
		type = t;

	while( t->parent )
		t = t->parent;
	if( t->st1.storage == scs_typedef )
		ns_modifier = t_typename;

	storage = t->st1.storage;
}


void Sym::addtype( PType t )
{
	if( !st1.type_tail ) {
		type = st1.type_tail = t;
	} else {
		TYPE(st1.type_tail)->parent = t;
		st1.type_tail = t;
	}

	// TODO optimize st1.type_tail

	while( st1.type_tail->parent )
		st1.type_tail = st1.type_tail->parent;
}


void Sym::addnext( Sym *s )
{
	if( s ) {
		SYM(st1.next_tail)->next = s;
		st1.next_tail = s->st1.next_tail;
	}
}


/*
	registers variable declaration (global, in structure or in function)
	!: if variable is function declaration, redirect to declare_function
*/
Node *Sym::register_self( Place loc, Compiler *cc )
{
	ASSERT( type );

	if( type->specifier == t_func ) {
		declare_function( NULL, cc->current, loc, cc );
		return NULL;
	}


	if( cc->func ) {

		// in function declaration

	} else {

		// global declaration

	}

	// TODO check for redefinition
	PSym s = cc->current->search_id( name, ns_modifier, 0 );

	cc->current->add_item( this );

	// TODO process initializers
	return NULL;
}


/*
	This function is called for each function declarator. On function with
	body it is called twice, the second one is for statement.
*/
void Sym::declare_function( Node *statement, Namespace *outer, Place loc, Compiler *cc )
{
	ASSERT( type );

	// do not process bad symbols
	if( BAD(this) )
		return;

	if( type->specifier != t_func ) {

		cc->error( LOC "only function declarator can have a body\n", loc );
		makebad( this );
		return;
	}


	// register function in a namespace
	if( outer ) {

		ASSERT( statement == NULL );

		// 1. A function declarator shall not specify a return type that 
		//	is a function type or an array type.
		if( type->return_value->specifier == t_array || type->return_value->specifier == t_func ) {
			cc->error( LOC "function cannot return function or array type\n", loc );
			makebad( this );
			return;
		}

		// 2. The only storage-class specifier that shall occur in a 
		//	parameter declaration is register.
		for( PSym param = type->params; param; param = param->next )
			if( param->storage != scs_register && param->storage != scs_none ) {
				cc->error( LOC "parameter `%s' has invalid storage-class specifier\n", loc, param->name );
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
		PSym s = cc->current->search_id( name, ns_modifier, 0 );
		outer->add_item( this );

	// add a body
	} else {

		ASSERT( outer == NULL );
		TYPE(type)->body = statement;
	}
}
