// node.cpp (CTimeInterval, CJTime, CNode)

#include "StdAfx.h"

static int dweek[]= { 0, 1, 2, 4, 8, 16, 32, 64 };

int CTimeInterval::isIN(CTime& t)
{
  if (day)
    if ((day&dweek[t.GetDayOfWeek()])==0) return 0;

  int tM = t.GetHour()*24*60+t.GetMinute()*60+t.GetSecond();
  if(tM>=start&&tM<=end) return 1;
  return 0;
}

int CJTime::isIN(CTime& w)
{
  for (int i=0;i<t.size();i++)
    if (t[i].isIN(w)) return 1;

  return 0;
}

int checktime(CString& t)
{
  return 1;
}
