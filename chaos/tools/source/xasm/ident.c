// ident.c

//#define _DEBUG_addlabel
//#define _DEBUG_getident

#include "i386.h"
#include "cc.h"

char ident_char[256]={
0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
_nm, _nm, _nm, _nm, _nm, _nm, _nm, _nm, _nm, _nm, 0,   0,   0,   0,   0,   0,
0,   _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch,
_ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, 0,   0,   0,   0,   _ch,
0,   _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch,
_ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, _ch, 0,   0,   0,   _ch, 0
};

struct IDENT{
  char *name;
  long def_area;
  struct TInstruction *iptr;
  struct IDENT *next;
} start_ident,*pid;
long total_idents,def_area,xarea;

struct DEFS{
  unsigned int type;
  struct DEFS *next;
  char *name,*value;
  long line;
} *sdf,start_defs;
long total_defs;

// Prepeare idents system to use
void InitIdents()
{
  total_idents=0;total_defs=0;def_area=0;xarea=0;
}

// check chars
int CheckIdent(char *idnt)
{
  int i,e,k;

  e=strlen(idnt);
  for (i=0;i<e;i++){
    k=ident_char[idnt[i]];
    if(k==0){error("illegal character in ident",efatal);return 0;}
    if(k==_nm&&i==0){error("illegal first character of label",efatal);return 0;}
  }

  return 1;
}

// add ident
void AddIdent(char *name,struct TInstruction *inst)
{
  long i;

  #ifdef _DEBUG_addlabel
    fprintf(stderr,"#aident:%s;\n",name);
  #endif

  if (!CheckIdent(name)) return;

  pid=&start_ident;
  for (i=1;i<total_idents;i++) pid=pid->next;
  if (total_idents){
    pid->next=(void *)malloc(sizeof(*pid));pid=pid->next;
    if (pid==NULL) error("memory allocation error",efatal);
  }
  total_idents++;
  i=strlen(name);i++;pid->name=(char *)malloc(i);pid->next=NULL;
  if (pid->name==NULL) error("memory allocation error",efatal);
  strcpy(pid->name,name);pid->def_area=def_area;pid->iptr=inst;
}

// get ident
unsigned long GetIdent(char *lname)
{
  long i;

  pid=&start_ident;
  if (tnum){
    for (i=0;i<total_idents;i++){
      if (strcmp(lname,pid->name)==0){
        #ifdef _DEBUG_getident
          fprintf(stderr,"#gident:%s=%i;\n",lname,pid->iptr->offs);
        #endif
        return pid->iptr->offs;
      }
      pid=pid->next;
    }
    if (tnum) error("unknown ident",easm);
  }
  return 0;
}

// return true if dname defined
int CheckDefine(char *dname)
{
  int i,e;

  sdf=&start_defs;
  for (i=0;i<total_defs;i++){

    if (strcmp(sdf->name,dname)==0)return 1;
    sdf=sdf->next;
  }
  return 0;
}

// add define { 0-def ; 1-fdef }
void AddDefine(char *name,char *value,char tpe)
{
  long i,e;

  if (!tpe) if (!CheckIdent(name)) return;
  if (CheckDefine(name)){ error("already defined",eerror);return; }

  sdf=&start_defs;
  for (i=1;i<total_defs;i++) sdf=sdf->next;
  if (total_defs){
    sdf->next=(void *)malloc(sizeof(*sdf));sdf=sdf->next;
    if (sdf==NULL) error("memory allocation error",efatal);
  }
  total_defs++;sdf->type=tpe;

  i=strlen(name);i++;sdf->name=(char *)malloc(i);
  i=strlen(value);i++;sdf->value=(char *)malloc(i);
  if (sdf->value==NULL||sdf->name==NULL) error("memory allocation error",efatal);
  strcpy(sdf->value,value);strcpy(sdf->name,name);

 #ifdef _DEBUG
   printf("#def:%s=%s;\n",sdf->name,sdf->value);
 #endif
}

// Free defined variable
int FreeDefine(char *dname)
{
  int i,e;

  sdf=&start_defs;
  for (i=0;i<total_defs;i++){

    if (strcmp(sdf->name,dname)==0){ sdf->name[0]=' ';return 1; }
    sdf=sdf->next;
  }
  return 0;
}

// image for _line in _lntp ( i - ident ; l - letter ; s - str ; c - comment)
void BuildImage(int use_st)
{
  int i,e,k;

  // Create image
  e=strlen(_line);_lntp[e]=0;if (use_st) k=st; else k=0;
  for (i=0;i<e;i++){
    switch(k){

     case 0:
        if (_line[i]=='"'){_lntp[i]='s';k=1;}
        else if (_line[i]==39){_lntp[i]='l';k=2;}
        else if (_line[i]=='/'&&_line[i+1]=='/'){_lntp[i]='c';k=3;}
        else if (_line[i]=='/'&&_line[i+1]=='*'){_lntp[i]='c';k=4;}
        else switch(_line[i]){
          case '<': case '>': case ' ': case '=': case '!': case '`': case '~':
          case '(': case ')': case '|': case '+': case '-': case '/': case '.':
          case '{': case '}': case '*': case '^': case ',': case '&': case '#':
          case '[': case ']': case ';': case ':': case '?': case '%':
            _lntp[i]=_line[i];break;

          default:
            _lntp[i]='i';break;
        }
        break;

     case 1:
        _lntp[i]='s';
        if (_line[i]=='"')k=0;
        break;

     case 2:
        _lntp[i]='l';
        if (_line[i]==39) k=0;
        break;

     case 3:
        _lntp[i]='c';
        break;

     case 4:
        _lntp[i]='c';
        if (_line[i-1]=='*'&&_line[i]=='/') k=0;
        break;
    }
  } if (use_st) st=k;

  // check status
  switch(st){
    case 1: error("Ending "" missing for string literal",eerror);break;
    case 2: error("Invalid character constant",eerror);break;
    case 3: st=0;break;
  }
}

// in >_line, _lntp<
void ProcessDefines()
{
  char s[max_line_size];
  int i,e,k,p,u;

  strcpy(s,_line);strcpy(line,s);
  _line=line;_lntp=lntp;BuildImage(0);

  k=strlen(line);p=0;
  for (e=0;e<=k;e++){

    if(p==0){
       if (ident_char[line[e]]==_ch&&(e==0||ident_char[line[e-1]]==0)&&lntp[e]=='i')
         {u=e;p=1;}
    } else {
       if (ident_char[line[e]]==0){
         p=line[e];line[e]=0;

         sdf=&start_defs;
         for (i=0;i<total_defs;i++){

            if (strcmp(sdf->name,line+u)==0){
               i=-1;break;
            }
            sdf=sdf->next;

         }
         line[e]=p;
         if(i==-1){
           strcpy(s,line+e);
           strcpy(line+u,sdf->value);
           strcat(line,s);k=strlen(line);
           e=u-1;BuildImage(0);
         }p=0;
       }
    }
  }

  // merging token strings
  k=0;e=strlen(line);
  for (i=0;i<e;i++){

    if (line[i]=='#'&&line[i+1]=='#'){
      i++;while(line[i+1]==' ') i++;
      while(k>0&&line[k-1]==' ') k--;

    } else line[k++]=line[i];

  } line[k]=0;
  _line=line;_lntp=lntp;
}
