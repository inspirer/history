// wcl386 ssmaker.c /l=DOS4G
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void main(int argc,char * argv[])
{
  FILE *ss,*inp,*list;
  char list_file[128]="sstruct.lst";
  char s[128],*l,b[200];
  unsigned long files=0;
  unsigned long size,a;

  if (argc>1) strcpy(list_file,argv[1]);
  if ((list=fopen(list_file,"rt"))==NULL){ fprintf(stderr,"%s: file not found",list_file);exit(0);}
  if ((ss=fopen("sstruct.bin","wb"))==NULL){ fprintf(stderr,"sstruct.bin: file not created");exit(0);}
  fwrite("SS#>    ",1,8,ss);

  while(fgets(s,128,list)!=NULL){
     l=s;while(*l!=0&&*l!=13&&*l!=10) l++;
     *l=0;
     if ((inp=fopen(s,"rb"))==NULL){ fprintf(stderr,"%s: file not found\n",s);exit(0);}
     files++;
     fseek(inp,0,SEEK_END);
     size=ftell(inp);
     fseek(inp,0,SEEK_SET);
     fwrite(&size,4,1,ss);
     printf("%s: (%i byte(s)) added to sstruct.bin\n",s,size);
     while((a=fread(b,1,200,inp))!=0) fwrite(b,1,a,ss);
     fclose(inp);
  }
  printf("OK\n");
  fseek(ss,4,SEEK_SET);
  fwrite(&files,1,4,ss);
  fclose(ss);fclose(list);
}
