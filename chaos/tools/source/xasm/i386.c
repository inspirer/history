// i386.c

#include "cc.h"
#include "i386.h"

//#define _DEBUG_strtolx
//#define _DEBUG_strtolong
//#define _DEBUG_ProcessNum
//#define _DEBUG_i386
//#define _DEBUG_defdata

struct TInstruction *ins,*pi,*_pi,*s_del,*c_del;
long instr;

 // if register
int i386reg(char *rg)
{
  int cf;

  for (cf=0;*i386_regtab[cf].reg_name!=0;cf++)
       if (!strcmp(i386_regtab[cf].reg_name,rg)) return 1;

  return 0;
}

 // if 386 prefix
int i386prefix(char *rg)
{
  int cf;

  for (cf=0;*i386_prefixtab[cf].prefix_name!=0;cf++)
       if (!strcmp(i386_prefixtab[cf].prefix_name,rg)) return 1;

  return 0;
}

int estrtol;
unsigned long strtolx(char *_st,int _BASE)
{
  unsigned long res,o,a,q;

  #ifdef _DEBUG_strtolx
   fprintf(stderr,"#strtolx: %s(%i)\n",_st,_BASE);
  #endif

  estrtol=res=o=0;
  while(*_st!=0){
    res*=_BASE;a=(unsigned long)(*_st);
    if (a>='0'&&a<='9') a-=48;
    else if (a>='A'&&a<='Z') a-=55;
    else if (a>='a'&&a<='z') a-=87;
    else { estrtol=1;return 0;}
    if (a>=_BASE){ estrtol=1;return 0;}
    q=res/_BASE;if (q!=o){ estrtol=2;return 0;}
    res+=a;o=res;_st++;
  }
  return res;
}

unsigned long strtolong(char *st)
{
  unsigned long res;
  char *ptr;

  res=0;
  if (*st=='0'&&*(st+1)=='x'){

    // HEX
    res=strtolx(st+2,16);


  } else if (*st>='0'&&*st<='9'&&st[strlen(st)-1]=='h'){

    // HEX
    st[strlen(st)-1]=0;
    res=strtolx(st,16);
    st[strlen(st)]='h';

  } else if (*st>='1'&&*st<='9'){

    // DEC
    res=strtolx(st,10);

  } else if (*st=='0'){

    // OCT
    res=strtolx(st,8);

  } else error("number error",easm);

  if (estrtol==2) error("number is out of range",easm);
  else if (estrtol==1) error("extra characters",easm);

  #ifdef _DEBUG_strtolong
   fprintf(stderr,"#strtolong: %s -> %08x\n",st,res);
  #endif

  return res;
}

char *_exp;

unsigned long get_next_oper(char *t)
{
  char *place=t;
  unsigned long res;
  int chs;

  while(*_exp==' ') _exp++;
  if (*_exp==39){
    _exp++;res=0;chs=0;
    while(*_exp&&*_exp!=39&&chs<4){ res+=(*_exp)<<(chs*8);chs++;_exp++;}
    if (*_exp!=0&&*_exp!=39) error("character constant is too big",easm);
    else if (*_exp!=39) error("apostrophe absent",easm);
    if (*_exp==39) _exp++;
    return res;

  } else {
    while(ident_char[*_exp]!=0) *place++ = *_exp++;
    *place=0;place=t;

    if (*place==0){ error("expression error (null operand)",easm);return 0;}
    if (ident_char[*place]==_nm)
      return strtolong(place);
    else return GetIdent(place);
  }
}

unsigned long _action(unsigned long p1,unsigned long p2,char _act)
{
  unsigned long i,e;

  switch(_act){
    case '+': e=p1+p2;break;
    case '/': e=p1/p2;break;
    case '-': e=p1-p2;break;
    case '*': e=p1*p2;break;
    case '%': e=p1%p2;break;
    case '^': e=p1^p2;break;
    case '*'|0x80: e=1;for(i=0;i<p2;i++) e*=p1;break;
    case '>'|0x80: e=p1>>p2;break;
    case '<'|0x80: e=p1<<p2;break;
    case '|': e=p1|p2;break;
    case '&': e=p1&p2;break;
    default: error("unknown action",easm);e=0;break;
  }

  #ifdef _DEBUG_ProcessNum
    fprintf(stderr,"#_action: %u:%c:%u = %u\n",p1,_act,p2,e);
  #endif
  return e;
}

unsigned long _pnum(int _deep,char *t,struct TInstruction *p)
{
  int sign='+',sign2='+',opnum=0;
  unsigned long res=0;
  long first_oper=0,sec_oper=0;

  #ifdef _DEBUG_ProcessNum
    fprintf(stderr,"#_pnum{ (%i), expr=%s;\n",_deep,_exp);
  #endif

  while(*_exp&&*_exp!=')') switch(*_exp){

     case '(':
        process_open_bracket:
        opnum++;_exp++;
        if (opnum==1) first_oper=_pnum(_deep+1,t,p);
        else if (opnum==2){
          sec_oper=_pnum(_deep+1,t,p);
          first_oper=_action(first_oper,sec_oper,sign);
          opnum=1;
        }
        break;

     case '>': case '<': case '%': case '/': case '*': case '^': case '|': case '&':
        sign = *_exp++;if (sign==*_exp){ sign|=0x80;_exp++; }
        while(*_exp==' ') _exp++;
        if (opnum==0){ error("first operand absent for action",easm);opnum++;}
        if (*_exp=='(') goto process_open_bracket;
        if (*_exp=='$'){
          first_oper=_action(first_oper,p->offs,sign);
          _exp++;
        } else {
          sec_oper=get_next_oper(t);
          first_oper=_action(first_oper,sec_oper,sign);
        }
        break;

     case '+': case '-':
        if (sign2=='-') first_oper=-first_oper;
        res+=first_oper;opnum=0;first_oper=0;
        sign2=*_exp++;
        break;

     case ' ': _exp++;break;

     case '$':
        _exp++;opnum++;
        if (opnum==1) first_oper=p->offs;
        else if (opnum==2){ error("no operation specified",easm);opnum=0;}
        break;

     default:
        if(ident_char[*_exp]==0&&*_exp!=39) error("unknown symbol encountered",easm);
        opnum++;if (opnum==1) first_oper=get_next_oper(t);
        else if (opnum==2){ error("no operation specified",easm);opnum=0;_exp++;}
        break;

  }

  if (sign2=='-') first_oper=-first_oper;
  res+=first_oper;

  if (*_exp==')'){
    _exp++;if (_deep==0) error("extra closing bracket",easm);
  } else if (_deep>0) error("closing bracket not found",easm);

  #ifdef _DEBUG_ProcessNum
    fprintf(stderr,"#_pnum} (%i), expr=%s; res=%08x\n",_deep,_exp,res);
  #endif

  return res;
}

unsigned long ProcessNum(char *nm,struct TInstruction *p)
{
  char t[100];

  #ifdef _DEBUG_ProcessNum
   fprintf(stderr,"#pn:%s;\n",nm);
  #endif

  _exp=nm;
  return _pnum(0,t,p);

}

long defdata(struct TInstruction *p,FILE *fout)
{
  char *s,*q,expr[200];
  int i,e,type;
  long size=0,a;

  #ifdef _DEBUG_defdata
    fprintf(stderr,"#defdata:%s=%s;\n",p->command,p->params[0]);
  #endif

  // get type
  s=p->command;
  if (*s=='d'&&s[2]==0) switch(s[1]){
    case 'b': type=1;break;
    case 'w': type=2;break;
    case 'd': type=4;break;
    default : type=0;break;
  } else type=0;
  if (!type){ error("data definition error",easm);return 0;}

  // process
  s=p->params[0];
  while(*s){
    while(*s==' ') s++;
    switch(*s){
      case '[':
        q=expr;s++;
        while(*s!=']'&&*s!=0){
          if (*s==39){
            *(q++)=*(s++);
            while(*s!=39) *(q++)=*(s++);
          }
          *(q++)=*(s++);
        } *q=0;q=expr;
        if (*s==0){ error("']' absent",easm);return 0;}
        a=ProcessNum(q,p)*type;size+=a;s++;
        if (fout!=NULL) while(a--) fputc(0,fout);
        break;

      case '"':
        s++;a=0;
        while(*s!='"'&&*s!=0){
          if (*s=='\\'){ s++;if (*s=='n'){ *s=10;*(--s)=13;} if (*s=='t') *s=9;}
          if (*s==0) continue;
          if (fout!=NULL) fputc(*s,fout);
          s++;a++;
        }
        if (*s!='"') error("string expression error",easm);
        size+=a;s++;
        break;

      case 0:
        break;

      default:
        q=expr;
        while(*s!=','&&*s!=0){
          if (*s==39){
            *(q++)=*(s++);
            while(*s!=39) *(q++)=*(s++);
          }
          *(q++)=*(s++);
        } *q=0;q=expr;
        a=ProcessNum(q,p);
        size+=type;
        if (fout!=NULL){
          for (i=0;i<type;i++){ fputc(a&0xFF,fout);a>>=8;}
          for (;i<4;i++){ if ((a&0xFF)!=0&&(a&0xFF)!=0xff) error("number is too big",easmw);a>>=8;}
        }
        break;

    }
    while(*s==' ') s++;
    if (*s!=','&&*s!=0) error("data expression error",easm);
    if (*s==',') s++;
  }

  #ifdef _DEBUG_defdata
    fprintf(stderr,"#%08x:%s; size=%i, type=%i\n",p->offs,p->params[0],size,type);
  #endif

  return size;
}

int i386(struct TInstruction *p)
{
  int a,b,pcmd,pmodrm;
  unsigned long o,so,cmdparam[3];

  // mem Variables
  unsigned int sibss,psib,sibindex,reg_base,field_used,usemem,h;
  char *pts,*cparam,*pfix,memindex[80],*gh;

  #ifdef _DEBUG_i386
    fprintf(stderr,"#i386(%i,%i):%s",p->cmdnum,p->direction,p->command);
    for (b=0;b<p->pnum;b++) fprintf(stderr,"_%s(%i)",p->params[b],p->ptype[b]);
    fprintf(stderr,"; (%s), line %i, file %s\n",p->prefix,p->sline,p->fname);
  #endif

  // Set Code prefix
  if (( (i386_optab[p->cmdnum].opcode_modifier&_C32)&&(code32==0) )||(
    (i386_optab[p->cmdnum].opcode_modifier&_C16)&&(code32==1) )){
    p->code[p->size++]=CodePrefix;
  }

  // get extra prefix (rep,addr,.. etc)
  gh=p->prefix;
  while(*gh){
    pfix=gh;while(*gh!=','&&*gh) gh++;
    if (*gh==',') *gh++=0;a=0;
    while(*i386_prefixtab[a].prefix_name){
       if (strcmp(pfix,i386_prefixtab[a].prefix_name)==0)
          p->code[p->size++]=i386_prefixtab[a].prefix_code;
       a++;
    }
  }

  // Code byte number
  o=i386_optab[p->cmdnum].base_opcode;a=0;
  while (o!=0) {o>>=8;a++;}if (a==0) a++;

  // Save opcode
  o=i386_optab[p->cmdnum].base_opcode;
  for (b=a;b>0;b--){ p->code[p->size+b-1]=o&0xFF;o>>=8; }
  p->size+=a;pcmd=p->size;pcmd--;

  // Save direction
  if (p->direction) p->code[pcmd]|=2;

  // Modrm present?
  if ((i386_optab[p->cmdnum].opcode_modifier&Modrm)!=0){
     pmodrm=p->size;p->code[pmodrm]=0;p->size++;

     // Save extension opcode
     if (i386_optab[p->cmdnum].extension_opcode!=None)
         p->code[pmodrm]=i386_optab[p->cmdnum].extension_opcode<<3;
  }

  // Save parameters
  for (a=0;a<p->pnum;a++){

    // Get parameter type
    if (!p->direction) cmdparam[a]=i386_optab[p->cmdnum].operand_types[a];
                  else cmdparam[a]=i386_optab[p->cmdnum].operand_types[1-a];

    // If unknown Disp/ImmOp => set it with code/code32
    if (cmdparam[a]==DispXX){
       if (code32) cmdparam[a]=Disp32;
              else cmdparam[a]=Disp16;
    }
    if (cmdparam[a]==ImmXX){
       if (code32) cmdparam[a]=Imm32;
              else cmdparam[a]=Imm16;
    }

    // Switch
    switch(cmdparam[a]&p->ptype[a]){

           // Save imm operand
      case Imm8:
           o=ProcessNum(p->params[a],p);
           so=o;so>>=8;
           if (so!=0&&so!=0xffffff){ error("imm8 is out of range",easm);return 0;}
           p->code[p->size++]=o&0xFF;
           break;
      case Imm16:
           o=ProcessNum(p->params[a],p);
           so=o;so>>=16;
           if (so!=0&&so!=0xffff){ error("imm16 is out of range",easm);return 0;}
           for (b=0;b<2;b++) p->code[p->size++]=((o>>(b*8))&0xFF);
           break;
      case Imm32:
           o=ProcessNum(p->params[a],p);
           for (b=0;b<4;b++) p->code[p->size++]=((o>>(b*8))&0xFF);
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
           o=ProcessNum(p->params[a],p);
           if (tnum) o=o-p->size-1-p->offs;
                else o=0;
           if ((o+0x80)>0xFF){ error("disp8 is out of range",easm);return 0;}
           p->code[p->size++]=o&0xFF;
           break;
      case Disp16:
           o=ProcessNum(p->params[a],p);
           if (tnum) o=o-p->size-2-p->offs;
                else o=0;
           if ((o+0x8000)>0xFFFF){ error("disp16 is out of range",easm);return 0;}
           for (b=0;b<2;b++) p->code[p->size++]=((o>>(b*8))&0xFF);
           break;
      case Disp32:
           o=ProcessNum(p->params[a],p);
           o=o-p->size-4-p->offs;
           for (b=0;b<4;b++) p->code[p->size++]=((o>>(b*8))&0xFF);
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
           for (b=0;*(i386_regtab[b].reg_name);b++){
              if (!strcmp(i386_regtab[b].reg_name,p->params[a])){

                   // Short reg form
                   if (i386_optab[p->cmdnum].opcode_modifier&ShortForm){
                      p->code[pcmd]|=i386_regtab[b].reg_num;

                   // Short SReg form
                   } else if (i386_optab[p->cmdnum].opcode_modifier&(Seg2ShortForm|Seg3ShortForm)){
                      p->code[pcmd]|=(i386_regtab[b].reg_num<<3);

                   // PmodRM
                   } else {
                     if (pmodrm==-1){ error("internal opcode/register error",easm);return 0;}

                     // Reg/Mem
                     if ((Mem|Mem8|Mem16|Mem32|MemReg)&cmdparam[a]){
                        p->code[pmodrm]|=(i386_regtab[b].reg_num|0xC0);

                     // Register
                     } else {
                        p->code[pmodrm]|=(i386_regtab[b].reg_num<<3);

                     }
                   }
                   b=-1;break;
              }
           }
           if (b!=-1){error("internal opcode/register error",easm);return 0;}
           break;

           // Memory
      case Mem:
      case Mem8:
      case Mem16:
      case Mem32:
      case MemFL:

           // init
           if (pmodrm==-1){ error("internal opcode/register error",easm);return 0;}
           cparam=p->params[a];usemem=0; // 1 - 16 ; 2 - 32

           // kill brackets
           while ((pts=strstr(p->params[a],"]["))!=NULL) *pts='+';
           while ((pts=strchr(p->params[a],']'))!=NULL) *pts='[';
           while ((pts=strchr(p->params[a],'['))!=NULL) do {
               pts[0]=pts[1];
               pts++;
           } while(pts[0]!=0);

           // get parameter types
           o=0;field_used=0;
           if (*cparam=='~'){ field_used=TinyBase;cparam++;}

           #ifdef _DEBUG_i386
             fprintf(stderr,"#cparam:%s;\n",cparam);
           #endif

           while (*cparam){

             // Get parameter name
             h=0;while ((cparam[h]!=0)&&((cparam[h]!='+'&&cparam[h]!='-')||h==0)) h++;
             strcpy(memindex,cparam);cparam+=h;
             memindex[h]=0;pfix=memindex;

             if (strlen(pfix)<1) continue;

             // Analyze
             if (*pfix!='-'){
                if (*pfix=='+') pfix++;

                if (!strcmp(pfix,"bx")){
                   if (field_used&BX_used){ error("bx used twice",easm);return 0;}
                   field_used|=BX_used;
                   if (usemem==2){ error("Use 32-bit register (bx->ebx)",easm);return 0;}
                   usemem=1;

                } else if (!strcmp(pfix,"bp")){
                   if (field_used&BP_used){ error("bp used twice",easm);return 0;}
                   field_used|=BP_used;
                   if (usemem==2){ error("Use 32-bit register (bp->ebp)",easm);return 0;}
                   usemem=1;

                } else if (!strcmp(pfix,"si")){
                   if (field_used&SI_used){ error("si used twice",easm);return 0;}
                   field_used|=SI_used;
                   if (usemem==2){ error("Use 32-bit register (si->esi)",easm);return 0;}
                   usemem=1;

                } else if (!strcmp(pfix,"di")){
                   if (field_used&DI_used){ error("di used twice",easm);return 0;}
                   field_used|=DI_used;
                   if (usemem==2){ error("Use 32-bit register (di->edi)",easm);return 0;}
                   usemem=1;

                } else if (pfix[1]=='*'){

                   if (usemem==1){ error("Use 32-bit register (<1/2/4/8>*<reg32>)",easm);return 0;}
                   usemem=2;
                   if (field_used&MemIndex){ error("Using two index registers is unavailable",easm);return 0;}
                   field_used|=MemIndex;

                   switch(*pfix){
                     case '1': sibss=0;break;
                     case '2': sibss=1;break;
                     case '4': sibss=2;break;
                     case '8': sibss=3;break;
                     default : error("Addressing mode error",easm);
                               return 0;
                   }

                   pfix+=2;

                   for (h=0;*(i386_regtab[h].reg_name);h++){
                      if (!strcmp(i386_regtab[h].reg_name,pfix)){
                          if (!(i386_regtab[h].reg_type&Reg32)) break;
                          sibindex=i386_regtab[h].reg_num;h=-1;break;
                      }
                   }

                   if (h!=-1){ error("Use 32-bit register as index",easm);return 0;}

                } else {

                   // Register
                   for (h=0;*(i386_regtab[h].reg_name);h++){
                      if (!strcmp(i386_regtab[h].reg_name,pfix)){
                          if (usemem==1){ error("Use 16-bit register",easm);return 0;}
                          if (field_used&MemBase){
                            field_used|=MemIndex;usemem=2;
                            if (!(i386_regtab[h].reg_type&Reg32)){
                              error("Use 32-bit register as base",easm);
                              return 0;
                            }
                            sibss=0;sibindex=i386_regtab[h].reg_num;h=-1;break;
                          } else {
                            field_used|=MemBase;usemem=2;
                            if (!(i386_regtab[h].reg_type&Reg32)){
                              error("Use 32-bit register as base",easm);
                              return 0;
                            }
                            reg_base=i386_regtab[h].reg_num;h=-1;break;
                          }
                      }
                   }

                   // Number
                   if (h!=-1){
                      field_used|=MemAddr;
                      o+=ProcessNum(pfix,p);
                   }
                }

             } else {

                // Number
                field_used|=MemAddr;
                pfix++;o-=ProcessNum(pfix,p);
             }
           }

           // Mem16 or Mem32
           if (usemem==0) usemem=code32+1;
           if ((usemem-1)!=code32){

             //insert MemoryPrefix
             for (b=p->size;b>=0;b--) p->code[b+1]=p->code[b];
             p->size++;p->code[0]=MemoryPrefix;
             if (pmodrm!=-1) pmodrm++;
             if (pcmd!=-1) pcmd++;
             if (psib!=-1) psib++;
           }

           // If S-I-B Present
           if ((field_used&MemIndex)||reg_base==4){

              // Create S-I-B byte
              psib=pmodrm+1;
              if (p->size>psib){ error("S-I-B byte error",easm);return 0;}
              p->size++;

              // setup S-I-B byte
              p->code[pmodrm]|=4;
              if (sibindex==4){ error("ESP register cannot be used as index",easm);return 0;}
              p->code[psib]=(sibss<<6)|(sibindex<<3);
              if (!(field_used&MemIndex)) p->code[psib]=0x20;

              // Base register
              if (field_used&MemBase){
                 p->code[psib]|=reg_base;
                 // if EBP used => MemAddr must be set
                 if ((reg_base==5)&&(!(field_used&MemAddr))){field_used|=MemAddr;o=0; }
              } else {
                 p->code[psib]|=5; // reg not present
                 if (!(field_used&MemAddr)){field_used|=MemAddr;o=0;}
              }

           // S-I-B not present
           } else {

              // Mem16
              if (usemem==1){

                 if (field_used&5){ error("mem16 addressing mode error",easm);return 0;}
                 reg_base=field_used>>4;
                 switch(reg_base){
                  case  5: reg_base=0;break;   //BX+SI
                  case  9: reg_base=1;break;   //BX+DI
                  case  6: reg_base=2;break;   //BP+SI
                  case 10: reg_base=3;break;   //BP+DI
                  case  4: reg_base=4;break;   //SI
                  case  8: reg_base=5;break;   //DI
                  case  2: reg_base=6;         //BP
                           if ((field_used&MemAddr)==0){
                             field_used|=MemAddr;
                             field_used|=TinyBase;
                             o=0;
                           }
                           break;
                  case  1: reg_base=7;break;   //BX
                  case  0: reg_base=6;break;   //none
                  default: error("addressing mode error",easm);
                           return 0;
                 }
                 p->code[pmodrm]|=reg_base;

              // Mem32
              } else if (field_used&MemBase){
                 p->code[pmodrm]|=reg_base;
                 // if EBP used => MemAddr must be set
                 if (reg_base==5&&(!(field_used&MemAddr))){field_used|=MemAddr;o=0;}
              } else {
                 p->code[pmodrm]|=5; // reg not present
                 if (!(field_used&MemAddr)){field_used|=MemAddr;o=0;}
              }

           }

           // Save imm Disp
           if (field_used&MemAddr){

             // Memory16
             if (usemem==1){

               // Disp16
               if (!(field_used&TinyBase)){
                  if ((o+0x8000)>0xFFFF){ error("reference overriding",easm);return 0;}
                  p->code[p->size++]=o&0xFF;p->code[p->size++]=(o>>8)&0xFF;
                  if (field_used>>4) p->code[pmodrm]|=0x80;

               // Disp8
               } else {
                 if ((o+0x80)>0xFF){ error("reference overriding",easm);return 0;}
                 if (!(field_used>>4)){ error("addressing mode error",easm);return 0;}
                 p->code[p->size++]=o&0xFF;
                 p->code[pmodrm]|=0x40;
               }

             // Memory32
             } else {

               // Disp32
               if (!(field_used&TinyBase)){
                  for (h=0;h<4;h++) p->code[p->size++]=((o>>(h*8))&0xFF);
                  if ((((field_used&MemAddr)!=0)&&((field_used&MemBase)==0))==0) p->code[pmodrm]|=0x80;

               // Disp8
               } else {
                 if ((o+0x80)>0xFF){ error("reference overriding",easm);return 0;}
                 if (((field_used&MemAddr)!=0)&&((field_used&MemBase)==0)){
                   error("reference overriding",easm);return 0;
                 }
                 p->code[p->size++]=o&0xFF;
                 p->code[pmodrm]|=0x40;
               }
             }
           }

           break;

           // Default unknown
      default:
           error("internal opcode error",easm);
           return 0;
    }
  }
  #ifdef _DEBUG_i386
   fprintf(stderr,"#i386: done: size=%i, offs=%i, area=%i :",p->size,p->offs,p->def_area);
   for (a=0;a<p->size;a++) fprintf(stderr,"%02x:",p->code[a]);
   fprintf(stderr,"\n");
  #endif
  return 1;
}
