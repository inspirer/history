// window.cpp (CCWindow, CCMainFrame)

#include "stdafx.h"

#define InfoHeight 7
#define QueueWidth 35
#define ResultHeight 10
#define ResultWidth 23

// CCWindow

void CCWindow::show(int dx,int dy,CString& w,int x,int y,int refr)
{
  COORD pos={0,0};
  DWORD i,e,xs,ys,k;

  caption=w;shadow=1;
  size.X=dx+2;size.Y=dy+2;
  GetConsoleCursorInfo(cn->conout,&cci);
  GetConsoleScreenBufferInfo(cn->conout,&cn->csb);
  prev_cursor=cn->csb.dwCursorPosition;

  if (x<0){ sr.Left=(cn->csb.dwSize.X-size.X)/2;if (sr.Left<0) sr.Left=0;} else sr.Left=x;
  if (y<0){ sr.Top=(cn->csb.dwSize.Y-size.Y)/2;if (sr.Top<0) sr.Top=0;} else sr.Top=y;
  sr.Right=sr.Left+size.X;sr.Bottom=sr.Top+size.Y;
  if (sr.Right>=cn->csb.dwSize.X-1) sr.Right=cn->csb.dwSize.X-1,shadow=0;
  if (sr.Bottom>=cn->csb.dwSize.Y-1) sr.Bottom=cn->csb.dwSize.Y-1,shadow=0;
  xs=sr.Right-sr.Left+1;ys=sr.Bottom-sr.Top+1;
  if (shadow) sr.Bottom++,sr.Right++;
  size.X=sr.Right-sr.Left+1;size.Y=sr.Bottom-sr.Top+1;

  p=new CHAR_INFO[size.X*size.Y];v=new CHAR_INFO[size.X*size.Y];
  ReadConsoleOutput(cn->conout,p,size,pos,&sr);

  e=size.X*size.Y;
  for(i=0;i<e;i++) v[i].Attributes=7,v[i].Char.AsciiChar=' ';

  v[0].Char.AsciiChar=(char)218;v[xs-1].Char.AsciiChar=(char)191;
  v[size.X*(ys-1)].Char.AsciiChar=(char)192;
  v[size.X*(ys-1)+xs-1].Char.AsciiChar=(char)217;
  for (i=1;i<xs-1;i++) 
    v[i].Char.AsciiChar=(char)196,v[size.X*(ys-1)+i].Char.AsciiChar=(char)196;
  for (i=1;i<ys-1;i++)
    v[size.X*i].Char.AsciiChar=(char)179,v[size.X*i+xs-1].Char.AsciiChar=(char)179;
  if (shadow){
    for (i=2;i<xs+1;i++)
      v[size.X*(ys)+i].Char.AsciiChar=(char)176;
    for (i=2;i<ys;i++)
      v[size.X*i+xs].Char.AsciiChar=(char)176;
  }
  client.X=xs-2;client.Y=ys-2;
  if (w.GetLength()>dx-10) w=w.Left(dx-10);i=w.GetLength();
  v[2].Char.AsciiChar='[';v[3].Char.AsciiChar=' ';k=4;
  for(e=0;e<i;e++,k++) v[k].Char.AsciiChar=w[(int)e],v[k].Attributes=3;
  v[k++].Char.AsciiChar=' ';v[k++].Char.AsciiChar=']';

  if (refr) WriteConsoleOutput(cn->conout,v,size,pos,&sr);
}

void CCWindow::hide()
{
  COORD pos={0,0};

  WriteConsoleOutput(cn->conout,p,size,pos,&sr);
  delete[] p;delete[] v;
  SetConsoleCursorPosition(cn->conout,prev_cursor);
  SetConsoleCursorInfo(cn->conout,&cci);
}

void CCWindow::clientgotoxy(int x,int y)
{
  static COORD q;q.X=x+1+sr.Left;q.Y=y+1+sr.Top;
  SetConsoleCursorPosition(cn->conout,q);
}

void CCWindow::clientcursorstate(int show,int size)
{
  CONSOLE_CURSOR_INFO cci;

  cci.bVisible=show;
  cci.dwSize=size;
  SetConsoleCursorInfo(cn->conout,&cci);
}


void CCWindow::clientwritexy(int x, int y, const char *s,int color,int refr,int width)
{
  int i,e,k;
  SMALL_RECT ps;
  
  if(x<0||y<0||x>=client.X||y>=client.Y) return;
  i=client.X-x;if (strlen(s)<i) i=strlen(s);
  k=x+1+(y+1)*size.X;
  if (width){
		for(e=0;e<width&&e<i;e++,k++) v[k].Char.AsciiChar=s[e],v[k].Attributes=color;
		for(;e<width;e++,k++) v[k].Char.AsciiChar=' ',v[k].Attributes=color;
  } else {
    for(e=0;e<i;e++,k++) v[k].Char.AsciiChar=s[e],v[k].Attributes=color;
  }
  if (refr&&e>0){
    ps.Left=x+1;ps.Right=x+e;ps.Top=ps.Bottom=y+1;
    refresh(ps);
  }
}

void CCWindow::refresh(SMALL_RECT &y)
{
  SMALL_RECT ps=y;
  COORD pos;

  pos.X=y.Left;pos.Y=y.Top;
  ps.Left+=sr.Left;ps.Top+=sr.Top;ps.Right+=sr.Left;ps.Bottom+=sr.Top;
  WriteConsoleOutput(cn->conout,v,size,pos,&ps);
}

void CCWindow::drawVertical(int x,int y1,int y2,int term)
{
  for(int i=y1;i<=y2;i++)
    v[x+i*size.X].Char.AsciiChar=(char)179,v[x+i*size.X].Attributes=7;
  if (term&1) v[x+y1*size.X].Char.AsciiChar=(char)194;
  if (term&2) v[x+y2*size.X].Char.AsciiChar=(char)193;
}

void CCWindow::drawHorizontal(int x1,int x2,int y,int term)
{
  for(int i=x1;i<=x2;i++)
    v[i+y*size.X].Char.AsciiChar=(char)196,v[i+y*size.X].Attributes=7;
  if (term&1) v[x1+y*size.X].Char.AsciiChar=(char)195;
  if (term&2) v[x2+y*size.X].Char.AsciiChar=(char)180;
}

// CCMainFrame

void CCMainFrame::init(CString& linename)
{
  COORD pos = {0,0};

  show(cn->csb.dwSize.X,cn->csb.dwSize.Y,(CString)jmail+version_mod+"."+linename,-1,-1,0);
  drawVertical(cn->csb.dwSize.X-QueueWidth,0,cn->csb.dwSize.Y-ResultHeight,1);
  drawHorizontal(cn->csb.dwSize.X-QueueWidth,cn->csb.dwSize.X-1,cn->csb.dwSize.Y-ResultHeight,2);
  v[cn->csb.dwSize.X-QueueWidth+size.X*(cn->csb.dwSize.Y-ResultHeight)].Char.AsciiChar=(char)192;
  drawVertical(cn->csb.dwSize.X-ResultWidth,cn->csb.dwSize.Y-ResultHeight+1,cn->csb.dwSize.Y-2,0);
  drawHorizontal(1,cn->csb.dwSize.X-QueueWidth-1,InfoHeight,0);
  WriteConsoleOutput(cn->conout,v,size,pos,&sr);
  refresh(sr);
  conprint((CString)jmail + " " + version + version_mod + "." + linename + build,1,1);
}

void CCMainFrame::deinit()
{
  hide();
}

void CCMainFrame::conresult(int color,const char *s,int refr)
{
  int i,e,k;
  SMALL_RECT ref;

  ref.Bottom=ref.Top=cn->csb.dwSize.Y-2;ref.Right=cn->csb.dwSize.X-2;ref.Left=cn->csb.dwSize.X-ResultWidth+2;
  e=cn->csb.dwSize.X*ref.Top;
  for (i=ref.Left;i<=ref.Right;i++)
    v[i+e].Char.AsciiChar=' ',v[i+e].Attributes=color;
  for (k=0,i=ref.Left;i<=ref.Right&&s[k];i++,k++)
    v[i+e].Char.AsciiChar=s[k];
  if (refr) refresh(ref);
}

void CCMainFrame::conprint(const char *s,int cnew,int _log)
{
  int i,e,k;
  SMALL_RECT ref;
  CString st;
  CTime tm;

  ref.Top=InfoHeight+1;ref.Left=1;ref.Right=cn->csb.dwSize.X-QueueWidth-1;ref.Bottom=cn->csb.dwSize.Y-2;
  k=cn->csb.dwSize.X;
  if (left_on_screen){
    for (i=ref.Top;i<ref.Bottom;i++)
      for (e=ref.Left;e<=ref.Right;e++) v[e+i*k]=v[e+(i+1)*k];
  }
  ref.Left=cn->csb.dwSize.X-QueueWidth;ref.Top=cn->csb.dwSize.Y-ResultHeight+1;
  ref.Right=cn->csb.dwSize.X-2;
  if (left_on_screen){
    for (i=ref.Top;i<ref.Bottom;i++) 
      for (e=ref.Left;e<=ref.Right;e++) v[e+i*k]=v[e+(i+1)*k];
  }
  for (e=1;e<=ref.Right;e++) v[e+ref.Bottom*k].Attributes=7,v[e+ref.Bottom*k].Char.AsciiChar=' ';
  v[cn->csb.dwSize.X-ResultWidth+ref.Bottom*k].Char.AsciiChar=(char)179;
  ref.Right=cn->csb.dwSize.X-2;
  if (left_on_screen)
    { ref.Top=InfoHeight+1;ref.Left=1;ref.Bottom=cn->csb.dwSize.Y-2;}
  else { ref.Bottom=ref.Top=cn->csb.dwSize.Y-2;ref.Left=1;}
  k*=cn->csb.dwSize.Y-2;
  tm = (_log) ? g->log->logstr((CString)s) : CTime::GetCurrentTime();
  st=tm.Format("%H:%M")+"  "+s;
  for (i=2,e=0;i<cn->csb.dwSize.X-ResultWidth&&e<st.GetLength();i++,e++) v[i+k].Char.AsciiChar=st[(int)e];

  refresh(ref);left_on_screen=cnew;
}
