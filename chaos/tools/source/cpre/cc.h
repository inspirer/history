// cc.h

#include <stdio.h>

#define file_buffer_size 2048
#define file_name_size   261
#define max_line_size    4096
#define max_ident_length 40
#define max_deep         256
#define label_buffer_sz  25000
#define define_buff_sz   250000
#define _signed          1

// cc.c
extern long lines,included;
extern char out[file_name_size],inf[file_name_size];
extern char line[max_line_size],lntp[max_line_size];
extern char parent_dir[file_name_size];

extern char *_line,*_lntp;
extern int st;

// file.c
typedef struct TFileStream
{
 struct TFileStream *next,*prev;
 char FileName[file_name_size],swp[file_buffer_size];
 unsigned long sz,ps;
 long lines;
 int i,e;FILE *fp;
};

extern struct TFileStream *tfs;
extern char ch;

extern int ReadFile();          // 1 - char in `ch'
extern int CloseFile();
extern int OpenFile(char *);    // 0 - OK ; 1 - MEM_ER ; 2 - FILE_ER

// error.c
extern void error(char *,int);
extern void ShowTotalInfo();

extern long warnings,errors;
extern long max_warn,max_err;

// directiv.c
extern int ifdef_lev,ifdef_loc;
extern int directive();

// defs.c
extern void process();

// ident.c
extern void BuildImage(int);
extern void InitIdents();
extern void IncDeep();
extern void DecDeep();
extern void CheckIdent(char *);
extern void AddDefine(char *,char *,char);
extern int  CheckDefine(char *);
extern void ProcessDefines();
extern int  FreeDefine(char *);
extern long deep;
