// CTL functions


int ValAsInt(char *val)
{
  unsigned long nt;

  nt=strtoul(val,0,0);
  if (strcmp(val,"0")!=0&&nt==0) ErrorExit(78,"Invalid integer format");
  //if (cfg.debuginfo) cout << "(" << nt << ") ";

  return nt;
}

void DefineVar(char *vr,char *val)
{
  strlwr(vr);
  //if (cfg.debuginfo) cout << "[" << vr << "," << val << "] ";

  if (!strcmp(vr,"modem_irq")) cfg.irq=ValAsInt(val)+8;
  else if (!strcmp(vr,"ioport")) cfg.ioport=ValAsInt(val);
  else if (!strcmp(vr,"baud")) cfg.baud=ValAsInt(val);
  else if (!strcmp(vr,"command_delay")) cfg.cmddelay=ValAsInt(val);
  else if (!strcmp(vr,"wait_answer")) cfg.waitans=ValAsInt(val);
  else if (!strcmp(vr,"dle")) cfg.dle=ValAsInt(val);
  else if (!strcmp(vr,"etx")) cfg.etx=ValAsInt(val);
  //modem_irq       4        cfg.irq
  //ioport          0x3F8    cfg.ioport
  //baud            2        cfg.baud
  //command_delay   100      cfg.cmddelay
  //wait_answer     4        cfg.waitans
  //dle             16       cfg.dle
  //etx             3        cfg.etx

  else if (!strcmp(vr,"ring")) cfg.ring=ValAsInt(val);
  else if (!strcmp(vr,"wait_offhook")) cfg.wallo=ValAsInt(val);
  else if (!strcmp(vr,"speaker_freq")) cfg.khz=ValAsInt(val);
  else if (!strcmp(vr,"sound_flag")) strcpy(cfg.soundfl,val);
  else if (!strcmp(vr,"pause_ring")) cfg.delayring=ValAsInt(val);
  else if (!strcmp(vr,"ring_to_line")) cfg.RTL=ValAsInt(val);
  else if (!strcmp(vr,"rec_message")) cfg.rec_time=ValAsInt(val);
  else if (!strcmp(vr,"wave_output")) cfg.wavout=ValAsInt(val);
  else if (!strcmp(vr,"wav_8bit")) cfg.wav8bit=ValAsInt(val);
  else if (!strcmp(vr,"hook")) cfg.hook=ValAsInt(val);
  //ring            1        cfg.ring
  //wait_offhook    500      cfg.wallo
  //speaker_freq    440      cfg.khz
  //cfg_sound_flag  ""       cfg.soundfl
  //pause_ring      3000     cfg.delayring
  //ring_to_line    4        cfg.RTL
  //rec_message     30       cfg.rec_time
  //WAVE_output     0        cfg.wavout
  //WAV_8bit        0        cfg.wav8bit
  //cfg_offhook     1        cfg.hook

  else if (!strcmp(vr,"detect_num")) cfg.limit=ValAsInt(val);
  else if (!strcmp(vr,"frame")) cfg.frame=ValAsInt(val);
  else if (!strcmp(vr,"log_level")) cfg.loglev=ValAsInt(val);
  else if (!strcmp(vr,"play_device")) cfg.pln=ValAsInt(val);
  else if (!strcmp(vr,"show_debug_info")) cfg.debuginfo=ValAsInt(val);
  else if (!strcmp(vr,"listen")) cfg.hear=ValAsInt(val);
  else if (!strcmp(vr,"use_gsm")) cfg.gsmframe=ValAsInt(val);
  else if (!strcmp(vr,"real_close")) cfg.realcl=ValAsInt(val);
  else if (!strcmp(vr,"auto_detect")) cfg.auto_detect=ValAsInt(val);
  else if (!strcmp(vr,"post_space")) cfg.postspace=ValAsInt(val);
  //detect_num      30       cfg.limit
  //frame           1000     cfg.frames
  //log_level       0        cfg.loglev
  //play_device     0        cfg.pln
  //show_debug_info 0        cfg.debuginfo
  //listen          15       cfg.hear
  //use_gsm         1        cfg.gsmframe
  //gsm_delay       15       ~
  //real_close      1        cfg.realcl
  //auto_detect     0        cfg.auto_detect
  //cfg_post_space  0        cfg.postspace

  else if (!strcmp(vr,"use_aon")) cfg.useaon=ValAsInt(val);
  else if (!strcmp(vr,"aon_request")) strcpy(cfg.aonreq.st,val);
  else if (!strcmp(vr,"aon_delay_bef")) cfg.delay1=ValAsInt(val);
  else if (!strcmp(vr,"aon_delay_after")) cfg.delay2=ValAsInt(val);
  else if (!strcmp(vr,"aon_signal")) cfg.minver=ValAsInt(val);
  else if (!strcmp(vr,"aon_digits")) cfg.digits=ValAsInt(val);
  //Use_AON            1
  //AON_request "AT#VTS=[500,500,1]"
  //AON_delay_bef      1
  //AON_delay_after    0
  //aon_signal         2000000 cfg.minver
  //AON_digits         7       cfg.digits

  else if (!strcmp(vr,"voice_dir")) strcpy(cfg.vdir,val);
  else if (!strcmp(vr,"file_allo")) strcpy(cfg.sallo,val);
  else if (!strcmp(vr,"file_wait")) strcpy(cfg.swait,val);
  else if (!strcmp(vr,"file_auto")) strcpy(cfg.sauto,val);
  else if (!strcmp(vr,"mailer_ata")) strcpy(cfg.ata,val);
  else if (!strcmp(vr,"logfile")) strcpy(cfg.log,val);
  //voice_dir       ""           cfg.vdir
  //file_allo       "allo.gsm"   cfg.sallo
  //file_wait       "wait.gsm"   cfg.swait
  //file_auto       "auto.gsm"   cfg.sauto
  //mailer_ata      "ata.sq"     cfg.ata
  //logfile         "squirrel.log" cfg.log

  else if (!strcmp(vr,"init_st")) strcpy(cfg.init.st,val);
  else if (!strcmp(vr,"deinit_st")) strcpy(cfg.deinit.st,val);
  else if (!strcmp(vr,"mailinit_st")) strcpy(cfg.mailinit.st,val);
  else if (!strcmp(vr,"offhook_st")) strcpy(cfg.offhook.st,val);
  else if (!strcmp(vr,"onhook_st")) strcpy(cfg.onhook.st,val);
  else if (!strcmp(vr,"voice_st")) strcpy(cfg.voice.st,val);
  else if (!strcmp(vr,"data_st")) strcpy(cfg.data.st,val);
  else if (!strcmp(vr,"stplay_st")) strcpy(cfg.stplay.st,val);
  else if (!strcmp(vr,"strec_st")) strcpy(cfg.strec.st,val);
  else if (!strcmp(vr,"beep_st")) strcpy(cfg.beep.st,val);
  else if (!strcmp(vr,"aabeep_st")) strcpy(cfg.abeep.st,val);
  else if (!strcmp(vr,"sphone_st")) strcpy(cfg.sphone.st,val);
  else if (!strcmp(vr,"mphone_st")) strcpy(cfg.mphone.st,val);
  //init_st         "ATZQ0E0#VTD=3F,3F,3F"
  //deinit_st       "ATZ"
  //mailinit_st     "ATM1"
  //offhook_st      "ATH1"
  //onhook_st       "ATH0"
  //voice_st        "AT#CLS=8#VRA=0#VRN=0#BDR=24#VSD=1#VSS=2#VSP=55"
  //data_st         "AT#CLS=0"
  //stplay_st       "~!"
  //strec_st        "~!"
  //beep_st         "AT#VTS=[425,425,10]"
  //aabeep_st       "AT#VTS=[200,200,5]"
  //sphone_st       "AT#VGR=5#VLS=2"
  //mphone_st       "AT#VGT=3#VLS=3"

  else if (!strcmp(vr,"play_st")) strcpy(cfg.play.st,val);
  else if (!strcmp(vr,"rec_st")) strcpy(cfg.rec.st,val);
  //play_st         "AT#VTX"
  //rec_st          "AT#VRX"

  else { cout<< vr <<": ";ErrorExit(73,"Unknown keyword");}

}

void ReadConfig(char *cname)
{
  cout << "reading CTL : " << cname << " : ";
  FILE *fp;
  int line=1;

  fp=fopen(cname,"rb");
  if (fp!=NULL){

    char b[100],rd,st[200],*keyw,*keyv;st[0]=0;
    while((rd=fread(b,1,100,fp))!=0){
      for (int cn=0;cn<rd;cn++){
        if (b[cn]==13||b[cn]==10){
           if (b[cn]==13) line++;
           if (strchr(st,';')!=NULL) *strchr(st,';')=0;

           keyw=st;
           while(strlen(keyw)>0&&keyw[0]==' ') keyw++;
           if (*keyw!=0){
             keyv=strchr(keyw,' ');

             if (keyv==NULL&&strlen(keyw)>0){ cout << "line " << line;ErrorExit(51," : cfg error");}

             keyv[0]=0;keyv++;
             while(strlen(keyv)>0&&keyv[0]==' ') keyv++;
             while(strlen(keyv)>0&&keyv[strlen(keyv)-1]==' ') keyv[strlen(keyv)-1]=0;

             if (strlen(keyv)==0){ cout << "line " << line;ErrorExit(58," : cfg error");}

             DefineVar(keyw,keyv);
           }

           st[0]=0;

        } else {
           st[strlen(st)+1]=0;
           st[strlen(st)]=b[cn];
        }
      }
    }

    fclose(fp);
  } else ErrorExit(92,"FATAL: cfg not found");
  cout << "OK" << endl;

}
