
struct mc { char st[80],ok[80]; };

struct config
 {
   mc init,offhook,onhook,voice,data,play,stplay,mailinit;
   mc rec,strec,beep,sphone,mphone,abeep,deinit,aonreq;

   char irq,dle,etx,ring;              // Interrupt,DLE,ETX,AnsOnRING
   unsigned int waitans,cmddelay;      // Wait for answer(1/10),Delay cmd (ms)
   unsigned int ioport,baud,khz;       // I/O Port,Baud Rate,PCSpeaker freq.
   int debuginfo,gsmframe,pln;         // Debug,Use GSM,Play/Rec to/from line
   char vdir[128],realcl,sallo[128];   // VoiceDIR,realclose?,sound:allo
   int hear,up,RTL,rec_time,wavout;    // time to listen,do off/on hook
   char swait[128],sauto[128],ata[128];// sound:wait, sound:auto

   unsigned int delayring,limit,wallo; // Pause RING,Limit,Wait After OffHook
   int wav8bit,delay1,delay2,useaon;   // 8-bit wav,AON delay1&2,use_aon
   int hook,digits;                    // aondebug,do offhook?
   long minver,auto_detect,postspace;  // min ver,auto detect voice
   char log[128],soundfl[128];         // log name
   int loglev,frame;              // debug?, frame noise level
 };

 config cfg;
 FILE *fp;
 char ch;
 FILE *Log;

void ErrorExit(int code,char *str)
 {
   while (kbhit()) getch();
   if (cfg.debuginfo){
      cout << " ! Press any key to return" << endl;
      while(kbhit()) getch();
      long _o=time(NULL)+10;
      while(kbhit()==0&&time(NULL)<=_o);
   }
   ExitScr();
   if (cfg.realcl&&prtst==-1){
     cout<< "real: ";
     RealCloseComm();
   } else CloseComm();

   nosound;fclose(Log);
   cout << str << " (ErrorLev " << code << ")" << endl << endl;
   exit (code);
 }

void write_log(char *lg)
{
  if (cfg.log[0]!=0){
    if (Log==NULL){
      Log=fopen(cfg.log,"a");
      if (Log==NULL) ErrorExit(82,"log appending error");
    }
    fprintf(Log,lg);
  }
}

void write_log_num(long nm)
{
  char num[20];

  num[0]=0;
  if (nm==0){
    num[0]='0';num[1]=0;
  } else while(nm>0){
    num[strlen(num)+1]=0;
    num[strlen(num)]=nm%10+'0';
    nm/=10;
  }strrev(num);
  write_log(num);
}

int DetectFlag(char *fname)
 {
   int fhandle;

   if ((fhandle=open(fname,O_RDONLY))!=-1) {
       close(fhandle);return 1;
   } else return 0;
 }

void CreateFlag(char *fname)
 {
   FILE *out;
   if ((out = fopen(fname,"wb"))!=NULL){ fclose(out);}
 }

void SetDefault()
 {
   // Strings
   strcpy(cfg.init.st,cfg_init_st);       strcpy(cfg.init.ok,"OK");
   strcpy(cfg.deinit.st,cfg_deinit_st);   strcpy(cfg.deinit.ok,"OK");
   strcpy(cfg.offhook.st,cfg_offhook_st); strcpy(cfg.offhook.ok,"OK");
   strcpy(cfg.onhook.st,cfg_onhook_st);   strcpy(cfg.onhook.ok,"OK");
   strcpy(cfg.voice.st,cfg_voice_st);     strcpy(cfg.voice.ok,"OK");
   strcpy(cfg.data.st,cfg_data_st);       strcpy(cfg.data.ok,"OK");
   strcpy(cfg.play.st,cfg_play_st);       strcpy(cfg.play.ok,"CONNECT");
   strcpy(cfg.rec.st,cfg_rec_st);         strcpy(cfg.rec.ok,"CONNECT");
   strcpy(cfg.stplay.st,cfg_stplay_st);   strcpy(cfg.stplay.ok,"VCON");
   strcpy(cfg.strec.st,cfg_strec_st);     strcpy(cfg.strec.ok,"VCON");
   strcpy(cfg.beep.st,cfg_beep_st);       strcpy(cfg.beep.ok,"OK");
   strcpy(cfg.abeep.st,cfg_aabeep_st);    strcpy(cfg.abeep.ok,"OK");
   strcpy(cfg.sphone.st,cfg_sphone_st);   strcpy(cfg.sphone.ok,"VCON");
   strcpy(cfg.mphone.st,cfg_mphone_st);   strcpy(cfg.mphone.ok,"VCON");
   strcpy(cfg.mailinit.st,cfg_mailinit_st);strcpy(cfg.mailinit.ok,"OK");
   strcpy(cfg.aonreq.st,AON_request);     strcpy(cfg.aonreq.ok,"OK");

   strcpy(cfg.sallo,cfg_file_allo);
   strcpy(cfg.swait,cfg_file_wait);
   strcpy(cfg.sauto,cfg_file_auto);
   strcpy(cfg.ata,cfg_mailer_ata);
   cfg.irq=cfg_modem_irq+8;
   cfg.ioport=cfg_ioport;
   cfg.baud=cfg_baud;
   cfg.dle=cfg_dle;
   cfg.etx=cfg_etx;
   cfg.ring=cfg_ring;
   cfg.waitans=cfg_wait_answer;
   cfg.cmddelay=cfg_command_delay;
   cfg.khz=cfg_speaker_freq;
   cfg.debuginfo=show_debug_info;
   cfg.gsmframe=cfg_use_gsm;
   cfg.pln=cfg_play_device;
   strcpy(cfg.vdir,cfg_voice_dir);
   cfg.delayring=cfg_pause_ring;
   cfg.limit=cfg_detect_num;
   cfg.wallo=cfg_wait_offhook;
   cfg.realcl=cfg_real_close;
   cfg.hear=cfg_listen;
   cfg.up=1;cfg.rec_time=cfg_rec_message;
   cfg.RTL=cfg_ring_to_line;
   cfg.wavout=WAVE_output;
   cfg.wav8bit=WAV_8bit;
   cfg.delay1=AON_delay_bef;
   cfg.delay2=AON_delay_after;
   cfg.useaon=Use_AON;
   cfg.minver=AON_signal;
   strcpy(cfg.log,logfile);
   strcpy(cfg.soundfl,cfg_sound_flag);
   cfg.hook=cfg_offhook;
   cfg.digits=AON_digits;
   cfg.auto_detect=cfg_auto_detect;
   cfg.postspace=cfg_post_space;
   cfg.frame=cfg_frame;
   cfg.loglev=cfg_log_level;
 }

void SendModemStr(mc *snd,char *str)
 {
   int wfor=0;
   for(int cmdnum=1;(cmdnum<=3)&&(wfor==0);cmdnum++){
      cout << "   " << str << " : ";
      for (int cnt=0;cnt<strlen(snd->st);cnt++){
        if (snd->st[cnt]=='~') snd->st[cnt]=cfg.dle;
        if (snd->st[cnt]=='!') snd->st[cnt]=cfg.etx;
      }

      if (cfg.cmddelay>0) Delay(cfg.cmddelay);
      WriteCommStr((*snd).st);WriteComm(13);
      if (cfg.debuginfo){
         cout << snd->st << "(" << snd->ok << ")" << endl;FixLine();
         wfor=WaitFor(snd->ok,cfg.waitans,1);
         cout << endl;FixLine();
         cout << "   Result: ";
      } else {
         wfor=WaitFor(snd->ok,cfg.waitans,0);
      }

      if (wfor){
          cout << "OK" << endl;FixLine();
      } else {
          cout << "ERROR" << endl;FixLine();
      }
   }
   if (!wfor) ErrorExit(4,"Command send error");
 }

char prbuf[4092];

void PlayFile(char *fname)
 {
   cout << " þ Playing file : " << fname << endl;FixLine();
   fp=fopen(fname,"rb");
   while(kbhit()) getch();

   // If fileopened;
   if (fp!=NULL){

      if (cfg.pln)
         SendModemStr(&cfg.sphone,"Phones");
      else
         if (cfg.up) SendModemStr(&cfg.offhook,"Offhook");

      SendModemStr(&cfg.play,"Voice playback");
      cout << "   Playing ...";
      while (fread(prbuf,1,33,fp)==33){

        if (cfg.gsmframe){
          WriteComm(0xFE);WriteComm(0xFE);
        }

        for (int buffs=0;buffs<33;buffs++){
          if (prbuf[buffs]==cfg.dle) WriteComm(cfg.dle);
          WriteComm(prbuf[buffs]);
        }
        if (cfg.gsmframe){
          WriteComm(0);WriteComm(0xA5);WriteComm(0xA5);
          delay(cfg_gsm_delay);
        }
        if (kbhit()){ cout << "(stopped)";break;}
      }
      cout << endl;FixLine();
      SendModemStr(&cfg.stplay,"Stop");
      if (!cfg.pln)
         if (cfg.up) SendModemStr(&cfg.onhook,"Onhook");

      fclose(fp);

   } else ErrorExit(5,"File not found");

 }

void RecFile(char *fname,int ttime)
 {
   cout << " þ Recording file : " << fname << endl;FixLine();
   if (cfg.wavout&&cfg.gsmframe){
     if (!Start_GSM_WAV(fname,cfg.wav8bit)) ErrorExit(122,".wav file not created");
   } else {
     fp=fopen(fname,"wb");
   }
   while(kbhit()) getch();
   int ctime=0;

   // If fileopened;
   if (fp!=NULL||cfg.wavout){

      if (cfg.pln)
         SendModemStr(&cfg.mphone,"Microphone");
      else
         if (cfg.up) SendModemStr(&cfg.offhook,"Offhook");

      SendModemStr(&cfg.rec,"Voice record");
      cout << "   Recording ...";
      int buffs=0;

      while ((ctime<ttime||ttime==0)&&(kbhit()==0)&&ctime!=-1){
       delay(990);ctime++;
       while(ReadComm(ch)){

          // Get character
          if (ch==cfg.dle){
             while(!ReadComm(ch));
             if (ch==cfg.dle){
               prbuf[buffs]=ch;buffs++;
             } else switch(ch){
               case 'b': cout << "b";ctime=-1;break;
               case 'd': cout << "d";ctime=-1;break;
             }
          } else {
            prbuf[buffs]=ch;
            buffs++;
          }

          // Save buffer?
          if (cfg.gsmframe){
            if (buffs==38){
               if (cfg.wavout) decode_block(prbuf+2,cfg.wav8bit);
                        else fwrite(prbuf+2,33,1,fp);
               buffs=0;
            }
          } else {
            if (buffs==40){fwrite(prbuf,40,1,fp);buffs=0;}
          }
       }
      }

      cout << endl;FixLine();
      SendModemStr(&cfg.strec,"Stop");
      SendModemStr(&cfg.voice,"Voice");
      if (!cfg.pln)
         if (cfg.up) SendModemStr(&cfg.onhook,"Onhook");

      if (cfg.wavout&&cfg.gsmframe) Close_GSM_WAV(); else fclose(fp);

   } else ErrorExit(121,".gsm file not created");
 }

int Detect()
 {
   cout << " þ Detecting" << endl;FixLine();

   SendModemStr(&cfg.rec,"Prepearing");
   cout << "   Listening ...";

   if (cfg.loglev&2){
     write_log("DETECTING: ");
   }

   long buffs=0,blcks=0,reslt=0;

   while(blcks<5*cfg.hear&&reslt>=0){
      while(!ReadComm(ch));
      if (ch==cfg.dle){
        while(!ReadComm(ch));
        if (ch==cfg.dle){
          prbuf[buffs]=ch;buffs++;
        } else switch(ch){
         case 'b': cout << "b";reslt=-1;break;
         case 'd': cout << "d";reslt=-2;break;
        }
      } else {
        prbuf[buffs]=ch;
        buffs++;
      }

      if (buffs==38){
        gsm_decode(&gsm,prbuf+2,wavb);
        blcks++;buffs=0;

        // Search for max volume
        int max=0;for (int _qq=0;_qq<160;_qq++) if (wavb[_qq]>max) max=wavb[_qq];

        // Check for silence
        reslt+=max/cfg.frame;
        if (cfg.loglev&2){
          write_log_num(max);write_log(" ");
        }
      }

   }
   if (cfg.loglev&2) write_log("\n");

   cout << endl;FixLine();
   SendModemStr(&cfg.strec,"Stop");
   SendModemStr(&cfg.voice,"Voice");

   cout << " ! (noise=" << reslt << ") ";
   if (cfg.loglev&2){
     write_log("noise=");write_log_num(reslt);write_log("\n");
   }

   if (reslt==-1){ cout << "BUSY" << endl;FixLine();return 3;}
   else if (reslt==-2){ cout << "DIAL TONE" << endl;FixLine();return 4;}
   else if (reslt>cfg.limit){ cout << "VOICE" << endl;FixLine();return 2;}
    else { cout << "MODEM"<< endl;FixLine();return 1; }

 }

void generate_name(char *nm)
{
  char tbl[]="0123456789";
  struct time t;
  unsigned char _a,_b;
  char nm2[128];

  gettime(&t);
  _a=t.ti_hour/10;_b=t.ti_hour%10;nm[0]=tbl[_a];nm[1]=tbl[_b];
  _a=t.ti_min/10;_b=t.ti_min%10;nm[3]=tbl[_a];nm[4]=tbl[_b];
  _a=t.ti_sec/10;_b=t.ti_sec%10;nm[6]=tbl[_a];nm[7]=tbl[_b];

  nm[2]='_';nm[5]='_';nm[8]=0;

  if (cfg.gsmframe){ if (cfg.wavout) strcat(nm,".wav"); else strcat(nm,".gsm");  }
     else strcat(nm,".pcm");
  strcpy(nm2,cfg.vdir);strcat(nm2,nm);strcpy(nm,nm2);
}
