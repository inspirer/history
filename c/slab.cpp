/*   slab.cpp
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

#include "slab.h"

slab::slab( int size )
{
	current = NULL;
	ssize = (size + 3) & ~3;
}

slab::~slab()
{
	destroy();
}


void *slab::allocate()
{
	void *res = NULL;

	if( current && current->size + ssize <= SLAB_SIZE ) {
		res = current->data + current->size;
		current->size += ssize;
	} else {
		struct block *tmp = new struct block;
		tmp->next = current;
		current = tmp;
		res = tmp->data;
		tmp->size = ssize;
	}

	return res;
}

void slab::destroy()
{
	while( current ) {
		struct block *tmp = current->next;
		delete current;
		current = tmp;
	}
}

