// color.cpp

#include "stdafx.h"

#define var(c) ((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_')
#define num(c) (c>='0'&&c<='9')

const static char* keywords[] = {
"cl_standart","cl_edit","cl_currentedit","cl_cureditsel","cl_editsel",
"cl_listinactive","cl_listactive",""
};

class errorsCfg {
public: char err[80];
  errorsCfg(const char *l){ strcpy(err,l); }
};

static int getfromint(char *val)
{
  int i = 0;
  while(num(*val)){ i*=10;i+=*val-'0';val++;}
  if (*val) throw errorsCfg("invalid number");
  return i;
}

static CString getfromCString(char *val)
{
  if(*val!='"'||val[1]==0||val[strlen(val)-1]!='"') throw errorsCfg("wrong string");
  val[strlen(val)-1]=0;return CString(val+1);
}

void cl_config::defvar(int varn,char *value)
{
  switch(varn){
   case 0: cl_standart=getfromint(value);break;
   case 1: cl_edit=getfromint(value);break;
   case 2: cl_currentedit=getfromint(value);break;
   case 3: cl_cureditsel=getfromint(value);break;
   case 4: cl_editsel=getfromint(value);break;
   case 5: cl_listinactive=getfromint(value);break;
   case 6: cl_listactive=getfromint(value);break;
  }
}

void cl_config::readconfig(const char *n,const char *section)
{
  FILE *f;
  char s[128],*h,*l;
  DWORD lines = 0,i,e,q=0;

  defaults();
  if ((f=fopen(n,"r"))==NULL){
    sprintf(s,"`%s' not found",n);
    throw errors(s);
  }

  while (fgets(s,128,f)!=NULL){
    lines++;h=s;
    while(*h) if (*h=='#'||*h=='\n') *h=0;else h++;
    h=s;while(*h==' ') h++;
    if (!*h) continue;
    if (*h=='['){ 
      l=h;while(*l) if (*l==']') *l=0;else l++;
      q=!strcmp(h+1,section);continue;
    }
    if(!q) continue;
    l=h;while(*l) l++;l--;
    while(l>h&&*l==' ') *l--=0;
    if (!var(*h)){
      sprintf(s,"%s(%i): wrong line",n,lines);
      throw errors(s);    
    }
    l=h;while(var(*l)) l++;
    if (*l=='=') *l++=0;
    else {
      *l++=0;
      while(*l&&*l!='=') l++;
      if (!*l){
        sprintf(s,"%s(%i): wrong line",n,lines);
        throw errors(s);    
      }
      *l++=0;
    }
    while(*l==' ') l++;
    for(i=e=0;*keywords[i];i++) if(!strcmp(h,keywords[i])){
      try {
      e=1;defvar(i,l);
      }
      catch(errorsCfg& e){
        sprintf(s,"%s(%i): %s",n,lines,e.err);
        throw errors(s);
      }
    }
    if (!e){
      sprintf(s,"%s(%i): unknown parameter `%s'",n,lines,h);
      throw errors(s);
    }
  }
  fclose(f);
}

void cl_config::writeconfig(const char *n,const char *section)
{
  FILE *f,*old=NULL;
  char w[128],*h;

  if(!rename(n,"jMail.tmp"))
    old=fopen("jMail.tmp","r");
  if ((f=fopen(n,"w"))==NULL){
    sprintf(w,"`%s' not created",n);
    throw errors(w);
  }

  if (old){
    while(fgets(w,128,old)!=NULL){
      h=w;while(*h) if (*h=='\n') *h=0;else h++;
      if (*w=='['){ h=w;while(*h) if (*h==']') *h=0;else h++; }
      if (*w=='['&&(!strcmp(w+1,section))){
        while(fgets(w,128,old)!=NULL){
          if (*w=='[') break;*w=0;
        }
        break;
      } else { fprintf(f,"%s\n",w);}
    }
    fprintf(f,"[%s]\n",section);
  } else {
    fprintf(f,"# %s\n\n[%s]\n",n,section);
  }
  
  fprintf(f,"\n");
  fprintf(f,"cl_standart=%i\n",cl_standart);
  fprintf(f,"cl_edit=%i\n",cl_edit);
  fprintf(f,"cl_currentedit=%i\n",cl_currentedit);
  fprintf(f,"cl_cureditsel=%i\n",cl_cureditsel);
  fprintf(f,"cl_editsel=%i\n",cl_editsel);
  fprintf(f,"cl_listinactive=%i\n",cl_listinactive);
  fprintf(f,"cl_listactive=%i\n",cl_listactive);

  if(old){
    fprintf(f,"\n");
    if (*w) fprintf(f,"%s",w);
    while (fgets(w,128,old)!=NULL) fprintf(f,"%s",w);
    fclose(old);
    unlink("jMail.tmp");
  }
  fclose(f);
}

void cl_config::defaults()
{

  cl_standart=7;
  cl_edit=23;
  cl_currentedit=31;
  cl_cureditsel=58;
  cl_editsel=18;
  cl_listinactive=23;
  cl_listactive=55;
}
