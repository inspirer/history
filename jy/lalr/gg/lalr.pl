#############################################################################
#  lalr.pl (Eugeniy Gryaznov, 2002, inspirer@inbox.ru)
#

$id = '[a-zA-Z_][a-zA-Z_0-9-]*';
$lexexpr = '\s*pp\(\s*([A-Za-z_][A-Za-z0-9_]*).*?,\s*([A-Za-z_][A-Za-z0-9_\s]*?)\s*,\s*\"([^\"]+)\"\s*[\)\,]';

$infile = "syntax";
$out = "Parse";
$input = "input";
$eoi = "EOI";
$warnlev = 0;
$context = "Context";
$E_SYNTAX = "";
$E_EOPARSE = "";
$core = 0;
$mutable = "";
$mutables = 0;

#############################################################################

    if ( $#ARGV>=0 && $ARGV[0]=~ /^-(\d+)$/ ){ $warnlev = $1;shift;}
    $infile = shift if ($#ARGV>=0);
    $out = shift if ($#ARGV>=0);

    open( SYNTAX, "< $infile")
        or die "grammar: $infile: $!\n";
	if ($core){
    	open( CORE, "> core")
        	or die "grammar: core: $!\n";
	}

    %terms = ();                            #  terminal -> symb_number
    %nterms = ();                           #  nonterm -> symb_number
    %empty_nterms = ();                     #  empty_nonterminals
    %types = ();                            #  symb_number -> type
    %efix = ();                             #  symb_number -> error
    %symbol_1 = ();                         #  symb_number -> name
    @rules = ();                            #  rules -> [$dest, $from, $action, $temp, $power]
    %optused = ();                          #  set of opt used symbs
    %usedr = ();                            #  set of rules
    $definition = "";                       #  all definitions
    $symN = 256;
    @textrepres = ();
    $maxpower = 0;
    @Priority = ();

    %chars = ();
    $#charterms = 255;
    for (@charterms){ $_ = 0; }
    $some_nterm_defined = 0;

    sub toNormalForm($$){
        my $rule = shift;
        my $result = $textrepres[$rule-256]." ::=";
        $rule = shift;
        while($rule =~ s/^\s*(\.?)(\d+)//){ if ($1 eq ""){$result.=" ".$textrepres[$2-256]} else {$result.=" [".$textrepres[$2-256]."]"}}
        return $result;
    }

    sub getsym($){
        my ($i,$e,$n);
        $e = shift;$n = $e;
        if ($e =~ /^\'(.)\'$/){
            $e = $1;
            $i = "gg${context}Sym".ord($e);
            $i = "gg${context}Sym_".$e if $e =~ /^[a-zA-Z0-9_]$/;
            $e = ord($e);
            unless(exists $terms{$i}){
                $terms{$i}=$symN;
                $charterms[$e]=$symN;
                $chars{$i}=$symN;
                $symbol_1{$symN}=$i;
                $types{$symN}="none";
                push @textrepres, "'".chr($e)."'";
                $symN++;
            }
            $n = $terms{$i};
        } else {
            $e =~ s/-/_/g;
            if (exists $terms{$e}){ $n = $terms{$e}; }
            else {
                unless(exists $nterms{$e}){
                    $nterms{$e}=$symN;
                    $symbol_1{$symN}=$e;
                    push @textrepres, $n;
                    $symN++;
                }
                $n = $nterms{$e};
            }
        }
        return $n;
    }

    LINE: while(<SYNTAX>){
        chomp;
        next LINE if (/^\s*#/ || /^\s*$/);
        if (/^\s*\.symbols\s+\"([^"]+)\"\s*$/){
            die "$infile($.): .symbols must be used before nonterm declaration\n" if $some_nterm_defined;
            my ($terminal_file,$prevline);
            $terminal_file = $1; $prevline = $.;
            open (TERMINAL, "< $terminal_file")
                or die "$infile($.): $terminal_file: $!\n";
            while(<TERMINAL>){
                if (/$lexexpr/){
                    die "$infile($prevline): $terminal_file($.): already exists: `$1'\n" if exists $terms{$1};
                    die "$infile($prevline): $terminal_file($.): already exists nonterminal: `$1'\n" if exists $nterms{$1};
                    $terms{$1}=$symN;
                    $symbol_1{$symN}=$1;
                    $types{$symN}=$2;
                    push @textrepres, $3;
                    $symN++;
                }
            }
            close (TERMINAL);
        } elsif(/^\s*\.input\s+($id)\s*$/){
            die "$infile($.): .input must be used before nonterm declaration\n" if $some_nterm_defined;
            $input = $1; $input =~ s/-/_/g;
        } elsif(/^\s*\.eoi\s+($id)\s*$/){
            die "$infile($.): .eoi must be used before nonterm declaration\n" if $some_nterm_defined;
            $eoi = $1; $eoi =~ s/-/_/g;
        } elsif(/^\s*\.context\s+($id)\s*$/){
            $context = $1;
        } elsif(/^\s*\.syntax\s+(.*)\s*$/){
            $E_SYNTAX = $1;
        } elsif(/^\s*\.eoparse\s+(.*)\s*$/){
            $E_EOPARSE = $1;
        } elsif(/^\s*\.mutable\s+(.*)\s*$/){
            $mutable = $1;
        } elsif(/^\s*\.(left|right|nonassoc)\s+(($id|\'.\')(\s*,\s*($id|\'.\'))*)\s*$/){
            my ($priority,$s,$count);
            $priority = "0:"; $count = 0;
            $priority = "1:" if $1 eq "right";
            $priority = "2:" if $1 eq "nonassoc";
            $s = $2;
            while($s =~ s/($id|\'.\')\s*\,?\s*//){
                my $i = $1;
                if ($i =~ /\'.\'/){
                    $i=&getsym($i);
                } else {
                    $i =~ s/-/_/g;
                    die "grammar: wrong priority symbol `$i'\n" unless (exists $terms{$i} || exists $nterms{$i});
                    $i = $terms{$i} if exists $terms{$i};
                    $i = $nterms{$i} if exists $nterms{$i};
                }
                $priority.=" ".($i-256);
                $count ++;
            }
            $priority = $count." ".$priority; 
            push @Priority, $priority;
            $some_nterm_defined = 1;
                        
        } elsif(/^\s*{{\s*$/){
            my $done = 0;
            CODEDEF: while(<SYNTAX>){
                if (/^\s*}}\s*$/){ $done = 1;last CODEDEF; }
                $definition.= $_;
            }
            die "$infile($.): syntax error, expected }}, found EOF\n" unless $done;
        } elsif(/^\s*($id)\s*::\s*($id)\s*(.*?)\s*$/){
			print CORE "\n$1::\n" if $core;
            my ($nttype,$ntname,$other,$errfix);
            $ntname = $1;$nttype = $2;$other = $3;
            die "$infile($.): terminal/nonterminal name conflict: `$ntname'\n" if exists $terms{$ntname};
            $some_nterm_defined = 1;
            die "$infile($.): optional name is given for `$ntname'\n" if $ntname =~ /opt$/;
            die "$infile($.): substitute name is given for `$ntname'\n" if $ntname =~ /sub$/;

            $ntname = &getsym($ntname);
            die "$infile($.): types conflict: `$types{$ntname}` and `$nttype`\n" if exists $types{$ntname} && $types{$ntname} ne $nttype;
            $types{$ntname} = $nttype unless exists $types{$ntname};

            $errfix = "";
            if ($other ne ""){
                die "$infile($.): nonterminal definition syntax error: $other\n" if $other !~ s/\s*,\s*on\s+error\s*:\s*//;
                while($other ne ""){
                    my $i;
                    $errfix.=" " if $errfix ne "";
                    die "$infile($.): `on error' syntax fault\n" unless $other =~ s/^(left|)\s*(\'.\'|$id)//;
                    $i = $2; $errfix.="!" if $1 ne "";
                    die "$infile($.): `on error' opt using fault: `$i'\n" if $i =~ /opt$/;
                    $errfix.=&getsym($i);
                    $other =~ s/\s*,\s*//;
                }
                $efix{$ntname}=$errfix;
            }

            my ($ruleN,$rule,$act,$ok,$power,$ruleprior,$lastsymbol,$substs);
            $ruleN = 1; $rule = $act = undef; $ok = 0; $power=0;
            RULE: while(<SYNTAX>){
                chomp;
                next RULE if (/^\s*\/\// || /^\s*$/);

                if (/^\s*;;\s*$/ || /^\s*=/){

					if ($core){
						my $core_print = $_;
						$core_print =~ s/\s*>>\s*none\s*$//;
						$core_print =~ s/\s*<<.*$//;
						$core_print =~ s/sub($|\W)/opt$1/g;
						print CORE "$core_print\n";
					}

                    if (defined $rule){
                        die "$infile($.): you cannot use more than 15 substitutions\n" if $substs > 15;
                        my $max = 1;
                        while($substs){ $substs--;$max *= 2;}
                        my ($current,$right,$_act,$dvdr,$principle,$ndest,$ncurr,$minus);

                        $current = 0;
                        while($current < $max){
                            $right = $rule; $_act = $act; $dvdr = $current; $minus = 0;
                            if ($max > 1){
                                $principle = undef; $ncurr = $ndest = 0;
                                while($right =~ s/^\s*(\-?)(\d+)//){
                                    if ($1 eq ""){
                                        if( defined $principle ){ $principle.=" ".$2;} else { $principle = $2;}
                                        if( $ncurr != $ndest ){ 
											$_act =~ s/\$$ncurr/\$$ndest/g if defined $_act; 
											$_act =~ s/\@$ncurr/\@$ndest/g if defined $_act;
										}
                                        die "$infile($.): wrong using {$ncurr} syntax (not substitute)\n" if defined $_act && ( $_act =~ /^\s*\{$ncurr\}/m || $_act =~ /^\s*\[$ncurr\]/m);
                                        $ndest++;
                                    } else {
                                        if( $dvdr%2 ){
                                            if (defined $principle){ $principle.=" ".$2;} else { $principle = $2;}
                                            if( $ncurr != $ndest ){ 
												$_act =~ s/\$$ncurr/\$$ndest/g if defined $_act; 
												$_act =~ s/\@$ncurr/\@$ndest/g if defined $_act; 
											}
                                            $_act =~ s/^\s*(\[\d+\]|\{\d+\})*\{$ncurr\}.*$//gm if defined $_act;
                                            $ndest++;
                                        } else {
                                            $minus++;
                                            $_act =~ s/\$$ncurr/not_defined/g if defined $_act;
                                            $_act =~ s/\@$ncurr/not_defined/g if defined $_act;
                                            $_act =~ s/^\s*(\[\d+\]|\{\d+\})*\[$ncurr\].*$//gm if defined $_act;
                                        }
                                        $dvdr /= 2;
                                    }
                                    $ncurr ++;
                                }
                                $_act =~ s/^\s*(\[\d+\]|\{\d+\})*\s*//gm if defined $_act;

                            } else { $principle = $rule; }

                            my $ur = $ntname." ::= ".$principle;
                            die "$infile($.): second rule: `@{[toNormalForm($ntname,$principle)]}'\n" if exists $usedr{$ur};
                            die "$infile($.): action for `follow` symbol defined: `@{[toNormalForm($ntname,$principle)]}'\n" if defined $act && $nttype eq "follow";
                            $usedr{$ur}=1;
                            
                            if ($principle eq ""){
                                if (defined $act){ $empty_nterms{$ntname} = 3;}
                                   else { $empty_nterms{$ntname} = 2;}
                            }
                            push @rules, [$ntname, $principle, $_act, 0, $power-$minus,$ruleprior,$rule_for_none];
                            #print "debug: @{[toNormalForm($ntname,$principle)]}\n" if $max>1;
                            $current ++;
                        }
                        $maxpower = $power if $maxpower < $power;
                        $rule = $act = undef; $power=0;
                    }
                }
                if(/^\s*;;\s*$/){ $ok=1;last RULE;}
                if (s/^\s*=\s*//){
                    $rule_for_none = $ruleprior = $lastsymbol = $substs = 0;
                    while(s/^(\'.\'(opt|sub)?|$id)\s*//){
                        my $i = $1;
                        my $e = 0;
                        my $substitute = 0;
                        die "$infile($.): wrong using of `$eoi'\n" if $eoi eq $i;
                        $e = 1 if $i =~ s/opt$//;
                        $substitute = 1 if $i =~ s/sub$//;
                        die "$infile($.): wrong using: subopt suffix\n" if $substitute && $e;
                        $i = &getsym($i);
                        $lastsymbol = $i if exists $terms{$symbol_1{$i}};
                        if ($e){
                            unless (exists $optused{$i}){
                                $e = $symbol_1{$i}."opt";
                                $nterms{$e}=$symN;
                                $symbol_1{$symN}=$e;
                                $types{$symN}="follow";
                                $e = $textrepres[$i-256]."opt";
                                push @textrepres, $e;
                                $empty_nterms{$symN} = 2;
                                push @rules, [$symN, "", undef, 0, 0, 0, 0];
                                push @rules, [$symN, $i, undef, 0, 1, 0, 0];
                                $optused{$i}=$symN++;
                            }
                            $i = $optused{$i};
                        }
                        if ($substitute){ $substs++; $i = -$i; }
                        if (defined $rule) { $rule.=" ".$i } else { $rule = $i };
                        $power++;
                    }
                    $ruleprior = $lastsymbol;
                    $rule = "" unless defined $rule;
                    if( s/^<<\s*($id|\'.\')\s*// ){
                        my $i = $1;
                        if ($i =~ /\'.\'/){
                            $i=&getsym($i);
                        } else {
                            $i =~ s/-/_/g;
                            die "grammar: wrong priority symbol `$i'\n" unless (exists $terms{$i} || exists $nterms{$i});
                            $i = $terms{$i} if exists $terms{$i};
                            $i = $nterms{$i} if exists $nterms{$i};
                        }
                        $ruleprior = $i;
                    }
                    if( s/^>>\s*none\s*// ){
                        $rule_for_none = 1;
                    }
                    die "$infile($.): wrong rule definition: `$_'\n" if $_ ne "";
                } elsif (defined $rule){
                    if (defined $act) {$act.="\n".$_} else {$act="#line $. \"$infile\"\n".$_}
                }
            }
            die "$infile($.): unexpected EOI\n" unless $ok;

        } else {
            print "$infile($.): skipped: $_\n";
        }
    }
    close( SYNTAX );
	if ($core){
		close( CORE );
	}

    $maxpower++;

	if( $mutable ){
		my $m = "";
		while($mutable =~ s/^(\'.\'|$id)\s*//){
			my $i = $1;
            die "wrong using of `$i' in mutable\n" if $eoi eq $i || $input eq $i;
            die "wrong using: sub or opt suffix ($i) in mutable\n" if $i =~ /opt$/ || $i =~ /sub$/;
            $i = &getsym($i);
            die "wrong mutable: $symbol_1{$i}\n" unless exists $terms{$symbol_1{$i}};
            $i-=256;$mutables++;
            if( $m ){ $m.="\n".$i; } else { $m = $i;}
        }
        die "wrong .mutable list: $mutable\n" if ( $mutable );
        $mutable = $m;
    }

    die "grammar: terminal symbol `$eoi' not defined\n" unless exists $terms{$eoi};
    die "grammar: symbol `$input' not defined\n" unless exists $nterms{$input};
    CHEKCKEYS: for (keys %nterms){
        die "grammar: type of nonterminal `$_' is unknown\n" unless exists $types{$nterms{$_}};
    }

    print "checking grammar ...\n";

    GETEMPTY: while(1){
        EMRULE: for (@rules){
            my($nform,$empty,$name);
            $name = $_->[0];
            $nform = $_->[1];
            $empty = 1;
            next EMRULE if exists $empty_nterms{$name};
            while($nform =~ s/^\s*(\d+)//){
                $empty = 0 unless exists $empty_nterms{$1};
            }
            if ($empty){
                $empty_nterms{$name}=1;
                next GETEMPTY;
            }
        }
        last GETEMPTY;
    }

    @ent = keys %empty_nterms;

    %useless = ();
    %Good = %empty_nterms;
    %Employed = ();

    GETGOOD: while(1){
        GRULE: for (@rules){
            my($nform,$good,$name);
            $name = $_->[0];
            $nform = $_->[1];
            $good = 1;
            next GRULE if exists $Good{$name};
            while($nform =~ s/^\s*(\d+)//){
                $good = 0 unless exists $Good{$1} || exists $terms{$symbol_1{$1}};
            }
            if ($good){
                $Good{$name}=1;
                next GETGOOD;
            }
        }
        last GETGOOD;
    }

    sub addEmployed($);
    sub addEmployed($){
        my $emp = shift;
        $Employed{$emp}=1;
        for (@rules){
            if ($_->[3] == 0 && $_->[0] == $emp){
                $_->[3]=1;
                my $rpor = $_->[1];
                while($rpor =~ s/^\s*(\d+)//){
                    addEmployed($1) unless exists $Employed{$1};
                }
                $_->[3]=0;
            }
        }
    }

    addEmployed($nterms{$input});

    for (values %nterms){
        $useless{$symbol_1{$_}} = 0 unless exists $Good{$_};
        $useless{$symbol_1{$_}} = 1 unless exists $Employed{$_};
    }

    undef %Good;undef %Employed;

    @useless_nterms = keys %useless;
    print "warning: useless: @useless_nterms\n" if $#useless_nterms>=0 && $warnlev>=1;
    print "warning: ${\(scalar(keys(%useless)))} useless nonterminals\n" if $#useless_nterms>=0 && $warnlev == 0;
    die "grammar: `$input' is useless\n" if $useless{$input};

    %ruletypes_ = ();
    @ruletypes = ();
    @ruleact = ();
    $reduceN = 0;

    push @ruletypes, "none";
    $ruletypes_{"none"}=0;
    
    for (@rules){
        if ($_->[1] eq "" && $empty_nterms{$_->[0]} == 2){
            push @ruleact, -2-$_->[0];
        } else {
            if ($types{$_->[0]} eq "follow"){
                push @ruleact, -1; $_->[2] = undef;
            } elsif ( $_->[6] ){
                push @ruleact, 0;
                $reduceN++ if defined $_->[2];
            } else {
                unless(exists $ruletypes_{$types{$_->[0]}}){
                    push @ruletypes, $types{$_->[0]};
                    $ruletypes_{$types{$_->[0]}}=$#ruletypes;
                }
                push @ruleact, $ruletypes_{$types{$_->[0]}};
                $reduceN++ if defined $_->[2];
            }
        }
    }

    for ( keys %nterms ){
        if( $types{$nterms{$_}} eq "follow" ){
            my $q = $_;
            $q =~ s/opt$//;
            if (exists $nterms{$q}){ $q = $nterms{$q};} else { $q = $terms{$q}; }
            $types{$nterms{$_}} = $types{$q};
        }
    }   

	$ctext_h = $context.".h"; $ctext_h =~ s/^C//;

#############################################################################

$total_symbols = $symN - 256;
$total_rules = scalar(@rules);
      
$class_parse=<<EOPARSE;
struct gg_msymbol {
	void *sym;
	int lexem;
	int state;
	int where;
};

struct gg_symbol {
    void *sym;
    int lexem;
	int where;
};

#include "$ctext_h"

EOPARSE

$realization=<<EOREAL;
void ${context}::gg_push_stack(gg_msymbol& s)
{
    gg_m[++gg_head] = s;
    gg_m[gg_head].state = gg_update_state(gg_m[gg_head-1].state,s.lexem);
}

void ${context}::gg_push_stack_null(int lexem)
{
    gg_m[++gg_head].lexem = lexem;
    gg_m[gg_head].sym = NULL;
    gg_m[gg_head].where = gg_next.where;
    info.use_place(gg_next.where,1);
    gg_m[gg_head].state = gg_update_state(gg_m[gg_head-1].state,lexem);
}

void ${context}::gg_pop_stack(int count)
{
    while(count){
        info.free_place(gg_m[gg_head].where);
        count--;gg_head--;
    }
}

void ${context}::gg_shift()
{
    gg_msymbol w;

    #ifdef DEBUG_parse
        printf("shift: %s\\n",gg_text[gg_next.lexem-256]);
    #endif

    w.sym = gg_next.sym;w.lexem = gg_next.lexem;w.where = gg_next.where;
    gg_push_stack(w);
    if( gg_next.lexem != Leoi){
	    gg_next.lexem = lex.gettoken(&gg_next.sym, &gg_next.where);
	    if (gg_next.lexem<256) gg_next.lexem=gg_translate[gg_next.lexem];
    }
}

void ${context}::parse()
{
    int i;

    gg_next.lexem = lex.gettoken(&gg_next.sym, &gg_next.where);
    if (gg_next.lexem<256) gg_next.lexem=gg_translate[gg_next.lexem];

    gg_head = 0;gg_m[0].state = 0;
    while(1){
        i=gg_next_state(gg_m[gg_head].state,gg_next.lexem);

        if( i>=0 ) gg_apply_rule(i);
        else if (i==-1) gg_shift();

        if (gg_next.lexem == $terms{$eoi} && gg_head == 1 && gg_m[1].lexem == $nterms{$input}) break;

        if( i==-2 || gg_m[gg_head].state == -1 ){
            if (gg_next.lexem == $terms{$eoi}){ $E_EOPARSE }
            	else { $E_SYNTAX }
            break;
        }
    }
}

EOREAL

#############################################################################

    print "saving files ...\n";

    $Context_h = "${context}.h";$Context_h =~ s/^C//;

    open( OUTF , "> ${out}.h") or die "grammar: $out: $!\n";
    {
        print OUTF "// ${out}.h\n\n";
        print OUTF "#ifndef ${out}_h_included\n#define ${out}_h_included\n\n";
        
        print OUTF "enum {\n";
        for (keys %nterms){
            print OUTF " $_ = $nterms{$_},\n";
        }
    }
    print OUTF "};\n\n";
    print OUTF "$class_parse";
    print OUTF "$definition";
    print OUTF "\n#endif\n";

    close OUTF;

    open( OUTF , "> ${out}.cpp") or die "grammar: $out: $!\n";
    print OUTF "// ${out}.cpp\n\n#include \"${out}.h\"\n";
    print OUTF "#include \"$Context_h\"\n";
    print OUTF "\nint ${context}::gg_translate[256]={\n";
    {
        my $i=0;for (@charterms){ printf OUTF " %4d,",$_;$i++;print OUTF "\n" unless $i%12; }
        print OUTF "\n" if $i%12;
    }
    print OUTF "};\n\n";

    print OUTF "char * ${context}::gg_text[]={\n";
    {
        my $i = "";
        for (@textrepres){
           if (length($i)+length($_)>63){
              print OUTF "$i\n";
              $i="";
           }
           $i.=" \"$_\",";
        }
        print OUTF "$i\n" if length($i);
    }
    print OUTF "};\n\n";

    print OUTF "short ${context}::gg_ract[$total_rules]={\n";
    {
        my $i=0;for (@ruleact){ printf OUTF " %4d,",$_;$i++;print OUTF "\n" unless $i%12; }
        print OUTF "\n" if $i%12;
    }
    print OUTF "};\n\n";

    print OUTF "short ${context}::gg_rlen[$total_rules]={\n";
    {
        my $i=0;for (@rules){ printf OUTF " %4d,",$_->[4];$i++;print OUTF "\n" unless $i%12; }
        print OUTF "\n" if $i%12;
    }
    print OUTF "};\n\n";

    print OUTF "short ${context}::gg_rlex[$total_rules]={\n";
    {
        my $i=0;for (@rules){ printf OUTF " %4d,",$_->[0];$i++;print OUTF "\n" unless $i%12; }
        print OUTF "\n" if $i%12;
    }
    print OUTF "};\n\n";

    print OUTF "$realization";

    print OUTF "void * ${context}::gg_create_rule_nterm(int type)\n{\n";
    {
        print OUTF "  switch(type){\n";
        my $i = 0;

        for ($i=1;$i<=$#ruletypes;$i++){
            print OUTF "    case $i: return (void *)(new $ruletypes[$i]);\n";
        }
    }
    print OUTF "  }\n  return NULL;\n}\n\n";

    print OUTF "void ${context}::gg_apply_rule(int rule)\n{\n";
    print OUTF "    #ifdef DEBUG_parse\n";
    print OUTF "        printf(\"reduce to %s\\n\",gg_text[gg_rlex[rule]-256]);\n";
    print OUTF "    #endif\n";
    print OUTF "    if (gg_ract[rule] < -1) gg_push_stack_null(-gg_ract[rule]-2);\n";
    print OUTF "    else if (gg_ract[rule] == -1){\n";
    print OUTF "        gg_pop_stack(gg_rlen[rule]-1);\n";
    print OUTF "        gg_m[gg_head].lexem=gg_rlex[rule];\n";
    print OUTF "        gg_m[gg_head].state=gg_update_state(gg_m[gg_head-1].state,gg_m[gg_head].lexem);\n";
    print OUTF "    } else {\n";

    print OUTF "        gg_msymbol gg;\n";
    print OUTF "        gg.lexem = gg_rlex[rule];\n";
    print OUTF "        gg.sym = gg_create_rule_nterm(gg_ract[rule]);\n";
    print OUTF "        gg.where = (gg_rlen[rule] == 0) ? gg_next.where : gg_m[gg_head+1-gg_rlen[rule]].where;\n";

    if ( $reduceN ){
        my ($a,$b,$c,$d,$e,$f);
        my $n = 0;
        print OUTF "        switch(rule){\n";
        for (@rules){
            if (defined $_->[2]){
                $a = $types{$_->[0]};$b = $_->[1];$c = $_->[2]; $e = $_->[4]-1;
                print OUTF "        case $n:{\n";
                $a = "(($a *)(gg.sym))";
                $c =~ s/\$\$/$a/g;
                $d = 0;
                while($b =~ s/\s*(\d+)//){
                    $f = $e - $d; $f = ($f == 0)? "" : -$f;
                    if ($types{$1} eq "none"){
                        #die "grammar: wrong action for `@{[toNormalForm($_->[0],$_->[1])]}`, present \$$d\n" if $c =~ /\$$d/;
						$a = "((void *)(gg_m[gg_head$f].sym))";
                    } else {
	                    $a = "(($types{$1} *)(gg_m[gg_head$f].sym))";
					}
  	                $c =~ s/\$$d/$a/g;
					$c =~ s/\@$d/(gg_m[gg_head$f].where)/g;
                    $d++;
                }
                $d = " "x12; $c =~ s/^\s*/$d/gm;
                print OUTF "$c\n            }break;\n";                                
            }
            $n++;
        }
        print OUTF "        }\n";
    }

    print OUTF "        info.use_place(gg.where,1);\n";
    print OUTF "        gg_pop_stack(gg_rlen[rule]);\n";
    print OUTF "        gg_push_stack(gg);\n";
    print OUTF "    }\n";
    print OUTF "}\n";

    close OUTF;

    open( OUTF , "> grammar") or die "grammar: $out: $!\n";

    @termsym = values %terms;

    print OUTF "$total_rules $total_symbols $maxpower @{[$nterms{$input}-256]} @{[$terms{$eoi}-256]}\n";
    for (@ent,@termsym){ $_-=256;}
    print OUTF "@ent\n";
    print OUTF "@termsym\n";
    print OUTF "${out}tbl.cpp\n";
    print OUTF "${out}.h\n";
    print OUTF "$context\n";
    for (@rules){
        my ($i,$e,$k,$j);
        ($i,$k) = ($_->[0],$_->[1]);$e = "";
        if ($_->[5]>255){ $j = $_->[5]-256;} else { $j = "";}
        $i-=256;while($k =~ s/(\d+)\s*//){ $e.=" ".($1-256);}
        print OUTF "$j: $i =$e\n";
    }
    for (@textrepres){
        print OUTF "$_\n";
    }
    print OUTF "@{[scalar(@Priority)]}\n";
    for (@Priority){
        print OUTF "$_\n";
    }

    print OUTF "$mutables\n$mutable\n";
    close OUTF;
