// dialog.cpp (CJWindow and Dialog components)

#include "StdAfx.h"

// CJWindow

void CJWindow::update()
{
  COORD pos={0,0};

  for (int i=0;i<content.size();i++) content[i]->draw();
  WriteConsoleOutput(cn->conout,v,size,pos,&sr);
}

int CJWindow::changeFocus(int elem,int direction)
{
  int i;

  clientcursorstate(0);
  for (i=0;i<content.size();i++) if (content[i]->focus) content[i]->focus=0,content[i]->draw();
  i=elem;do {
    i+=direction;if (i<0) i=content.size()-1;if (i==content.size()) i=0;
  } while(content[i]->need_focus==0&&i!=elem);
  if (content[i]->need_focus) content[i]->focus=1,content[i]->draw();
  return i;
}

int CJWindow::run(int dx,int dy,char *title,int esc)
{
  int i,r,e;
  int rx,ry;

  if (!content.size()) return 0;
  escaped=esc;
  show(dx,dy,(CString)title);
  clientcursorstate(0);
  for (i=0,e=0;i<content.size();i++){
    content[i]->focus=0;
    content[i]->init();
    if (!e && content[i]->need_focus) e=1,content[i]->focus=1;
    content[i]->draw();
  }

  r=i=0;
  while(!r){
    if (cn->p=cn->getkey())
      if ((e=content[i]->process_event(cn->p))==0)
        if (cn->p->EventType==KEY_EVENT&&cn->p->Event.KeyEvent.bKeyDown){
          if (cn->p->Event.KeyEvent.wVirtualKeyCode==VK_TAB
             &&(cn->p->Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED))
                cn->p->Event.KeyEvent.wVirtualKeyCode=VK_UP;
          switch(cn->p->Event.KeyEvent.wVirtualKeyCode){
            case VK_UP: 
              i=changeFocus(i,-1);
              break;
            case VK_DOWN: case VK_TAB:
              i=changeFocus(i,1);
              break;
            case VK_ESCAPE:
              r=escaped;
              break;
          }
          
        } else if (cn->p->EventType==MOUSE_EVENT&&cn->p->Event.MouseEvent.dwEventFlags==0){
          if(cn->p->Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED){
            rx=cn->p->Event.MouseEvent.dwMousePosition.X-sr.Left-1;
            ry=cn->p->Event.MouseEvent.dwMousePosition.Y-sr.Top-1;
            if (rx>=0&&ry>=0){
              for (int w=0;w<content.size();w++)
                if(content[w]->el.Top<=ry&&content[w]->el.Bottom>=ry
                &&content[w]->el.Left<=rx&&content[w]->el.Right>=rx
                &&content[w]->need_focus){
                   i=w;changeFocus(w,0);
                   e=content[w]->exitid;
                }
            }
          }
        }
      if (e==2||e==3) r=e;
  }
  for (i=0,e=0;i<content.size();i++) content[i]->deinit();
  hide();
  return r-2;
}

CJWindow& CJWindow::operator<<(CJWElem &e)
{
  e.assignWnd(this);
  content.push_back(&e);
  return *this;
}

// CJButton

int CJButton::process_event(PINPUT_RECORD p)
{
  if (p->EventType==KEY_EVENT&&p->Event.KeyEvent.bKeyDown){
    switch(p->Event.KeyEvent.wVirtualKeyCode){
      case VK_LEFT: p->Event.KeyEvent.wVirtualKeyCode=VK_UP;break;
      case VK_RIGHT: p->Event.KeyEvent.wVirtualKeyCode=VK_DOWN;break;
      case VK_RETURN: if (btype>1) return btype;
        if (qwe) qwe(cml);return 1;
    }
  }
  return 0;
}

void CJButton::draw()
{
  if (focus) wnd->clientwritexy(el.Left,el.Top,(LPCTSTR)("["+label+"]"));
    else wnd->clientwritexy(el.Left,el.Top,(LPCTSTR)(" "+label+" "));
}

// CJStatic

void CJStatic::draw()
{
  wnd->clientwritexy(el.Left,el.Top,(LPCTSTR)label);
}

// CJList

void CJList::draw()
{
  SMALL_RECT rf=el;
  for(int i=0;i<height;i++)
    if(rpos+i>=cnt) wnd->clientwritexy(el.Left,el.Top+i,"",g->z.cl_listinactive,0,width);
    else if (i==pos) wnd->clientwritexy(el.Left,el.Top+i,lst[i+rpos],g->z.cl_listactive,0,width);
    else wnd->clientwritexy(el.Left,el.Top+i,lst[i+rpos],g->z.cl_listinactive,0,width);
  rf.Bottom++;rf.Left++;rf.Right++;rf.Top++;
  wnd->refresh(rf);
}

int CJList::process_event(PINPUT_RECORD p)
{
  if (p->EventType==KEY_EVENT&&p->Event.KeyEvent.bKeyDown){
    switch(p->Event.KeyEvent.wVirtualKeyCode){
      case VK_UP:
        if(pos+rpos==0) return 1;
        pos--;if(pos<0) pos++,rpos--;draw();return 1;
      case VK_DOWN:
        if (pos+rpos>=cnt-1) return 1;
        pos++;if(pos>=height) pos--,rpos++;draw();return 1;
      case VK_LEFT: p->Event.KeyEvent.wVirtualKeyCode=VK_UP;break;
      case VK_RIGHT: p->Event.KeyEvent.wVirtualKeyCode=VK_DOWN;break;
    }
  }
  return 0;
}

// CJEdit

void CJEdit::setcstate()
{
  wnd->clientcursorstate(true,insmode?10:100);
}

void CJEdit::setcpos()
{
  wnd->clientgotoxy(el.Left+pos,el.Top);
}

void CJEdit::showstr(int active)
{
  int c1=g->z.cl_currentedit,c2=g->z.cl_cureditsel;
  if (!active) c1=g->z.cl_edit,c2=g->z.cl_editsel;

  if (sel){
    int i1=sel_start-rpos,i2=sel_end-rpos;
    if (i1<0) i1=0;if (i2<0) i2=0;
    if (i1>width) i1=width;if (i2>width) i2=width;
    if (i1) wnd->clientwritexy(el.Left,el.Top,txt+rpos,c1,1,i1);
    if (i2-i1) wnd->clientwritexy(el.Left+i1,el.Top,txt+rpos+i1,c2,1,i2-i1);
    if (width-i2) wnd->clientwritexy(el.Left+i2,el.Top,txt+rpos+i2,c1,1,width-i2);
  } else wnd->clientwritexy(el.Left,el.Top,txt+rpos,c1,1,width);
  if (active){
    setcpos();
    setcstate();
  }
}

int CJEdit::process_event(PINPUT_RECORD p)
{
  int i,e;

  if (p->EventType==KEY_EVENT&&p->Event.KeyEvent.bKeyDown){
    char c=p->Event.KeyEvent.uChar.AsciiChar;
    if (c>=' '){
      if(insmode){
		    if (len>=size) return 1;
        if(rpos+pos>sel_start&&rpos+pos<sel_end) sel_end++;
        if(rpos+pos<=sel_start) sel_end++,sel_start++;
        for (int i=len+1;i>pos+rpos;i--) txt[i]=txt[i-1];
				len++;txt[pos+rpos]=c;
      } else {
        if (txt[pos+rpos]==0){
					if (len>=size) return 1;
					txt[pos+rpos]=c,txt[pos+rpos+1]=0,len++;
				} else txt[pos+rpos]=c;
			}
	    pos++;if (pos>=width) pos--,rpos++;
			showstr();
      return 1;
    }
    switch(p->Event.KeyEvent.wVirtualKeyCode){
      case VK_LEFT:
        if (pos==0&&rpos==0) return 1;
        if (p->Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED){
          if (sel&&sel_start==pos+rpos) sel_start--;
          else if (sel&&sel_end==pos+rpos){
            sel_end--;if (sel_end==sel_start) sel=0;
          } else sel_start=pos+rpos-1,sel_end=pos+rpos;
          sel=1;showstr();
        }
				pos--;if(pos==-1) rpos--,pos++,showstr();
				  else setcpos();
				return 1;
      case VK_RIGHT: 
				if (rpos+pos>=len) return 1;
        if (p->Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED){
          if (sel&&sel_end==pos+rpos) sel_end++;
          else if (sel&&sel_start==pos+rpos){
            sel_start++;if (sel_end==sel_start) sel=0;
          } else sel_start=pos+rpos,sel_end=pos+rpos+1;
          sel=1;showstr();
        }
        pos++;
				if (pos>=width) pos--,rpos++,showstr();
				  else setcpos();
				return 1;
      case VK_BACK: 
        if (p->Event.KeyEvent.dwControlKeyState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)) return 1;
				if (pos==0&&rpos==0) return 1;pos--;
				if(pos==-1) rpos--,pos++;
  	  case VK_DELETE:
        if (p->Event.KeyEvent.dwControlKeyState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)){
          if (sel){
            e=sel_end-sel_start;
            for(i=sel_end;i<=len;i++) txt[i-e]=txt[i];
            rpos=sel=0;pos=sel_start;len-=e;
            while(pos>=width) pos-=width,rpos+=width;
            showstr();
          }
          return 1;
        }
		    if(txt[rpos+pos]==0) return 1;
        if(rpos+pos>=sel_start&&rpos+pos<sel_end) sel_end--;
        if(rpos+pos<sel_start) sel_end--,sel_start--;
        if(sel_start==sel_end) sel=0;
        for (i=rpos+pos+1;i<len;i++) txt[i-1]=txt[i];
        txt[len-1]=0;len--;showstr();
		    return 1;
      case VK_INSERT: 
        if (p->Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED){
          if (OpenClipboard(NULL)){
            char *t=(char *)GetClipboardData(CF_TEXT);
            if (t){
              sel_start=pos+rpos;
              while(*t){
                if (len>=size) break;
                for (int i=len+1;i>pos+rpos;i--) txt[i]=txt[i-1];
				        len++;txt[pos+rpos]=*t;
                pos++;t++;if (pos>=width) pos--,rpos++;                
              }
              sel_end=pos+rpos;CloseClipboard();
              sel=sel_start-sel_end;showstr();
            } else CloseClipboard();
          }
        } else if (p->Event.KeyEvent.dwControlKeyState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)){
          if (OpenClipboard(NULL)){
            EmptyClipboard();// ????
            CloseClipboard();
          }
        } else {
		      insmode=!insmode;setcstate();
        } return 1;
      case VK_RETURN: p->Event.KeyEvent.wVirtualKeyCode=VK_DOWN;break;
      case VK_HOME: 
        i=rpos;
        if (p->Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED){
          if (sel&&pos+rpos==sel_start) sel_start=0,i=1;
          else if(pos+rpos) sel=i=1,sel_start=0,sel_end=pos+rpos;
        }
        rpos=pos=0;if(i)showstr();else setcpos();return 1;
      case VK_END: 
        i=rpos;
        if (p->Event.KeyEvent.dwControlKeyState&SHIFT_PRESSED){
          if (sel&&pos+rpos==sel_end) sel_end=len,i=-1;
          else if(len-pos-rpos) i=-1,sel=1,sel_start=pos+rpos,sel_end=len;
        }
        pos=len;rpos=0;
				if(pos>=width) rpos=pos-width+1,pos=width-1;
        if (i==rpos) setcpos();else showstr();
				return 1;
    }
  }
  return 0;
}

void CJEdit::draw()
{
  showstr(focus);
}
