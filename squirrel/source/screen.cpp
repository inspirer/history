unsigned char Scr[4096],Prg[4096],x,y,inited=0;

void StartupScr()
{
  int q;
  movedata(0xB800, 0, _DS, (unsigned)Scr, 4096);
  x=wherex();y=wherey();
  clrscr();
  cout << "       Squirrel v1.16/release (c)2000 Eugeniy Gryaznov (2:5030/611.23,566.8)" << endl;
  for (q=0;q<80;q++) cout << "Ä";
  gotoxy(1,24);
  for (q=0;q<80;q++) cout << "Ä";
  gotoxy(1,3);inited=1;
}

void FixLine()
{
  if (wherey()>22){
    movedata(0xB800, 480, 0xB800, 320, 3200);
    gotoxy(1,wherey()-1);
  }
}

void ExitScr()
{
  if (inited){
    movedata(_DS, (unsigned)Scr, 0xB800, 0, 4096);
    gotoxy(x,y);inited=0;
  }
}

void writeinfoxy(int x,char *st)
{
  int _x,_y;
  _x=wherex();_y=wherey();gotoxy(x,25);
  cout << st;gotoxy(_x,_y);
}
