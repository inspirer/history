// Borland C++ v3.1
//   Standard DOS EXE
//   model = small

// Settings
#define cfg_modem_irq      4     // modem IRQ (3 or 4)
#define cfg_ioport         0x3F8 // modem I/O port
#define cfg_baud           2     // baud_divisor=(115200/baudr) (Ex:2=57600)
#define cfg_command_delay  100   // command delay
#define cfg_wait_answer    4     // Wait for modem answer(sec)
#define cfg_dle            16    // DLE char
#define cfg_etx            3     // ETX char

#define cfg_ring           1     // Answer on ring
#define cfg_wait_offhook   500   // Wait after offhook
#define cfg_speaker_freq   440   // speaker frequency
#define cfg_sound_flag     ""    // create flag?
#define cfg_pause_ring     3000  // pause (ms) between rings
#define cfg_ring_to_line   4     // Rings to line before autoanswer
#define cfg_rec_message    30    // ~ Record message time (sec)
#define WAVE_output        0     // use .wav output
#define WAV_8bit           0     // use 8bit wav
#define cfg_offhook        1     // offhook?

#define cfg_detect_num     30    // noise level
#define cfg_frame          1000  // noise level of frame
#define cfg_log_level      0     // log info
#define cfg_play_device    0     // 0 => device=line ; 1 => (micro)phone
#define show_debug_info    0     // by default: 1 - debug info ; 0 - no info
#define cfg_listen         30    // Line listening (1/10sec)
#define cfg_use_gsm        1     // if modem supports "GSM framed" set 1 here
#define cfg_gsm_delay      15    // Delay between frames, ms (def: 17)
#define cfg_real_close     1     // Close port after exit
#define cfg_auto_detect    0     // Detect tube before AA
#define cfg_post_space     0     // must sysop press space after speach

// AON
#define Use_AON            1
#define AON_request "AT#VTS=[500,500,1]"
#define AON_delay_bef      1
#define AON_delay_after    0
#define AON_digits         7
#define AON_signal         20000000

// Files
#define cfg_voice_dir  ""         // Directory to store voice files
#define cfg_file_allo  "allo.gsm" // Allo sound file
#define cfg_file_wait  "wait.gsm" // Wait sound file
#define cfg_file_auto  "auto.gsm" // AutoAnswer sound file
#define cfg_mailer_ata "ata.sq"   // Flag which force mailer answer call
#define logfile        ""         // Log file

// strings ("~" => DLE ; "!" => ETX)
#define cfg_init_st        "ATZQ0E0#VTD=3F,3F,3F"
#define cfg_deinit_st      "ATZ"
#define cfg_mailinit_st    "ATM1"
#define cfg_offhook_st     "ATH1"
#define cfg_onhook_st      "ATH0"
#define cfg_voice_st       "AT#CLS=8#VRA=0#VRN=0#BDR=24#VSD=1#VSS=2#VSP=55"
#define cfg_data_st        "AT#CLS=0"
#define cfg_stplay_st      "~!"
#define cfg_strec_st       "~!"
#define cfg_beep_st        "AT#VTS=[425,425,10]"
#define cfg_aabeep_st      "AT#VTS=[200,200,5]"
#define cfg_sphone_st      "AT#VGR=5#VLS=2"
#define cfg_mphone_st      "AT#VGT=3#VLS=3"

// if (use_gsm == 0) => use ADPCM
#if cfg_use_gsm==1
 #define cfg_play_st       "AT#VTX"
 #define cfg_rec_st        "AT#VRX"
#else
 #define cfg_play_st       "AT#VSM=129,8000#VTX"
 #define cfg_rec_st        "AT#VSM=129,8000#VRX"
#endif

#pragma warn -aus
#pragma warn -par
#pragma warn -use
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <io.h>
#include <fstream.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "screen.cpp"
#include "comm.cpp"
#include "gsm.cpp"
#include "modem.cpp"
#include "ctl.cpp"
#include "aon.cpp"

void main (int argc, char *argv[])
 {
   cout << "Squirrel 1.16/release, Adaptive answer Service for USR Voice modems" << endl;
   cout << "Copyright (c)2000 Eugeniy Gryaznov, Compiled on 02.05.00 at 21:09" << endl;
   if(argc<2) {
      cout << " Use <squirrel.exe> <action> [<action param>] [switches]" << endl;
      cout << "  action:   PLAY,REC,MAILER,MAIN" << endl;
      cout << "      PLAY f.gsm     Play file to speaker(modem)" << endl;
      cout << "      REC f.gsm      Record from microphone(modem) to file" << endl;
      cout << "      MAILER         Mailer compatible mode" << endl;
      cout << "      MAIN           Run in master mode" << endl;
      cout << "      CONV f.gsm     Convert GSM ->WAV" << endl;
      cout << "  switches: [/L] [/D] [/P]" << endl;
      cout << "      /L             switch Playing/Recording device" << endl;
      cout << "      /D             switch Show debug info" << endl;
      cout << "      /P             switch close/real_close port" << endl;
      cout << "      /B             switch 8/16 bit wave output" << endl;
      cout << " Ex: squirrel play allo.gsm /L /D" << endl;
      cout << "     squirrel main" << endl;
      ErrorExit(0,"Help screen");
   }

   // Get default CTL name
   char ctl[128];
   char *ext = "CTL";
   strcpy(ctl,argv[0]);
   strcpy(ctl+strlen(ctl)-3,ext);
   SetDefault();

   // Check ARGV
   int Task = 0, swch = 0; // 1 - PLAY, 2 - REC, 3 - MAILER, 4 - MAIN
   char TParam[128],fname[128],*outw;
   int postdo = 0;

   // Get task type
   if (strcmp(strupr(argv[1]),"PLAY")==0) { strcpy(TParam,argv[2]);Task=1; }
   else if (strcmp(strupr(argv[1]),"REC")==0) { strcpy(TParam,argv[2]);Task=2; }
   else if (strcmp(strupr(argv[1]),"MAILER")==0) { strcpy(TParam,argv[2]);Task=3; }
   else if (strcmp(strupr(argv[1]),"MAIN")==0) { strcpy(TParam,argv[2]);Task=4; }
   else if (strcmp(strupr(argv[1]),"CONV")==0) { strcpy(TParam,argv[2]);Task=5; }
   else ErrorExit(1,"Unknown action");
   if ((Task==1||Task==2||Task==5)&&argc==2) ErrorExit(1,"not enough params");

   // Process switches
   for (int argnum=2;argnum<argc;argnum++){
      if ((Task!=1&&Task!=2&&Task!=5)||argnum!=2){
        if (strcmp(strupr(argv[argnum]),"/D")==0) swch|=1;
        else if (strcmp(strupr(argv[argnum]),"/L")==0) swch|=2;
        else if (strcmp(strupr(argv[argnum]),"/P")==0) swch|=4;
        else if (strcmp(strupr(argv[argnum]),"/B")==0) swch|=8;
        else ErrorExit(1,"Unknown switch");
      }
   }

   cout << "TASK: ";
   switch(Task){
     case 1: cout << "playing file (device <- " << TParam << ")" << endl;break;
     case 2: cout << "recording file (device -> " << TParam << ")" << endl;break;
     case 3: cout << "mailer mode" << endl;break;
     case 4: cout << "master mode" << endl;break;
   }

   if (Task<5){

     // Read config + FIX switches
     ReadConfig(ctl);
     if (swch&1) cfg.debuginfo=(cfg.debuginfo+1)%2;
     if (swch&2) cfg.pln=(cfg.pln+1)%2;
     if (swch&4) cfg.realcl=(cfg.realcl+1)%2;
     if (swch&8) cfg.wav8bit=(cfg.wav8bit+1)%2;

     // Open COMPort
     OpenComm(cfg.baud,cfg.ioport,cfg.irq);
     if(prtst!=-1) ErrorExit(3,"Communication port not found");

     // Init screen
     StartupScr();

     // Init modem
     SendModemStr(&cfg.init,"Initializing modem");
     SendModemStr(&cfg.voice,"Voice mode");
   }

   // Start Log
   if (Task==3||Task==4){
      struct time _t;
      struct date _d;
      gettime(&_t);getdate(&_d);

      write_log("\n -- executed on ");
      write_log_num(_d.da_day);write_log(".");
      write_log_num(_d.da_mon);write_log(".");
      write_log_num(_d.da_year);write_log(" at ");
      write_log_num(_t.ti_hour);write_log(":");
      write_log_num(_t.ti_min);write_log(" --\n");
   }

   // Main work
   switch(Task){

     case 1:
       PlayFile(TParam);
       break;

     case 2:
       cfg.wavout=0;
       RecFile(TParam,0);
       break;

     case 3:

       if (!cfg.gsmframe) ErrorExit(78,"mailer mode require USE_GSM=1");
       cfg.up=0;cfg.pln=0;
       if (cfg.useaon&1){
         AON();
       } else {
         if (cfg.hook){
           SendModemStr(&cfg.offhook,"Offhook");
           delay(cfg.wallo);
         }
       }
       PlayFile(cfg.sallo);
       if (cfg.loglev&2) write_log("detecting\n");
       switch (Detect()){

         case 1: // Modem
            CreateFlag(cfg.ata);
            SendModemStr(&cfg.data,"Data mode");
            SendModemStr(&cfg.mailinit,"Initializing modem to connect");
            write_log("Detected: MODEM\n");
            ErrorExit(0,"modem detected");
            break;

         case 2: // Voice
            write_log("Detected: VOICE\n");
            PlayFile(cfg.swait);
            for (int curring=0;curring<cfg.RTL;curring++){
               if (!kbhit()){
                if (strlen(cfg.soundfl))
                   CreateFlag(cfg.soundfl);
                else sound(cfg.khz);
                SendModemStr(&cfg.beep,"Beep");
                nosound();
               }
               if (kbhit()) break;
               if (curring+1!=cfg.RTL) delay(cfg.delayring);
            }
            ch=0;while (kbhit()) ch=getch();
            if (cfg.auto_detect&&ch!=27&&ch!=32){
              // check if voice present

              cout << " ! auto : speach in line" << endl;FixLine();
              if (cfg.loglev&2) write_log("detecting\n");
              ch=0;cfg.limit=cfg.auto_detect;
              if ((ch=Detect())==2) ch=32;
              if (ch==3||ch==4) break;
            }
            if (ch!=27) if (ch==32){
              cout << " ! autoanswer skipped" << endl;FixLine();
              write_log("autoanswer skipped\n");
              postdo=1;
            } else {
              PlayFile(cfg.sauto);
              SendModemStr(&cfg.abeep,"aBeep");
              generate_name(fname);
              write_log("Recording: ");write_log(fname);write_log("\n");
              RecFile(fname,cfg.rec_time);
            }
            break;
         case 3:
            write_log("Detected: BUSY\n");break;
         case 4:
            write_log("Detected: DIAL TONE\n");break;
       }
       SendModemStr(&cfg.onhook,"Onhook");
       break;

     case 4:
       int wring;
       char rng[100];cfg.up=0;cfg.pln=0;
       while(kbhit()) getch();

       if (cfg.gsmframe==0&&cfg.useaon!=0) ErrorExit(73,"AON require USE_GSM=1");
       while(!kbhit()){
         cout << "  Waiting for RING ...";rng[0]=0;
         while(!kbhit()&&strstr(rng,"RING")==NULL){
           while(ReadComm(ch)){
             rng[strlen(rng)+1]=0;
             rng[strlen(rng)]=ch;
             if (strlen(rng)==95) rng[0]=0;
           }
         }

         if (!kbhit()){
           cout << endl;FixLine();
           cout << " ! RING .";
           for(wring=0;wring<(cfg.ring-1);wring++){
             if (!WaitFor("RING",7,cfg.debuginfo))
               { cout << " <no more rings>" << endl;FixLine();wring=0;break;}
             else
               cout << ".";
           }

         }
         if (!kbhit()&&wring){

           // Wait cfg.ring

           cout << endl;FixLine();

           if (cfg.useaon&2){
             AON();
           } else {
             SendModemStr(&cfg.offhook,"Offhook");
             delay(cfg.wallo);
           }
           cfg.up=0;
           PlayFile(cfg.sauto);
           SendModemStr(&cfg.abeep,"aBeep");
           generate_name(fname);
           RecFile(fname,cfg.rec_time);
           SendModemStr(&cfg.onhook,"Onhook");
           SendModemStr(&cfg.init,"Initializing modem");
           SendModemStr(&cfg.voice,"Voice mode");
           while(kbhit()) getch();
         }
       }
       cout << endl;FixLine();
       while(kbhit()) getch();
       break;

     case 5:

       // Open files
       if (swch&8) cfg.wav8bit=(cfg.wav8bit+1)%2;
       fp=fopen(TParam,"rb");
       if (fp==NULL) ErrorExit(93,"error: .gsm input");
       outw=TParam;
       cout << "GSM->WAV converting: " << TParam << " -> ";
       while (strchr(outw,'\\')!=NULL) outw=strchr(outw,'\\');
       while (strchr(outw,'/')!=NULL) outw=strchr(outw,'/');
       if (strlen(outw)==0) ErrorExit(153,"out name error");
       if (strchr(outw,'.')!=NULL) *strchr(outw,'.')=0;
       strcat(strlwr(TParam),".wav");
       cout << TParam;if(cfg.wav8bit) cout << " (8bit)";cout << endl;
       if (!Start_GSM_WAV(TParam,cfg.wav8bit)){ cout << "output file error";exit(1);}

       while (fread(gsmb,1,33,fp)==33){
          decode_block(gsmb,cfg.wav8bit);
       }

       // close file
       fclose(fp);
       Close_GSM_WAV();
       ErrorExit(0,"OK");

   }

   // Deinit
   SendModemStr(&cfg.data,"Data mode");
   SendModemStr(&cfg.deinit,"Deinitializing modem");

   if (postdo&&cfg.postspace){
     cout << " ! Press any key to return to Mailer";
     getch();sound(440);delay(5);nosound();
     cout << endl;FixLine();
   }

   // Close COMPort & Exit
   ErrorExit(0,"All ok");

 }
