// error.c

#include "cc.h"

long max_warn=0,max_err=20;
long warnings=0,errors=0;

void error(char *s,int e_type){
  switch(e_type){
   case 0:
      fprintf(stderr,"fatal: %s\n",s);
      exit(1);

   case 1:
      if (errors<max_err) fprintf(stderr,"%s(%i): error: %s\n",tfs->FileName,tfs->lines,s);
      errors++;
      break;

   case 2:
      if (errors<max_err) fprintf(stderr,"%s(%i): warning: %s\n",tfs->FileName,tfs->lines,s);
      warnings++;
      break;

   case 3:
      errors++;
      fprintf(stderr,"%s(%i): fatal: %s\n",tfs->FileName,tfs->lines,s);
      fprintf(stderr,"%s: %i warning(s), %i error(s)\n",inf,warnings,errors);
      exit(1);
  }
}

void ShowTotalInfo()
{
  included-=lines;
  if (errors||warnings){
    fprintf(stderr,"PRE: %s: %i line(s), %i included, %i warning(s), %i error(s)\n",inf,lines,included,warnings,errors);
    fclose(stdout);
    remove(out);
  }
}
