// console.cpp (CConsole)

#include "stdafx.h"

void CConsole::initconsole()
{
  conin =  GetStdHandle(STD_INPUT_HANDLE);
  conout = GetStdHandle(STD_OUTPUT_HANDLE);

  ZeroMemory(&ovc,sizeof(ovc));
  ovc.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
  if(ovc.hEvent == INVALID_HANDLE_VALUE) {
     throw errors("CreateEvent error");
  }

  SetConsoleMode(conin,ENABLE_MOUSE_INPUT);
  GetConsoleScreenBufferInfo(conout,&csb);
  if (csb.dwSize.X<80||csb.dwSize.Y<25){
     CloseHandle(ovc.hEvent);
     throw errors("console size is too small");
  }
  FlushConsoleInputBuffer(conin);
  head=tail=0;
  cci.bVisible=0;cci.dwSize=10;
  SetConsoleCursorInfo(conout,&cci);
  SetConsoleTitle(jmail);
}

void CConsole::deinitconsole()
{
  FlushConsoleInputBuffer(conin);
  CloseHandle(ovc.hEvent);
  SetConsoleMode(conin,0);
}

void CConsole::congetnext()
{
   INPUT_RECORD buf[10];
   DWORD read=0,i;

   while(1){
      i=WaitForSingleObject(conin,0);
      if (i==WAIT_TIMEOUT) break;

      if(!ReadConsoleInput(conin,buf,10,&read)){
        // ???? if (merror) merror("ReadConsole failed");
        read=0;
      }
      if(read) {
        for (i=0;i<read;i++){
           inp[head++]=buf[i];
           head%=INSIZE;
        }
        read=0;
      }
   }
}

PINPUT_RECORD CConsole::getkey()
{
  congetnext();
  if (head!=tail){
    int i = tail;
    tail++;tail%=INSIZE;
    return &inp[i];
  } else return NULL;
}
