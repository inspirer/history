###################################
#
#   Generate import block for PE
#

$id = '[a-zA-Z_][a-zA-Z_0-9]*';
$infile = "import.def";
$out = "import";
%dlls = ();
$current = "";


    $infile = shift if ($#ARGV>=0);
    $out = shift if ($#ARGV>=0);

    open( IMPORT, "< $infile")
        or die "import: $infile: $!\n";

    LINE: while( <IMPORT> ){
		chomp;
        next LINE if /^\s*$/;
        if( /^\s*($id)\s*:\s*$/ ){
        	$current = $1;
            $dlls{$current} = [] unless exists $dlls{$current};
        } elsif( /^\s*(0x[0-9a-fA-F]+)\s*,\s*($id)\s*$/ ){
        	push @{$dlls{$current}}, [$1, $2];
        } else {
        	die "import: wrong: $_\n";
        }
    }

    open( OUTF , "> ${out}") or die "grammar: $out: $!\n";

    print OUTF "section import.table {\n";
    for( keys %dlls ){
		print OUTF "	dd import.$_ - image_base,0,0xffffffff,import.$_.name - image_base,import.$_.result - image_base\n";
    }

    print OUTF "}\n\nsection import.table.names {\n";
    for( keys %dlls ){
    	print OUTF "	import.$_.name db \"$_.dll\",0\n	align 4\n";
    }

    print OUTF "}\n\nsection import.func {\n";
    for ( keys %dlls ){
    	for( @{$dlls{$_}} ){
    		print OUTF "\tfunc.$_->[1] dw $_->[0]\n\tdb \"$_->[1]\",0\n	align 4\n";
        }
    }
    print OUTF "}\n\n";

    for ( keys %dlls ){
    	print OUTF "section import.$_ {\n";
        for( @{$dlls{$_}} ){
        	print OUTF "\tdd func.$_->[1] - image_base\n";
        }
    	print OUTF "}\nsection import.$_.result {\n";
        for( @{$dlls{$_}} ){
			print OUTF "\t_$_->[1] dd func.$_->[1] - image_base\n";
        }
        print OUTF "}\n";
    }

    print OUTF "\nsection import.tables {\n";
    for( keys %dlls ){
	    print OUTF "	import.$_: section import.$_;\n";
	    print OUTF "	dd 0\n";
	    print OUTF "	import.$_.result: section import.$_.result;\n";
	    print OUTF "	dd 0\n";
    }
	print OUTF "}\n";
