#!/usr/bin/perl

#
#   generates il.cpp il_ops.h
#

%cmds = ();

@names = ('');
$SUFFIXES = "";
@valid = ();

$suff = "";
$szcount = 1;

open( IN, "< il.descr" );
R: while( <IN> ) {
	chomp;
	next R if( /^#/ || /^\s*$/ );

	if( /^SUFFIXES:\s*([A-Z]+)/ ) {
		$SUFFIXES = $1;
		while( $SUFFIXES =~ /(.)/g ) {
			$suffcount++;
			$suff .= "$1 = $suffcount, ";
		}

	} elsif( /^SIZES:\s*([a-z]+)/ ) {
		$SIZES = $1;
		while( $SIZES =~ /(.)/g ) {
			$sz .= "\t_$1 = $szcount,\n";
			$szcount *= 2;
		}

	} elsif( /^([A-Z]+)\((\d+)(,(\w\w?))?\)\s*$/ ) {
		#print $1." ",$2." ".$4."\n";
		$name = $1;
		if( not exists $cmds{$name}) {
			$cmds{$name} = ++$cmdcounter;
			push @names, $name;
		}

	} elsif( /^\t([$SUFFIXES])\(([$SIZES]*)\)\s*$/ ) {
		#print "\t".$1." ".$2."\n";
		$letter = $1;
		$avail = $2;
		$list = "";

		while( $avail =~ /(.)/g ) {
			$list .= $list ? "|_$1" : "_$1";
		}
		$list = "0" unless $list;
		push @valid, "$name+$letter, $list";
	} else {
		die;
	}
}

close( IN );

$HEADER = <<END;
 *   C Compiler project (cc)
 *   Copyright (C) 2002-03  Eugeniy Gryaznov (gryaznov\@front.ru)
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
END

open( OUT, "> il_ops.h" );

print OUT <<END;
/*   il_ops.h
 *
$HEADER */

#ifndef il_ops_h_included
#define il_ops_h_included

END

print OUT "enum {\n";
while( ($i,$e) = each %cmds ) {
print OUT "\t$i = $e<<4,\n";
}
print OUT "\tCMDMAX = $cmdcounter<<4\n};\n\n";

print OUT "enum { $suff};\n\n";

print OUT "enum {\n$sz};\n\n";

print OUT "#endif\n";

close( OUT );

open( OUT, "> il.cpp" );

print OUT <<END;
/*   il.c
 *
$HEADER */

#include "cc.h"

END

print OUT "static const char *opnames[] = {\n";
for( @names ) { print OUT "\t\"$_\",\n"; }
print OUT "};\n";

$SUFFIXES .= "?" while length($SUFFIXES) < 15;

print OUT <<END;

static const char suffixes[] = "?$SUFFIXES";

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
END

for( @valid ) { print OUT "\t{ $_ },\n"; }

print OUT "\t{ 0, 0 }\n};\n\n";

print OUT <<END;
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
		if( valid_ops[i].op_spec == specific(op) && ( !size || (valid_ops[i].sizes&size) ) )
			return 1;
	return 0;
}

END

close( OUT );
