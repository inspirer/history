/*   expr.cpp
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
	Main constructor for expressions
*/
Expr *Expr::create( int type, int subtype, Compiler *cc )
{
	Expr *e;
	
	// get from pool
	if( cc->free_expr ) {
		e = cc->free_expr;
		cc->free_expr = e->next_free;

	} else 
		e = (Expr *)cc->expr_sl.allocate();

	e->type = type;
	e->sub = subtype;

	return e;
}

/*
	parses string in the following regular expression and returns the 
	corresponding Expr structure: 

    ([1-9][0-9]*|0[0-7]*|0[xX][0-9a-fA-F]+)([uU](l|L|ll|LL)?|(l|L|ll|LL)[uU]?)?

    TODO: fix overflow
*/
Expr *Expr::create_icon_from_string( const char *s, Compiler *cc )
{
	Expr *e = create( e_direct, t_int, cc );
	vlong value = 0;

	// hexadecimal
	if( *s == '0' && ( s[1] == 'x' || s[1] =='X' ) ) {
		for( s += 2; *s >= '0' && *s <='9' || *s >= 'a' && *s <= 'f' || *s >= 'A' && *s <= 'F'; s++ ) {
			value <<= 4;
			value |= (*s <= '9') ? *s - '0' : *s - (*s <= 'F'?'A':'a') + 10;
		}

	// octal
	} else if( *s == '0' ) {
		s++;
		while( *s >= '0' && *s <= '7' ) {
			value <<= 3;
			value |= *s++ - '0';		
		}

	// decimal
	} else {
		while( *s >= '0' && *s <= '9' ) {
			value *= 10;
			value += *s++ - '0';
		}
	}

	// unsigned
	if( *s == 'u' || *s == 'U' ) {
		s++;
        if( *s == 'l' || *s == 'L' ) {
        	if( s[1] == s[0] )
        		e->sub = t_ullong;
        	else
        		e->sub = t_ulong;
        } else 
        	e->sub = t_uint;

	// long
	} else if( *s == 'l' || *s == 'L' ) {
		if( s[0] == s[1] ) {
			if( s[2] == 'u' || s[2] == 'U' )
				e->sub = t_ullong;
			else
				e->sub = t_llong;
		} else {
			if( s[1] == 'u' || s[1] == 'U' )
				e->sub = t_ulong;
			else
				e->sub = t_long;
		}
	}

	e->value = value;

	return e;
}


/*
 *	process escape characters
 */
static int escape_seq( const char * &s, Compiler *cc ) {

	int value;

	switch( *s ) {
		case '\'': case '\"': case '?': case '\\':
			return *s++;
		case '0':
			for( value = 0; *s >= '0' && *s < '8'; s++ ) {
				if( value & (7<<29) )
					cc->error( "overflow in octal escape sequence\n" );
				value = ( value << 3 ) | ( *s - '0' );
			}
			return value;
		case 'a': return s++, '\a';
		case 'b': return s++, '\b';
		case 'f': return s++, '\f';
		case 'n': return s++, '\n';
		case 'r': return s++, '\r';
		case 't': return s++, '\t';
		case 'v': return s++, '\v';
		case 'x': 
			s++;
			for( value = 0; (*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F'); s++ ) {
				if( value & (15<<28) )
					cc->error( "overflow in hexadimal escape sequence\n" );
				value = ( value << 4 ) | ( *s <= '9' ? *s - '0' : 10 + ((*s <= 'F') ? *s - 'A' : *s - 'a') );
			}
			return value;
		default:
			ASSERT(0);
			return 0;
	}
}


/*
	parses string in the following regular expression and returns the 
	corresponding Expr structure: 

    L?'([^\n\\']|\\(['"?\\abfnrtv]|x[0-9a-fA-F]+|[0-7]([0-7][0-7]?)?))+'
*/
Expr *Expr::create_ccon_from_string( const char *s, Compiler *cc )
{
	Expr *e = create( e_direct, t_int, cc );
	vlong value = 0;

	// wide character constant
	if( *s == 'L' ) {
		s += 2;
		
		cc->error( "wide character constants are not implemented\n" );

	// integer character constant
	} else {

		s++;
		while( *s != '\'' ) {

			if( value & (255<<24) )
				cc->error( "overflow in character constant\n" );
			if( *s == '\\' ) {
				s++;
				value = (value << 8) | escape_seq( s, cc );

			} else {
				value = (value << 8) | *s;
				s++;
			}
		}
	}

	e->value = value;

	return e;
}

/*
	DESC: moves the expression to free_expr list
*/
void Expr::free( Compiler *cc )
{
	next_free = cc->free_expr;
    cc->free_expr = this;
}


vlong Expr::calculate( int dest_type, Compiler *cc )
{
	if( type == e_direct && sub == type ) {
    	return value;
	}

	cc->error( "expression is not constant\n" ); // TODO
	return 0;
}
