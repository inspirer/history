// error.c

#include "i386.h"
#include "cc.h"

long max_warn=20,max_err=20;
long warnings=0,errors=0;

void error(char *s,int e_type)
{
  switch(e_type){

   // FATAL
   case 0:
      fprintf(stderr,"xasm: fatal: %s\n",s);
      exit(1);

   // ERROR
   case eerror:
      if (errors<max_err) fprintf(stderr,"%s(%i): error: %s\n",tfs->FileName,tfs->lines,s);
      errors++;
      break;

   // WARNING
   case ewarn:
      if (errors<max_err) fprintf(stderr,"%s(%i): warning: %s\n",tfs->FileName,tfs->lines,s);
      warnings++;
      break;

   case efatal:
      errors++;
      fprintf(stderr,"%s(%i): fatal: %s\n",tfs->FileName,tfs->lines,s);
      fprintf(stderr,"xasm: %s: %i warning(s), %i error(s)\n",inf,warnings,errors);
      exit(1);
   case easm:
      if (errors<max_err) fprintf(stderr,"%s(%i): error: %s\n",pi->fname,pi->sline,s);
      errors++;
      break;
   case easmw:
      if (warnings<max_warn) fprintf(stderr,"%s(%i): warning: %s\n",pi->fname,pi->sline,s);
      warnings++;
      break;
  }
}
