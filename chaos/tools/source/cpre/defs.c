// defs.c

#include "cc.h"

void process()
{
  int Signed;

  if (_line[0]==0||ifdef_lev!=0) return;
  ProcessDefines();

  printf("%s\n",_line);
}
