// log.cpp (CLog, CJFile)

#include "stdafx.h"

void CLog::initlog()
{
  logfile=sopen(g->v.log,O_WRONLY|O_APPEND|O_CREAT|O_TEXT,SH_DENYWR,S_IREAD|S_IWRITE);
  if (logfile==-1)
    throw errors("Unable to open log file "+g->v.log);
  write(logfile,"\r\n",2);
}

void CLog::deinitlog()
{
  close(logfile);
}

CTime& CLog::logstr(CString& str)
{
  static CTime t;
  t=CTime::GetCurrentTime();
  CString s=t.Format("%d/%m %H:%M:%S")+" "+str+"\n";
  write(logfile,(LPCTSTR) (s),strlen(s));
  return t;
}

// CJFile (tx/rx file state)

int CJFile::OpenFileForWrite()
{
  long i;
  _finddata_t fd;

  while((i=fname.Find('\\'))>=0||(i=fname.Find('/'))>=0) fname=fname.Mid(i+1);
  if (!fname.GetLength()) fname=sname;
  realName = g->v.inbound+fname;
  
  i = _findfirst(realName,&fd);
  if (i!=-1) _findclose(i);
  if (i!=-1&&fd.size==Filesize){ Offset=-1;return 0;}

  f=CreateFile(realName,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,
     FILE_ATTRIBUTE_HIDDEN,NULL);
  if (f==INVALID_HANDLE_VALUE){ f=NULL;Offset=-2;return 0;}
  i=0;Offset=SetFilePointer(f,0,&i,FILE_END);

  return 1;
}

int CJFile::OpenFileForRead()
{
  _finddata_t fd;
  char sfn[128];
  long i;

  fname=realName;
  while((i=fname.Find('\\'))>=0||(i=fname.Find('/'))>=0) fname=fname.Mid(i+1);

  f=CreateFile(realName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
     0,NULL);
  if (f==INVALID_HANDLE_VALUE){ f=NULL;return 0;}
  i=0;Filesize=SetFilePointer(f,0,&i,FILE_END);

  i = _findfirst(realName,&fd);
  if (i!=-1) _findclose(i),Timestamp=fd.time_create;else Timestamp=0;

  i=GetShortPathName(realName,sfn,128);
  if (i==0||i>127) return 0;else sname=sfn;
  while((i=sname.Find('\\'))>=0||(i=sname.Find('/'))>=0) sname=sname.Mid(i+1);
  i=0;Offset=SetFilePointer(f,0,&i,FILE_BEGIN);

  return 1;
}

void CJFile::FileClose(int ok)
{
  if (f){
    CloseHandle(f);
    if (ok) SetFileAttributes(realName,FILE_ATTRIBUTE_NORMAL);
    f=NULL;
  }
}

int CJFile::FileWrite(void *b,int size)
{
  if (f){
    DWORD i;
    WriteFile(f,b,size,&i,NULL);
    return (i==size);
  }
  return 0;
}

int CJFile::FileRead(void *b,int size)
{
  if (f){
    DWORD i;
    ReadFile(f,b,size,&i,NULL);
    return i;
  }
  return 0;
}

int CJFile::FileSeek(ulong pos)
{
  if (f){
    Offset=SetFilePointer(f,pos,NULL,FILE_BEGIN);
    return (Offset!=-1);
  }
  return 0;
}
