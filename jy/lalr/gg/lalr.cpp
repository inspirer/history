// lalr.cpp (Eugeniy Gryaznov, 2002, inspirer@inbox.ru)

#include "gram.h"

#define getnext *t=0;if (!fgets(t,32768,f)) return 1;
#define chomp l=t;while(*l!='\r'&&*l!='\n'&&*l!=0) l++;*l=0;

int debuglev=0;

int readinfo(char *file,info *i)
{
    char * t = new char[32768];
    FILE *f;
    int a,b,*e;
    char *l,*p;

    if ((f=fopen(file,"r"))){

        getnext;
        if (sscanf(t,"%d %d %d %d %d",&i->rules,&i->symbols,&i->power,&i->input,&i->eoi)!=5) return 1;
        e = i->symbol = new int[i->symbols];
        for (a=0;a<i->symbols;a++) *e++=0;        

        getnext;chomp;
        l=t;while(*l){
            p=l;while(*l != ' '&& *l!=0) l++;while(*l == ' ') l++;
            if (sscanf(p,"%d",&a)!=1) return 1;
            i->symbol[a]|=EMPTY;
        }

        getnext;chomp;
        l=t;while(*l){
            p=l;while(*l != ' '&& *l!=0) l++;while(*l == ' ') l++;
            if (sscanf(p,"%d",&a)!=1) return 1;
            i->symbol[a]|=TERM;
        }

        getnext;chomp;
        i->file_cpp=new char[strlen(t)+1];
        strcpy(i->file_cpp,t);

        getnext;chomp;
        i->file_h=new char[strlen(t)+1];
        strcpy(i->file_h,t);

        getnext;chomp;
        i->class_name=new char[strlen(t)+1];
        strcpy(i->class_name,t);

        i->rule = new int*[i->rules];
        i->length = new int [i->rules];
        i->reduce = new int [i->rules];
        for (a=0;a<i->rules;a++){
            getnext;chomp;
            i->rule[a] = new int[i->power];
            i->length[a] = 0;

            l=t;
            if (*l == ':'){
                l++;
                i->reduce[a] = -1;
            } else {
                sscanf(l,"%d",&i->reduce[a]);
                while(*l!=':'&& *l!=0 ) l++;if (*l) l++;
            }

            while(*l == ' ' ) l++;           
            if(!*l) return 1;
            sscanf(l,"%d",&i->rule[a][0]);
            while(*l!='='&& *l!=0 ) l++;if (*l) l++;
            while(*l == ' ' ) l++;
            b=0;
            while (*l){
                b++;
                if (b >= i->power) return 1;
                if(*l>'9'||*l<'0') return 1;
                if(sscanf(l,"%d",&i->rule[a][b])!=1) return 1;
                while(*l<='9'&&*l>='0') l++;
                while(*l == ' ') l++;
            }
            i->length[a]=b;
        }

        i->name = new char *[i->symbols];
        for( a=0;a<i->symbols;a++){
            getnext;chomp;
            i->name[a] = new char[strlen(t)+1];
            strcpy(i->name[a],t);
        }

        getnext;chomp;
        if (sscanf(t,"%d",&i->priorities)!=1) return 1;
        i->prior = new info::priority[i->priorities];

        for( a=0; a<i->priorities; a++ ){
            getnext;chomp;
            if (sscanf(t,"%d %d:",&i->prior[a].count,&i->prior[a].assoc)!=2) return 1;
            l=t;
            while(*l != ':' && *l != 0 ) l++;if (*l) l++;
            i->prior[a].place = new int[i->prior[a].count];
            i->prior[a].s = new set(i->symbols);
            while(*l == ' ' ) l++;
            int q=0;
            while(*l){
                if (sscanf(l,"%d",&b)!=1) return 1;
                i->prior[a].s->setbit(b);
                if (i->prior[a].count <= q) return 1;
                i->prior[a].place[q++] = b;
                while(*l != ' ' && *l != 0 ) l++;
                while(*l == ' ' ) l++;
            }
            if (i->prior[a].count != q) return 1;
        }

        getnext;chomp;
        if (sscanf(t,"%d",&i->mutables)!=1) return 1;
		for( a=0; a<i->mutables; a++){
	        getnext;chomp;
		    if (sscanf(t,"%d",&b)!=1) return 1;
			i->symbol[b]|=MUTABLE;			
		}

        fclose(f);
    } else return 2;

    delete[] t;
    return 0;
}

void info::First()
{
    int added=1,a,b,c,d,e;

    printf("calculating first & follow ...\n");

    first = new set*[symbols];
    for (e=0;e<symbols;e++){
        first[e]=new set(symbols);
        if (symbol[e] & TERM){ first[e]->setbit(e); }
    }

    while( added ){
        added = 0;
        for( a=0; a<rules; a++){
            c = rule[a][0];
            for( b=1; b<=length[a]; b++){
                d = rule[a][b];
                added |= first[c]->add(first[d]);
                if ((symbol[d] & EMPTY) == 0) break;
            }
        }
    }
}

void info::Follow()
{
    int added=1,a,b,c,d,e;
    set dyn(symbols);

    follow = new set*[symbols];
    for( e=0; e<symbols; e++) follow[e]=new set(symbols);
    follow[input]->setbit(eoi);


    while( added ){
        added = 0;
        for( a=0; a<rules; a++){
            c = rule[a][0];
            dyn.clear();
            dyn.add(follow[c]);
            for ( b=length[a]; b>=1; b--){
                d = rule[a][b];
                added |= follow[d]->add(&dyn);
                if ((symbol[d] & EMPTY) == 0) dyn.clear();
                dyn.add(first[d]);
            }
        }
    }
}

void info::enumerate()
{
    int a,b,c;

    for( a=0,b=0; a<rules; a++) b+=length[a]+1;
    situations = b;

    sRule = new int [b];
    sPos = new int [b];
    index = new int [rules];
    c = 0;
    for( a=0; a<rules; a++){
        index[a] = c;
        for( b=0; b<=length[a]; b++){ sRule[c]=a;sPos[c++]=b;}
    }
}

void print_situation(FILE *out,info *i,int num)
{
    int a;
    fprintf(out,"  %s ::=",i->name[i->rule[i->sRule[num]][0]]);
    for( a=1; a<=i->length[i->sRule[num]]; a++ ){
        if (a-1 == i->sPos[num]) fprintf(out," _");
        fprintf(out," %s",i->name[i->rule[i->sRule[num]][a]]);
    }
    if (i->length[i->sRule[num]] == i->sPos[num]) fprintf(out," _");
    fprintf(out,"\n");
}

void info::extendSet(set& dyn, set& els)
{
    int a,b,c,added=1;

    while( added ){
        added=0;
        for( a=dyn.next(0); a!=-1; a=dyn.next(a+1))
            if (length[sRule[a]]>sPos[a] && els(rule[sRule[a]][sPos[a]+1])==0 ){
                c = rule[sRule[a]][sPos[a]+1];
                els.setbit(c);
                for( b=0; b<rules; b++)
                    if (rule[b][0]==c && dyn(index[b]) == 0){
                        added = 1; dyn.setbit(index[b]);
                    }
            }
    }
}

void info::buildV()
{
    int a,b,c,d;
    set dyn(situations),prev(situations),els(symbols),shiftsym(symbols);

    printf("building states ...\n");

    dyn.clear();els.clear();els.setbit(input);
    for( a=0; a<rules; a++) if (rule[a][0] == input) dyn.setbit(index[a]);
    extendSet(dyn,els);
    V.add(&dyn);
    
    if(debuglev){
        checkdebug();
        fprintf(debug,"0: (initial)\n");
        for( d=0; d<situations; d++) if (dyn(d)) print_situation(debug,this,d);
    }

    states = 0;
    chstate=(int **)malloc(100*sizeof(int *));
    chstate[states++] = new int [symbols+2];
    for( d=0; d<symbols+2; d++) chstate[0][d]=-1;

    for( a=0; a<states; a++){
        prev = *V.getn(a);

        shiftsym.clear();
        for( c=prev.next(0); c!=-1; c=prev.next(c+1))
            if (length[sRule[c]]>sPos[c]) shiftsym.setbit(rule[sRule[c]][sPos[c]+1]);
        
        for( b=shiftsym.next(0); b!=-1; b=shiftsym.next(b+1)){
            dyn.clear();els.clear();

            for( c=prev.next(0); c!=-1; c=prev.next(c+1)){
                if (length[sRule[c]]>sPos[c] && rule[sRule[c]][sPos[c]+1] == b )
                    dyn.setbit(c+1);
            }

            if (!dyn.empty()){
                extendSet(dyn,els);
                c = V.add(&dyn);
                if (c >= states){
                    if (debuglev){
                        checkdebug();
                        fprintf(debug,"%i: (from %i by %s)\n",c,a,name[b]);
                        for( d=0; d<situations; d++) if (dyn(d)) print_situation(debug,this,d);
                    }
                    if ((states%100) == 99){
                        chstate=(int **)realloc(chstate,(states+101)*sizeof(int *));
                    }
                    chstate[states] = new int [symbols+2];
                    for( d=0; d<symbols+2; d++) chstate[states][d]=-1;
                    states++;
                }
            } else c = -1;
            chstate[a][b]=c;
            if (c!=-1){
                if (chstate[a][symbols]==-1) chstate[a][symbols]=b;
                chstate[a][symbols+1]=b;
            }
        }
    }
}

int info::extendAvan(set& dyn,set **s)
{
    int a,b,c,d,e,added=1,ga=0;
    set f(symbols);

    while( added ){
        added=0;
        for( a=dyn.next(0); a!=-1; a=dyn.next(a+1))
            if (length[sRule[a]]>sPos[a]){
                e=1;f.clear();
                for( d=sPos[a]+2; e==1 && d<=length[sRule[a]]; d++ ){
                    if ((symbol[rule[sRule[a]][d]]&EMPTY) == 0) e=0;
                    f.add(first[rule[sRule[a]][d]]);

                }
                if (e) f.add(s[a]);

                c = rule[sRule[a]][sPos[a]+1];
                for( b=0; b<rules; b++)
                    if (rule[b][0]==c){
                        added |= s[index[b]]->add(&f);
                    }
            }
        if (added) ga=1;
    }
    return ga;
}

void info::buildAvan()
{
    int a,b,c,d,added;

    printf("building look-ahead strings ...\n");

    avan = new set **[states];
    for( a=0; a<states; a++ ){
        set *p = V.getn(a);avan[a]=new set *[situations];
        for( b=0; b<situations; b++)
            if ((*p)(b)) avan[a][b]=new set(symbols);else avan[a][b]=NULL;
    }

    for( a=0; a<rules; a++) if (rule[a][0] == input)
        avan[0][index[a]]->setbit(eoi);

    int * used = new int[states];
    for( a=0; a<states; a++) used[a] = 0;
    used[0]=1;

    added = 1;
    while( added ){
        added = 0;

        for( a=0; a<states; a++ ) if (used[a]){
            set * j = V.getn(a);
            b = extendAvan(*j,avan[a]);
            added |= b;            
            
                for( b=j->next(0); b!=-1; b=j->next(b+1) )
                    if (length[sRule[b]]>sPos[b]){
                        c = rule[sRule[b]][sPos[b]+1];
                        d = chstate[a][c];
                        c = avan[d][b+1]->add(avan[a][b]);
                        used[d] |= c;
                        added |= c;
                    }

            used[a]=0;
        }
    }

    if (debuglev==2){
        checkdebug();
        for( a=0; a<states; a++ ){
            set *j = V.getn(a);
            fprintf(debug,"%i:\n",a);
            for( d=j->next(0); d!=-1; d=j->next(d+1)){
                print_situation(debug,this,d);
                fprintf(debug,"  >>");
                for (c=avan[a][d]->next(0);c!=-1;c=avan[a][d]->next(c+1))
                    fprintf(debug," %s",name[c]);
                fprintf(debug,"\n\n");
            }
        }
    }
}

void info::buildLast()
{
    int a,b,c,d,e,f,g,h,x,y,z,sr=0,rr=0,mut;
    set *j;

    printf("building state machine ...\n");
    symtoterm = new int[symbols];
    termtosym = new int[symbols];
    for( a=0,terminals=0; a<symbols; a++)
        if (symbol[a]&TERM){
            termtosym[terminals]=a;
            symtoterm[a]=terminals++;
        } else symtoterm[a]=0;
    chnext = new int *[states];

    for( a=0; a<states; a++ ){
        j = V.getn(a);
        chnext[a]=new int[terminals];
		mut = -2;

        for( f=0; f<terminals; f++ ){
            b = termtosym[f];

            d=-2; g=0;
            for( c=j->next(0); c!=-1; c=j->next(c+1) ) 
                if (length[sRule[c]] > sPos[c] && rule[sRule[c]][sPos[c]+1] == b){
                    d = -1;
                    e = c;
                    break;
            }
            for( c=j->next(0); c!=-1; c=j->next(c+1) ) if (length[sRule[c]] == sPos[c]){

                if ((*avan[a][c])(b)) if (g == 0){
                    if (d >= 0 || d == -1){

                        y = -1; z = -1; g = (d>=0) ? reduce[sRule[e]] : b ;
                        if( reduce[sRule[c]]>=0 && g>=0 ) for( h=0; h<priorities; h++ ){
                            if ((*prior[h].s)(g) && (*prior[h].s)(reduce[sRule[c]])){
                                if (z != -1 && z != prior[h].assoc){
                                    checkerror();
                                    fprintf(err,"priority associativity conflict: `%s` and `%s`\n",name[reduce[sRule[c]]],name[g]);
                                    break;
                                }
                                x = y; y = -1;
                                for( z=0; z<prior[h].count; z++){
                                    if (prior[h].place[z] == g){ y=0;break;}
                                    else if (prior[h].place[z] == reduce[sRule[c]]){ y=1;break;}
                                }
                                if (y == -1 || x >= 0 && x != y){
                                    checkerror();
                                    fprintf(err,"priority definition conflict: `%s` ? `%s`\n",name[reduce[sRule[c]]],name[g]);
                                    y = 0;
                                }
                                z = prior[h].assoc;
                            }
                        }
                        g = 0; if (reduce[sRule[c]] == g) y = 2;

                        if ( d >= 0){
                            if (z == -1 || y == 2){
                                checkerror();
                                fprintf(err,"conflict: reduce/reduce (%i, next %s)\n",a,name[b]);
                                print_situation(err,this,e);
                                print_situation(err,this,c);rr++;
                                g=1;
                            } else if (y == 1){
                                checkerror();
                                fprintf(err,"fixed: reduce/reduce (%i, next %s)\n",a,name[b]);
                                print_situation(err,this,c);
                                print_situation(err,this,e);
                                d = sRule[c];
                                e = c;
                            } else {
                                checkerror();
                                fprintf(err,"fixed: reduce/reduce (%i, next %s)\n",a,name[b]);
                                print_situation(err,this,e);
                                print_situation(err,this,c);
                            }
                        } else {
                            if (z == 0){
                                checkerror();
                                fprintf(err,"fixed: reduce/shift (%i, next %s)\n",a,name[b]);
                                print_situation(err,this,c);
                                print_situation(err,this,e);
                                d = sRule[c];
                                e = c;
                            } else if (z!=1){
                                checkerror();
                                fprintf(err,"conflict: shift/reduce (%i, next %s)\n",a,name[b]);
                                print_situation(err,this,e);
                                print_situation(err,this,c);sr++;
                                g=1;
                            } else {
                                checkerror();
                                fprintf(err,"fixed: shift/reduce (%i, next %s)\n",a,name[b]);
                                print_situation(err,this,e);
                                print_situation(err,this,c);
                            }
                        }
                    } else {
                        d = sRule[c];
                        e = c;
                    }
                } else {
                    fprintf(err,"  reduce:");
                    print_situation(err,this,c);
                }
            }

			if( mut != -3 && symbol[b]&MUTABLE)
				if( mut==-2 ) mut=d;
				else if( d!=mut && d!=-2 ){ checkerror();fprintf(err,"mutable warning (%i): %i and %i\n",a,mut,d);mut=-3;}

            chnext[a][f] = d;
        }
        for( f=0; f<terminals; f++ ){
	        b = termtosym[f];
			if( symbol[b] & MUTABLE ){
				if( mut == -3 ) chnext[a][f] = 0x4000 | chnext[a][f] + 3;
				else chnext[a][f] = mut;
			}
		}
    }

    if (sr+rr) printf("conflicts: %i shift/reduce and %i reduce/reduce\n",sr,rr);
}

void info::checkdebug()
{
    if (!debug) debug=fopen("debug","w");
}

void info::checkerror()
{
    if (!err) err=fopen("errors","w");
}

void info::outTables()
{
    FILE *out;
    int i,e,n,o;

    if ((out=fopen(file_cpp,"w"))){
        fprintf(out,"// %s\n\n#include \"%s\"\n\n",file_cpp,file_h);
                                    
        fprintf(out,"static short chState[]={\n",class_name);
        for (i=0, n=0;i<states;i++){
            o=n;
            if (chstate[i][symbols]!=-1) for (e=chstate[i][symbols]; e<=chstate[i][symbols+1]; e++){
                fprintf(out," %3i,",chstate[i][e]);
                if (++n%12 == 0) fprintf(out,"\n");
            }
            chstate[i][0] = o;
        }
        if (n%12 != 0) fprintf(out,"\n");
        fprintf(out,"};\n\n");

        fprintf(out,"static int chStateI[]={\n",class_name);
        for (i=0,n=0;i<states;i++){
                fprintf(out," %5i, %3i, %3i,  ",chstate[i][0],chstate[i][symbols],chstate[i][symbols+1]);
                if (++n%4 == 0) fprintf(out,"\n ");
            }
        if (n%4 != 0) fprintf(out,"\n");
        fprintf(out,"};\n\n");

        fprintf(out,"static short symtoterm[]={\n",class_name);
        for (i=0,n=0;i<symbols;i++){
                fprintf(out," %3i,",symtoterm[i]);
                if (++n%12 == 0) fprintf(out,"\n");
            }
        if (n%12 != 0) fprintf(out,"\n");
        fprintf(out,"};\n\n");

        fprintf(out,"static short action[]={\n",class_name);
        for (i=0,n=0;i<states;i++)
            for (e=0;e<terminals;e++){
                fprintf(out," %3i,",chnext[i][e]);
                if (++n%12 == 0) fprintf(out,"\n");
            }
        if (n%12 != 0) fprintf(out,"\n");
        fprintf(out,"};\n\n");

        fprintf(out,
            "int %s::gg_update_state(int state,int lexem)\n"
            "{\n"
            "    lexem-=256;\n"
            "    if (lexem >= chStateI[state*3+1] && lexem <= chStateI[state*3+2])\n"
            "        return chState[chStateI[state*3]-chStateI[state*3+1]+lexem];\n\n"
            "    return -1;\n"
            "}\n\n"
            ,class_name
        );

        fprintf(out,
            "int %s::gg_next_state(int state,int lexem)\n"
            "{\n"
            "   return action[state*%i + symtoterm[lexem-256]];\n"
            "}\n\n"
            ,class_name, terminals
        );

        fclose(out);
    } else fprintf(stderr,"error: file `%s` not created\n",file_cpp);
}

int main(int argc,char *argv[])
{
    info *i = new info;
    int e;

    if (argc>1 && strcmp(argv[1],"d")==0) debuglev=1;
    if (argc>1 && strcmp(argv[1],"e")==0) debuglev=2;

    if ((e=readinfo("grammar",i))) switch(e){
        case 1: fprintf(stderr,"error: wrong `grammar` syntax\n");return 0;
        case 2: fprintf(stderr,"error: file `grammar` not found\n");return 0;
    }

    i->First();
    i->Follow();
    i->enumerate();
    i->buildV();
    i->buildAvan();
    i->buildLast();

    i->outTables();
    if (i->err) fclose(i->err);
    if (i->debug) fclose(i->debug);
    return 0;
}
