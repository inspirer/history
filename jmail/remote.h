// remote.h

#define max_emsi_string 4096

class CAddress {
private:
  enum {ADDR_FTS, ADDR_STR} type;
  int z,n,f,p;
  CString str;
public:
  CAddress(int _z,int _n,int _f,int _p=0){type=ADDR_FTS;z=_z;n=_n;f=_f;p=_p;}
  CAddress(const CString &s=""){*this=s;}
  operator=(const CString &s);
  operator CString();
  operator LPCTSTR();
  int& operator [](int _n);
  int isFTS(){ return type==ADDR_FTS;}
};

class CConnect {
public:
  int baud;
  BOOL correction,compression;
  load(CString);
  operator CString();
};

class CJFile { 
public:
  ulong Filesize,Offset,status;
  time_t Timestamp,Start;
  CString fname,sname,realName;
  HANDLE f;

  int OpenFileForWrite();
  int OpenFileForRead();
  int FileWrite(void *b,int size);
  int FileRead(void *b,int size);
  int FileSeek(ulong pos);
  void FileClose(int ok=0);
};

class CTimeInterval {
public:
  int day; // if(day!=0) bitfield(1 - Sunday, 2 - Monday, 4 - ...)
  int start,end;
  int isIN(CTime& t);
  CTimeInterval(): day(0), start(0), end(0){}
};

class CJTime {
private:
  std::vector<CTimeInterval> t;
public:
  int isIN(CTime& w);
  CJTime(){ t.clear(); }

  CString temp;
};

int checktime(CString& t);

class CEmsi {
public:
  char emsi[max_emsi_string+20],s[max_emsi_string+20],s2[max_emsi_string+20];

  std::vector<CAddress> addr;
  CString password;
  CString mailer_code,mailer_name,mailer_version,mailer_sn;
  enum { PUA=1, PUP=2, NPU=4, HAT=8, HXT=16, HRQ=32, RH1=64, N81=128 };
  int link_codes;
  enum { ZAP=1, ZMO=2, JAN=4, KER=8, NCP=16, NRQ=32, ARC=64, XMA=128, HYD=256, FNC=512 };
  int compatibility;

  enum { IDENT=1, TRAF=2, TZUTC=4, TRX=8, OHFR=16 };
  int extra_used;
  CString system,city,sysop,phone,baud,flags;
  int tzUtc;
  long traf_mail,traf_file;
  CJTime wtime,ftime;
  time_t cTime;
  CString x;

  defaults(int out);
  void fix_string_for_emsi(char *l,char c);
  void tostr();
  int fromstr();
  int prepeareDAT(char *emsi_buffer);
};

class CNode {
public:
  CAddress addr;
  CString phone,name,flags,homedir;
  CJTime work_time;
};

class CFileList {
private:
  std::vector<CString> fl;
  WIN32_FIND_DATA w;

public:
  void clear(){ fl.clear();}
  void erase(int i){ fl[i].Empty();}
  void add(CString& l){ fl.push_back(l);}
  void adddir(CString& dir){
    HANDLE sf=FindFirstFile(dir + "\\*.*",&w);
    if (sf==INVALID_HANDLE_VALUE) return;
    do {
      if (!(w.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)) add(dir+"\\"+w.cFileName);
    } while(FindNextFile(sf,&w));
    FindClose(sf);
  }
  CString get(int n){ return fl[n];}
  int size(){ return fl.size();}
};

class CQueue {
public:
  CNode node;
  int mail,file,hold,status,tries;
  CFileList ls;

  operator CString();
  CQueue(): mail(0), file(0), hold(0), status(0), tries(0) {};
};

int checktimezone(const char *w);
int gettimezone(const char *w);
