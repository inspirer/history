// directive.c

// supported directives:
//   #include "local_file_name" or #include <glob_file_name>
//   #define <label> <text....><endl>
//   #undef <label>
//   #ifdef <label>
//   #ifndef <label>
//   #else
//   #endif
//   #error <message>
// after directive must be <endl> or ";"

//#define _DEBUG_directive

#include "i386.h"
#include "cc.h"

int ifdef_lev,ifdef_loc;  // 0-OK else LEVEL, structure checker

int directive()
{
  char s[fn_size],*x;
  int i,e;
  unsigned long offs_;

  if (strncmp(_line,"#include",8)==0){

     if (ifdef_lev) return 1;

     _line+=8;_lntp+=8;

     if (*_line!=' '){ error("#include: Invalid directive",eerror);return 1; }
     while(*_line==' '){ _line++;_lntp++;}

     if (*_line=='<') i='>';
       else if (*_line=='"') i='"';
         else { error("#include: Invalid directive",eerror);return 1; }

     if(i=='>'){ strcpy(s,include);e=strlen(s); } else e=0;
     _line++;_lntp++;

     while(*_line!=0&&*_line!=i&&e<fn_size){
       s[e++]=*_line++;
     } s[e]=0;

     if (*_line==0){ error("#include: Invalid directive",eerror);return 1;}
     if (e>=fn_size){ error("too big string constant",eerror);return 1;}
     _line++;_lntp++;

     while(*_line==' '){ _line++;_lntp++;}

     if (*_line!=';'&&*_line!=0) error("end of line and `;' not found in include",ewarn);

     e=OpenFile(s);
     switch(e){
       case 1:error("memory allocation",eerror);return 1;
       case 2:error("included file not found",eerror);return 1;
     }
     tfs->lines--;
     tfs->prev->lines++;

     return 1;

  } else if (strncmp(_line,"#define",7)==0){

     if (ifdef_lev) return 1;

     _line+=7;_lntp+=7;

     if (*_line!=' '){ error("#define: Invalid directive",eerror);return 1; }
     while(*_line==' '){ _line++;_lntp++;}

     if(*_lntp!='i'){
       error("Define directive needs an identifier",eerror);
       *_line=0;*_lntp=0;return 1;
     }

     x=_line;

     while(*_lntp=='i'){ _lntp++;_line++;}

     // Macro function
     if (*_lntp=='('){

        *_line='*';
        while(*_lntp!=')'){
           _lntp++;_line++;
           if (*_lntp!='i'&&*_lntp!=','&&*_lntp!=')'&&*_lntp!=' '){
              error("invalid macro argument syntax",eerror);
              *_lntp=0;*_line=0;
              return 1;
           }
        }

        *_line=0;_line++;
        while(*_line==' '){ _line++;}
        AddDefine(x,_line,1);

     // simple defines
     } else {

        if (*_line!=0){
          *_line=0;_line++;
          while(*_line==' '){ _line++;}
        }
        AddDefine(x,_line,0);

     }

     return 1;

  } else if (strncmp(_line,"#undef",6)==0){

     if (ifdef_lev) return 1;

     _line+=6;_lntp+=6;
     if (*_line!=' '){ error("#undef: Invalid directive",eerror);return 1; }
     while(*_line==' '){ _line++;_lntp++;}

     if(*_lntp!='i'){ error("Bad undef directive syntax",eerror);return 1;}
     x=_line;

     while(*_lntp=='i'){ _line++;_lntp++;}
     while(*_line==' ') _line++;
     if(*_line!=0&&*_line!=';') error("Expecting end of line or `;' in undef",eerror);

     *_line=0;
     if (!FreeDefine(x)) error("#undef: unknown symbol",ewarn);

     return 1;

  } else if (strncmp(_line,"#ifdef",6)==0){

     _line+=6;_lntp+=6;

     if (*_line!=' '){ error("#ifdef: Invalid directive",eerror);return 1; }
     while(*_line==' '){ _line++;_lntp++;}

     if(*_lntp!='i'){ error("#ifdef: directive needs an identifier",eerror);return 1;}
     x=_line;

     while(*_lntp=='i'){ _line++;_lntp++;}
     while(*_line==' ') _line++;
     if(*_line!=0&&*_line!=';') error("#ifdef: Expecting end of line or `;'",eerror);

     *_line=0;

     if (ifdef_lev==0){
       if (!CheckDefine(x)) ifdef_lev++;
     } else ifdef_lev++;
     ifdef_loc++;
     return 1;

  } else if (strncmp(_line,"#ifndef",7)==0){

     _line+=7;_lntp+=7;

     if (*_line!=' '){ error("#ifndef: Invalid directive",eerror);return 1; }
     while(*_line==' '){ _line++;_lntp++;}

     if(*_lntp!='i'){ error("#ifndef: Directive needs an identifier",eerror);return 1;}
     x=_line;

     while(*_lntp=='i'){ _line++;_lntp++;}
     while(*_line==' ') _line++;
     if(*_line!=0&&*_line!=';') error("#ifndef: Expecting end of line or `;'",eerror);
     *_line=0;

     if (ifdef_lev==0){
       if (CheckDefine(x)) ifdef_lev++;
     } else ifdef_lev++;
     ifdef_loc++;
     return 1;

  } else if (strncmp(_line,"#endif",6)==0){

     if (ifdef_loc==0) error("#endif: Misplaced directive",efatal);
     _line+=6;
     while(*_line==' ') _line++;

     if(*_line!=0&&*_line!=';') error("#endif: Expecting end of line or `;'",eerror);

     if (ifdef_lev) ifdef_lev--;
     ifdef_loc--;
     return 1;

  } else if (strncmp(_line,"#else",5)==0){

     if (ifdef_loc==0) error("#else: Misplaced directive",efatal);
     _line+=5;
     while(*_line==' ') _line++;

     if(*_line!=0&&*_line!=';') error("#else: Expecting end of line or `;'",eerror);

     if (ifdef_lev==0) ifdef_lev++; else if (ifdef_lev==1) ifdef_lev--;

     return 1;

  } else if (strncmp(_line,"#error",6)==0){

     if (ifdef_lev) return 1;

     _line+=6;
     if (*_line!=' '){ error("#error: Invalid directive",eerror);return 1; }
     while(*_line==' ') _line++;
     error(_line,efatal);

     return 1;

  } else if (strncmp(_line,"#pragma",7)==0){

     // NOP
     if (ifdef_lev) return 1;
     _line+=7;_lntp+=7;

     if (*_line!=' '){ error("#pragma: Invalid directive",eerror);return 1; }
     while(*_line==' '){ _line++;_lntp++;}

     #ifdef _DEBUG_directive
      printf("#pragma:%s;\n",_line);
     #endif

     return 1;
  } else if (strncmp(_line,"#code",5)==0){

     // NOP
     if (ifdef_lev) return 1;
     _line+=5;_lntp+=5;

     if (*_line!=' '){ error("#code: Invalid directive",eerror);return 1; }
     while(*_line==' '){ _line++;_lntp++;}

     #ifdef _DEBUG_directive
      printf("#code:%s;\n",_line);
     #endif

     if (strcmp(_line,"16")==0) code32=0;
     else if (strcmp(_line,"32")==0) code32=1;
     else error("#code: unknown code type",eerror);

     sprintf(line,"xcode %i",code32);
     _line=line;_lntp=lntp;

     return 0;
  } else if (strncmp(_line,"#align",6)==0){

     // NOP
     if (ifdef_lev) return 1;
     _line+=6;_lntp+=6;

     if (*_line!=' '){ error("#align: Invalid directive",eerror);return 1; }
     while(*_line==' '){ _line++;_lntp++;}

     i=strtol(_line,NULL,10);
     if (i==1) return 1;
     if (i<2||i>64) error("#align: range error",eerror);
     sprintf(line,"db [(%i-($%%%i))%%%i]",i,i,i);
     _line=line;_lntp=lntp;

     return 0;

  } else if (strncmp(_line,"#org",4)==0){

     // NOP
     if (ifdef_lev) return 1;
     _line+=4;_lntp+=4;

     if (*_line!=' '){ error("#org: Invalid directive",eerror);return 1; }
     while(*_line==' '){ _line++;_lntp++;}

     offs_=strtoul(_line,NULL,10);
     if (errno!=0){ error("#org: offset error",eerror);return 1; }

     sprintf(line,"org %u",offs_);
     _line=line;_lntp=lntp;

     return 0;

  } else if (*_line=='#'){

     error("unknown directive",eerror);
     return 1;
  }

  return 0;
}

// LOCAL: xarea++;def_area=xarea;
