// --------------------------------------------------------------------------
// ChaOS ASseMbler v1.18 (c)1999 Eugeniy Gryaznov
// Needed :  "files.cpp"      Common routines
//           "i386.cpp"       Assembly functions
//           "i386def.h"      Defines for i386.h
//           "i386ins.h"      List of instructions
//           "i386flt.h"      List of float instructions
// --------------------------------------------------------------------------

#define MaxIfDef 5000
#define lbufsize 100000
//#define debug_list 1
//#define DEBUG

#include <fstream.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Buffer for labels
char lbn[lbufsize];          // Label's buffer
long lend=0;                 // Pointer to end

// Vars
char spath[128],datf[128];   // start path
char second,spc;             // pass num,space counter
char s[256],p[256];          // main strings
char outfile[128];           // Name of output file
FILE *out,*dat;              // OutFile,DataFile
char oldc;                   // Previous char
unsigned long outoffs,lines; // offset in outfile,lines in source
char waitfor,wst[256];       // 0 - noth ; 1 - if { ; 2 - while { ; ...
char lcl[128];               // local label extension
int macrost;                 // Macro definition status
char mn[128];                // Macro end
char mparam[10][128];        // Params table
char ifdef[MaxIfDef];        // Table of ifdef
int cdef;

// temp vars
char txt;                    // text ident
char q[256],z[256];          //
unsigned long a,b,c,d;       //
char *l,*h;                  // pointers to str

// {}
char lbls[255][32],lble[255][32],lblt[255];
int op,st,lb;

fstream output( "nul" );

// Include libs
#include "files.cpp"
#include "i386.cpp"

// ============================ Add label to base ===========================

// (error:70-73)
void AddLabel(char *ll,unsigned long lps,char *plc)
{
  char al_P[128],al_Q[128];
  unsigned long al_A;

  // DEBUG
  #ifdef debug_list
    cout << ll << ":" << endl;
  #endif

  if (second==0){

    // Check for valid letters
    for (al_A=0;al_A<strlen(ll);al_A++)
      if (!(((ll[al_A]>96)&&(ll[al_A]<123))||(ll[al_A]=='@')||(ll[al_A]=='_')||((ll[al_A]>47)&&(ll[al_A]<58))))
        ErrorExit(70);

    // First letter mustn't be number
    if ((ll[0]>47)&&(ll[0]<58)) ErrorExit(71);

    // Check for double label
    al_A=0;al_P[0]=0;
    while (al_A<lend){
       if (lbn[al_A]==0){
          if (strchr(al_P,':')!=NULL) *strchr(al_P,':')=0;
          if (!strcmp(al_P,ll)) ErrorExit(72);
          al_P[0]=0;
       } else {
          al_P[strlen(al_P)+1]=0;
          al_P[strlen(al_P)]=lbn[al_A];
       }
       al_A++;
    }

    // If local => set
    strcpy(al_Q,ll);
    if (al_Q[0]=='@'&&al_Q[1]!='@'){ strcpy(al_Q,lcl);strcat(al_Q,ll+1); }

    // Prepeare
    Hex32(lps,al_P);strcat(al_Q,":");
    if (plc!=NULL) strcat(al_Q,plc); else strcat(al_Q,al_P);
    if ((lend+strlen(al_Q)+1)>=lbufsize) ErrorExit(73);

    // Add
    for (al_A=0;al_A<strlen(al_Q);al_A++) lbn[al_A+lend]=al_Q[al_A];
    lend=lend+strlen(al_Q)+1;
    lbn[lend-1]=0;
  }
}

// ====================== Process numbers/labels etc. =======================

// (error: 74-76)
void ProcessPLabel()
{
  char *pp_L,w[128],memq;
  unsigned long pp_C;

   strlwr(p);

   if (!strcmp(p,"$")){Hex32(offs,p);return;}
   if (!strcmp(p,"%")){p[0]=0;return;}

   // Macro params
   if (p[0]=='?'&&p[1]=='?'&&strlen(p)==3){
     strcpy(p,mparam[p[2]-48]);
   }

   // Number
   if ((p[0]>47)&&(p[0]<58)){         // p[0] in ['0'..'9']

      // Convert number to normal type
      if (p[0]=='0'&&p[1]=='x'){ p[1]=0;pp_C=strtolong(p+2);Hex32(pp_C,p);}
       else if (p[0]=='0'){pp_C=strtolong(p+1);Hex32(pp_C,p);}
       else {pp_C=StrDectolong(p);Hex32(pp_C,p);}

   } else {

      // Convert label to number
      if (!i386Reg(p)){

         // Convert ?V/?^ jumps
         if (p[0]=='?'){
            if (strlen(p)!=3 || (p[1]!='^'&&p[1]!='v') || (p[2]<48||p[2]>57))
               ErrorExit(74);

            if ((p[2]-47)>op) ErrorExit(75);
            if (p[1]=='^') strcpy(p,lbls[op+48-p[2]]);
                      else strcpy(p,lble[op+48-p[2]]);

         }

         // If mem addressing
         memq=0;
         if (p[0]=='%'){memq=1;strcpy(p,p+1);}

         // Local labels
         if (p[0]=='@'&&p[1]!='@'&&strlen(lcl))
           { strcpy(w,lcl);strcat(w,p+1);strcpy(p,w); }

         // Convert ident to number
         pp_C=0;w[0]=0;
         while (pp_C<lend&&pp_C!=-1){
            if (lbn[pp_C]==0){

                // Found
                if (strchr(w,':')!=NULL){
                   pp_L=strchr(w,':');pp_L[0]=0;pp_L++;
                   if (!strcmp(w,p)){
                      if (memq){
                        strcpy(p,"[");strcat(p,pp_L);strcat(p,"]");
                      } else strcpy(p,pp_L);
                      pp_C=-2;
                   }
                }
                w[0]=0;
            } else {
                w[strlen(w)+1]=0;
                w[strlen(w)]=lbn[pp_C];
            }pp_C++;
         }

        // If not found => error
        if (second){
          if (pp_C!=-1) ErrorExit(76);
        } else {
          if (pp_C!=-1) if (memq) strcpy(p,"[00]");
                             else strcpy(p,"00");
        }
      }
   }
}

// =============================== Write Data ===============================

void WriteOut(void *buff,int size) // Write data to out file (error:32)
{
  if (second){
     if (fwrite(buff,1,size,out)!=size) ErrorExit(32);
  }
  outoffs+=size;
}

void WriteData(char *dta) // Write to temp data file (error:34)
{
  char dln[2] = {13,10};

  // DEBUG: cout << dta << "|" << endl;
  if (dat==NULL){
    dat=fopen(datf,"ab");
    if (dat==NULL) ErrorExit(33);
  }
  if (fwrite(dta,1,strlen(dta),dat)!=strlen(dta)) ErrorExit(34);
  if (fwrite(dln,1,2,dat)!=2) ErrorExit(34);
}

// =========================== Process asm code =============================

// (error: 40,41,43,56,80,14,1..21)
void ProcessCode(char *cmd) // Process "d? num,num...","<label>:","<code>[parms]"
{                           // q,z,a,b,c,txt,spc

   // 'xx' => <number> && kill spaces (cmd => q)
   KillSpaces(cmd);q[0]=0;txt=0;
   for (a=0;a<strlen(cmd);a++){
     if (cmd[a]==39&&(!txt)){
       a++;l=cmd+a;
       if (strchr(l,39)!=NULL) *strchr(l,39)=0;else ErrorExit(40);
       if (strlen(l)>4||strlen(l)==0) ErrorExit(43);
       a+=strlen(l);b=0;
       for (c=0;c<strlen(l);c++) b+=(l[c])<<(8*c);
       Hex32(b,p);strcat(q,p);cmd[a]=39;
     } else {
       q[strlen(q)+1]=0;
       q[strlen(q)]=cmd[a];
       if (cmd[a]=='"'){if (txt) txt=0; else txt=1;}
     }
   }

   // string != NULL
   if (strlen(q)){

      // Kill spaces (prepeare to assembly) && lowercase (q => z)
      z[0]=0;txt=0;spc=1;b=0;
      for (a=0;a<strlen(q);a++){

        // If text found => save it ("sdsad")
        if (q[a]=='"'){if (txt) txt=0; else txt=1;}
        if (txt||q[a]!=' '){
           z[strlen(z)+1]=0;
           z[strlen(z)]=q[a];
           if (!txt) strlwr(z+strlen(z)-1);

        }else{

          // if space available here => set it && check for Prefix
          if (spc){
            l=z;while (strchr(l,' ')!=NULL) l=strchr(l,' ')+1;
            if (!i386Prefix(l)){spc=0;b=a+1;}
            z[strlen(z)+1]=0;
            z[strlen(z)]=q[a];
          }
        }
      }
      p[0]=0;q[0]=0;txt=0;

      // Convert idents to numbers (b - pointer to parameters(in z))
      if (b!=0){
        for (a=b;a<strlen(z);a++){
          if (txt){
             q[strlen(q)+1]=0;q[strlen(q)]=z[a];
             if (z[a]=='"') txt=0;

          } else switch(z[a]){

           // List of divisors
            case '"':
              txt=1;q[strlen(q)+1]=0;q[strlen(q)]=z[a];break;
            case ':':

              // underfine mem => do not save
              if (strcmp(p,"u")){ strcat(q,p);q[strlen(q)+1]=0;q[strlen(q)]=z[a];}
              p[0]='%';p[1]=0;break;
            case '.':
              z[a]='+';
            case '[': case ']': case ',': case '+': case '-': case '/': case '*': case '~': case '!':
              if (strlen(p)){ProcessPLabel();strcat(q,p);}
              q[strlen(q)+1]=0;q[strlen(q)]=z[a];p[0]=0;
              break;

            default:
              p[strlen(p)+1]=0;
              p[strlen(p)]=z[a];
              break;
          }

        }
        strcpy(z+b,q);if (strlen(p)){ProcessPLabel();strcat(z,p);}
      }

      //DEBUG: cout << z << endl;

      // if define: "#<name> <equ>" or shortdef: "!<name> <equ>"
      if (z[0]=='#'||z[0]=='!'){

        if ((second==1&&z[0]=='#')||(second==0&&z[0]=='!')){

          l=z+1;if (!strlen(l)) ErrorExit(56);

          if (strchr(l,' ')!=NULL){
             h=strchr(l,' ')+1;
             strchr(l,' ')[0]=0;
             KillSpaces(h);
             if (strlen(h)==0) ErrorExit(56);
           } else ErrorExit(56);

          c=ProcessNum(h);
          second=0;
          AddLabel(l,c,NULL);
          second=1;
          if (z[0]=='!') second=0;
        }

      // Data
      } else if (strstr(z,"db ")==z||strstr(z,"dw ")==z||strstr(z,"dd ")==z){

        // Process data
        p[0]=0;strcat(z,",");
        for (a=3;a<strlen(z);a++){
           switch(z[a]){

            // Text "<smth>"
            case '"':
              if (strlen(p)) ErrorExit(80);
              a++;q[0]=0;
              while (z[a]!='"'){
                if (a>strlen(z)) ErrorExit(41);
                q[strlen(q)+1]=0;q[strlen(q)]=z[a];
                a++;
              }
              if (z[a+1]!=',') ErrorExit(80);
              WriteOut(q,strlen(q));offs+=strlen(q);
              break;

            // Array
            case '[':
              if (strlen(p)) ErrorExit(80);
              a++;q[0]=0;
              while (z[a]!=']'){
                if (a>strlen(z)) ErrorExit(44);
                q[strlen(q)+1]=0;q[strlen(q)]=z[a];
                a++;
              }
              if (z[a+1]!=',') ErrorExit(80);
              b=strtolong(q);
              for (c=0;c<4;c++) q[c]=0;
              c=1;if (z[1]=='w') c=2;if (z[1]=='d') c=4;
              offs+=b*c;
              while (b>0){WriteOut(q,c);b--;}
              break;

            // Numbers
            case ',':
              if (strlen(p)>0){
                b=ProcessNum(p);
                switch(z[1]){
                 case 'b':if ((b+0xFF)>0x1FE) ErrorExit(14);
                      if (b>0xFF) b+=0x100;offs++;
                      WriteOut(&b,1);break;
                 case 'w':if ((b+0xFFFF)>0x1FFFE) ErrorExit(14);
                      if (b>0xFFFF) b+=0x10000;offs+=2;
                      WriteOut(&b,2);break;
                 case 'd':
                      WriteOut(&b,4);offs+=4;break;
                }
              }
              p[0]=0;
              break;

            // Default - save number
            default:
              p[strlen(p)+1]=0;
              p[strlen(p)]=z[a];
              break;
          }
        }

      // Code
      } else {

         // DEBUG
         #ifdef debug_list
           cout << offs << " : " << z << endl;
         #endif

         // Assembly command
         // DEBUG: cout << "<" << z << ">";
         //fprintf(stdout,"%s:   ",z);
         output << z << endl;
         i386asm(z);
         //fprintf(stdout,"o=%i",opsz);
         offs+=opsz;
         WriteOut(opcode,opsz);
         //fprintf(stdout," w=%i\n",opsz);
      }
   }
}

// ======================== Process hi-level extension ======================

void MainProcess(char *prx,int tp)
{

  // Local vars
  int e,k,n,pc,io;
  char pr[256],tt[256],rt[256],*y,*j;
  strcpy(pr,prx);

  // Simple Expression process
  if (tp>0x80){

     KillSpaces(pr);
     if (!strlen(pr)) return;

     if (pr[0]=='~'){
       strcpy(pr,pr+1);

     } else {

        // Search for compare  <=  ==  =>  <  !=  >
        y=pr;
        while (y[0]!=0){
          if (y[0]=='"') do { y++; } while (y[0]!='"');
          if (y[0]==39)  do { y++; } while (y[0]!=39);
          tt[0]=y[0];tt[1]=y[1];tt[2]=0;
          if (strcmp(tt,"=>")==0 || strcmp(tt,"<=")==0 || strcmp(tt,"==")==0
            || strcmp(tt,"!=")==0 || y[0]=='>' || y[0]=='<') break;
          y++;
        }

        // not found => error, else SAVE
        if (y[0]==0) ErrorExit(81);
        if ((y[0]=='>'||y[0]=='<')&&y[1]!='='){y[0]=0;y++;tt[1]=0;} else {y[0]=0;y+=2;}
        KillSpaces(pr);KillSpaces(y);

        // DEBUG: cout << "-" << pr << "?" << tt << "?" << y << "-" << endl;

        // Here we proceed funcs & equals
        if (pr[strlen(pr)-1]==')'){
           MainProcess(pr,0);
           *strchr(pr,'(')=0;KillSpaces(pr);
           strcpy(p,"@@fr");strcat(p,pr);
           if (p[strlen(p)-1]=='?') ErrorExit(67);
           ProcessPLabel();
           if ((!strcmp(p,"0"))||strlen(p)==0) ErrorExit(99);
           pc=strlen(p)+1;strcpy(p+pc,y);y=pr+pc;
           strcpy(pr,p);strcpy(y,p+pc);
        }

        // Compare
        strcpy(s,"cmp ");strcat(s,pr);strcat(s,",");strcat(s,y);
        ProcessCode(s);pr[0]=0;s[0]=0;

        // create conditional jump
        if (strcmp(tt,"==")==0)      strcpy(pr,"z");
        else if (strcmp(tt,"!=")==0) strcpy(pr,"nz");
        else if (strcmp(tt,"<=")==0) strcpy(pr,"le");
        else if (strcmp(tt,"=>")==0) strcpy(pr,"ge");
        else if (strcmp(tt,"<")==0)  strcpy(pr,"l");
        else if (strcmp(tt,">")==0)  strcpy(pr,"g");
     }
     // DEBUG: cout << "<" << pr << ">";

     // Depends on type
     switch(tp^0x80){

       // if,while
       case 1: case 2: case 3: case 9: case 10: case 11:

         k=op;

         // Or
         if (lblt[k]==202){
            while(lblt[k-1]==202) k--;
            strcpy(tt,"j");strcat(tt,pr);
            strcat(tt," ");strcat(tt,lble[k]);
            ProcessCode(tt);

         // And
         } else if (lblt[k]==201){
            while(lblt[k-1]==201) k--;
            strcpy(tt,"jn");if (pr[0]=='n') strcpy(tt+1,pr+1); else strcat(tt,pr);
            strcat(tt," ");strcat(tt,lble[k]);
            if (((tp&0x8)!=0)&&(lblt[k-1]<201)) strcat(tt,"_");
            ProcessCode(tt);

         }
         break;
     }
     return;

  // if/while
  } else if (tp!=0){

     // proceed
     tt[0]=0;lblt[op]=201;
     for (e=0;e<strlen(pr);e++) switch(pr[e]){
        case '(':

           // if it's finction
           k=e-1;
           while (k>0){
             if (pr[k]=='(' || (pr[k]=='|'&&pr[k-1]=='|')
                || (pr[k]=='&'&&pr[k-1]=='&')) k=0;
             else if (pr[k]!=' ') k=-1;
             k--;
           }

           // yes
           if (e>0&&k==-2){

             // skip it
             k=1;tt[strlen(tt)+1]=0;tt[strlen(tt)]=pr[e];

             while(k>0&&pr[e]!=0){
               e++;tt[strlen(tt)+1]=0;tt[strlen(tt)]=pr[e];
               switch(pr[e]){
                 case '"': do { e++;
                     tt[strlen(tt)+1]=0;tt[strlen(tt)]=pr[e];
                   } while(pr[e]!='"'&&pr[e]!=0);
                   if (pr[e]==0) ErrorExit(41);break;

                 case 39: do { e++;
                     tt[strlen(tt)+1]=0;tt[strlen(tt)]=pr[e];
                   } while(pr[e]!=39&&pr[e]!=0);
                   if (pr[e]==0) ErrorExit(40);
                   break;

                 case '(':
                   k++;break;

                 case ')':
                   k--;break;
               }
             }

             // Check
             if (pr[e]==0) ErrorExit(45);

           } else {

             // If unknown type (&& or ||) => found it
             if (e>0&&lblt[op]==0){
               n=op;
               for (k=e;k<strlen(pr);k++){
                 switch(pr[k]){
                  case '(': n++;break;
                  case ')': n--;break;
                  case '"': do {k++;} while (pr[k]!='"');break;
                  case  39: do {k++;} while (pr[k]!=39);break;
                 }
                 if (n<op) { k=-2;break; } // And
                 if (n==op&&pr[k]=='&'&&pr[k+1]=='&') { k=-2;break; } // And
                 if (n==op&&pr[k]=='|'&&pr[k+1]=='|') { k=-3;break; } // OR
               }
               if (k==-2) lblt[op]=201;
               else if (k==-3) lblt[op]=202;
               else ErrorExit(45);
             }

             MainProcess(tt,0x80|tp);tt[0]=0;op++;lblt[op]=0;
             if (op>250) ErrorExit(51);Hex32(lb,p);lb++;
             strcpy(lbls[op],"@@ls");strcat(lbls[op],p);
             strcpy(lble[op],"@@le");strcat(lble[op],p);
             AddLabel(lbls[op],offs,NULL);
           }
           break;

        case ')':
           if (lblt[op]==0) lblt[op]=201;

           // Close
           MainProcess(tt,0x80|tp);

           // if end of OR => put jump
           if (lblt[op]==202){
             k=op-1;while(lblt[k-1]==201) k--;
             strcpy(tt,"jmp ");strcat(tt,lble[k]);
             ProcessCode(tt);
           }

           // if Last + AND + Long
           if (pr[e+1]==0&&lblt[op]==201&&((tp&0x8)!=0)){
             strcpy(tt,"jmp ?v0");ProcessCode(tt);
             strcpy(tt,lble[op-1]);strcat(tt,"_");
             AddLabel(tt,offs,NULL);
             strcpy(tt,"jmp ?v1");ProcessCode(tt);
           }

           if (op==0) ErrorExit(52);
           AddLabel(lble[op],offs,NULL);op--;
           tt[0]=0;break;

        case '&':
           if (pr[e+1]=='&'){
             if (lblt[op]==202) ErrorExit(93);lblt[op]=201;
             MainProcess(tt,0x80|tp);tt[0]=0;e++;break;
           } else {
             tt[strlen(tt)+1]=0;tt[strlen(tt)]=pr[e];break;
           }

        case '|':
           if (pr[e+1]=='|'){
             if (lblt[op]==201) ErrorExit(93);lblt[op]=202;
             MainProcess(tt,0x80|tp);tt[0]=0;e++;break;
           } else {
             tt[strlen(tt)+1]=0;tt[strlen(tt)]=pr[e];break;
           }

        case '"':
           tt[strlen(tt)+1]=0;tt[strlen(tt)]=pr[e];
           do {
             e++;tt[strlen(tt)+1]=0;tt[strlen(tt)]=pr[e];
           } while(pr[e]!='"'&&pr[e]!=0);
           if (pr[e]==0) ErrorExit(41);
           break;

        case 39:
           tt[strlen(tt)+1]=0;tt[strlen(tt)]=pr[e];
           do {
             e++;tt[strlen(tt)+1]=0;tt[strlen(tt)]=pr[e];
           } while(pr[e]!=39&&pr[e]!=0);
           if (pr[e]==0) ErrorExit(40);
           break;

        default:
           tt[strlen(tt)+1]=0;
           tt[strlen(tt)]=pr[e];
     }
     lblt[op]=waitfor; //  If/while
     return;
  }

  // ------ start of normal MainProcess ------

  // Need to save at data
  if (pr[0]=='!'){
     WriteData(pr+1);
     return;
  }

  // ------ Label present ------
  for (e=0;e<strlen(pr);e++){
    switch(pr[e]){
     case ' ':  while(pr[e]==' ') e++;if (pr[e]!=':'){e=-2;break;}
     case ':':  e=-1;break;
     case '(': case '"': case '=': case 39:
       e=-2;break;
    }if (e<0) break;
  }

  // Yes => proceed
  if (e==-1){
    y=strchr(pr,':');y[0]=0;strlwr(pr);
    if (i386Prefix(pr) || (strlen(pr)==1&&(pr[0]=='b'||pr[0]=='w'||pr[0]=='d'))){
      y[0]=':';

    // Label
    } else {
      KillSpaces(pr);
      AddLabel(pr,offs,NULL);
      y++;strcpy(pr,y);
    }
  }

  // ------ hi-level data definition ------
  KillSpaces(pr);n=0;
  if (strstr(pr,"char ")==pr) n=1;
  else if (strstr(pr,"uint ")==pr) n=2;
  else if (strstr(pr,"ulong ")==pr) n=4;

  // If found => process
  if (n>0){
    y=strchr(pr,' ')+1;tt[0]=0;strcat(y,",");
    while(y[0]!=0){
      switch(y[0]){
         case ' ': break;

         case '"':
           tt[strlen(tt)+1]=0;tt[strlen(tt)]=y[0];
           do {
             y++;tt[strlen(tt)+1]=0;tt[strlen(tt)]=y[0];
           } while(y[0]!='"'&&y[0]!=0);
           if (y[0]==0) ErrorExit(41);
           break;

         case 39:
           tt[strlen(tt)+1]=0;tt[strlen(tt)]=y[0];
           do {
             y++;tt[strlen(tt)+1]=0;tt[strlen(tt)]=y[0];
           } while(y[0]!=39&&y[0]!=0);
           if (y[0]==0) ErrorExit(41);
           break;

         // Process data
         case ',':
           if (strlen(tt)==0) ErrorExit(96);

           // If direct data present => save in y
           j=strchr(tt,'=');if (j!=NULL){ j[0]=0;j++; }e=0;
           if (strchr(tt,'"')!=NULL||strchr(tt,39)!=NULL) ErrorExit(97);
           l=strchr(tt,'[');if (l!=NULL){
             l[0]=0;l++;h=strchr(l,']');if (h==NULL) ErrorExit(98);
             h[0]=0;h++;if (h[0]!=0) ErrorExit(97);strcpy(p,l);
             if (strlen(p)){ ProcessPLabel();e=strtolong(p); } else e=0;
           }

           // e=Array ; tt - label ; y - data
           strcpy(s,tt);strcat(s,":");
           switch(n){ case 1: strcat(s,"db ");break;
            case 2: strcat(s,"dw ");break;case 4: strcat(s,"dd ");break;}

           // data
           if (e==0&&j==NULL) strcat(s,"0");
           else if (j==NULL){ Hex32(e,p);strcat(s,"[");strcat(s,p);strcat(s,"]");}
           WriteData(s);

           tt[0]=0;
           break;

         default:
           tt[strlen(tt)+1]=0;
           tt[strlen(tt)]=y[0];
      }
      y++;
    }
    return;
  }

  // ------ Equal ------
  txt=0;k=0;
  for (e=0;e<strlen(pr);e++){
    if (txt&&pr[e]==39) txt=0;
    else switch(pr[e]){
     case  39: txt=1;break;
     case '(': case '"': e=-2;break;
     case '=': k=e;e=-1;break;
    }
    if (e<0) break;
  }

  // yes? => Proceed
  if (e==-1){

    // <smth>=??  pc - skob
    j=pr+k;j[0]=0;j++;KillSpaces(pr);KillSpaces(j);strcat(j,")");
    pc=1;while (j[0]=='('){pc++;j++;KillSpaces(j);}tt[0]=0;
    while(j[0]!=0){

      if (j[0]==')'){
        if (pc==0) ErrorExit(62);

        // Proceed expression without () ; k - cmd ; p - param ; n - execute
        strcat(tt,"|");y=NULL;
        k=100;n=0;p[0]=0;io=0;for (e=0;e<strlen(tt);e++){
          switch(tt[e]){
           case '+': n=k;k=1;
                     if (tt[e+1]=='+'){e++;k=3;}
                     break;
           case '-': n=k;k=2;
                     if (tt[e+1]=='-'){e++;k=4;}
                     break;
           case '*': n=k;k=10;break;
           case '/': n=k;k=11;break;
           case '<': n=k;k=5;
                     if (tt[e+1]=='<'){e++;k=12;}
                     break;
           case '>': n=k;k=6;
                     if (tt[e+1]=='>'){e++;k=13;}
                     break;
           case '&': n=k;k=7;break;
           case '|': n=k;k=8;break;
           case '^': n=k;k=9;break;
           case ' ': break;
           case '"': p[strlen(p)+1]=0;p[strlen(p)]='"';do
             {e++;p[strlen(p)+1]=0;p[strlen(p)]=tt[e];} while(tt[e]!='"');
             break;
           case  39: p[strlen(p)+1]=0;p[strlen(p)]=39;do
             {e++;p[strlen(p)+1]=0;p[strlen(p)]=tt[e];} while(tt[e]!=39);
             break;

           case '(':
              // Saving function params
              io=1;p[strlen(p)+1]=0;p[strlen(p)]=tt[e];

              while(io>0&&tt[e]!=0){
                e++;p[strlen(p)+1]=0;p[strlen(p)]=tt[e];
                switch(tt[e]){
                  case '"': do { e++;
                      p[strlen(p)+1]=0;p[strlen(p)]=tt[e];
                    } while(tt[e]!='"'&&tt[e]!=0);
                    if (tt[e]==0) ErrorExit(41);break;

                  case 39: do { e++;
                      p[strlen(p)+1]=0;p[strlen(p)]=tt[e];
                    } while(tt[e]!=39&&tt[e]!=0);
                    if (tt[e]==0) ErrorExit(40);
                    break;

                  case '(':
                    io++;break;

                  case ')':
                    io--;break;
                }
              }
              io=1;
              break;

           default:
             p[strlen(p)+1]=0;p[strlen(p)]=tt[e];
          }

          if (n>0){

            if (y!=NULL&&n>2) ErrorExit(65);

            // Here we change Func to code ????
            if (io){

               strcpy(rt,p);
               MainProcess(rt,0);
               *strchr(rt,'(')=0;KillSpaces(rt);
               strcpy(p,"@@fr");strcat(p,rt);
               if (p[strlen(p)-1]=='?') ErrorExit(67);
               ProcessPLabel();
               if ((!strcmp(p,"0"))||strlen(p)==0) ErrorExit(99);
            }
            io=0;

            switch(n){
              case 1:
                strcpy(q,"add ");strcat(q,pr);strcat(q,",");strcat(q,p);
                if (p[0]==0) ErrorExit(63);MainProcess(q,0);y=q;
                break;
              case 2:
                strcpy(q,"sub ");strcat(q,pr);strcat(q,",");strcat(q,p);
                if (p[0]==0) ErrorExit(63);MainProcess(q,0);y=q;
                break;
              case 3:
               strcpy(q,"inc ");strcat(q,pr);
                if (p[0]!=0) ErrorExit(64);MainProcess(q,0);
                break;
              case 4:
                strcpy(q,"dec ");strcat(q,pr);
                if (p[0]!=0) ErrorExit(64);MainProcess(q,0);
                break;
              case 5:
                strcpy(q,"rol ");strcat(q,pr);strcat(q,",");strcat(q,p);
                if (p[0]==0) ErrorExit(63);MainProcess(q,0);
                break;
              case 6:
                strcpy(q,"ror ");strcat(q,pr);strcat(q,",");strcat(q,p);
                if (p[0]==0) ErrorExit(63);MainProcess(q,0);
                break;
              case 7:
                strcpy(q,"and ");strcat(q,pr);strcat(q,",");strcat(q,p);
                if (p[0]==0) ErrorExit(63);MainProcess(q,0);
                break;
              case 8:
                strcpy(q,"or ");strcat(q,pr);strcat(q,",");strcat(q,p);
                if (p[0]==0) ErrorExit(63);MainProcess(q,0);
                break;
              case 9:
                strcpy(q,"xor ");strcat(q,pr);strcat(q,",");strcat(q,p);
                if (p[0]==0) ErrorExit(63);MainProcess(q,0);
                break;
              case 10:
              case 11:
                break;
              case 12:
                strcpy(q,"shl ");strcat(q,pr);strcat(q,",");strcat(q,p);
                if (p[0]==0) ErrorExit(63);MainProcess(q,0);
                break;
              case 13:
                strcpy(q,"shr ");strcat(q,pr);strcat(q,",");strcat(q,p);
                if (p[0]==0) ErrorExit(63);MainProcess(q,0);
                break;
              case 100:
                if (p[0]!=0){
                  strcpy(q,"mov ");strcat(q,pr);strcat(q,",");strcat(q,p);
                  MainProcess(q,0);
                }
                break;
            }
            n=0;p[0]=0;
          }
        }

        pc--;tt[0]=0;

      } else if (j[0]=='('){

        // Saving function params
        k=1;tt[strlen(tt)+1]=0;tt[strlen(tt)]=j[0];

        while(k>0&&j[0]!=0){
          j++;tt[strlen(tt)+1]=0;tt[strlen(tt)]=j[0];
          switch(j[0]){
            case '"': do { j++;
                tt[strlen(tt)+1]=0;tt[strlen(tt)]=j[0];
              } while(j[0]!='"'&&j[0]!=0);
              if (j[0]==0) ErrorExit(41);break;

            case 39: do { j++;
                tt[strlen(tt)+1]=0;tt[strlen(tt)]=j[0];
              } while(j[0]!=39&&j[0]!=0);
              if (j[0]==0) ErrorExit(40);
              break;

            case '(':
              k++;break;

            case ')':
              k--;break;
          }
        }

        // Check
        if (j[0]==0) ErrorExit(45);

      } else {
        tt[strlen(tt)+1]=0;
        tt[strlen(tt)]=j[0];
      }
      j++;
    }

  // ------ Function or expression ------
  } else if (pr[strlen(pr)-1]==')'){

    // Separate name & params
    y=strchr(pr,'(');if (y==NULL) ErrorExit(90);
    *y=0;y++;if (strchr(pr,'"')!=NULL||strchr(pr,39)!=NULL) ErrorExit(90);
    KillSpaces(pr);strlwr(pr);k=0;

    if (!strcmp(pr,"if")) k=1;
    else if (!strcmp(pr,"while")) k=2;
    else if (!strcmp(pr,"ifelse")) k=3;
    else if (strstr(pr,"function ")==pr) k=5;
    else if (!strcmp(pr,"_if")) k=1+8;
    else if (!strcmp(pr,"_while")) k=2+8;
    else if (!strcmp(pr,"_ifelse")) k=3+8;
     //else if (!strcmp(pr,"until")) k=4; ????
     //else if (!strcmp(pr,"for")) k=6;   ????
     //else if (!strcmp(pr,"_until")) k=4+8; ????
     //else if (!strcmp(pr,"_for")) k=6+8;   ????

    // ------ function definition  "function <name>:<type>(<params>)" ------
    if (k==5){

       // Get params
       strcpy(wst,y);waitfor=5;wst[strlen(wst)-1]=0;KillSpaces(wst);

       // Get name
       strcpy(tt,pr+8);KillSpaces(tt);strcpy(pr,tt);
       y=strchr(pr,':');if (y==NULL) ErrorExit(94);
       y[0]=0;y++;AddLabel(pr,offs,NULL);
       strcpy(tt,"@@fr");strcat(tt,pr);
       AddLabel(tt,offs,y);

       // Save param nums
       strcpy(tt,"@@pn");strcat(tt,pr);strcpy(p,wst);
       if (strlen(p)){
         ProcessPLabel();AddLabel(tt,offs,p);
       }

    // ------ if/while/until ------
    } else if (k!=0){
       strcpy(wst,"(");strcat(wst,y);waitfor=k;

    // ------ Call function ------
    } else {

      // Macro call?
      if (pr[strlen(pr)-1]=='?'){
         pr[strlen(pr)-1]=0;KillSpaces(pr);
         if (macrost>0) ErrorExit(84);macrost=3;
         for (int ppq=0;ppq<10;ppq++) mparam[ppq][0]=0;
      }

      // Proceed params (in y)
      k=0;pc=0;tt[0]=0;y[strlen(y)-1]=0;strrev(y);if (strlen(y)) strcat(y,",");
      for (e=0;e<strlen(y);e++){

        if (k==1||k==2){
          tt[strlen(tt)+1]=0;
          tt[strlen(tt)]=y[e];
          if ((y[e]==39&&k==1)||(y[e]=='"'&&k==2)) k=0;
        } else switch(y[e]){
         case '"':
            tt[strlen(tt)+1]=0;
            tt[strlen(tt)]=y[e];
            k=2;break;
         case 39 :
            tt[strlen(tt)+1]=0;
            tt[strlen(tt)]=y[e];
            k=1;break;
         case ',':
            strrev(tt);
            if (strlen(tt)==0) ErrorExit(91);
            pc++;

            // Data
            if (tt[0]=='"'||tt[0]=='!'){

              txt=0;
              for (n=0;n<strlen(tt);n++) switch(txt){
               case 0:
                  if (tt[n]=='"') txt=2;
                  if (tt[n]==39) txt=1;
                  if (tt[n]=='|') tt[n]=',';
                  break;
               case 1:
                  if (tt[n]==39) txt=0;
                  break;
               case 2:
                  if (tt[n]=='"') txt=0;
                  break;
              }
              if (txt!=0) ErrorExit(92);
              if (tt[strlen(tt)-1]=='"') strcat(tt,",0");

              // Create label  "@@ld0xxxx: db <yourstr>"
              Hex32(lb,p);lb++;
              strcpy(s,"!@@ld");strcat(s,p);strcpy(p,s+1);
              strcat(s,": db ");strcat(s,tt);strcpy(tt,p);
              MainProcess(s,0);
            }

            // Test for equals/functions ????
            // ....

            // Save params
            if (macrost==3){
               strcpy(mparam[pc-1],tt);

            // Function => push it
            } else {

               if (tt[0]=='%'){
                  if (code32) {strcpy(s,"pushw ");strcat(s,tt+1);}
                         else {strcpy(s,"pushd ");strcat(s,tt+1);}
               } else {
                 strcpy(s,"push ");strcat(s,tt);
               }
               ProcessCode(s);
            }

            tt[0]=0;
            break;
         default:
            tt[strlen(tt)+1]=0;
            tt[strlen(tt)]=y[e];
            break;
        }
      }
      if (macrost==3){
        macrost=2;
        strcpy(p,"@@mf");strcat(p,pr);
        ProcessPLabel();
        if (p[0]==0) ErrorExit(85);
        strcpy(tt,p);

        strcpy(p,"@@mb");strcat(p,pr);
        ProcessPLabel();
        if (p[0]==0) ErrorExit(85);
        a=strtolong(p);

        strcpy(p,"@@me");strcat(p,pr);
        ProcessPLabel();
        if (p[0]==0) ErrorExit(85);
        b=strtolong(p)-a;
        OpenFile(tt,a,b);

      } else {
        strcpy(tt,"call ");strcat(tt,pr);
        ProcessCode(tt);
      }

      // Check for parameters
      if (second){
        strcpy(tt,"@@pn");strcat(tt,pr);

        // Search for number parameters
        n=0;pr[0]=0;while (n<lend&&n!=-1){
           if (lbn[n]==0){

               // if Found => check
               if (strchr(pr,':')!=NULL){
                  y=strchr(pr,':');y[0]=0;y++;
                  if (!strcmp(pr,tt)){
                     a=strtolong(y);
                     if (a!=pc) ErrorExit(95);
                     n=-2;
                  }
               }
               pr[0]=0;
           } else {
               pr[strlen(pr)+1]=0;
               pr[strlen(pr)]=lbn[n];
           }n++;
        }
      }

    }

  } else ProcessCode(pr);
}

// ======================= process base language constr =====================

void ProcessCommand()
{
  KillSpaces(s);

  // Macro?
  if (waitfor==0xAA){
    if (strcmp(strlwr(s),"#endm")==0){

       AddLabel(mn,FGetPos(),NULL);
       mn[0]=0;macrost=0;waitfor=0;
    }
    s[0]=0;return;

  // end of ifdef
  } else if (waitfor==0xAB){
    if (strcmp(strlwr(s),"#endif")==0) waitfor=0;
    s[0]=0;return;
  }

  if (strlen(s)){
    if (waitfor==27&&strcmp(s,"else")==0){ s[0]=0;waitfor=28;return;}
    if (waitfor!=0) ErrorExit(60);

    // Directives

    // Data
    if (!strcmp(s,"#data")){
       if (dat!=NULL){
         fclose(dat);dat=NULL;
         OpenFile(datf,0,0);
       }

    // Local
    } else if (!strcmp(s,"#local")){
       strcpy(lcl,"@@ll");
       Hex32(lb,p);lb++;
       strcat(lcl,p);

    // EndLocal
    } else if (!strcmp(s,"#endl")){
        lcl[0]=0;

    // endif
    } else if (!strcmp(s,"#endif")){
       // noth

    // EndMacro
    } else if (!strcmp(s,"#endm")){
        macrost=0;
        for (int rrq=0;rrq<10;rrq++) mparam[rrq][0]=0;

    // With params
    } else if (s[0]=='#'){

      strlwr(s);
      if (strchr(s,' ')!=NULL){
        l=strchr(s,' ');
        l[0]=0;l++;
        KillSpaces(l);
        if (!strlen(l)) ErrorExit(56);
      } else ErrorExit(56);

      // Include file
      if (!strcmp(s,"#include")){
        if (l[0]=='$'){
          strcpy(q,spath);
          strcat(q,l+1);
          l=q;
        }
        OpenFile(l,0,0);

      // Define
      } else if (!strcmp(s,"#define")){
        s[1]=0;strcat(s,l);
        ProcessCode(s);

      // ShortDef
      } else if (!strcmp(s,"#shortdef")){
        s[1]=0;strcat(s,l);s[0]='!';
        ProcessCode(s);

      // Outfile
      } else if (!strcmp(s,"#output")){
        strcpy(outfile,l);

      // Ifdef/ifndef
      } else if (strcmp(s,"#ifdef")==0||strcmp(s,"#ifndef")==0){
         if (second){
            if (ifdef[cdef]) waitfor=0xAB;
            cdef++;
         } else {
            strcpy(p,l);
            ProcessPLabel();
            if (strcmp(p,"0")==0) ifdef[cdef]=0xAB;
             else ifdef[cdef]=0;

            // ifndef => invert
            if (strcmp(s,"#ifndef")==0)
              if (ifdef[cdef]==0xAB) ifdef[cdef]=0; else ifdef[cdef]=0xAB;

            waitfor=ifdef[cdef];
            cdef++;if (cdef>=MaxIfDef) ErrorExit(66);
         }

      // Org
      } else if (!strcmp(s,"#org")){
        if (l[0]=='0'&&l[1]=='x') offs=strtolong(l+2);
         else if (l[0]=='0') offs=strtolong(l+1);
         else offs=StrDectolong(l);

      // Align
      } else if (!strcmp(s,"#align")){
        a=StrDectolong(l);
        b=offs%a;b=(a-b)%a;
        s[0]=0;offs+=b;
        for (;b>0;b--) WriteOut(s,1);

      // Code 16/32
      } else if (!strcmp(s,"#code")){
        a=StrDectolong(l);
        if (a==16) code32=0; else if (a==32) code32=1;
          else ErrorExit(58);

      // Include binary file
      } else if (!strcmp(s,"#binary")){
        OpenFile(l,0,0);offs+=tfs->sz;
        s[0]=ch;
        if (second) while (ReadFile()) WriteOut(&ch,1);
        ch=s[0];
        CloseFile();

      // Macro defenition (l - name)
      } else if (!strcmp(s,"#macro")){
        strcpy(q,"@@mf");strcat(q,l);
        AddLabel(q,0,tfs->FileName);
        strcpy(q,"@@mb");strcat(q,l);
        AddLabel(q,FGetPos(),NULL);
        strcpy(mn,"@@me");strcat(mn,l);
        waitfor=0xAA;macrost=1;

      // Unknown
      } else ErrorExit(57);

    } else {

      // Code => compile
      MainProcess(s,0);
    }
    s[0]=0;
  }
}

// ========================== ChaOS ASseMbler ===============================

void main(int argc,char *argv[])
{
   if(argc<2) {
      cout << "Use ""chasm.exe <file.csm>""" << endl;
      exit(0);
   }

   // Get self path & get data filename
   strcpy(spath,argv[0]);a=0;
   for (b=0;b<strlen(spath);b++){if (spath[b]==92||spath[b]=='/') a=b;}
   spath[a+1]=0;strcat(spath,"ch\\");
   strcpy(datf,"$chasm$.dat");remove(datf);

   // 2 passes
   for (second=0;second<2;second++){

     // If second => open out
     if (second){
        strcpy(s,argv[1]);if (strchr(s,'.')!=NULL) *strchr(s,'.')=0;
        if (!strlen(outfile)){ strcat(s,".out"); } else { if (outfile[0]=='.')
           { strcat(s,outfile); } else { strcpy(s,outfile); }}
        out=fopen(s,"wb");if (out==NULL) ErrorExit(31);
        printf("CHASM: (second) out: %s\n",s);
     } else printf("CHASM: (first) in: %s\n",argv[1]);
     
     OpenFile(argv[1],0,0);
     s[0]=0;op=0;lb=0;oldc=0;lines=0;outoffs=0;offs=0;waitfor=0;lcl[0]=0;
     macrost=0;code32=0;cdef=0;dat=NULL;

     do {
        while(ReadFile()){
           switch(st){

             // Code
             case 0:
                 switch(ch){
                   case ';': case 13: case 10:
                         ProcessCommand();break;
                   case '{':
                         ProcessCommand();

                         if (waitfor==28) waitfor=0;
                         if (waitfor==27) ErrorExit(61);

                         op++;if (op>250) ErrorExit(51);
                         Hex32(lb,p);lb++;lblt[op]=0;
                         strcpy(lbls[op],"@@ls");strcat(lbls[op],p);
                         strcpy(lble[op],"@@le");strcat(lble[op],p);
                         AddLabel(lbls[op],offs,NULL);

                         if (waitfor!=0&&waitfor!=5) MainProcess(wst,waitfor);
                         waitfor=0;

                         break;
                   case '}':
                         ProcessCommand();

                         // End of while
                         if ((lblt[op]&0x7)==2){
                           strcpy(s,"jmp ?^0");
                           ProcessCommand();s[0]=0;
                         }

                         // End of ifelse
                         if ((lblt[op]&0x7)==3){
                           strcpy(s,"jmp @@le");Hex32(lb,p);strcat(s,p);
                           ProcessCommand();s[0]=0;
                           waitfor=27;
                         }

                         if (op==0) ErrorExit(52);
                         AddLabel(lble[op],offs,NULL);op--;
                         break;

                   case '/':

                         if (oldc=='/'){
                            s[strlen(s)-1]=0;
                            ProcessCommand();
                            st=3;
                         } else {
                            s[strlen(s)+1]=0;
                            s[strlen(s)]=ch;
                         }
                         break;

                   case 39:
                         s[strlen(s)+1]=0;
                         s[strlen(s)]=ch;
                         st=1;
                         break;

                   case '"':
                         s[strlen(s)+1]=0;
                         s[strlen(s)]=ch;
                         st=2;
                         break;

                   default:
                         s[strlen(s)+1]=0;
                         s[strlen(s)]=ch;
                         break;
                 }
                 break;

             // Text >'<
             case 1:
                 s[strlen(s)+1]=0;
                 s[strlen(s)]=ch;
                 if (ch==39) st=0;
                   else if ((ch==13)||(ch==10)) ErrorExit(40);
                 break;

             // Text >"<
             case 2:
                 s[strlen(s)+1]=0;
                 s[strlen(s)]=ch;
                 if (ch=='"') st=0;
                   else if ((ch==13)||(ch==10)) ErrorExit(41);
                 break;

             // Comment
             case 3:
                 if ((ch==13)||(ch==10)) st=0;
                 break;

           }
           oldc=ch;
           if (strlen(s)>250) ErrorExit(42);
        }
        lines+=tfs->lines;
     } while(CloseFile());

     if (st==1) ErrorExit(40);
     if (st==2) ErrorExit(41);
     if (op>0) ErrorExit(53);
     if (waitfor!=0) ErrorExit(59);

     if (second){
        cout << argv[1] << ": " << lines << " line(s), " << ftell(out) <<" byte(s) of code, "
          << (lend*100/lbufsize) << "% memory used " << endl;
        fclose(out);
     } else AddLabel("end",offs,NULL);
     if (dat!=NULL){fclose(dat);ErrorExit(35);}
     remove(datf);
   }

#ifdef DEBUG
  a=0;q[0]=0;
  while (a<lend){
     if (lbn[a]==0){
         cout << "?" << q << endl;
         q[0]=0;
     }else {
         q[strlen(q)+1]=0;
         q[strlen(q)]=lbn[a];
     }
    a++;
  }
#endif
}











