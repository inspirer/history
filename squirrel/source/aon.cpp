unsigned char gsmb[33];

// AON procedure (H1)
void AON()
{
  while(kbhit()) getch();
  if (cfg.hook) SendModemStr(&cfg.offhook,"Offhook");
  if (cfg.delay1>0) delay(cfg.delay1);
  SendModemStr(&cfg.aonreq,"AON Request");
  if (cfg.delay2>0) delay(cfg.delay2);

  long cmddel;

  cmddel=cfg.cmddelay;cfg.cmddelay=0;
  cout << " þ Detecting number ..." << endl;FixLine();
  SendModemStr(&cfg.rec,"Listening line");
  cout << "   Listening ...";
  cfg.cmddelay=cmddel;

  int buffs;
  char phone[80],blc=0;
  long i,k,A[6],sum,ver[80];
  short Nu[6] = { 700, 900, 1100, 1300, 1500, 1700 };
  double w;

  while (blc<80){

    buffs=0;
    while(buffs<38){
      while(!ReadComm(ch));

      // Get character
      if (ch==cfg.dle){
         while(!ReadComm(ch));
         if (ch==cfg.dle){ prbuf[buffs]=ch;buffs++;}
      } else {
        prbuf[buffs]=ch;
        buffs++;
      }
    }

    // Decode & calculate A
    gsm_decode(&gsm,prbuf+2,wavb);
    for (i=0;i<6;i++){
      w=3.141592*Nu[i]/4000;
      sum=0;for (k=0;k<160;k++) sum+=wavb[k]*cos(w*k);sum/=100;sum*=sum;
      A[i]=sum;
      sum=0;for (k=0;k<160;k++) sum+=wavb[k]*sin(w*k);sum/=100;sum*=sum;
      A[i]+=sum;
    }

    long max,i1,i2,nm;max=0;i1=0;i2=0;
    for (i=0;i<6;i++) if (A[i]>max){ max=A[i];i1=i;}
    max=0;for (i=0;i<6;i++) if (A[i]>max&&i!=i1){ max=A[i];i2=i;}
    if (i1>i2) nm=i2*6+i1; else nm=i1*6+i2;

    switch(nm){
     case  1: nm=1;break;  case  2: nm=2;break;
     case  8: nm=3;break;  case  3: nm=4;break;
     case  9: nm=5;break;  case 15: nm=6;break;
     case  4: nm=7;break;  case 10: nm=8;break;
     case 16: nm=9;break;  case 22: nm=0;break;
     case  5: nm=11;break; case 11: nm=12;break;
     case 17: nm=13;break; case 23: nm=14;break;
     case 29: nm=15;break; default: nm=-1;break;
    }

    max=A[i1]+A[i2];for (i=0;i<6;i++) if (i!=i1&&i!=i2) max-=A[i];

    phone[blc]=nm;
    ver[blc]=max;
    blc++;

    if (cfg.loglev&1){
      write_log_num(i1);write_log(" ");
      write_log_num(i2);write_log(" : ");
      write_log_num(nm);write_log(" , ");
      write_log_num(max);write_log("  :  ");
      for (i=0;i<6;i++){
         write_log_num(A[i]);write_log(" ");
      }
      write_log("\n");
      //cout << i1 << " " << i2 << " : " << nm << " , " << max <<"   :   ";
      //for (i=0;i<6;i++) cout << A[i] << " ";
      //cout << endl;FixLine();
    }
  }

  cout << endl;FixLine();
  SendModemStr(&cfg.strec,"Stop");
  SendModemStr(&cfg.voice,"Voice");

  char realph[20];realph[0]=0;k=0;
  for (i=blc-1;i>=0;i--){

     if (phone[i]==13&&phone[i+1]==13&&phone[i-1]!=13){
       for (k=0;k<cfg.digits;k++){

          if ((k*2+2)>i){
             ch='-';
          } else {
             if (ver[i-1-2*k]>ver[i-2-2*k]){
                if (ver[i-1-2*k]<cfg.minver)ch='?';
                else ch=phone[i-1-2*k];
             } else {
                if (ver[i-2-2*k]<cfg.minver)ch='?';
                else ch=phone[i-2-2*k];
             }
             if (ch!=14) ch+='0'; else ch=realph[strlen(realph)-1];
          }
          realph[strlen(realph)+1]=0;
          realph[strlen(realph)]=ch;
       }
       k=1;
       break;
     }
  }
  if (!k) for (k=0;k<cfg.digits;k++){realph[k]='-';realph[k+1]=0;}
  cout << " ! Phone: " << realph << endl;FixLine();
  write_log("Phone: ");write_log(realph);write_log("\n");
}
