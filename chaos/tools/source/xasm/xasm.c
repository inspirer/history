// cc.c

#include "i386.h"
#include "cc.h"

//#define _DEBUG_optimization

// Public
long lines=0,included=0;
char out[fn_size],inf[fn_size];
char include[fn_size];
char line[max_line_size],lntp[max_line_size];
char *_line,*_lntp;
int  optimizing;

// Local
int i,e,k,st;

void main(int argc,char *argv[])
{
  int argi;
  FILE *outf;
  long a,b,c,d,e;
  int optimize=0;

  // basic values
  *out=*inf=*idname=instr=_deep=_cnter=optimizing=0;
  s_del=c_del=ins=pi=NULL;code32=tnum=0;
  strcpy(include,argv[0]);a=strlen(include);
  a--;while(a&&include[a]!='\\') a--;
  if (include[a]=='\\') include[a+1]=0;
  strcat(include,"ch\\");

  // generate index table
  for (a=0;a<256;a++) _index[a]=-1;
  for (a=0;*(i386_optab[a].name)!=0;a++){
    b=*(i386_optab[a].name);
    if (_index[b]==-1) _index[b]=a;
  }
  for (b=0;b<256;b++) if (_index[b]==-1) _index[b]=a;

  // Process params
  for (argi=1;argi<argc;argi++){

    if (*argv[argi]!='/'&&*argv[argi]!='-'){

      // In/Out
      if (*inf==0)
         strcpy(inf,argv[argi]);
      else if (*out==0)
         strcpy(out,argv[argi]);
      else
         error(argv[argi],0);

    } else {

      // switches
      switch(argv[argi][1]){
        case 'x': freopen("xasm.err","wt",stderr);break;
        case 'p': strcpy(include,argv[argi]+2);break;
        case 'w': max_warn=strtol(argv[argi]+2,NULL,0);break;
        case 'e': max_err=strtol(argv[argi]+2,NULL,0);break;
        case 'o': optimize=1;break;
        case '?':
            printf("\nxasm x86/watcom/build of Sa 18-11-2000\n");
            printf("help: use xasm.exe infile outfile [[/switch1] [/switch2] ... ]\n");
            printf(" ?\t\tshow this info\n");
            printf(" p<path>\tset dir with .H files \n");
            printf(" w<num>\t\tset max number of warnings to display\n");
            printf(" e<num>\t\tset max number of errors to display\n");
            printf(" x\t\twrite errors to xasm.err\n");
            printf(" i\t\tgenerate 'xasm.txt'\n");
            printf(" o\t\toptimize code\n\n");
            exit(1);
        case 'i': generate_list();exit(1);
        default: fprintf(stderr,"xasm: fatal: unknown switch '%s'\n",argv[argi]);exit(1);
      }
    }

  }

  if(*inf==0) error("no input",0);
  if(*out==0) error("no output",0);

  InitIdents();
  ifdef_lev=0;ifdef_loc=0;

  // Open input
  i=OpenFile(inf);
  switch(i){
    case 1: fprintf(stderr,"xasm: fatal: memory allocation\n");exit(1);
    case 2: fprintf(stderr,"xasm: fatal: can't read `%s'\n",inf);exit(1);
  }

  fprintf(stderr,"xasm: compiling %s\n",inf);

  // analyze
  st=0;
  do {
    while(ReadFile()){

       if (ch==13){

         // Kill spaces at end
         i=strlen(line);while(i>0&&line[i-1]==' ') i--;
         line[i]=0;

         // Continue in next line?
         if (i>0&&line[i-1]=='\\'){
           line[--i]=0;tfs->lines++;
           while(i>0&&line[i-1]==' ') i--;
           line[i]=0;

         } else {

          _line=line;_lntp=lntp;

          // Create Image
          BuildImage(1);

          // Kill comments
          e=strlen(_line);k=0;for (i=0;i<e;i++){
            if(_lntp[i]!='c'){
              _line[k]=_line[i];
              _lntp[k++]=_lntp[i];
            }
          } _line[k]=0;_lntp[k]=0;

          // Kill spaces
          while(*_line==' '){ _line++;_lntp++;}

          // Process directives
          if (!directive()){

            // no directive => Kill ' ' & ';'
            while(*_line==' '||*_line==';'){ _line++;_lntp++;}

            process();
          }
          tfs->lines++;
          *line=0;
         }

       } else if (ch!=10){
         i=strlen(line);
         line[i]=ch;line[i+1]=0;
         if (i>=max_line_size) error("line is too big",efatal);
       }
    }
    lines=tfs->lines;
    included+=lines;
  } while(CloseFile());

  included-=lines;
  if (ifdef_loc) error("Unexpected end of conditional",eerror);
  if (_deep) error("Unexpected '}'",eerror);
  _line=line;strcpy(line,"nop");process();instr--;

  // link code
  if (!errors){

     // if need => optimize code
     if (optimize){

       fprintf(stderr,"xasm: optimizing\n");optimizing=1; // for process()

       pi=ins;_pi=NULL;b=instr;
       for (a=0;a<b;a++){

         if (pi->cmdnum>=0){ // not xcode/org/dw/db/dd etc. => kill it ?

           for (c=0;*(opti[c].command);c++) if (strcmp(pi->command,opti[c].command)==0&&pi->pnum==opti[c].args){

             e=1;for (d=0;d<pi->pnum;d++){
               if (!(pi->ptype[d]&opti[c].t[d])) e=0;
               if (e&&*(opti[c].a[d])){
                  if (strcmp(opti[c].a[d],pi->params[d])!=0) e=0;
               }
             }
             if(e){
               #ifdef _DEBUG_optimization
                 fprintf(stderr,"#optimizer:(%s)%s(%i,%i):",opti[c].change,pi->command,pi->pnum,c);
                 for (d=0;d<pi->pnum;d++) fprintf(stderr,"%s(%s):",pi->params[d],opti[c].a[d]);
                 fprintf(stderr,"\n");
               #endif

               // move this instruction to deleted list
               instr--;
               if (_pi!=NULL) _pi->next=pi->next;
                         else ins=pi->next;
               if (s_del==NULL){
                 s_del=c_del=pi;c_del->next=NULL;
               } else {
                 pi->next=c_del;s_del=c_del=pi;
               }
               c_del->from=_pi->next;
               pi=_pi;

               // if new instruction present => insert it
               if (*(opti[c].change)!=0){

                 // save queue
                 _pi=pi->next;pi->next=NULL;

                 // call
                 _line=line;_lntp=opti[c].change;st=0;
                 while(*_lntp)
                   if (*_lntp!='%')
                     *_line++=*_lntp++;
                   else {
                     d=*(++_lntp)-'0';_lntp++;
                     strcpy(_line,c_del->params[d]);
                     _line+=strlen(c_del->params[d]);
                   }
                 *_line=0;

                 // compile new opcode
                 #ifdef _DEBUG_optimization
                   fprintf(stderr,"#%s\n;",line);
                 #endif
                 _line=line;_lntp=lntp;process();

                 // restore queue
                 pi->next=_pi;

               }
               break;
             }
           }

         }

         _pi=pi;pi=pi->next;
       }
       optimizing=0;
     }

     // link code
     fprintf(stderr,"xasm: linking %s\n",out);
     for (tnum=0;tnum<2;tnum++){

       pi=ins;if (pi) pi->offs=0;code32=1;
       for (a=0;a<instr;a++){

         if (pi->cmdnum<0){ // xcode/org/dw/db/dd etc.

           if (pi->command[0]=='o'){

             // org
             pi->offs=strtoul(pi->params[0],NULL,10);
             pi->size=0;

           } else if (pi->command[0]=='x'){

             // code 16/32 (xcode 0(1))
             code32=strtoul(pi->params[0],NULL,10);
             pi->size=0;

           } else if (!tnum){

             // calculate size of it
             pi->size=0;
             pi->size=defdata(pi,NULL);

           }
         } else {

           pi->size=0;
           if (!i386(pi)) pi->size=0;
         }
         if (pi->next) pi->next->offs=pi->offs+pi->size;

         pi=pi->next;
       }
       if (errors) break;

       // fix deleted list
       c_del=s_del;
       while(c_del!=NULL){
         c_del->offs=c_del->from->offs;
         c_del=c_del->next;
       }
     }

     // if linking OK
     if (!errors){

       // save data to file
       pi=ins;
       if ((outf=fopen(out,"wb"))!=NULL){

         b=0;
         for (a=0;a<instr;a++){
           if (pi->cmdnum<0){ // xcode/org/dw/db/dd etc.

             if (pi->command[0]!='o'&&pi->command[0]!='x')
                b+=defdata(pi,outf);

           } else {
             b+=fwrite(pi->code,1,pi->size,outf);
           }

           pi=pi->next;
         }
         fclose(outf);
       } else error("output creating error",0);
     }
  }

  // Show info
  fprintf(stderr,"%s: %i line(s), %i included, %i warning(s), %i error(s)\n",inf,lines,included,warnings,errors);
  if (errors){ unlink(out);exit(1);fprintf(stderr,"\n");}
        else fprintf(stderr,"code size: %i byte(s)\n",b);
}
