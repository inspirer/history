/*   cc.h
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

#ifndef cc_h_included
#define cc_h_included

/*
 *	Common headers and types.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

/*
 *	Place structure is redefinition of lapg_place structure.
 */

struct Place {
	int line;
};

#define LOC "%i: "

// compiler-specific types
typedef unsigned __int64 vlong;
typedef unsigned long word;

#include "types.h"
#include "slab.h"
#include "stmt.h"

#define ASSERT(x) assert(x)


class Compiler {
private:
    char b[1025], *l, *end;

	char *identifier( char *token, int *n, int tnID );

public:
    slab sym_sl, type_sl, ns_sl, expr_sl, node_sl;
    Type *basic[t_basiccnt];
    Expr *free_expr; 						// see comment in stmt.h
    Node *free_node; 						// -----""-----
    Namespace *current, global, *func;

public:
    Compiler();
    void error( char *r, ... );
    int parse();
    void fillb();
};

#endif
