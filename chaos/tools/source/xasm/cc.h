// cc.h

#ifndef _INCLUDED_
 #define _INCLUDED_
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
#endif

#define fbuff_size       2048
#define fn_size          261
#define max_line_size    4096
#define max_ident_length 40
#define max_deep         100
#define _signed          1

// xasm.c
extern long lines,included;
extern char out[fn_size],inf[fn_size];
extern char line[max_line_size],lntp[max_line_size];
extern char include[fn_size];
extern int  optimizing;

extern char *_line,*_lntp;
extern int st;
extern char idname[];

// file.c
typedef struct TFileStream
{
 struct TFileStream *next,*prev;
 char *FileName,swp[fbuff_size];
 unsigned long sz,ps;
 long lines;
 int i,e;FILE *fp;
};
extern struct TFileStream *tfs;
extern char ch;
int ReadFile();          // 1 - char in `ch'
int CloseFile();
int OpenFile(char *);    // 0 - OK ; 1 - MEM_ER ; 2 - FILE_ER

// error.c
extern long warnings,errors;
extern long max_warn,max_err;
extern void error(char *,int);
#define eerror   1
#define ewarn    2
#define efatal   3
#define easm     4
#define easmw    5

// directiv.c
extern int ifdef_lev,ifdef_loc;
extern int directive();

// defs.c
extern void process();
extern int _cnter,_deep;

// ident.c
#define _ch  1
#define _nm  2
extern char ident_char[];
extern long total_idents,def_area,xarea;
void BuildImage(int);
void InitIdents();
void AddIdent(char *,struct TInstruction *);
unsigned long GetIdent(char *lname);
int  CheckIdent(char *);
void AddDefine(char *,char *,char);
int  CheckDefine(char *);
void ProcessDefines();
int  FreeDefine(char *);
