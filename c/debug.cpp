/*   debug.cpp
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

#ifdef DEBUG_TREE

void debug_show_sym( Sym *s, int deep )
{
	char *head;

	while( s ) {
		switch( s->ns_modifier ) {
 			case 0: 		head = "sym";	break;
			case t_struct:  head = "struct_tag";break;
			case t_union:   head = "union_tag";	break;
			case t_enum: 	head = "enum_tag";	break;
			case t_typename:head = "typedef";break;
			case t_label: 	head = "label";	break;
			default:		head = "?????"; break;
		}
		printf( "%*s%s(%s) [\n", deep, "", head, s->name ? s->name : "" );
		printf( "%*stype (\n", deep+4, "" );
		debug_show_type( s->type, deep+8 );
		printf( "%*s)\n", deep+4, "" );
		s = s->next;
		if( s ) 
			printf( "%*s] (next) => \n", deep, "" );
		else
			printf( "%*s]\n", deep, "" );
	}
}


void debug_show_type( Type *t, int deep )
{
	char *spec, *stor, qual[128] = "";
	Type *s = t;

	while( s ) {
		switch( s->specifier ) {
			case t_void: spec = "void"; break;
			case t_char: spec = "char"; break;
			case t_schar: spec = "signed char"; break;
			case t_uchar: spec = "unsigned char "; break;
			case t_short: spec = "short"; break;
			case t_ushort: spec = "unsigned short"; break;
			case t_int: spec = "int"; break;
			case t_uint: spec = "unsigned"; break;
			case t_long: spec = "long"; break;
			case t_ulong: spec = "unsigned long"; break;
			case t_llong: spec = "long long"; break;
			case t_ullong: spec = "unsigned long long"; break;
			case t_float: spec = "float"; break;
			case t_double: spec = "double"; break;
			case t_ldouble: spec = "long double"; break;
			case t_bool: spec = "_Bool"; break;
			case t_fcompl: spec = "float _Complex"; break;
			case t_dcompl: spec = "double _Complex"; break;
			case t_lcompl: spec = "long double _Complex"; break;
			case t_fimg: spec = "float _Imaginary"; break;
			case t_dimg: spec = "double _Imaginary"; break;
			case t_limg: spec = "long double _Imaginary"; break;
			case t_typename: spec = "typename"; break;
			case t_struct: spec = "struct"; break;
			case t_union: spec = "union"; break;
			case t_array: spec = "array"; break;
			case t_func: spec = "function"; break;
			case t_pointer: spec = "*"; break;
			case t_enum: spec = "enum"; break;
			default: spec = "?????"; break;
		}
		switch( s->storage ) {
		    case scs_extern:   stor = "extern "; break;
		    case scs_static:   stor = "static "; break;
		    case scs_auto:     stor = "auto "; break;
		    case scs_register: stor = "register "; break;
		    case scs_typedef:  stor = "typedef "; break;
		    case scs_imm:  stor = "im "; break;
		    default: stor = "";
		}
		*qual = 0;
		if( s->qualifier & tq_const ) strcat( qual, "const " );
		if( s->qualifier & tq_volatile ) strcat( qual, "volatile " );
		if( s->qualifier & tq_restrict ) strcat( qual, "restrict " );

		printf( "%*s%s%s%s%s\n", deep, "", stor, qual, spec, s->parent ? " =>" : "" );

		if( (s->specifier == t_struct || s->specifier == t_union) && s == t && s->params ) {
			debug_show_sym( s->params, deep + 4 );
		} else if( s->specifier == t_func ) {
			if( s->params ) {
				printf( "%*sparams:\n", deep, "" );
				debug_show_sym( s->params, deep + 4 );
			}
			printf( "%*sreturn value:\n", deep, "" );
			deep += 4;
		}

		s = s->parent;
	}

}

#endif
