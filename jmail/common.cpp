// common.cpp

#include "StdAfx.h"

int checkstring(CString set,CString str,char c)
{
  int i;

  while((i=set.Find(c))>=0){
    if (set.Left(i)==str) return 1;
    set=set.Mid(i+1);
  }
  return (set==str);
}

