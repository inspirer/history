// --------------------------- Watcom C++ modem ----------------------------
// void OpenComm(int baud,int address,int intrq)
// void CloseComm()
// void RealCloseComm()
// char ReadComm(char& chc)
// void WriteComm(char chsend)
// void WriteCommStr(char *sts)
// int WaitFor(char *ws,int tm)
// ----------------------------------(c) Eugeniy Gryaznov 2:5030/611.23 ----

const bufsize=4092;

 int _iobase = 0x3F8;                   // Base I/O
 int _irq = 12;                         // IRQ
 void interrupt (*oldvect)(...);        // OldVec
 int RHead,RTail;                       // Pointers
 char mstat;                            // Modem status
 int prtst = 0;                         // Port status (0xFFFF - opened)
 char rbuff[bufsize];                   // Receive/Transmit Buffers

void Delay(unsigned long ms)
 {
  REGS rg;

  rg.h.ah=0x86;
  ms*=1000;
  rg.x.cx=ms>>16;
  rg.x.dx=ms&0xFFFF;
  int86(0x15,&rg,&rg);
 }

 // Modem handler
void interrupt modemint(...)
 {
   char bt;
   bt=inp(_iobase+2);

   rbuff[RHead]=inp(_iobase);
   RHead++;RHead=RHead%bufsize;

   if ((_irq==10)||(_irq==2)) outp(0xA0,0x20);
   outp(0x20,0x20);
 }

 // Open communication port
void OpenComm(int baud,int address,int intrq)
 {
   // Install interrupt handler
   _iobase=address;_irq=intrq;
   oldvect = _dos_getvect(_irq);
   _dos_setvect(_irq,modemint);

   // Enable interrupts
   if(_irq==12){outp(0x21,inp(0x21)&0xEF);}
    else {if(_irq==11) outp(0x21,inp(0x21)&0xF7);}

   // Configure Port
   outp(_iobase+3,0x80);
   outp(_iobase+1,0);
   outp(_iobase,baud);
   outp(_iobase+3,3);
   outp(_iobase+1,1);
   outp(_iobase+4,11);  // 8

   // Startup values
   RHead=0;RTail=0;mstat=inp(_iobase+6);prtst=-1;
   if((mstat&0x30)==0) prtst=0;
 }

 // Close communication port
void CloseComm()
 {
   // Uninstall interrupt handler
   if(prtst==-1) _dos_setvect(_irq,oldvect);
   prtst=0;
 }

 // Full Close communication port
void RealCloseComm()
 {
   if(prtst==-1){
     // Disable interrupts
     if(_irq==12){outp(0x21,inp(0x21)|0x10);}
      else {if(_irq==11) outp(0x21,inp(0x21)|0x08);}

     // Close modem
     outp(_iobase+3,inp(_iobase+3)&0x7F);
     outp(_iobase+1,0);
     outp(_iobase+4,0);

     // Uninstall interrupt handler
     _dos_setvect(_irq,oldvect);
   }
   prtst=0;
 }

 // Write Comm
void WriteComm(char chsend)
 {
   while((inp(_iobase+5)&0x20)==0);// {for(int ih=0;ih<100;ih++);}
   outp(_iobase,chsend);
 }

 // Read Comm
char ReadComm(char& chc)
 {
   if(RTail!=RHead){
     chc=rbuff[RTail];
     RTail++;RTail=RTail%bufsize;
     return 1;
   } else return 0;
 }

 // Write Set of Characters to Comm
void WriteCommStr(char *sts)
 {
   for(int i=0;sts[i]!=0;i++) WriteComm(sts[i]);
 }

 // Wait for sequence
int WaitFor(char *ws,int tm,int wfdebug)
 {
   char ch,str[100],wf[100];
   int strln,sec100;
   struct time t;
   time_t et;

   str[0]=0;strcpy(wf,ws);strln=strlen(wf);
   wf[strln]=13;wf[strln+1]=10;wf[strln+2]=0;
   et=time(NULL);gettime(&t);sec100=t.ti_hund;et+=tm;

   do{
     while(ReadComm(ch)){
       if (wfdebug==1){ cout << ch;FixLine();}
       strln=strlen(str);
       str[strln+1]=0;
       str[strln]=ch;
       if (strlen(str)==90) for (int dfg=10;dfg<=90;dfg++) str[dfg-10]=str[dfg];
       if(strstr(str,wf)!=NULL) return 1;
     }
     gettime(&t);
   } while((et>time(NULL))||(et==time(NULL)&&sec100<t.ti_hund));
//   cout<< str;
   return 0;
 }
