/*   il.c
 *
 *   C Compiler project (cc)
 *   Copyright (C) 2002-03  Eugeniy Gryaznov (gryaznov.ru)
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

static const char *opnames[] = {
	"",
	"ADDRF",
	"ADDRG",
	"ADDRL",
	"CNST",
	"BCOM",
	"CVF",
	"CVI",
	"CVP",
	"CVU",
	"INDIR",
	"NEG",
	"ADD",
	"BAND",
	"BOR",
	"BXOR",
	"DIV",
	"LSH",
	"MOD",
	"MUL",
	"RSH",
	"SUB",
	"ASGN",
	"EQ",
	"GE",
	"GT",
	"LE",
	"LT",
	"NE",
	"ARG",
	"CALL",
	"RET",
	"JUMP",
	"LABEL",
};

static const char suffixes[] = "?FIUPVB?????????";

extern char *stringf(const char *fmt, ...);

char *opname( int op )
{
	return stringf("%s%s%s",
		opindex(op) > 0 && opindex(op) < CMDMAX ? opnames[opindex(op)] : "???",
		suffixes[optype(op)], opsize(op) > 0 ? stringf("%i",opsize(op)) : "");
}

static const struct {
	int op_spec;
	int sizes;

} valid_ops[] = {
	{ ADDRF+P, _p },
	{ ADDRG+P, _p },
	{ ADDRL+P, _p },
	{ CNST+F, _f|_d|_x },
	{ CNST+I, _c|_s|_i|_l|_h },
	{ CNST+U, _c|_s|_i|_l|_h },
	{ CNST+P, _p },
	{ BCOM+I, _i|_l|_h },
	{ BCOM+U, _i|_l|_h },
	{ CVF+F, _f|_d|_x },
	{ CVF+I, _i|_l|_h },
	{ CVI+F, _f|_d|_x },
	{ CVI+I, _c|_s|_i|_l|_h },
	{ CVI+U, _c|_s|_i|_l|_h|_p },
	{ CVP+U, _p },
	{ CVU+I, _c|_s|_i|_l|_h },
	{ CVU+U, _c|_s|_i|_l|_h },
	{ CVU+P, _p },
	{ INDIR+F, _f|_d|_x },
	{ INDIR+I, _c|_s|_i|_l|_h },
	{ INDIR+U, _c|_s|_i|_l|_h },
	{ INDIR+P, _p },
	{ INDIR+B, 0 },
	{ NEG+F, _f|_d|_x },
	{ NEG+I, _i|_l|_h },
	{ ADD+F, _f|_d|_x },
	{ ADD+I, _i|_l|_h },
	{ ADD+U, _i|_l|_h|_p },
	{ ADD+P, _p },
	{ BAND+I, _i|_l|_h },
	{ BAND+U, _i|_l|_h },
	{ BOR+I, _i|_l|_h },
	{ BOR+U, _i|_l|_h },
	{ BXOR+I, _i|_l|_h },
	{ BXOR+U, _i|_l|_h },
	{ DIV+F, _f|_d|_x },
	{ DIV+I, _i|_l|_h },
	{ DIV+U, _i|_l|_h },
	{ LSH+I, _i|_l|_h },
	{ LSH+U, _i|_l|_h },
	{ MOD+I, _i|_l|_h },
	{ MOD+U, _i|_l|_h },
	{ MUL+F, _f|_d|_x },
	{ MUL+I, _i|_l|_h },
	{ MUL+U, _i|_l|_h },
	{ RSH+I, _i|_l|_h },
	{ RSH+U, _i|_l|_h },
	{ SUB+F, _f|_d|_x },
	{ SUB+I, _i|_l|_h },
	{ SUB+U, _i|_l|_h|_p },
	{ SUB+P, _p },
	{ ASGN+F, _f|_d|_x },
	{ ASGN+I, _c|_s|_i|_l|_h },
	{ ASGN+U, _c|_s|_i|_l|_h },
	{ ASGN+P, _p },
	{ ASGN+B, 0 },
	{ EQ+F, _f|_d|_x },
	{ EQ+I, _i|_l|_h },
	{ EQ+U, _i|_l|_h|_p },
	{ GE+F, _f|_d|_x },
	{ GE+I, _i|_l|_h },
	{ GE+U, _i|_l|_h|_p },
	{ GT+F, _f|_d|_x },
	{ GT+I, _i|_l|_h },
	{ GT+U, _i|_l|_h|_p },
	{ LE+F, _f|_d|_x },
	{ LE+I, _i|_l|_h },
	{ LE+U, _i|_l|_h|_p },
	{ LT+F, _f|_d|_x },
	{ LT+I, _i|_l|_h },
	{ LT+U, _i|_l|_h|_p },
	{ NE+F, _f|_d|_x },
	{ NE+I, _i|_l|_h },
	{ NE+U, _i|_l|_h|_p },
	{ ARG+F, _f|_d|_x },
	{ ARG+I, _i|_l|_h },
	{ ARG+U, _i|_l|_h },
	{ ARG+P, _p },
	{ ARG+B, 0 },
	{ CALL+F, _f|_d|_x },
	{ CALL+I, _i|_l|_h },
	{ CALL+U, _i|_l|_h },
	{ CALL+P, _p },
	{ CALL+V, 0 },
	{ CALL+B, 0 },
	{ RET+F, _f|_d|_x },
	{ RET+I, _i|_l|_h },
	{ RET+U, _i|_l|_h },
	{ RET+P, _p },
	{ RET+V, 0 },
	{ JUMP+V, 0 },
	{ LABEL+V, 0 },
	{ 0, 0 }
};

int valid_op( int op, Backend *be )
{
	int i, size;

	size = 0;
	if( opsize(op) > 0 ) {
		for( i = 1; i < t_basiccnt; i++ )
			if( opsize(op) == be->o._met[i].size )
				size |= tdescr[i].size_name;
		if( opsize(op) == be->o._ptr.size )
			size |= _p;
	}
	
	for( i = 0; valid_ops[i].op_spec; i++ )
		if( valid_ops[i].op_spec == specific(op) )
			return 1;
	return 0;
}

