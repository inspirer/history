// file.c

#include "cc.h"

struct TFileStream *tfs,*tt;
int FileOpened=0;
char ch;

 // ReadFile
int ReadFile(){
  if (FileOpened>0){

    // If needed => read next block
    if (tfs->e==tfs->i){
     tfs->i=fread(tfs->swp,1,file_buffer_size,tfs->fp);
     tfs->e=0;if (tfs->i==0) return 0;
    }

    // Save character from block
    tfs->e++;tfs->ps++;
    ch=tfs->swp[tfs->e-1];
    if (tfs->sz==0) return 0;
    tfs->sz--;
    if (ch==9) ch=' ';

    return 1;
  } else return 0;
}

 // Close file : 0 - file present ; 1 - absent
int CloseFile(){
  if (FileOpened>0){

    fclose(tfs->fp);
    tt=tfs->prev;
    free(tfs);
    FileOpened--;
    if (FileOpened>0){ tfs=tt;return 1;} else { return 0;}
  } else return 0;
}

 // Open file for reading : 0 - OK ; 1 - mem ; 2 - file
int OpenFile(char *fname){

  // Create new entry
  if (FileOpened==0){
     tfs=(void *)malloc(sizeof(*tfs));
     if (tfs==NULL) return 1;
  } else {
     tfs->next=(void *)malloc(sizeof(*tfs));
     if (tfs->next==NULL) return 1;
     tt=tfs;tfs=tfs->next;tfs->prev=tt;
  }
  FileOpened++;strcpy(tfs->FileName,fname);tfs->lines=1;

  // Open File
  tfs->fp=fopen(tfs->FileName,"rb");
  if (tfs->fp==NULL){
     tt=tfs->prev;free(tfs);FileOpened--;
     if (FileOpened>0) tfs=tt;
     return 2;
  }

  // Prepearing entries
  tfs->i=0;tfs->e=0;tfs->ps=0;
  fseek(tfs->fp,0,SEEK_END);tfs->sz=ftell(tfs->fp);
  fseek(tfs->fp,0,SEEK_SET);
  return 0;
}
