// remote.cpp (CConnect, CAddress)

#include "StdAfx.h"

// CConnect

CConnect::load(CString s)
{
  CString q=g->v.error_correction;
  int i;

  baud=correction=compression=0;
  sscanf(s,"CONNECT %d",&baud);
  if (q.GetLength()){
    while((i=q.Find('|'))>=0){
      if (s.Find(q.Left(i))){ correction=1;break;}
      q=q.Mid(i+1);
    } if (s.Find(q)) correction=1;
  }
}

CConnect::operator CString()
{
  CString s;
  s.Format("Baud: %d; Compression: %s; Correction: %s",
    baud,
    compression?"Yes":"No",
    correction?"Yes":"No");
  return s;
}

// CAddress

CAddress::operator=(const CString &s)
{
  type=ADDR_STR;str=s;p=0;
  
  if (sscanf(s,"%d:%d/%d.%d",&z,&n,&f,&p)==4) type=ADDR_FTS;
  else if (sscanf(s,"%d:%d/%d",&z,&n,&f)==3) type=ADDR_FTS;
}

CAddress::operator CString()
{
  if (type==ADDR_STR)
    return str;
  else
  {
    if (p==0)
      str.Format("%d:%d/%d",z,n,f);
    else
      str.Format("%d:%d/%d.%d",z,n,f,p);
    return str;
  }
}

CAddress::operator LPCTSTR()
{
  return (LPCTSTR)((CString)*this);
}

int& CAddress::operator [](int _n)
{
  if ((n>1)&&(n<4)&&(type==ADDR_STR)){ p=f=n=z=0;type=ADDR_FTS;}
  switch (_n){
    case 1: return z;
    case 2: return n;
    case 3: return f;
    case 4: return p;
  }
  throw errors("Bad address index " + CString(_n));
}
