#include <conio.h>

void main()
{
  int i,ID=alloc_console();

  while(!kbhit()) getch();
  
  getch();
  free_console(ID);
}
