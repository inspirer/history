#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void main()
{
  int i,ID=alloc_console();

  printf("Demo application\n");

  while(kbhit()) getch();

  getch();
  free_console(ID);
}
