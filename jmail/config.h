// config.h

class config {
public: 

  CString address;
  CString sysop;
  CString bbsname;
  CString phone;
  CString place;
  CString flags;
  int timezone;
  CString inbound;
  CString inboundunsec;
  CString log;
  CString chat_log;

  CString comport;
  int baudrate;
  int min_baud_in;
  int min_baud_out;
  int waitmdmanswer;
  int modem_delay;
  int init_interval;
  CString init1;
  CString init2;
  CString ok;
  CString busy;
  CString connect;
  CString no_dial;
  CString ring;
  CString ncarrier;
  CString dial_terminate;
  CString dial_prefix;
  CString dial_suffix;
  CString onhook;
  CString modem_answer;
  CString error_correction;

  int time_dial;
  int call_tries;
  int circle;
  CString call_time;

  CString answer;
  CString emsi_oh;
  int answer_ring;
  int wait_carrier;
  enum { pr_hydra=0,pr_zmodem };
  int protocol_use;

  void defvar(int varn,char *value);
  void readconfig(const char *n,const char *section);
  void writeconfig(const char *n,const char *section);
  void defaults();
};
