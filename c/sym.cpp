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


sym *sym::create( char *s, Compiler *cc )
{
	sym *m = (sym *)cc->allocate(1,sizeof(sym));
	m->name = s;

	m->storage = scs_none;
	m->ns_modifier = 0;
	m->type = m->st1.type_tail = NULL;
	m->next = m->hashed = NULL;
	m->st1.next_tail = m;
	return m;
}


sym *sym::create_imm( char *s, Type t, ullong i, Compiler *cc )
{
	sym *m = create( s, cc );
	m->storage = scs_imm;
	m->type = t;
    m->loc.val.i = i;
    return m;
}

// symbol_created must prepeare 'storage' and 'loc' variables

void sym::symbol_created( Type t, int fix, Place loc, Compiler *cc )
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

	switch( fix ) {
	case fix_global: 
		if( storage == scs_none )
			storage = scs_extern;
		break;
	case fix_infunc: 
		break;
	case fix_member:
		// 6.7.2.1.2 A structure or union shall not contain a member with incomplete
		// or function type (hence, a structure shall not contain an instance of itself,
		// but may contain a pointer to an instance of itself), except that the last member
		// of a structure with more than one named member may have incomplete array type;
		// such a structure (and any union containing, possibly recursively, a member that
		// is such a structure) shall not be a member of a structure or an element of an array.
		if( T(type) == t_func ) {
			cc->error( LOC "function '%s' cannot be member of struct/union\n", loc, type->toString() );
			makebad(this);
			return;
		}

		if( INCOMPLETE(type) && T(type) != t_array ) {
			cc->error( LOC "'%s': object with incomplete type cannot be member of struct/union\n", loc, type->toString() );
			makebad(this);
			return;
		}

		// .4 A bit-field shall have a type that is a qualified or unqualified version
		//    of _Bool, signed int, unsigned int, or some other implementation-defined type.
		if( this->loc.member.bitsize >= 0 && !INTTYPE(type) ) {
			cc->error( LOC "'%s': type of bit field must be integral\n", loc, type->toString() );
			makebad(this);
			return;
		}

		// .3 The expression that specifies the width of a bit-field shall be an integer constant
		// expression that has nonnegative value that shall not exceed the number of bits in an
		// object of the type that is specified if the colon and expression are omitted. 
		if( this->loc.member.bitsize >= 0 && this->loc.member.bitsize > 8*type->size ) {
			cc->error( LOC "'%s:%i': type of bit field too small for number of bits\n", loc, type->toString(), this->loc.member.bitsize );
			makebad(this);
			return;
		}

		storage = this->loc.member.bitsize >= 0 ? scs_bitfield : scs_member;
		break;
	case fix_param:
		if( storage != scs_register && storage != scs_none ) {
			cc->error( LOC "parameter `%s' has invalid storage-class specifier\n", loc, name );
			makebad( this );
			return;
		}
		storage	= (storage == scs_register) ? scs_arg_reg : scs_arg;
		break;
	case fix_func:		
		if( storage == scs_none )
			storage = scs_extern;
		break;
	}

	if( !name )
		return;

	switch( fix ) {
	case fix_global: case fix_infunc: case fix_member:
		register_self( loc, cc );
		break;
	case fix_func:
		declare_function( NULL, cc->current->parent, loc, cc );
		break;
	}
}


void sym::addtype( Type t )
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


void sym::addnext( sym *s )
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
Node sym::register_self( Place loc, Compiler *cc )
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
	Sym s = cc->current->search_id( name, ns_modifier, 0 );

	cc->current->add_item( this );

	// TODO process initializers
	return NULL;
}


/*
	This function is called for each function declarator. On function with
	body it is called twice, the second one is for statement.
*/
void sym::declare_function( Node statement, Namespace *outer, Place loc, Compiler *cc )
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
		// moved to symbol_created

		// 3. An identifier list in a function declarator that is not part 
		//	of a definition of that function shall be empty.

		// TODO

		// 4. After adjustment, the parameters in a parameter type list in 
		//	a function declarator that is part of a definition of that
		//	function shall not have incomplete type.

		// TODO

		// check for redefinition
		Sym s = cc->current->search_id( name, ns_modifier, 0 );
		outer->add_item( this );

	// add a body
	} else {

		ASSERT( outer == NULL );
		this->loc.body = statement;
	}
}
