/*   ns.cpp
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

/*
    TODO: destructor call from slab
*/

#include "cc.h"

enum { HASH_SIZE = 1023 };

static inline unsigned get_hash( char *name )
{
	unsigned long code = 0x102;
    while( *name )
    	code = ((code^(*name++))<<1) | (code>=0x80000000);
	return code%HASH_SIZE;
}


Namespace::Namespace()
{
	parent = NULL;
	associated = NULL;
	hash = new Sym *[HASH_SIZE];

	for( int i = 0; i < HASH_SIZE; i++ )
		hash[i] = NULL;
}


Namespace::~Namespace()
{
	delete[] hash;
}

    
void Namespace::add_item( Sym *i )
{
	unsigned hs;
	Type *t = i->type;

	ASSERT( i && i->name );
	hs = get_hash( i->name );
	i->hashed = hash[hs];
    hash[hs] = i;

    if( t ) {
		while( t->parent )
			t = t->parent;
		if( t->storage & scs_typedef ) {
			i->ns_modifier = t_typename;
			t->storage &= ~scs_typedef;
		}
	}
}


Sym *Namespace::search_id( char *name, int modifier, int go_deep )
{
    int hs;
	Sym *res;
	Namespace *n;

	ASSERT( name );
	hs = get_hash( name );
	for( n = this; n; n = go_deep?n->parent:NULL )
	 	for( res = n->hash[hs]; res; res = res->hashed )
 			if( !strcmp(res->name,name) && res->ns_modifier == modifier )
 				return res;

	return NULL;
}

void Namespace::newns( Compiler *cc ) 
{ 
	Namespace *n = new (cc->ns_sl.allocate()) Namespace; 
	n->parent = this;
	cc->current = n;
}


void Namespace::close_ns( Compiler *cc )
{
	cc->current = cc->current->parent;
}


#ifdef DEBUG_TREE

void debug_show_namespace( Namespace *ns, int deep )
{
	Sym *s;
	int hash_id;

	printf( "%*snamespace [\n", deep, "" );
	for( hash_id = 0; hash_id < HASH_SIZE; hash_id++ )
		for( s = ns->hash[hash_id]; s; s = s->hashed ) {
			debug_show_sym( s, deep+4 );
		}
	printf( "%*s]\n", deep, "" );
}

#endif
