/*   cc.cpp
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

Compiler::Compiler() : 
    sym_sl(sizeof(Sym)), 
    type_sl(sizeof(Type)), 
    ns_sl(sizeof(Namespace)),
    expr_sl(sizeof(Expr)),
    node_sl(sizeof(Node))
{
	current = &global;
	func = NULL;
	free_expr = NULL;
	free_node = NULL;
}

/*
	detects whether the ID is a typename, and if so puts tnID into *n.
	returns a pointer to allocated string (like strdup)
*/
char *Compiler::identifier( char *token, int *n, int tnID )
{
	char *id = new char[ 1 + sizeof(Type*) + strlen(token)];

	*(Type **)id = NULL;
	strcpy( id + sizeof(Type*), token );

	if( Sym *s = current->search_id( token, t_typename, 1 ) ) {
		*(Type **)id = s->type;
		*n = tnID;
	}

	return id + sizeof(Type*);
}


void Compiler::error( char *r, ... )
{
    va_list arglist;
    va_start( arglist, r );
    vfprintf( stderr, r, arglist );
}


void Compiler::fillb()
{
    int size = fread( b, 1, 1024, stdin );
    b[size] = 0; end = b + size; l = b;
}


int main( int argc, char *argv[] )
{
    int  i;
    char *input = "-";
    Compiler p;
    
    for( i = 1; i < argc; i++ ) {
        if( argv[i][0]!='-' || argv[i][1]==0 )
            input = argv[i];
    }

    if( input[0] != '-' || input[1] != 0 )
        if( !freopen( input, "r", stdin ) ) {
            perror( input );
            return 1;
        }

    p.fillb();
    p.parse();
    #ifdef DEBUG_TREE
    printf("DEBUG:\n" );
    debug_show_namespace( &p.global, 0 );
    #endif
    return 0;
}
