// window.h

class CJWElem;
class CJWindow : public CCWindow {
private:
  std::vector<CJWElem *> content;
  int escaped;

public:
  void update();
  int changeFocus(int elem,int direction);
  void clear(){ content.clear();}
  CJWindow& operator<<(CJWElem &e);
  int run(int,int,char*,int =2);
};

class CJWElem {
protected:
  CJWindow *wnd;
public:
  SMALL_RECT el;
  int need_focus;
  int exitid;

  CJWElem(int _x,int _y,int q,int dx,int dy,int eid=1):need_focus(q), exitid(eid)
    { wnd=NULL;el.Top=_y;el.Bottom=_y+dy-1;el.Left=_x;el.Right=_x+dx-1; }
  void assignWnd(CJWindow *w){ wnd=w;}
  int focus;

  virtual void init(){};
  virtual void deinit(){};
  virtual void draw(){};
  virtual int process_event(PINPUT_RECORD p){ return 0;};
};

// some elements

class CJButton : public CJWElem {
private:
  CString label;
  void (*qwe)(context *);
  context *cml;
  int btype;
public:
  CJButton(int _x,int _y,CString& s,int t) : CJWElem(_x,_y,1,s.GetLength()+2,1,t), btype(t) { label=s; }
  int process_event(PINPUT_RECORD);
  void OnClick(context *m,void(*q)(context *)){ cml=m;qwe=q;}
  void draw();
};

class CJStatic : public CJWElem {
private:
  CString label;
public:
  CJStatic(int _x,int _y,CString& s) : CJWElem(_x,_y,0,s.GetLength(),1){ label=s; }
  void draw();
};

class CJList : public CJWElem {
private:
  std::vector<CString> lst;
  int pos,rpos,width,height,cnt;
public:
  CJList(int _x,int _y,int dx,int dy):CJWElem(_x,_y,1,dx,dy),pos(0),rpos(0),height(dy),width(dx),cnt(0){ lst.clear(); }
  int process_event(PINPUT_RECORD);
  void Add(CString& c){ lst.push_back(c);cnt++;}
  void Clear(){ lst.clear();cnt=0;}
  CString& current(){ return lst[rpos+pos];}
  void draw();
};

class CJEdit : public CJWElem {
private:
  char *txt;
  int  len,rpos,pos,insmode,width,size,sel,sel_start,sel_end;
  void setcstate();
  void setcpos();
  void showstr(int active=1);
public:
  CJEdit(int _x,int _y,int w,int msize):CJWElem(_x,_y,1,w,1), width(w){
    insmode=true;size=msize;
    txt=new char[msize+1];
    txt[0]=0;sel=rpos=pos=len=0;
  }
  CJEdit(int _x,int _y,int w,int msize,const char *str):CJWElem(_x,_y,1,w,1), width(w){
    insmode=true;size=msize;
    txt=new char[msize+1];
    strcpy(txt,str);rpos=sel=0;
    pos=len=strlen(str);
        if(pos>=width) rpos=pos-width+1,pos=width-1;
  }
        void LoadStr(char *s){
    len=strlen(s);if(len>size) len=size;
                strncpy(txt,s,len);txt[len]=0;pos=len;sel=rpos=0;
        if(pos>=width) rpos=pos-width+1,pos=width-1;
                insmode=true;
        }
  ~CJEdit(){ delete[] txt; }
  int process_event(PINPUT_RECORD);
  void draw();
};
