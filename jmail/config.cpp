// config.cpp

#include "stdafx.h"

#define var(c) ((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_')
#define num(c) (c>='0'&&c<='9')

const static char* keywords[] = {
"address","sysop","bbsname","phone","place",
"flags","timezone","inbound","inboundunsec","log",
"chat_log","comport","baudrate","min_baud_in","min_baud_out",
"waitmdmanswer","modem_delay","init_interval","init1","init2",
"ok","busy","connect","no_dial","ring",
"ncarrier","dial_terminate","dial_prefix","dial_suffix","onhook",
"modem_answer","error_correction","time_dial","call_tries","circle",
"call_time","answer","emsi_oh","answer_ring","wait_carrier",
"protocol_use",""
};

class errorsCfg {
public: char err[80];
  errorsCfg(const char *l){ strcpy(err,l); }
};

static int getfromint(char *val)
{
  int i = 0;
  while(num(*val)){ i*=10;i+=*val-'0';val++;}
  if (*val) throw errorsCfg("invalid number");
  return i;
}

static CString getfromCString(char *val)
{
  if(*val!='"'||val[1]==0||val[strlen(val)-1]!='"') throw errorsCfg("wrong string");
  val[strlen(val)-1]=0;return CString(val+1);
}

void config::defvar(int varn,char *value)
{
  switch(varn){
   case 0: address=getfromCString(value);break;
   case 1: sysop=getfromCString(value);break;
   case 2: bbsname=getfromCString(value);break;
   case 3: phone=getfromCString(value);break;
   case 4: place=getfromCString(value);break;
   case 5: flags=getfromCString(value);break;
   case 6: timezone=getfromint(value);break;
   case 7: inbound=getfromCString(value);break;
   case 8: inboundunsec=getfromCString(value);break;
   case 9: log=getfromCString(value);break;
   case 10: chat_log=getfromCString(value);break;
   case 11: comport=getfromCString(value);break;
   case 12: baudrate=getfromint(value);break;
   case 13: min_baud_in=getfromint(value);break;
   case 14: min_baud_out=getfromint(value);break;
   case 15: waitmdmanswer=getfromint(value);break;
   case 16: modem_delay=getfromint(value);break;
   case 17: init_interval=getfromint(value);break;
   case 18: init1=getfromCString(value);break;
   case 19: init2=getfromCString(value);break;
   case 20: ok=getfromCString(value);break;
   case 21: busy=getfromCString(value);break;
   case 22: connect=getfromCString(value);break;
   case 23: no_dial=getfromCString(value);break;
   case 24: ring=getfromCString(value);break;
   case 25: ncarrier=getfromCString(value);break;
   case 26: dial_terminate=getfromCString(value);break;
   case 27: dial_prefix=getfromCString(value);break;
   case 28: dial_suffix=getfromCString(value);break;
   case 29: onhook=getfromCString(value);break;
   case 30: modem_answer=getfromCString(value);break;
   case 31: error_correction=getfromCString(value);break;
   case 32: time_dial=getfromint(value);break;
   case 33: call_tries=getfromint(value);break;
   case 34: circle=getfromint(value);break;
   case 35: call_time=getfromCString(value);break;
   case 36: answer=getfromCString(value);break;
   case 37: emsi_oh=getfromCString(value);break;
   case 38: answer_ring=getfromint(value);break;
   case 39: wait_carrier=getfromint(value);break;
   case 40:
    if (!strcmp(value,"hydra")) protocol_use=pr_hydra;
    else if (!strcmp(value,"zmodem")) protocol_use=pr_zmodem;
    else throw errorsCfg("invalid value");break;
  }
}

void config::readconfig(const char *n,const char *section)
{
  FILE *f;
  char s[128],*h,*l;
  DWORD lines = 0,i,e,q=0;

  defaults();
  if ((f=fopen(n,"r"))==NULL){
    sprintf(s,"`%s' not found",n);
    throw errors(s);
  }

  while (fgets(s,128,f)!=NULL){
    lines++;h=s;
    while(*h) if (*h=='#'||*h=='\n') *h=0;else h++;
    h=s;while(*h==' ') h++;
    if (!*h) continue;
    if (*h=='['){ 
      l=h;while(*l) if (*l==']') *l=0;else l++;
      q=!strcmp(h+1,section);continue;
    }
    if(!q) continue;
    l=h;while(*l) l++;l--;
    while(l>h&&*l==' ') *l--=0;
    if (!var(*h)){
      sprintf(s,"%s(%i): wrong line",n,lines);
      throw errors(s);    
    }
    l=h;while(var(*l)) l++;
    if (*l=='=') *l++=0;
    else {
      *l++=0;
      while(*l&&*l!='=') l++;
      if (!*l){
        sprintf(s,"%s(%i): wrong line",n,lines);
        throw errors(s);    
      }
      *l++=0;
    }
    while(*l==' ') l++;
    for(i=e=0;*keywords[i];i++) if(!strcmp(h,keywords[i])){
      try {
      e=1;defvar(i,l);
      }
      catch(errorsCfg& e){
        sprintf(s,"%s(%i): %s",n,lines,e.err);
        throw errors(s);
      }
    }
    if (!e){
      sprintf(s,"%s(%i): unknown parameter `%s'",n,lines,h);
      throw errors(s);
    }
  }
  fclose(f);
}

void config::writeconfig(const char *n,const char *section)
{
  FILE *f,*old=NULL;
  char s[30],w[128],*h;

  if(!rename(n,"jMail.tmp"))
    old=fopen("jMail.tmp","r");
  if ((f=fopen(n,"w"))==NULL){
    sprintf(w,"`%s' not created",n);
    throw errors(w);
  }

  if (old){
    while(fgets(w,128,old)!=NULL){
      h=w;while(*h) if (*h=='\n') *h=0;else h++;
      if (*w=='['){ h=w;while(*h) if (*h==']') *h=0;else h++; }
      if (*w=='['&&(!strcmp(w+1,section))){
        while(fgets(w,128,old)!=NULL){
          if (*w=='[') break;*w=0;
        }
        break;
      } else { fprintf(f,"%s\n",w);}
    }
    fprintf(f,"[%s]\n",section);
  } else {
    fprintf(f,"# %s\n\n[%s]\n",n,section);
  }
  
  fprintf(f,"\n");
  fprintf(f,"address=\"%s\"\n",address);
  fprintf(f,"sysop=\"%s\"\n",sysop);
  fprintf(f,"bbsname=\"%s\"\n",bbsname);
  fprintf(f,"phone=\"%s\"\n",phone);
  fprintf(f,"place=\"%s\"\n",place);
  fprintf(f,"flags=\"%s\"\n",flags);
  fprintf(f,"timezone=%i\n",timezone);
  fprintf(f,"inbound=\"%s\"\n",inbound);
  fprintf(f,"inboundunsec=\"%s\"\n",inboundunsec);
  fprintf(f,"log=\"%s\"\n",log);
  fprintf(f,"chat_log=\"%s\"\n",chat_log);
  fprintf(f,"\n");
  fprintf(f,"comport=\"%s\"\n",comport);
  fprintf(f,"baudrate=%i\n",baudrate);
  fprintf(f,"min_baud_in=%i\n",min_baud_in);
  fprintf(f,"min_baud_out=%i\n",min_baud_out);
  fprintf(f,"waitmdmanswer=%i\n",waitmdmanswer);
  fprintf(f,"modem_delay=%i\n",modem_delay);
  fprintf(f,"init_interval=%i\n",init_interval);
  fprintf(f,"init1=\"%s\"\n",init1);
  fprintf(f,"init2=\"%s\"\n",init2);
  fprintf(f,"ok=\"%s\"\n",ok);
  fprintf(f,"busy=\"%s\"\n",busy);
  fprintf(f,"connect=\"%s\"\n",connect);
  fprintf(f,"no_dial=\"%s\"\n",no_dial);
  fprintf(f,"ring=\"%s\"\n",ring);
  fprintf(f,"ncarrier=\"%s\"\n",ncarrier);
  fprintf(f,"dial_terminate=\"%s\"\n",dial_terminate);
  fprintf(f,"dial_prefix=\"%s\"\n",dial_prefix);
  fprintf(f,"dial_suffix=\"%s\"\n",dial_suffix);
  fprintf(f,"onhook=\"%s\"\n",onhook);
  fprintf(f,"modem_answer=\"%s\"\n",modem_answer);
  fprintf(f,"error_correction=\"%s\"\n",error_correction);
  fprintf(f,"\n");
  fprintf(f,"time_dial=%i\n",time_dial);
  fprintf(f,"call_tries=%i\n",call_tries);
  fprintf(f,"circle=%i\n",circle);
  fprintf(f,"call_time=\"%s\"\n",call_time);
  fprintf(f,"\n");
  fprintf(f,"answer=\"%s\"\n",answer);
  fprintf(f,"emsi_oh=\"%s\"\n",emsi_oh);
  fprintf(f,"answer_ring=%i\n",answer_ring);
  fprintf(f,"wait_carrier=%i\n",wait_carrier);
  switch(protocol_use){
    case pr_hydra: strcpy(s,"hydra");break;
    case pr_zmodem: strcpy(s,"zmodem");break;
  } fprintf(f,"protocol_use=%s\n",s);

  if(old){
    fprintf(f,"\n");
    if (*w) fprintf(f,"%s",w);
    while (fgets(w,128,old)!=NULL) fprintf(f,"%s",w);
    fclose(old);
    unlink("jMail.tmp");
  }
  fclose(f);
}

void config::defaults()
{

  address="unknown";
  sysop="unknown";
  bbsname="bbs";
  phone="-Unpublished-";
  place="unknown";
  flags="";
  timezone=0;
  inbound="";
  inboundunsec="";
  log="";
  chat_log="";

  comport="com2";
  baudrate=57600;
  min_baud_in=9600;
  min_baud_out=9600;
  waitmdmanswer=4;
  modem_delay=2;
  init_interval=20;
  init1="ATZ|";
  init2="";
  ok="OK";
  busy="BUSY";
  connect="CONNECT";
  no_dial="NO DIAL TONE";
  ring="RING";
  ncarrier="NO CARRIER";
  dial_terminate="v\''^\'|";
  dial_prefix="ATD";
  dial_suffix="|";
  onhook="~v~~^~ATH0|";
  modem_answer="ATA|";
  error_correction="V42|MNP|COMP";

  time_dial=60;
  call_tries=15;
  circle=10;
  call_time="0:00-24:00";

  answer="";
  emsi_oh="0:00-0:00";
  answer_ring=0;
  wait_carrier=50;
  protocol_use=pr_hydra;
}
