#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#define TREE_debug
//#define MAKE_debug

struct TAct
{
  char *action;
  struct TAct *next;
  long line;
} *w=NULL;

struct TMStr
{
  char *source,*dest;
  struct TAct *act;
  struct TMStr *next;
  long line;
} *targ=NULL,*v=NULL;

int linkfile=0,make_all=0;

void makelink()
{
  if (freopen("makexasm","wt",stdout)==NULL){
    fprintf(stderr,"cmake: error creating 'makexasm'\n");
    exit(0);
  }
  printf("#code 32\n");printf(" db \"CE\",26\n");
  printf(" db [9]\n");printf(" dw 4\n");
  printf(" dw (pend/4096 + 1)\n");printf("#org 0\n");
  printf(" call _main\n");printf(" mov ah,0x21\n int 0x20\n\n");
  linkfile=1;
}

int make(char *name)
{
  char s[128],*p,*u,*q;
  FILE *tmp;
  int a;
  struct TAct *w;
  struct TMStr *v;

  v=targ;
  while(v!=NULL){
    if (!strcmp(name,v->dest)){
      p=v->source;

      // create source
      if (p!=NULL) while(*p){
        u=s;while(*p&&*p!=' ') *u++=*p++;
        while(*p==' ') p++;*u=0;

        if (!make(s)) return 0;
      }

      // make dest from source
      w=v->act;
      while(w!=NULL){
        p=w->action;
        if (*p=='$'){
          p++;while(*p==' ') p++;

          // process internal cmake command
          if (strncmp(p,"remove ",7)==0){
            p+=7;while(*p==' ') p++;
            unlink(p);fprintf(stderr,"cmake: removed: '%s'\n",p);

          } else if (strncmp(p,"write ",6)==0){
            p+=6;while(*p==' ') p++;
            fprintf(stderr,"cmake: %s\n",p);

          } else if (strcmp(p,"add")==0){

            if (linkfile==2) fprintf(stderr,"cmake: ERROR: $add cannot be used in link process\n");
            else {
              if (!linkfile) makelink();
              printf("#include \"%s\"\n",v->dest);
            }
            unlink("makeincl");

          } else if (strcmp(p,"fadd")==0){

            if (linkfile==2) fprintf(stderr,"cmake: ERROR: $fadd cannot be used in link process\n");
            else {
              if (!linkfile) makelink();
              printf("#include \"%s\"\n",v->dest);
              freopen("makeincl","rt",stdin);
              if (stdin!=NULL){
                while(gets(s)!=NULL){
                  a=strlen(s)-1;while(s[a]==13||s[a]==10||s[a]==' '){ s[a]=0;a--;}
                  printf("%s\n",s);
                }
                fclose(stdin);
              }
            }
            unlink("makeincl");

          } else if (strcmp(p,"link")==0){
            if (linkfile!=1) fprintf(stderr,"cmake: ERROR: link must be used after $(f)add");
            else {
              printf("\npend:nop\n");
              fflush(stdout);
              linkfile++;
            }

          } else if (strcmp(p,"end")==0){
            if (linkfile!=2) fprintf(stderr,"cmake: ERROR: end of link must be used after link");
            else {
              fclose(stdout);unlink("makexasm");linkfile=0;
            }

          } else fprintf(stderr,"cmake: unknown command: '%s'\n",p);

        } else {
          fprintf(stderr,"cmake: %s\n",p);
          system(p);
        }
        w=w->next;
      }

      if (!strcmp(v->dest,"all")) return 1;
      if ((tmp=fopen(v->dest,"rb"))!=NULL){ fclose(tmp);return 1;}
      fprintf(stderr,"cmake: '%s' not received\n",v->dest);
      return 0;
    }
    v=v->next;
  }

  if ((tmp=fopen(name,"rb"))!=NULL){ fclose(tmp);return 1;}
  fprintf(stderr,"cmake: result does not exist and cannot be made from existing files\n");
  return 0;
}

void main(int argc,char *argv[])
{
  long a,b,c,lines=0;
  char mfile[128]="makefile";
  char s[256],*p,*u,*q;

  // process cmdline
  for (a=1;a<argc;a++){
    p=argv[a];if(*p!='-'&&*p!='/') strcpy(mfile,p);
    else {
      p++;
      switch(*p){
        case 'a': make_all=1;break;
        case '?': printf("\ncmake x86/watcom/build of Su 15-10-2000\n");
                  printf("help: use cmake.exe [makefile] [[/switch1] [/switch2] ... ]\n");
                  printf(" ?\t\tshow this info\n");
                  printf(" a\t\tbuild all\n\n");
                  exit(0);break;
        default: fprintf(stderr,"cmake: unknown switch '/%c'",*p);break;
      }
    }
  }

  // open makefile in stdin
  if (freopen(mfile,"rb",stdin)==NULL){
    fprintf(stderr,"cmake: no targets (in %s)\n",mfile);
    exit(0);
  }

  // read input and build tree
  while(gets(s)!=NULL){
    a=strlen(s)-1;p=s;lines++;strlwr(s);
    while(s[a]==13||s[a]==10||s[a]==' '){ s[a]=0;a--;}
    while(*p==' ') p++;

    // new TMStr ?
    u=p;while(*u!=0&&*u!=' '&&*u!=':') u++;q=u;while(*u==' ') u++;
    if (*u==':'){

      // yes
      *q=0;u++;while(*u==' ') u++;

      // allocate space for TMStr
      if (v!=NULL){
        v->next=(void *)malloc(sizeof(*v));
        v=v->next;
      } else {
        v=(void *)malloc(sizeof(*v));
        targ=v;
      }
      if (v==NULL){
        fprintf(stderr,"cmake: not enough memory\n");
        exit(0);
      }

      a=strlen(u);a++;v->source=(char *)malloc(a);
      a=strlen(p);a++;v->dest=(char *)malloc(a);
      if (v->source==NULL||v->dest==NULL){
        fprintf(stderr,"cmake: not enough memory\n");
        exit(0);
      }
      strcpy(v->dest,p);strcpy(v->source,u);
      v->line=lines;w=NULL;v->next=NULL;v->act=NULL;

    } else if(*p){

      // no: get action -> allocate space for TAct
      if (w!=NULL){
        w->next=(void *)malloc(sizeof(*w));
        w=w->next;
      } else {
        w=(void *)malloc(sizeof(*w));
        v->act=w;
      }
      if (w==NULL){
        fprintf(stderr,"cmake: not enough memory\n");
        exit(0);
      }

      a=strlen(p);a++;w->action=(char *)malloc(a);
      if (w->action==NULL){
        fprintf(stderr,"cmake: not enough memory\n");
        exit(0);
      }
      strcpy(w->action,p);w->line=lines;w->next=NULL;
    }
  }

  make("all");



  #ifdef TREE_debug
    v=targ;
    while(v!=NULL){
      fprintf(stderr,"!(%i): (%s)->(%s)\n",v->line,v->source,v->dest);
      w=v->act;
      while(w!=NULL){
        fprintf(stderr," (%i): (%s)\n",w->line,w->action);
        w=w->next;
      }
      v=v->next;
    }
  #endif
}
