// ---------------------------------------------[ Intel386 Assembler ]-------
// ul:strtolong(char *st)         Convert hex number "0xxxxx" to ulong
// i386asm(char *str)             Assembly function
// i386Prefix(char *rg)           is it prefix?
// i386Reg(char *rg)              is it i386reg?
// ProcessNum(char *sn)           "<num>+<num>" => ulong
// Hex32(long,PP)                 Longint -> String (Hex) in PP
// StrDecToLong(string)           Number(dec) in string => uns long
// KillSpaces(string)             Kill Spaces at begin and end of line
// --------------------------------------------------------------------------
// opcode[]                       Array of returned code
// opsz                           size
// offs                           Current offset
// strlon                         Result of strtolon
// code32                         1 - code32 ; 0 - code16
// --------------------------------------------------------------------------

#include "i386ins.h"

char opcode[15];
int  opsz;
unsigned long offs;
int code32=0;

 // String to Long
unsigned long strtolong(char *st)
 {
   unsigned long rnum = 0;
   unsigned char ch;
   char *sss;

   // DEBUG: cout << "<" << st << ">" << endl;

   if (strchr(st,'/')!=NULL){
      sss=strchr(st,'/');
      *sss=0;
      rnum=strtolong(st)/strtolong(sss+1);
      *sss='/';

   } else if (strchr(st,'*')!=NULL){
      sss=strchr(st,'*');
      *sss=0;
      rnum=strtolong(st)*strtolong(sss+1);
      *sss='*';

   } else {
     strupr(st);
     if (strlen(st)>8) ErrorExit(13);
     for (int pw=0;pw<strlen(st);pw++){
       ch=*(st+pw)-'0';
       if (ch>9) ch-=7;
       if (ch>15) ErrorExit(13);
       rnum<<=4;rnum+=ch;
     }
   }

   return rnum;
 }

 // if 386 register
int i386Reg(char *rg)
 {
   for (int cf=0;strlen(i386_regtab[cf].reg_name);cf++){
      if (!strcmp(i386_regtab[cf].reg_name,rg))
                  return 1;
   }
   return 0;
 }

 // if 386 prefix
int i386Prefix(char *rg)
 {
   for (int cf=0;strlen(i386_prefixtab[cf].prefix_name);cf++){
      if (!strcmp(i386_prefixtab[cf].prefix_name,rg))
                  return 1;
   }
   return 0;
 }

 // Process "0xx+0yy-0zz"
unsigned long ProcessNum(char *sn)
{
  char _q[128];
  unsigned long rnum,_c,_txt;

  // DEBUG: cout << "<" << sn << ">";

  rnum=0;_q[0]=0;_txt=0;
  for (_c=0;_c<strlen(sn);_c++){

     if (((sn[_c]>47)&&(sn[_c]<58))||((sn[_c]>96)&&(sn[_c]<103))||sn[_c]=='*'||sn[_c]=='/'){
       _q[strlen(_q)+1]=0;
       _q[strlen(_q)]=sn[_c];
     } else if (sn[_c]=='+'||sn[_c]=='-'){

       if (strlen(_q))
         if (_txt) rnum-=strtolong(_q+1);
              else rnum+=strtolong(_q+1);
       if (sn[_c]=='+') _txt=0; else _txt=1;
       _q[0]=0;
     } else ErrorExit(16);
  }
  if (strlen(_q)){
     if (_txt) rnum-=strtolong(_q+1);
          else rnum+=strtolong(_q+1);
  }

  return rnum;
}

 // I386 Assembler
void i386asm(char *str)
 {
   // Variables
   char asmstr[128],*parm[4],*pfix,memindex[80];
   int i,e,k,h,pcount,pcmd,pmodrm,psib,direction;
   unsigned long parmtp[4],cmdparam[3],o,so;
   opsz=0;pcmd=-1;pmodrm=-1;
   char __near *opcd;

   // Prepearing parameters (parm[0] - instruct ; 1..3 - parameters)
   parm[0]=strlwr(strcpy(asmstr,str));psib=-1;
   for (pcount=1;pcount<4;pcount++){
      if (pcount==1) parm[pcount]=strchr(parm[pcount-1],' ');
                else parm[pcount]=strchr(parm[pcount-1],',');
      if (parm[pcount]!=NULL){
         *parm[pcount]=0;parm[pcount]++;
         if (pcount==1){
            // Check for Prefix
            for (i=0;strlen(i386_prefixtab[i].prefix_name);i++){
               if (!strcmp(parm[0],i386_prefixtab[i].prefix_name)){
                  parm[0]=parm[1];pcount=0;
                  opcode[opsz]=i386_prefixtab[i].prefix_code;
                  opsz++;
                  break;
               }
            }
         }
      } else break;

   }pcount--;

   // Getting parameter type
   for (i=1;i<=pcount;i++){
      parmtp[i]=0;

      // Register
      for (e=0;strlen(i386_regtab[e].reg_name);e++){
         if (!strcmp(i386_regtab[e].reg_name,parm[i]))
                     parmtp[i]=i386_regtab[e].reg_type;
      }
      if (parmtp[i]==0){
         // Immediate
         if (*parm[i]=='0'||*parm[i]=='+'||*parm[i]=='-'){
             parmtp[i]=Imm|(Disp8|Disp16|Disp32|DispXX);

         // Memory
         } else {

             // Prefixes
             e=0;
             while (strchr(parm[i],':')!=NULL){
               pfix=parm[i];
               parm[i]=strchr(parm[i],':');
               *parm[i]=0;parm[i]++;
               if (pfix[0]=='%') pfix++;

               // w/b/d prefixes
               if (strlen(pfix)==1){
                  switch(*pfix) {
                    case 'b':e=Mem8;break;
                    case 'w':e=Mem16;break;
                    case 'd':e=Mem32;break;
                    case 'q':e=MemFL;break;
                  }
                  if (e==0) ErrorExit(2);

               // segment prefix
               } else {

                    for (int _e=0;strlen(i386_prefixtab[_e].prefix_name);_e++){
                       if (!strcmp(pfix,i386_prefixtab[_e].prefix_name)){
                          opcode[opsz]=i386_prefixtab[_e].prefix_code;
                          _e=-1;opsz++;break;
                       }
                    }
                    if (_e!=-1) ErrorExit(2);
               }
             }
             if (*parm[i]!='[') ErrorExit(3);
             if (e==0) parmtp[i]=Mem;   // Mem
                  else parmtp[i]=e;     // Mem8/16/32
         }
      }
   }

   // Search for opcode
   for (i=0;strlen(i386_optab[i].name);i++){
       if ((!strcmp(i386_optab[i].name,parm[0]))&&(i386_optab[i].operands==pcount)){

         // Check for parameters
         k=0;direction=0;for (e=1;e<=pcount;e++){
            if ((parmtp[e]&i386_optab[i].operand_types[e-1])==0) k=1;}

         // Check for Direction present
         if ((k==1)&&(i386_optab[i].opcode_modifier&D)&&(i386_optab[i].operands==2)){
            k=0;direction=1;
            if ((parmtp[1]&i386_optab[i].operand_types[1])==0) k=1;
            if ((parmtp[2]&i386_optab[i].operand_types[0])==0) k=1;
         }

         // k=0 => all ok
         if (k==0) {

           // Set Code prefix
           if (( (i386_optab[i].opcode_modifier&_C32)&&(code32==0) )||(
             (i386_optab[i].opcode_modifier&_C16)&&(code32==1) )){
             opcode[opsz]=CodePrefix;opsz++;
           }

           // Code byte number
           o=i386_optab[i].base_opcode;e=0;
           while (o!=0) {o=o/0x100;e++;}if (e==0) e++;

           // Save opcode
           o=i386_optab[i].base_opcode;
           for (k=e;k>0;k--) {opcode[opsz+k-1]=o&0xFF;o=o/0x100;}
           opsz+=e;pcmd=opsz-1;

           // Save direction
           if (direction){ if ((opcode[pcmd]&2)==0) opcode[pcmd]+=2; }

           // Modrm present?
           if ((i386_optab[i].opcode_modifier&Modrm)!=0){
              pmodrm=opsz;opcode[pmodrm]=0;opsz++;

              // Save extension opcode
              if (i386_optab[i].extension_opcode!=None)
                  opcode[pmodrm]=i386_optab[i].extension_opcode*8;
           }

           // Save parameters
           for (k=1;k<pcount+1;k++){

             // Get parameter type
             if (!direction) cmdparam[k]=i386_optab[i].operand_types[k-1];
                        else cmdparam[k]=i386_optab[i].operand_types[2-k];

             // If unknown Disp/ImmOp => set it with code/code32
             if (cmdparam[k]==DispXX){
                if (code32) cmdparam[k]=Disp32;
                       else cmdparam[k]=Disp16;
             }
             if (cmdparam[k]==ImmXX){
                if (code32) cmdparam[k]=Imm32;
                       else cmdparam[k]=Imm16;
             }

             // Switch
             switch(cmdparam[k]&parmtp[k]){

                    // Save imm operand
               case Imm8:
                    o=ProcessNum(parm[k]);
                    so=o;so>>=8;
                    if (so!=0&&so!=0xffffff) ErrorExit(4);
                    opcode[opsz]=o&0xFF;
                    opsz++;
                    break;
               case Imm16:
                    o=ProcessNum(parm[k]);
                    so=o;so>>=16;
                    if (so!=0&&so!=0xffff) ErrorExit(4);
                    for (e=0;e<2;e++) opcode[opsz+e]=((o>>(e*8))&0xFF);
                    opsz+=2;
                    break;
               case Imm32:
                    o=ProcessNum(parm[k]);
                    for (e=0;e<4;e++) opcode[opsz+e]=((o>>(e*8))&0xFF);
                    opsz+=4;
                    break;

                    // Fixed register => nul
               case Acc8:
               case Acc16:
               case Acc32:
               case FloatAcc:
               case InOutPortReg:
               case ShiftCount:
                    break;

                    // Save Disp
               case Disp8:
                    o=ProcessNum(parm[k]);
                    if (second) o=o-opsz-1-offs;
                           else o=0;
                    if ((o+0x80)>0xFF) ErrorExit(5);
                    opcode[opsz]=o&0xFF;
                    opsz++;
                    break;
               case Disp16:
                    o=ProcessNum(parm[k]);
                    if (second) o=o-opsz-2-offs;
                           else o=0;
                    if ((o+0x8000)>0xFFFF) ErrorExit(5);
                    for (e=0;e<2;e++) opcode[opsz+e]=((o>>(e*8))&0xFF);
                    opsz+=2;
                    break;
               case Disp32:
                    o=ProcessNum(parm[k]);
                    o=o-opsz-4-offs;
                    for (e=0;e<4;e++) opcode[opsz+e]=((o>>(e*8))&0xFF);
                    opsz+=4;
                    break;

                    // Registers
               case Reg8:
               case Reg16:
               case Reg32:
               case FloatReg:
               case SReg2:
               case SReg3:
               case Debug:
               case Control:
               case Test:
                    for (e=0;strlen(i386_regtab[e].reg_name);e++){
                       if (!strcmp(i386_regtab[e].reg_name,parm[k])){

                            // Short reg form
                            if (i386_optab[i].opcode_modifier&ShortForm){
                               opcode[pcmd]=((opcode[pcmd])|(i386_regtab[e].reg_num));

                            // Short SReg form
                            } else if (i386_optab[i].opcode_modifier&(Seg2ShortForm|Seg3ShortForm)){
                               opcode[pcmd]=((opcode[pcmd])|(i386_regtab[e].reg_num<<3));

                            // PmodRM
                            } else {
                              if (pmodrm==-1) ErrorExit(6);

                              // Reg/Mem
                              if ((Mem|Mem8|Mem16|Mem32|MemReg)&cmdparam[k]){
                                 opcode[pmodrm]=((opcode[pmodrm])|(i386_regtab[e].reg_num|0xC0));

                              // Register
                              } else {
                                 opcode[pmodrm]=((opcode[pmodrm])|(i386_regtab[e].reg_num<<3));

                              }
                            }
                            e=-1;break;
                       }
                    }
                    if (e!=-1) ErrorExit(6);
                    break;

                    // Memory
               case Mem:
               case Mem8:
               case Mem16:
               case Mem32:
               case MemFL:

                    // Variables
                    unsigned int sibss,sibindex,reg_base,field_used,usemem;

                    if (pmodrm==-1) ErrorExit(6);

                    usemem=0; // 1 - 16 ; 2 - 32

                    // DEBUG: cout << "<!" << parm[k] << ">";
                    char *pts;
                    while (strstr(parm[k],"][")!=NULL) *strstr(parm[k],"][")='+';
                    while (strchr(parm[k],']')!=NULL) *strchr(parm[k],']')='[';
                    while (strchr(parm[k],'[')!=NULL){
                      pts=strchr(parm[k],'[');
                      do {
                        pts[0]=pts[1];
                        pts++;
                      } while(pts[0]!=0);
                    }
                    // DEBUG: cout << "<" << parm[k] << ">";

                    o=0;field_used=0;
                    if (parm[k][0]=='~'){field_used=TinyBase;parm[k]++;}
                    while (strlen(parm[k])){

                      // Get parameter name
                      h=0;while ((parm[k][h]!=0)&&((parm[k][h]!='+'&&parm[k][h]!='-')||h==0)) h++;
                      strcpy(memindex,parm[k]);parm[k]=parm[k]+h;
                      memindex[h]=0;pfix=memindex;

                      if (strlen(pfix)<=1) continue;

                      // Analyze
                      if (pfix[0]!='-'){
                         if (pfix[0]=='+') pfix++;

                         if (!strcmp(pfix,"bx"))
                           {if (field_used&BX_used) ErrorExit(19);field_used|=BX_used;if (usemem==2) ErrorExit(17);usemem=1;}
                         else if (!strcmp(pfix,"bp"))
                           {if (field_used&BP_used) ErrorExit(19);field_used|=BP_used;if (usemem==2) ErrorExit(17);usemem=1;}
                         else if (!strcmp(pfix,"si"))
                           {if (field_used&SI_used) ErrorExit(19);field_used|=SI_used;if (usemem==2) ErrorExit(17);usemem=1;}
                         else if (!strcmp(pfix,"di"))
                           {if (field_used&DI_used) ErrorExit(19);field_used|=DI_used;if (usemem==2) ErrorExit(17);usemem=1;}
                         else if (strchr(pfix,'*')==pfix+2&&pfix[0]=='0'){

                            if (usemem==1) ErrorExit(18);
                            usemem=2;
                            if (field_used&MemIndex) ErrorExit(7);
                            // Index
                            field_used|=MemIndex;
                            if (pfix[0]=='0'&&pfix[1]=='1') sibss=0;
                            else if (pfix[0]=='0'&&pfix[1]=='2') sibss=1;
                            else if (pfix[0]=='0'&&pfix[1]=='4') sibss=2;
                            else if (pfix[0]=='0'&&pfix[1]=='8') sibss=3;
                            else ErrorExit(8);

                            if (pfix[2]!='*') ErrorExit(9);
                            pfix+=3;

                            for (h=0;strlen(i386_regtab[h].reg_name);h++){
                               if (!strcmp(i386_regtab[h].reg_name,pfix)){
                                   if (!(i386_regtab[h].reg_type&Reg32)) ErrorExit(20);
                                   sibindex=i386_regtab[h].reg_num;h=-1;break;
                               }
                            }

                            if (h!=-1) ErrorExit(20);

                         } else {

                            // Register
                            for (h=0;strlen(i386_regtab[h].reg_name);h++){
                               if (!strcmp(i386_regtab[h].reg_name,pfix)){
                                   if (usemem==1) ErrorExit(18);
                                   if (field_used&MemBase){
                                     field_used|=MemIndex;usemem=2;
                                     if (!(i386_regtab[h].reg_type&Reg32)) ErrorExit(21);
                                     sibss=0;sibindex=i386_regtab[h].reg_num;h=-1;break;
                                   } else {
                                     field_used|=MemBase;usemem=2;
                                     if (!(i386_regtab[h].reg_type&Reg32)) ErrorExit(21);
                                     reg_base=i386_regtab[h].reg_num;h=-1;break;
                                   }
                               }
                            }

                            // Number
                            if (h!=-1){
                               field_used|=MemAddr;
                               o+=strtolong(pfix+1);
                            }
                         }

                      } else {

                         // Number
                         field_used|=MemAddr;
                         pfix++;o-=strtolong(pfix+1);
                      }
                    }

                    // Mem16 or Mem32
                    if (usemem==0) usemem=code32+1;
                    if ((usemem-1)!=code32){

                      //insert 0x67
                      for (int qr=opsz;qr>=0;qr--) opcode[qr+1]=opcode[qr];
                      opsz++;opcode[0]=MemoryPrefix;
                      if (pmodrm!=-1) pmodrm++;
                      if (pcmd!=-1) pcmd++;
                      if (psib!=-1) psib++;
                    }

                    // If S-I-B Present
                    if ((field_used&MemIndex)||(reg_base==4)){

                       // Create S-I-B byte
                       psib=pmodrm+1;
                       if (opsz>psib){
                         ErrorExit(22);
                       }
                       opsz++;

                       // setup S-I-B byte
                       opcode[pmodrm]|=4;
                       if (sibindex==4) ErrorExit(12);
                       opcode[psib]=(sibss<<6)|(sibindex<<3);
                       if (!(field_used&MemIndex)) opcode[psib]=0x20;

                       // Base register
                       if (field_used&MemBase){
                          opcode[psib]|=reg_base;
                          // if EBP used => MemAddr must be set
                          if ((reg_base==5)&&(!(field_used&MemAddr))){field_used|=MemAddr;o=0; }
                       } else {
                          opcode[psib]|=5; // reg not present
                          if (!(field_used&MemAddr)){field_used|=MemAddr;o=0;}
                       }

                    // S-I-B not present
                    } else {

                       // Mem16
                       if (usemem==1){

                          if (field_used&5) ErrorExit(8);
                          reg_base=field_used>>4;
                          switch(reg_base){
                           case 5:reg_base=0;break;   //BX+SI
                           case 9:reg_base=1;break;   //BX+DI
                           case 6:reg_base=2;break;   //BP+SI
                           case 10:reg_base=3;break;  //BP+DI
                           case 4:reg_base=4;break;   //SI
                           case 8:reg_base=5;break;   //DI
                           case 2:reg_base=6;         //BP
                                   if ((field_used&MemAddr)==0){
                                     field_used|=MemAddr;
                                     field_used|=TinyBase;
                                     o=0;
                                   }
                                   break;
                           case 1:reg_base=7;break;   //BX
                           case 0:reg_base=6;break;   //none
                           default: ErrorExit(8);
                          }
                          opcode[pmodrm]|=reg_base;

                       // Mem32
                       } else if (field_used&MemBase){
                          opcode[pmodrm]|=reg_base;
                          // if EBP used => MemAddr must be set
                          if ((reg_base==5)&&(!(field_used&MemAddr))){field_used|=MemAddr;o=0;}
                       } else {
                          opcode[pmodrm]|=5; // reg not present
                          if (!(field_used&MemAddr)){field_used|=MemAddr;o=0;}
                       }

                    }

                    // Save imm Disp
                    if (field_used&MemAddr){

                      // Memory16
                      if (usemem==1){

                        // Disp16
                        if (!(field_used&TinyBase)){
                           if ((o+0x8000)>0xFFFF) ErrorExit(15);
                           opcode[opsz]=o&0xFF;opsz++;opcode[opsz]=(o>>8)&0xFF;opsz++;
                           if (field_used>>4) opcode[pmodrm]|=0x80;

                        // Disp8
                        } else {
                          if ((o+0x80)>0xFF) ErrorExit(15);
                          if (!(field_used>>4)) ErrorExit(8);
                          opcode[opsz]=o&0xFF;
                          opsz++;opcode[pmodrm]|=0x40;
                        }


                      // Memory32
                      } else {

                        // Disp32
                        if (!(field_used&TinyBase)){
                           for (h=0;h<4;h++) opcode[opsz+h]=((o>>(h*8))&0xFF);
                           opsz+=4;if ((((field_used&MemAddr)!=0)&&((field_used&MemBase)==0))==0) opcode[pmodrm]|=0x80;

                        // Disp8
                        } else {
                          if ((o+0x80)>0xFF) ErrorExit(15);
                          if (((field_used&MemAddr)!=0)&&((field_used&MemBase)==0)) ErrorExit(15);
                          opcode[opsz]=o&0xFF;
                          opsz++;opcode[pmodrm]|=0x40;
                        }
                      }
                    }

                    break;

                    // Default unknown
               default:
                    ErrorExit(6);
             }
           }

           i=-1;break;
         }
       }
   }
   if (i!=-1) ErrorExit(1);
 }

// Longint -> String (Hex) in PP
void Hex32(unsigned long qw,char *pp){
     ultoa(qw,pp,16);for (int cir=strlen(pp)+1;cir>0;cir--) pp[cir]=pp[cir-1];pp[0]='0';}

// Number(dec) in string => uns long
unsigned long StrDectolong(char *str)
{
   unsigned long rnum = 0;
   unsigned char ch;

   // DEBUG: cout << "<" << str << ">" << endl;

   for (int pw=0;pw<strlen(str);pw++){
     ch=*(str+pw)-'0';
     if (ch>9) ErrorExit(13);
     rnum=rnum*10+ch;
   }

   return rnum;
}

// Kills spaces at begin and end of string
void KillSpaces(char *ks){ while (ks[0]==' '){for (int kscn=1;kscn<strlen(ks);kscn++) ks[kscn-1]=ks[kscn];
     ks[strlen(ks)-1]=0;}while ((strlen(ks))&&(ks[strlen(ks)-1]==' ')) ks[strlen(ks)-1]=0;}

// --------------------------------------------------------------------------
