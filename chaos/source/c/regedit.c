#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <regs.h>

void main()
{
  int ID=alloc_console();
  char s[200];
  int key;

  printf("regedit console\n");

  do {
    printf("#");
    gets(s);
    printf("\n");

    if (!strcmp(s,"exit")) break;

    if (s[0]=='/'&&s[1]==0) key=0;
           else key=reg_get(s);
    if(key!=REG_ERR) key=reg_get_sub(key);
                else printf("not found: %s\n",s);

    while (key!=REG_ERR){

       reg_get_name(key,s);
       printf(" %-08x  %s\n",key,s);
       key=reg_get_next(key);

    }

  } while(1);

  free_console(ID);
}
