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


// sym::storage
const static char *sym_storage[] = {
    "scs_none",    // 0,
    "scs_extern",  // 1, 
    "scs_static",  // 2, 
    "scs_auto",    // 3, 
    "scs_register",// 4, 
    "scs_typedef", // 5,
	"scs_???",     // 6,
    "scs_imm",     // 7, 
	"scs_bitfield",// 8,
	"scs_member",  // 9,
	"scs_arg",     // 10,
	"scs_arg_reg", // 11,
	"scs_label",   // 12,
	"scs_code",    // 13,
};

// sym::ns_modifier
static const char *sym_ns_mod( int i ) {

	switch( i ) {
		case 0: 		return "sym";
		case t_struct:  return "struct";
		case t_union:   return "union";
		case t_enum: 	return "enum";
		case t_typename:return "typename";
		case t_label: 	return "label";
		default:		return "unknown_ns_modifier";
	}
}


void debug_show_sym( Sym s, int deep )
{
	while( s ) {
		const char *storage, *tag;

		storage = ( s->storage >= 0 && s->storage < scs_max ) ? sym_storage[s->storage] : "scs_unkn";
		tag = sym_ns_mod( s->ns_modifier );

		printf( "sym(\n%*sname='%s' mod='%s'", deep+4, "", s->name ? s->name : "", tag );
		if( !s->ns_modifier )
			printf( " storage='%s'", storage );
		switch( s->storage ) {
			case scs_imm: printf( " value=%I64u", s->loc.val.u );break;
			case scs_arg: case scs_arg_reg: printf( " offset=%u", s->loc.arg.offset );break;
			case scs_member: case scs_bitfield: printf( " offset=%u", s->loc.member.offset );break;
		}
		printf( "\n%*s", deep+4, "" );
		debug_show_type( s->type, deep+4 );
		s = s->next;
		printf( "%*s)%s", deep, "", s ? " => " : "\n" );
	}
}


const char *aggr_type_spec[] = {
    "t_typename",	// 0x40
    "t_struct",		// 0x41
    "t_union",		// 0x42
    "t_array",		// 0x43
    "t_func",		// 0x44
    "t_pointer",	// 0x45
    "t_enum",		// 0x46
    "t_label",		// 0x47
};


void debug_show_type( Type t, int deep )
{
	const char *spec;
	char qual[128];
	Type s = t;

	printf( "type(\n" );
	while( s ) {
		spec = ( BASICTYPE(s) ) ? tdescr[T(s)].name : 
				( ( T(s) >= t_typename && T(s) <= t_label ) ?
					aggr_type_spec[T(s)-t_typename] : 
					"unkn_spec" );

		qual[0] = qual[1] = 0;
		if( Q(s) & tq_const ) strcat( qual, " const" );
		if( Q(s) & tq_volatile ) strcat( qual, " volatile" );
		if( Q(s) & tq_restrict ) strcat( qual, " restrict" );

		printf( "%*sspecifier='%s' qual='%s' size=%i align=%i", deep+4, "", spec, qual+1, s->size, s->align );
		if( ( T(s) == t_struct || T(s) == t_union ) && s->tagname )
			printf( " tag='%s'", s->tagname );

		if( T(s) == t_array ) {
			if( s->ar_size_val > 0 )
				printf( " dim=%i", s->ar_size_val );
			else if( s->ar_size_val == 0 )
				printf( " dim=undef" );
			else
				printf( " dim=var" );
		}

		printf( s->parent ? " =>\n" : "\n" );

		if( (T(s) == t_struct || T(s) == t_union) && s == t && s->params ) {
			printf( "%*scontains = ", deep+4, "" );
			debug_show_sym( s->params, deep + 8 );
		} else if( s->specifier == t_func ) {
			if( s->params ) {
				printf( "%*sparams = ", deep+4, "" );
				debug_show_sym( s->params, deep + 8 );
			}
			printf( "%*sreturn = ", deep+4, "" );
			debug_show_type( s->parent, deep + 8 );
			break;
		}

		s = s->parent;
	}
	printf( "%*s)\n", deep, "" );
}

#endif
