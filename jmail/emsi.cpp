// emsi.cpp (EMSI handshake, CEmsi)

#include "StdAfx.h"

#define emsi_req "**EMSI_REQA77E"
#define emsi_inq "**EMSI_INQC816"
#define emsi_ack "**EMSI_ACKA490"
#define emsi_nak "**EMSI_NAKEEC3"

#define send_emsi_inq      4
#define emsi_inq_time      5
#define send_emsi_dat      5
#define emsi_rec_dat_time  20
#define emsi_send_dat_time 10
#define emsi_total_time    60

const char hEx[]="0123456789ABCDEF";

void context::emsi_connect_in(const char *connect)
{
  f->conprint(CString("incoming call: ") + connect,1,1);
  cc.load(connect);
  if (cc.baud<v.min_baud_in)
    f->conprint((CString)"error: Connection too slow, required at least "+(CString)v.min_baud_in+" bps.",1,1);
  else {
    f->conprint("analysing mailer's presence",1,0);
    here.defaults(0);

    Sleep(1000);
  }
  bmodemsendcomm("hangup",v.onhook,1);
  f->left_on_screen=0;
}

void context::emsi_connect_out(const char *connect, CQueue& w)
{
  int i,e,step,q;
  time_t et,t2;
  CTime ct;
  char *l;
  CFileList lst=w.ls;

  f->conprint(CString("outgoing call: ") + connect,1,1);
  lst.adddir(w.node.homedir);
  cc.load(connect);
  if (cc.baud<v.min_baud_out)
    f->conprint("error: Connection too slow, required at least "+(CString)v.min_baud_out+" bps.",1,1);
  else {
    here.defaults(1);
    mdm->transmit("\r\n",2);
    t2=time(NULL) + emsi_total_time;

    step=e=0;
    for(i=0;i<send_emsi_inq&&step==0;i++){
       f->conprint("sending EMSI Inquiry",1,0);
       mdm->transmit(emsi_inq "\r\n",16);
       et=time(NULL)+emsi_inq_time;
       if (et>t2) et=t2;

       do {
         while(q=bmodemgetchar()){
           emsi_buffer[e++]=q;emsi_buffer[e]=0;
           if (e>max_emsi_string){             
             strcpy(emsi_buffer,emsi_buffer+200);
             e=strlen(emsi_buffer);
           }
           if (strstr(emsi_buffer,emsi_req)!=NULL){
             f->conprint("received EMSI Request",1,0);
             step=1;break;
           }           
         }

         if (f->cn->p = f->cn->getkey())
           if (f->cn->p->EventType==KEY_EVENT&&f->cn->p->Event.KeyEvent.bKeyDown)
             if (f->cn->p->Event.KeyEvent.uChar.AsciiChar=='h'&&
               f->cn->p->Event.KeyEvent.dwControlKeyState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
                 step=-1;
         
         if ((mdm->mstat&MS_RLSD_ON)==0) step=-2;

       } while(et>=time(NULL)&&step==0);
       if (t2<time(NULL)){ step=-3;break;}
    }

    int size;e=0;
    if (step==1) for(i=0;i<send_emsi_dat&&step==1;i++){
      size = here.prepeareDAT(emsi_buffer);
      mdm->transmit(emsi_buffer,size);
      f->conprint("sending EMSI Data",1,0);
      et=time(NULL)+emsi_send_dat_time;
      if (et>t2) et=t2;

      do {
         while(q=bmodemgetchar()){
           emsi_buffer[e++]=q;emsi_buffer[e]=0;
           if (e>max_emsi_string){             
             strcpy(emsi_buffer,emsi_buffer+200);
             e=strlen(emsi_buffer);
           }
           if (strstr(emsi_buffer,emsi_ack)!=NULL){
             f->conprint("received EMSI Acknowledgement",1,0);
             step=2;break;
           } else if (strstr(emsi_buffer,emsi_nak)!=NULL){
             f->conprint("received EMSI NAK",1,0);
             et=0;break;
          }
         }

         if (f->cn->p = f->cn->getkey())
           if (f->cn->p->EventType==KEY_EVENT&&f->cn->p->Event.KeyEvent.bKeyDown)
             if (f->cn->p->Event.KeyEvent.uChar.AsciiChar=='h'&&
               f->cn->p->Event.KeyEvent.dwControlKeyState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
                 step=-1;
           
         if ((mdm->mstat&MS_RLSD_ON)==0) step=-2;
     
      } while(et>=time(NULL)&&step==1);
      if (t2<time(NULL)){ step=-3;break;}
    }
   
    i=0;while(step==2&&i<send_emsi_dat){
      f->conprint("receiving EMSI Data",1,0);
      e=0;et=time(NULL)+emsi_rec_dat_time;
      if (et>t2) et=t2;

      do {
         while(q=bmodemgetchar()){
           emsi_buffer[e++]=q;emsi_buffer[e]=0;
           if (e>max_emsi_string){             
             strcpy(emsi_buffer,emsi_buffer+200);
             e=strlen(emsi_buffer);
           }
           if (strstr(emsi_buffer,"**EMSI_DAT")!=NULL){
              f->conresult(15,"EMSI_DAT");
              strcpy(emsi_buffer,"EMSI_DAT");
              size=0;step=4;e=8;
              do { 
                while(step&&(q=bmodemgetchar())!=0){
                  size<<=4;step--;emsi_buffer[e++]=q;
                  if (q>='0'&&q<='9') size+=q-48;
                  else if (q>='A'&&q<='F') size+=q-55;
                  else if (q>='a'&&q<='f') size+=q-87;
                  else { step=0;size=-1;}
                } 
              } while(et>=time(NULL)&&step);
              if (step||size<=0||size>max_emsi_string){ step=0;break;}
              size+=4;l=emsi_buffer+e;
              do {
                if (f->cn->p = f->cn->getkey()){
                  if (f->cn->p->EventType==KEY_EVENT&&f->cn->p->Event.KeyEvent.bKeyDown){
                    if (f->cn->p->Event.KeyEvent.uChar.AsciiChar=='h'&&
                      f->cn->p->Event.KeyEvent.dwControlKeyState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
                    { step=-1;break;}
                  }
                }
                e=bmodemgetdata(l);
                if (e<=size) l+=e,size-=e;
                  else { l+=size;size=0;}
              } while(et>=time(NULL)&&size>0);
              *l=0;
              if (step==-1) break;
              if (size){ step=0;break;}
              f->conresult(15,"RECEIVED");
              strcpy(remote.emsi,emsi_buffer);
              if (remote.fromstr()){ step=3;break;}

              step=0;break;
           }
         }

         if (f->cn->p = f->cn->getkey())
           if (f->cn->p->EventType==KEY_EVENT&&f->cn->p->Event.KeyEvent.bKeyDown)
             if (f->cn->p->Event.KeyEvent.uChar.AsciiChar=='h'&&
               f->cn->p->Event.KeyEvent.dwControlKeyState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
                 step=-1;

         if ((mdm->mstat&MS_RLSD_ON)==0) step=-2;

      } while(et>=time(NULL)&&step==2);

      i++;
      if (t2<time(NULL)) step=-3;
      if (step<0) break;
      if (step!=3){
        mdm->transmit(emsi_nak "\r\n",16);
        if (step==2) f->conresult(15,"TIME LIMIT");
           else f->conresult(15,"ERROR");
        step=2;
      } else f->conresult(15,"OK");
    }

    switch(step){
      case -1: f->conresult(15,"(cancelled)");break;
      case -2: f->conresult(15,"(carrier lost)");break;
      case -3: f->conresult(15,"(line is dead)");break;
    }

    if (step==3){
       mdm->transmit(emsi_ack "\r\n",16);
       mdm->transmit(emsi_ack "\r\n",16);

       f->conprint(remote.addr[0] + ", using " + remote.mailer_name + " " + remote.mailer_version + "/" + remote.mailer_sn,1,1);
       x.Empty();
       if (remote.addr.size()>1){
         for(i=1;i<remote.addr.size();i++){
           x+=" " + remote.addr[i];
           { f->conprint("AKA" + x,1,1);x.Empty();}
         }
         if (!x.IsEmpty()) f->conprint("AKA" + x,1,1);
       }
       if (!remote.password.IsEmpty()) f->conprint("password protected session",1,1);
       if (remote.extra_used&CEmsi::TRX){
         ct=remote.cTime;x="remote date/time is ";
         x+=ct.Format( "%H:%M, %A, %d %B, %Y");
         sprintf(emsi_buffer," [%+03i00]",remote.tzUtc);
         if (remote.extra_used&CEmsi::TZUTC) x+=(CString)emsi_buffer;
         f->conprint(x,1,1);
       }
       if (remote.extra_used&CEmsi::OHFR){
         f->conprint("Accepts calls: " + remote.wtime.temp,1,1);
         f->conprint("Accepts freqs: " + remote.ftime.temp,1,1);
       }
       if (remote.extra_used&CEmsi::IDENT){
         f->conprint("Flags: " + remote.flags + " [" + remote.baud +"]" ,1,1);
         f->conprint("SysOp: " + remote.sysop + ", " + remote.phone,1,1);
         f->conprint("System: " + remote.system+ ", " + remote.city,1,1);
       } else f->conprint("short EMSI",1,1);
       if (remote.extra_used&CEmsi::TRAF){
         sprintf(emsi_buffer,"Traffic prognosis: Mail: %i bytes, Files: %i bytes.",remote.traf_mail,remote.traf_file);
         f->conprint(emsi_buffer,1,1);
       }

       if ((remote.link_codes&CEmsi::N81)==0){
         f->conprint("connection must be at 8N1",1,1);

       } else if (remote.compatibility&CEmsi::NCP){
         f->conprint("remote mailer have no compatible protocols",1,1);

       } else if (remote.compatibility&CEmsi::HYD){
         CHydra *h = new CHydra;
         h->ls=&lst;
         h->Start();
         delete h;
       
       } else f->conprint("compatible protocol not found",1,1);
    }
  }
  bmodemsendcomm("hangup",v.onhook,1);
  f->left_on_screen=0;
}

// CEmsi

int checktimezone(const char *w)
{
  if (*w!='+'&&*w!='-') return 0;
  for (int i=1;i<3;i++)
    if (w[i]<'0'||w[i]>'9') return 0;
  if (strcmp(w+3,"00")) return 0;
  return 1;
}

int gettimezone(const char *w)
{
  if (checktimezone(w)){
      int i=(w[1]-'0')*10+w[2]-'0';
      if (w[0]=='-') i=-i;
      return i;
  } else return 0;
}

CEmsi::defaults(int out)
{
  addr.push_back(g->v.address);password="";
  mailer_code="fe";mailer_name="jMail";mailer_version=version;
  mailer_sn="noncommercial";link_codes=(out) ? PUA : 0;
  compatibility=HYD;traf_mail=0;traf_file=0;
  system=g->v.bbsname;city=g->v.place;sysop=g->v.sysop;
  phone=g->v.phone;baud=(CString)g->v.baudrate;/*????*/flags=g->v.flags;
  extra_used=IDENT|TRX|OHFR|TZUTC;
  tzUtc=g->v.timezone;
  wtime.temp=g->v.emsi_oh;ftime.temp=g->v.emsi_oh;
  cTime=time(NULL);
}

void CEmsi::fix_string_for_emsi(char *l,char c)
{
  char *h,*p;

  h=l;p=s2;
  while(*h){
    if(*h<32||*h>127){
      *p++='\\';int i=*h,e;
      if (i<0) i+=256;
      e=i%16;i/=16;if (i>9) i+=7;if (e>9) e+=7;
      i+='0';e+='0';*p++=i;*p++=e;
    } else if (*h==c||*h=='\\'||*h=='}'){
      *p++=*h;*p++=*h;
    } else *p++=*h;
    h++;
  }
  *p=0;strcpy(l,s2);
}

#define fix_l while(*l) l++;
#define strforemsi(q,c) strcpy(s,q);fix_string_for_emsi(s,c);

void CEmsi::tostr()
{
  char *l = emsi;
  CString w;w.Empty();

  for (int i=0;i<addr.size();i++){ if (i) w+=" ";w+=(CString)addr[i]; }
  strforemsi(w,'}');sprintf(l,"{EMSI}{%s}",s);fix_l;
  strforemsi(password,'}');sprintf(l,"{%s}",s);fix_l
  strcpy(s,"8N1");
  if (link_codes&PUA) strcat(s,",PUA");if (link_codes&PUP) strcat(s,",PUP");
  if (link_codes&NPU) strcat(s,",NPU");if (link_codes&HAT) strcat(s,",HAT");
  if (link_codes&HXT) strcat(s,",HXT");if (link_codes&HRQ) strcat(s,",HRQ");
  sprintf(l,"{%s}",s);fix_l;*s=0;
  if (compatibility&ZAP) strcat(s,"ZAP,");if (compatibility&ZMO) strcat(s,"ZMO,");
  if (compatibility&JAN) strcat(s,"JAN,");if (compatibility&KER) strcat(s,"KER,");
  if (compatibility&NCP) strcat(s,"NCP,");if (compatibility&NRQ) strcat(s,"NRQ,");
  if (compatibility&ARC) strcat(s,"ARC,");if (compatibility&XMA) strcat(s,"XMA,");
  if (compatibility&HYD) strcat(s,"HYD,");if (compatibility&FNC) strcat(s,"FNC,");
  if (s[strlen(s)-1]==',') s[strlen(s)-1]=0;sprintf(l,"{%s}",s);fix_l
  strforemsi(mailer_code,'}');sprintf(l,"{%s}",s);fix_l
  strforemsi(mailer_name,'}');sprintf(l,"{%s}",s);fix_l
  strforemsi(mailer_version,'}');sprintf(l,"{%s}",s);fix_l
  strforemsi(mailer_sn,'}');sprintf(l,"{%s}",s);fix_l
  if (extra_used&CEmsi::IDENT){
    sprintf(l,"{IDENT}{");fix_l
    strforemsi(system,']');sprintf(l,"[%s]",s);fix_l
    strforemsi(city,']');sprintf(l,"[%s]",s);fix_l
    strforemsi(sysop,']');sprintf(l,"[%s]",s);fix_l
    strforemsi(phone,']');sprintf(l,"[%s]",s);fix_l
    strforemsi(baud,']');sprintf(l,"[%s]",s);fix_l
    strforemsi(flags,']');sprintf(l,"[%s]",s);fix_l
    sprintf(l,"}");fix_l
  }
  if (extra_used&CEmsi::TRAF){
    sprintf(l,"{TRAF}{%X %X}",traf_mail,traf_file);fix_l
  }
  if (extra_used&CEmsi::OHFR){
    sprintf(l,"{OHFR}{%s %s}",wtime.temp,ftime.temp);fix_l
  }
  if (extra_used&CEmsi::TRX){
    sprintf(l,"{TRX#}{[%X]}",cTime+tzUtc*60*60);fix_l
  }
  if (extra_used&CEmsi::TZUTC){
    sprintf(l,"{TZUTC}{[%+03i00]}",tzUtc);fix_l
  }
}

static char * getstrfromEMSI(char *from,char last,char *buffer)
{
  char *l=from,*p=buffer,c;

  if (*l!='{'&&*l!='[') return NULL;
  l++;
  while(*l){
    if (*l==last||*l=='}'){
      c=*l++;
      if (*l!=c&&c==last) break;
    }
    if (*l=='\\'){
      l++;if (*l=='\\') *p++=*l++;
      else {
        int i,e;i=*l++;e=*l++;
        if (i>='0'&&i<='9') i-=48;else if (i>='A'&&i<='F') i-=55;
        else if (i>='a'&&i<='f') i-=87;else return NULL;
        if (e>='0'&&e<='9') e-=48;else if (e>='A'&&e<='F') e-=55;
        else if (e>='a'&&e<='f') e-=87;else return NULL;
        *p++=i*16+e;
      }
      continue;
    }
    *p++=*l++;
  } *p=0;

  return l;
}

static int getlinkcodes(CString& s)
{
  if (s=="PUA") return CEmsi::PUA;
  if (s=="PUP") return CEmsi::PUP;
  if (s=="NPU") return CEmsi::NPU;
  if (s=="HAT") return CEmsi::HAT;
  if (s=="HXT") return CEmsi::HXT;
  if (s=="HRQ") return CEmsi::HRQ;
  if (s=="RH1") return CEmsi::RH1;
  if (s=="8N1") return CEmsi::N81;
  return 0;
}

static int getcompatibility(CString& s)
{
  if (s=="ZAP") return CEmsi::ZAP;
  if (s=="ZMO") return CEmsi::ZMO;
  if (s=="JAN") return CEmsi::JAN;
  if (s=="KER") return CEmsi::KER;
  if (s=="NCP") return CEmsi::NCP;
  if (s=="NRQ") return CEmsi::NRQ;
  if (s=="ARC") return CEmsi::ARC;
  if (s=="XMA") return CEmsi::XMA;
  if (s=="HYD") return CEmsi::HYD;
  if (s=="FNC") return CEmsi::FNC;
  return 0;
}

#define getemsistr if ((l=getstrfromEMSI(l,'}',s2))==NULL) return 0;
#define getexemsi if ((l=getstrfromEMSI(l,']',s2))==NULL) return 0;

int CEmsi::fromstr()
{
  char *l = emsi;
  int len,i;
  long CRC,e;
  
  len=strlen(emsi)-4;
  CRC=0;for(i=0;i<len;i++){
    e=emsi[i]<<8;CRC^=e;
    for(e=0;e<8;e++){
       if (CRC&0x8000) CRC=(CRC<<1)^0x1021;else CRC<<=1;
    }
  }
  strupr(emsi+len);sprintf(s2,"%04hX",CRC);
  if (strcmp(s2,emsi+len)) return 0;
  *(emsi+len)=0;if (strncmp(l,"EMSI_DAT",8)) return 0;
  l+=12;if (strncmp(l,"{EMSI}",6)) return 0;l+=6;

  // cerr << '|' << emsi << '|' << endl;

  getemsistr;x=s2;
  addr.clear();
  while((i=x.Find(' '))>=0){
    addr.push_back(CAddress(x.Left(i)));
    x=x.Mid(i+1);
  }
  addr.push_back(CAddress(x));
  getemsistr;password=s2;
  getemsistr;link_codes=0;
  x=s2;while((i=x.Find(','))>=0){
    link_codes|=getlinkcodes(x.Left(i));
    x=x.Mid(i+1);
  } link_codes|=getlinkcodes(x);
  getemsistr;compatibility=0;
  x=s2;while((i=x.Find(','))>=0){
    compatibility|=getcompatibility(x.Left(i));
    x=x.Mid(i+1);
  } compatibility|=getcompatibility(x);
  getemsistr;mailer_code=s2;
  getemsistr;mailer_name=s2;
  getemsistr;mailer_version=s2;
  getemsistr;mailer_sn=s2;
  extra_used=0;

  while(*l=='{'){
    getemsistr;
    if (!strcmp(s2,"IDENT")){
      l++;
      getexemsi;system=s2;
      getexemsi;city=s2;
      getexemsi;sysop=s2;
      getexemsi;phone=s2;
      getexemsi;baud=s2;
      getexemsi;flags=s2;
      l++;extra_used|=IDENT;

    } else if (!strcmp(s2,"OHFR")){
      getemsistr;x=s2;i=x.Find(' ');
      if (i>0){
        wtime.temp=x.Left(i);ftime.temp=x.Mid(i+1);
        extra_used|=OHFR;
      }
    } else if (!strcmp(s2,"TZUTC")){
      l++;getexemsi;extra_used|=TZUTC;
      tzUtc=gettimezone(s2);l++;
    } else if (!strcmp(s2,"TRX#")){
      l++;getexemsi;extra_used|=TRX;
      sscanf(s2,"%x",&cTime);l++;
    } else if (!strcmp(s2,"TRAF")){
      getemsistr;
      if (sscanf(s2,"%x %x",&traf_mail,&traf_file)==2)
        extra_used|=TRAF;
    } else {
      l++;while(*l){
        if (*l=='}'){ l++;if (*l!='}') break;}
        l++;
      }
    }
  }
  if ((extra_used&TRX)&&(extra_used&TZUTC)){
      cTime-=tzUtc*60*60;
  }
  return 1;
}

int CEmsi::prepeareDAT(char *emsi_buffer)
{
  long CRC,i,e,len;

  tostr();
  sprintf(emsi_buffer,"**EMSI_DAT0000%s",emsi);

  i = strlen(emsi);
  emsi_buffer[10]=hEx[i>>12];emsi_buffer[11]=hEx[(i>>8)&0xF];
  emsi_buffer[12]=hEx[(i>>4)&0xF];emsi_buffer[13]=hEx[i&0xF];
  
  CRC=0;len=strlen(emsi_buffer);
  for(i=2;i<len;i++){
    e=emsi_buffer[i]<<8;CRC^=e;
    for(e=0;e<8;e++){
      if (CRC&0x8000) CRC=(CRC<<1)^0x1021;else CRC<<=1;
    }
  }
  emsi_buffer[len]=hEx[(CRC>>12)&0xF];
  emsi_buffer[len+1]=hEx[(CRC>>8)&0xF];
  emsi_buffer[len+2]=hEx[(CRC>>4)&0xF];
  emsi_buffer[len+3]=hEx[CRC&0xF];emsi_buffer[len+4]=0;
  return len+4;
}
