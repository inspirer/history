// ident.c

#include "cc.h"

// Buffer for labels
char labels[label_buffer_sz];
char *lend;

// Buffer for defines, types & etc.
char defines[define_buff_sz];
char *dend;

long lbsz,dfsz,deep;  // label's buffer size, defs buff, deep of brackets

#define _ch  1
#define _nm  2

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
  unsigned int type;
  struct IDENT *next;
  char *name,*value;
} *sid,*sv;

struct IDENTS{
  long total;
  char *lnd;
  struct IDENT *first;
} loc[max_deep];

struct DEFS{
  unsigned int type;
  struct DEFS *next;
  char *name,*value;
} *sdf,start_defs;
long total_defs;

// Prepeare idents system to use
void InitIdents()
{
  lend=labels;lbsz=0;dfsz=0;dend=defines;
  deep=0;loc[0].total=0;loc[0].lnd=lend;
  total_defs=0;
}

// deep level ++
void IncDeep()
{
  deep++;if (deep>max_deep) error("too much brackets `{'",3);
  loc[deep].total=0;loc[0].lnd=lend;
}

// deep level --
void DecDeep()
{
  int i,e;

  sid=loc[deep].first;e=loc[deep].total;
  for (i=0;i<e;i++){
    sv=sid->next;
    free(sid);
    sid=sv;
  }
  lend=loc[deep].lnd;
  if (deep==0) error("too much brackets `}'",3);
  deep--;
}

// check chars
void CheckIdent(char *idnt)
{
  int i,e,k;

  e=strlen(idnt);
  for (i=0;i<e;i++){
    k=ident_char[idnt[i]];
    if(k==0){error("illegal character in ident",1);return;}
    if(k==_nm&&i==0){error("illegal first character of label",1);return;}
  }
}

// add define { 0-def ; 1-fdef }
void AddDefine(char *name,char *value,char tpe)
{
  long i,e;

  i=strlen(name)+strlen(value)+2+dfsz;
  if (i>define_buff_sz) error("defines buffer overflow",3);

  if (tpe==0)
    CheckIdent(name);

  if (CheckDefine(name)){ error("already defined",1);return; }

  sdf=&start_defs;
  for (i=1;i<total_defs;i++) sdf=sdf->next;
  if (total_defs!=0){
    sdf->next=(void *)malloc(sizeof(*sdf));sdf=sdf->next;
    if (sdf==NULL) error("memory allocation error",3);
  }
  total_defs++;sdf->type=tpe;

  sdf->name=dend;strcpy(dend,name);i=strlen(name);i++;dend+=i;dfsz+=i;
  sdf->value=dend;strcpy(dend,value);i=strlen(value);i++;dend+=i;dfsz+=i;

  //DEBUG: printf("%s=%s;\n",sdf->name,sdf->value);
}

// return true if dname defined
int CheckDefine(char *dname)
{
  int i,e;

  sdf=&start_defs;
  for (i=0;i<total_defs;i++){

    if (strcmp(sdf->name,dname)==0) return 1;
    sdf=sdf->next;
  }
  return 0;
}

char s[max_line_size];

// in >_line, _lntp<
void ProcessDefines()
{
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
    } else {
      line[k]=line[i];k++;
    }
  } line[k]=0;
  _line=line;_lntp=lntp;BuildImage(0);
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
          case '[': case ']': case ';': case ':': case '?':
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
    case 1: error("Ending "" missing for string literal",1);break;
    case 2: error("Invalid character constant",1);break;
    case 3: st=0;break;
  }
}
