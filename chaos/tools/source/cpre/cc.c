// cc.c

#include "cc.h";

// Public
long lines=0,included=0;
char out[file_name_size],inf[file_name_size];
char parent_dir[file_name_size];
char line[max_line_size],lntp[max_line_size];
char *_line,*_lntp;

// Local
int i,e,k,st;

void main(int argc,char *argv[])
{
  int argi,a;
  out[0]=inf[0]=0;

  strcpy(parent_dir,argv[0]);a=strlen(parent_dir);
  a--;while(a&&parent_dir[a]!='\\') a--;
  if (parent_dir[a]=='\\') parent_dir[a+1]=0;
  strcat(parent_dir,"h\\");

  // Process params
  for (argi=1;argi<argc;argi++){

    if (*(argv[argi])!='/'&&*(argv[argi])!='-'){

      // In/Out
      if (inf[0]==0)
         strcpy(inf,argv[argi]);
      else if (out[0]==0)
         strcpy(out,argv[argi]);
      else
         error(argv[argi],0);

    } else {

      // switches
      switch(argv[argi][1]){
        case 'p': strcpy(parent_dir,argv[argi]+2);break;
        case 'w': max_warn=strtol(argv[argi]+3,NULL,0);break;
        case 'e': max_err=strtol(argv[argi]+3,NULL,0);break;
        case '?': printf("\ncpre x86/watcom/build of Su 15-10-2000\n");
                  printf("help: use cpre.exe infile outfile [[/switch1] [/switch2] ... ]\n");
                  printf(" ?\t\tshow this info\n");
                  printf(" p<path>\tset dir with .H files\n");
                  printf(" w<num>\t\tset max number of warnings to display\n");
                  printf(" e<num>\t\tset max number of errors to display\n\n");
                  exit(1);
        default:  printf("cpre: fatal: unknown switch '%c'",argv[argi][1]);exit(1);
      }
    }

  }

  if(inf[0]==0) error("no input",0);
  if(out[0]==0) error("no output",0);

  InitIdents();
  ifdef_lev=0;

  // Open input
  i=OpenFile(inf);
  switch(i){
    case 1: fprintf(stderr,"fatal: memory allocation\n");exit(1);
    case 2: fprintf(stderr,"fatal: can't read `%s'\n",inf);exit(1);
  }

  // Create output
  if (freopen(out,"w",stdout)==NULL){
     fprintf(stderr,"fatal: can't write `%s'\n",out);
     exit(1);
  }

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
           while(i>0&&line[i-1]==' ') line[--i]=0;

         } else {

          _line=line;_lntp=lntp;

          // Create Image
          BuildImage(1);

          // Kill comments
          e=strlen(_line);k=0;for (i=0;i<e;i++){
            if(_lntp[i]!='c'){
              _line[k]=_line[i];
              _lntp[k]=_lntp[i];
              k++;
            }
          } _line[k]=0;_lntp[k]=0;

          // Kill spaces
          while(_line[0]==' '){ _line++;_lntp++;}

          // Process directives
          if (!directive()){

            // no directive => Kill ' ' & ';'
            while(_line[0]==' '||_line[0]==';'){ _line++;_lntp++;}

            process();
          }
          tfs->lines++;
          line[0]=0;
         }

       } else if (ch!=10){
         i=strlen(line);
         line[i]=ch;line[i+1]=0;
         if (i>=max_line_size) error("line is too big",3);
       }
    }
    lines=tfs->lines;
    included+=lines;
  } while(CloseFile());

  if (ifdef_loc) error("Unexpected end of conditional",1);
  if (deep) error("Compound statement missing `}'",1);
  ShowTotalInfo();
}
