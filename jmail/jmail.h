// jmail.h

// common
extern const char jmail[],build[],version[],version_mod[];

#define receive_buffer_size 65536
#define transmit_buffer_size 4096
#define INSIZE 4096

class CConsole {
private:
  void congetnext();

public:
  HANDLE conin,conout;
  OVERLAPPED ovc;
  INPUT_RECORD inp[INSIZE];
  PINPUT_RECORD p;
  CONSOLE_SCREEN_BUFFER_INFO csb;
  CONSOLE_CURSOR_INFO cci;
  int head,tail;

  void initconsole();
  void deinitconsole();
  PINPUT_RECORD getkey();
};

class CCWindow {
protected:
  PCHAR_INFO p,v;
  CString caption;
  int shadow;
  SMALL_RECT sr;
  COORD client,prev_cursor,size;
  CONSOLE_CURSOR_INFO cci;

public:
  CConsole *cn;

  void show(int dx,int dy,CString& w,int x=-1,int y=-1,int refr=1);
  void hide();
  void clientgotoxy(int x,int y);
  void clientcursorstate(int show,int size=10);
  void clientwritexy(int x,int y,const char *,int color=7,int refr=1,int width=0);
  void refresh(SMALL_RECT &y);
  void drawVertical(int x,int y1,int y2,int term);
  void drawHorizontal(int x1,int x2,int y,int term);
};

class CLog {
private:
  int logfile;

public:
  void initlog();
  void deinitlog();
  CTime& logstr(CString&);
};

class CCMainFrame : public CCWindow {
public:
  int left_on_screen;

  CCMainFrame(){ cn=new CConsole;}
  ~CCMainFrame(){ delete cn; }
  void init(CString&);
  void conresult(int color,const char *s,int refr=1);
  void conprint(const char *s,int cnew,int _log);
  void deinit();
};

class CComm {
private:
  HANDLE h;
  OVERLAPPED ovr,ovw;
  HANDLE txenable,threadalive;
  friend unsigned int WINAPI mthread(void *);
  void receive(char *buf,int read);

public:
  int bhead,btail;
  uchar bm_in[receive_buffer_size];
  CComm() : h(NULL), bhead(0), btail(0){}
  DWORD mstat,status;

  int init(const char *l,int baud = 57600);
  void deinit();
  int transmit(const char *buf, DWORD size);
  void setdtr();
  void clrdtr();
  int enable(){ return (WaitForSingleObject(txenable,0)==WAIT_OBJECT_0);}
  void clear(){ SetEvent(txenable); }
};

class context {
public:
  CCMainFrame *f;
  CComm *mdm;
  CLog *log;
  CString line_name,color_name;
  config v;
  cl_config z; 

public:
  std::vector<CQueue> queue;
  int q_current,q_base;
  context(){ f=new CCMainFrame;log=new CLog;mdm=new CComm; }
  ~context(){ delete mdm;delete log;delete f; }
  int circle();

private:
  int  mst; // { 0 - CIRCLE ; 1 - CALL ; 2 - ANSWER ; 3 - RING }
  long start_time,end_time,current,yo;
  char s[128];
  char emsi_buffer[max_emsi_string+20];
  long ringno;
  CEmsi remote,here;
  CString x;
  CConnect cc;
  CQueue default_queue;

  void init_circle(int enable_call);
  void call_node();
  void stop_call();
  void answer_node();
  void stop_answer();
  void next_ring();
  void ShowHelp();
  void MakeConfig();
  void TestDialogItem();

  void emsi_connect_in(const char *connect);
  void emsi_connect_out(const char *connect, CQueue& w);
  int  bmodemgetchar();
  int  bmodemgetdata(char *l);
  void bmodemsendstr(const char *s);
  int  bmodemsendcomm(const char *info, const char *s,int show);
  int  bmodeminit();
  printqueue();
  buildqueue();
};

extern __declspec(thread) context *g;

// common functions
int checkstring(CString set,CString str,char c);
