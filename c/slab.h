/*   slab.h
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

#ifndef slab_h_included
#define slab_h_included

#include <stdlib.h>

enum { SLAB_SIZE = 32768 - sizeof(void *) - sizeof(int) };

class slab {
private:
    int ssize;

    struct block {
	struct block *next;
        int  size;
        char data[SLAB_SIZE];
    } *current;

public:
    slab(int size);
    void *allocate();
    void destroy();
    ~slab();
};

#endif
