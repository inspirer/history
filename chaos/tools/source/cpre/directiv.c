// directive.c

// supported directives:
//   #include "local_file_name" or #include <glob_file_name>
//   #define <label> <text....><endl>
//   #undef <label>
//   #ifdef <label>
//   #ifndef <label>
// ? #if <expr>
//   #else
// ? #elif <expr>
//   #endif
//   #error <message>
// after directive must be <endl> or ";"

#include "cc.h";

int ifdef_lev,ifdef_loc;  // 0-OK else LEVEL, structure checker

// Process IF & ELIF expressions
int if_expr(char *exp)
{
  return 1; // ????
}

int directive()
{
  char s[file_name_size],*x;
  int i,e;

  if (strncmp(_line,"#include",8)==0){

     if (ifdef_lev) return 1;

     _line+=8;_lntp+=8;

     if (_line[0]!=' '){ error("#include: Invalid directive",1);return 1; }
     while(_line[0]==' '){ _line++;_lntp++;}

     if (_line[0]=='<') i='>';
       else if (_line[0]=='"') i='"';
         else { error("#include: Invalid directive",1);return 1; }

     if(i=='>'){ strcpy(s,parent_dir);e=strlen(s); } else { e=0; }

     do {_line++;_lntp++;s[e]=_line[0];e++;} while(_line[1]!=i&&_line[1]!=0&&e<file_name_size);
     s[e]=0;_line++;_lntp++;

     if (_line[0]==0){ error("#include: Invalid directive",1);return 1;}
     if (e>=file_name_size){ error("too big string constant",1);return 1;}
     _line++;_lntp++;

     while(_line[0]==' '){ _line++;_lntp++;}

     if (_line[0]!=';'&&_line[0]!=0) error("end of line and `;' not found in include",2);

     e=OpenFile(s);
     switch(e){
       case 1:error("memory allocation",1);return 1;
       case 2:error("included file not found",1);return 1;
     }
     tfs->lines--;
     tfs->prev->lines++;

     return 1;
  } else if (strncmp(_line,"#define",7)==0){

     if (ifdef_lev) return 1;

     _line+=7;_lntp+=7;

     if (_line[0]!=' '){ error("#define: Invalid directive",1);return 1; }
     while(_line[0]==' '){ _line++;_lntp++;}

     if(_lntp[0]!='i'){ error("Define directive needs an identifier",1);_line[0]=0;_lntp[0]=0;return 1;}

     x=_line;

     while(_lntp[0]=='i'){ _lntp++;_line++;}

     // Macro function
     if (_lntp[0]=='('){

        _line[0]='*';
        while(_lntp[0]!=')'){
           _lntp++;_line++;
           if (_lntp[0]!='i'&&_lntp[0]!=','&&_lntp[0]!=')'&&_lntp[0]!=' '){
              error("invalid macro argument syntax",1);
              _lntp[0]=0;_line[0]=0;
              return 1;
           }
        }

        _line[0]=0;_line++;
        while(_line[0]==' '){ _line++;}
        AddDefine(x,_line,1);

     // simple defines
     } else {

        if (_line[0]!=0){
          _line[0]=0;_line++;
          while(_line[0]==' '){ _line++;}
        }
        AddDefine(x,_line,0);

     }

     return 1;
  } else if (strncmp(_line,"#undef",6)==0){

     if (ifdef_lev) return 1;

     _line+=6;_lntp+=6;
     if (_line[0]!=' '){ error("#undef: Invalid directive",1);return 1; }
     while(_line[0]==' '){ _line++;_lntp++;}

     if(_lntp[0]!='i'){ error("Bad undef directive syntax",1);_line[0]=0;_lntp[0]=0;return 1;}
     x=_line;

     while(_lntp[0]=='i'){ _line++;_lntp++;}
     while(_line[0]==' ') _line++;

     if(_line[0]!=0&&_line[0]!=';') error("Expecting end of line or `;' in undef",1);

     _line[0]=0;

     if (!FreeDefine(x)) error("#undef: Undefined symbol",2);

     return 1;
  } else if (strncmp(_line,"#ifdef",6)==0){

     _line+=6;_lntp+=6;

     if (_line[0]!=' '){ error("#ifdef: Invalid directive",1);return 1; }
     while(_line[0]==' '){ _line++;_lntp++;}

     if(_lntp[0]!='i'){ error("#ifdef: directive needs an identifier",1);_line[0]=0;_lntp[0]=0;return 1;}
     x=_line;

     while(_lntp[0]=='i'){ _line++;_lntp++;}
     while(_line[0]==' ') _line++;

     if(_line[0]!=0&&_line[0]!=';') error("#ifdef: Expecting end of line or `;'",1);

     _line[0]=0;

     if (ifdef_lev==0){
       if (!CheckDefine(x)) ifdef_lev++;
     } else ifdef_lev++;
     ifdef_loc++;
     return 1;
  } else if (strncmp(_line,"#ifndef",7)==0){

     _line+=7;_lntp+=7;

     if (_line[0]!=' '){ error("#ifndef: Invalid directive",1);return 1; }
     while(_line[0]==' '){ _line++;_lntp++;}

     if(_lntp[0]!='i'){ error("#ifndef: Directive needs an identifier",1);_line[0]=0;_lntp[0]=0;return 1;}
     x=_line;

     while(_lntp[0]=='i'){ _line++;_lntp++;}
     while(_line[0]==' ') _line++;

     if(_line[0]!=0&&_line[0]!=';') error("#ifndef: Expecting end of line or `;'",1);
     _line[0]=0;

     if (ifdef_lev==0){
       if (CheckDefine(x)) ifdef_lev++;
     } else ifdef_lev++;
     ifdef_loc++;
     return 1;
  } else if (strncmp(_line,"#endif",6)==0){

     if (ifdef_loc==0) error("#endif: Misplaced directive",3);
     _line+=6;
     while(_line[0]==' ') _line++;

     if(_line[0]!=0&&_line[0]!=';') error("#endif: Expecting end of line or `;'",1);

     if (ifdef_lev) ifdef_lev--;
     ifdef_loc--;
     return 1;
  } else if (strncmp(_line,"#else",5)==0){

     if (ifdef_loc==0) error("#else: Misplaced directive",3);
     _line+=5;
     while(_line[0]==' ') _line++;

     if(_line[0]!=0&&_line[0]!=';') error("#else: Expecting end of line or `;'",1);

     if (ifdef_lev==0) ifdef_lev++; else if (ifdef_lev==1) ifdef_lev--;

     return 1;
  } else if (strncmp(_line,"#error",6)==0){

     if (ifdef_lev) return 1;

     _line+=6;
     if (_line[0]!=' '){ error("#error: Invalid directive",1);return 1; }
     while(_line[0]==' ') _line++;
     error(_line,3);

     return 1;
  } else if (strncmp(_line,"#if",3)==0){

     _line+=3;
     if (_line[0]!=' '){ error("#if: Invalid directive",1);return 1; }
     while(_line[0]==' ') _line++;

     // Process IF
     if (ifdef_lev==0){
       if (!if_expr(_line)) ifdef_lev++;
     } else ifdef_lev++;

     ifdef_loc++;
     return 1;
  } else if (strncmp(_line,"#elif",5)==0){

     _line+=5;
     if (_line[0]!=' '){ error("#elif: Invalid directive",1);return 1; }
     while(_line[0]==' ') _line++;

     // Process ELIF
     if (ifdef_lev==0) ifdef_lev++;
      else if (ifdef_lev==1)
        if(if_expr(_line)) ifdef_lev--;

     return 1;
  } else if (strncmp(_line,"#pragma",7)==0){

     if (ifdef_lev) return 1;

     // NOP
     printf("%s\n",_line);

     return 1;
  } else if (_line[0]=='#'){

     error("unknown directive",1);
     return 1;
  }
  return 0;
}
