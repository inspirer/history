/*   il_ops.h
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

#ifndef il_ops_h_included
#define il_ops_h_included

enum {
	INDIR = 10<<4,
	BCOM = 5<<4,
	GT = 25<<4,
	LABEL = 33<<4,
	RSH = 20<<4,
	MOD = 18<<4,
	BAND = 13<<4,
	ARG = 29<<4,
	ADDRF = 1<<4,
	ADDRG = 2<<4,
	CNST = 4<<4,
	JUMP = 32<<4,
	CALL = 30<<4,
	DIV = 16<<4,
	ADDRL = 3<<4,
	BOR = 14<<4,
	CVF = 6<<4,
	NEG = 11<<4,
	LE = 26<<4,
	ASGN = 22<<4,
	MUL = 19<<4,
	CVI = 7<<4,
	NE = 28<<4,
	BXOR = 15<<4,
	CVP = 8<<4,
	SUB = 21<<4,
	CVU = 9<<4,
	LT = 27<<4,
	ADD = 12<<4,
	GE = 24<<4,
	LSH = 17<<4,
	RET = 31<<4,
	EQ = 23<<4,
	CMDMAX = 33<<4
};

enum { F = 1, I = 2, U = 3, P = 4, V = 5, B = 6, };

enum {
	_c = 1,
	_s = 2,
	_i = 4,
	_l = 8,
	_h = 16,
	_f = 32,
	_d = 64,
	_x = 128,
	_p = 256,
};

#endif
