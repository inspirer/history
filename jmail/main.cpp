// main.cpp

#include "StdAfx.h"

#define nUmber "1"
//#define nUmber "4547043"
//#define nUmber "2426364"
//#define nUmber "3141109"

void context::bmodemsendstr(const char *s)
{
  const char *l=s;
  while(*l){
    switch(*l){
     case '|':  mdm->transmit("\r\n",2);Sleep(200);break;
     case 'v':  mdm->clrdtr();break;
     case '^':  mdm->setdtr();break;
     case '\'': Sleep(100);break;
     case '~':  Sleep(500);break;
     default:   mdm->transmit(l,1);break;
    }
    l++;
  }
}

int context::bmodemgetchar()
{
  if (mdm->bhead==mdm->btail) return 0;
  else {
    int i = mdm->bm_in[mdm->btail++];
    mdm->btail%=receive_buffer_size;
    return i;
  }
}

int context::bmodemgetdata(char *p)
{
  int size=0;
  char *l=p;

  while(mdm->bhead!=mdm->btail){
    *l++=mdm->bm_in[mdm->btail++];mdm->btail%=receive_buffer_size;
    size++;if (size>1024) break;
  }
  return size;
}

// 1 => show + send + wait ; 2 => show + send ; 0 => send
int context::bmodemsendcomm(const char *info, const char *s,int show)
{
  CString q;
  int o = 0,result;

  do {
    if (show) f->conprint(info,1,0);
    Sleep(v.modem_delay*100);
    bmodemsendstr(s);
    if (show==0||show==2) return 1;
    DWORD c = GetTickCount() + v.waitmdmanswer*1000;
    result = 0;
    while (c>=GetTickCount()){
      while(int i = bmodemgetchar()){
        if (i==13||i==10){
          if (q==v.ok) result = 1;
          q.Empty();
        } else q+=char(i);
      }
      if (result) break;
      Sleep(0);
    }
    o++;
    if (result) f->conresult(14,"OK",0); else f->conresult(14,"ERROR",0);
  } while(!result&&o<3);

  return result;
}

int context::bmodeminit()
{
  int i = (v.init2!="") ? 2 : 1;
  if (i==1){
    if (!bmodemsendcomm("initializing modem ...",v.init1,1)) return 0;
  } else {
    if (!bmodemsendcomm("initializing modem (1 of 2)",v.init1,3)) return 0;
    f->left_on_screen=0;
    if (!bmodemsendcomm("initializing modem (2 of 2)",v.init2,1)) return 0;
  }
  return 1;
}

void context::init_circle(int enable_call)
{
  if (enable_call){
     // if we can call: call_node then return;
  }
  mst=0;buildqueue();
  time(&start_time);end_time=v.circle+start_time;
  current=start_time-1;
  f->conprint("waiting for call ...",0,0);
}

void context::call_node()
{
  mst=1;
  CString number = v.dial_prefix + nUmber + v.dial_suffix;
  bmodemsendcomm("calling number: " nUmber,number,2);
  time(&start_time);end_time=v.time_dial+start_time;
  current=start_time-1;
}

void context::stop_call()
{
  mst=0;
  bmodemsendcomm(NULL,v.dial_terminate,0);
  f->conresult(14,"NO CARRIER");
  init_circle(0);
}

void context::answer_node()
{
  mst=2;
  bmodemsendcomm("answering...",v.modem_answer,2);
  time(&start_time);end_time=v.wait_carrier+start_time;
  current=start_time-1;
}

void context::stop_answer()
{
  mst=0;
  bmodemsendcomm(NULL,v.dial_terminate,0);
  f->conresult(14,"NO CARRIER");
  init_circle(0);
}

void context::next_ring()
{
  mst=3;ringno++;
  time(&start_time);end_time=start_time+6;
  current=start_time-1;if (ringno>1) f->left_on_screen=0;
  f->conprint((CString)"incoming ring N" + _itoa(ringno,s,10),1,0);
  if (ringno==v.answer_ring){
     f->conresult(15,"ANSWERING");
     answer_node();
  }
}

void context::ShowHelp()
{
  CCWindow w;

  w.cn=f->cn;
  w.show(w.cn->csb.dwSize.X,w.cn->csb.dwSize.Y,(CString)"Help",0,0);
  w.clientwritexy(10,1,"Help Window, press Escape to cancel");
  while(1){
    if (w.cn->p = w.cn->getkey())
      if (w.cn->p->EventType==KEY_EVENT&&w.cn->p->Event.KeyEvent.bKeyDown){
         if (w.cn->p->Event.KeyEvent.wVirtualScanCode==1)
           break;
      }
    while(int q=bmodemgetchar());
  }
  w.hide();
}

void sd_(context *m)
{
  CJWindow w;w.cn=m->f->cn;
  static CJButton ok(35,10,(CString)"OK",3), cancel(45,10,(CString)"CANCEL",1);
  w<<ok<<cancel;
  w.run(70,10,"i1");
}

void context::TestDialogItem()
{
  CJWindow w;w.cn=f->cn;
  static CJButton ok(35,10,(CString)"OK",3), cancel(45,10,(CString)"CANCEL",1);
  static CJEdit first(10,3,20,50),second(10,5,30,20);
  static CJStatic s(5,8,(CString)"Yoklmn");
  static CJList p(45,2,10,3);
  cancel.OnClick(this,sd_);

  p.Add((CString)"Yo1");p.Add((CString)"Yo2");
  p.Add((CString)"Yo3 very_long_string_vah");
  p.Add((CString)"Yo4");p.Add((CString)"Yo5");
  p.Add((CString)"Yo6");p.Add((CString)"Yo7");
  p.Add((CString)"Yo8");p.Add((CString)"Yo9");

  w<<ok<<cancel<<first<<second << s << p;
  w.run(60,14,"test Dialog");
}

void context::MakeConfig()
{
  CJWindow w;w.cn=f->cn;

  static CJButton save(54,17,(CString)"Save",3),cancel(62,17,(CString)"Cancel",2);
  
  w << save << cancel;
  sprintf(s,"line `%s' configuration",line_name);
  if(w.run(71,18,s)){
    // ????
    CJButton ok(14,3,(CString)"OK",3);
    CJStatic Message(7,1,(CString)"configuration saved");
    w.clear();sprintf(s,"line `%s'",line_name);
    w << ok << Message;
    w.run(30,4,s);
  }
}

int context::circle()
{
  char r[200];int last=0;

  mst=0;

  default_queue.node.homedir="d:\\temp\\out";
  while(1){
    if (!bmodeminit()) return 7;
    init_circle(0);

    while(1){
      if (f->cn->p = f->cn->getkey())
        if (f->cn->p->EventType==KEY_EVENT&&f->cn->p->Event.KeyEvent.bKeyDown){
          
          switch(f->cn->p->Event.KeyEvent.uChar.AsciiChar){
            case 'a': case 'A': if (mst==0||mst==3) answer_node();break;
            case 'c': case 'C': if (mst==0) call_node();break;
            case 'r': case 'R': TestDialogItem();break;
            case 'x': 
              if (mst==0&&(f->cn->p->Event.KeyEvent.dwControlKeyState&(LEFT_ALT_PRESSED))) return 0;
              break;

          }

          switch(f->cn->p->Event.KeyEvent.wVirtualKeyCode){
            case VK_F1: ShowHelp();break;
            case VK_F2: MakeConfig();break;
            case VK_SPACE: case VK_ESCAPE:
                switch(mst){
                  case 0: time(&yo);if (v.circle>end_time-yo) init_circle(1);break;
                  case 1: stop_call();break;
                  case 2: stop_answer();break;
                  case 3: f->conresult(15,"NO MORE RINGS",0);init_circle(0);break;
                }
                break;
          }
        }
      Sleep(10);
      if (mst!=-1){
        time(&yo);
        if (current!=yo){
          current=yo;
          if (yo>=end_time){
            switch(mst){
              case 0: init_circle(1);break;
              case 1: stop_call();Sleep(400);call_node();break;
              case 2: stop_answer();break;
              case 3: f->conresult(15,"NO MORE RINGS");init_circle(0);break;
            }
            continue;
          }
          int i = end_time - yo;
          sprintf(s,"%02i:%02i",i/60,i%60);
          f->conresult(15,s);
        }
      }
      if (int q=bmodemgetchar()){
        if (q==13||q==10){
          r[last]=0;
          if (last){
            if (checkstring(v.ncarrier+"|"+v.no_dial+"|"+v.busy,r,'|')){
              f->conresult(14,r);
              if (mst==1) Sleep(400),call_node();
                else init_circle(0);
            } else if (v.ring==r){
              if (mst!=3) ringno=0;
              next_ring();
            } else if (!strncmp(r,v.connect,v.connect.GetLength())){
              f->conresult(14,v.connect);
              if (mst==1)
                emsi_connect_out(r,default_queue);
              else 
                emsi_connect_in(r);
              last=0;
              break;
            }
            last=0;
          }
        } else {
          if (last>198) last=0;
          r[last++]=q;
        }
      }
    }
  }
}
