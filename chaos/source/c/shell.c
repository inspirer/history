#include <conio.h>
#include <string.h>
#include <process.h>
#include <stdio.h>
#include <dir.h>

srec sr;

void showdir(char *path)
{
  char s[262];
  int res,size=0,files=0;

  strcpy(s,path);strcat(s,"*.*");
  printf(" current directory: %s\n\n",s);
  res=findfirst(s,&sr,_FF_NORMAL);
  while(res==0){
    printf(" %-15s   %-9i\n",sr.ff_name,sr.ff_fsize);
    size+=sr.ff_fsize;files++;
    res=findnext(&sr);
  }
  printf("\n %i byte(s) in %i files\n",size,files);
}

void main()
{
  char s[200],cdir[200]="/",tmp[200];
  int a,b,ID=alloc_console();
  FILE *fp;

  printf("ChaOS shell ID=%x\n",ID);

  do {
    printf(">");
    gets(s);
    printf("\n");

    if (!strcmp(s,"exit")) break;

    if (s[0]=='t'&&s[1]=='y'&&s[2]=='p'&&s[3]=='e'&&s[4]==' '){
        fp=fopen(s+5,"rb");
        while((a=fread(tmp,1,199,fp))) for (b=0;b<a;b++) putch(tmp[b]);
        fclose(fp);
        continue;
    }

    if (!strcmp(s,"dir")){
        showdir(cdir);continue;
    }

    if (!strcmp(s,"cls")){
       clrscr();
       continue;
    }

    if (s[0]=='c'&&s[1]=='d'&&s[2]==' '){
      if (s[3]=='.'&&s[4]=='.'&&s[5]==0){
         if (strlen(cdir)<=1) continue;
         b=strlen(cdir);b--;
         cdir[b]=0;
         for(b--;b>=0;b--) if (cdir[b]=='/'){ cdir[b+1]=0;break;}
         continue;
      }

      if (s[3]=='/')
         strcpy(tmp,s+3);
      else
         { strcpy(tmp,cdir);strcat(tmp,s+3);}
      if (tmp[strlen(tmp)-1]!='/') strcat(tmp,"/");
      if (strlen(tmp)>1){
        tmp[strlen(tmp)-1]=0;
        if (findfirst(tmp,&sr,_FF_NORMAL)==0){
           strcpy(cdir,tmp);strcat(cdir,"/");
        } else printf("not found: %s\n",tmp);
      } else strcpy(cdir,tmp);
      continue;
    }

    if (!strcmp(s,"mem")){
      printf("free = %i\n",get_free_mem()<<12);
      continue;
    }

    // Execute
    if (s[0]==0) continue;
    if (s[0]!='/'){ strcpy(tmp,cdir);strcat(tmp,s);strcpy(s,tmp); }
    if ((a=system(s))!=0)
       printf("not found: %s (%x)\n",s,a);

  } while(1);

  free_console(ID);
}
