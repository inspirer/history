// comm.cpp (CComm)

#include "stdafx.h"

unsigned int WINAPI mthread(void *);

int CComm::init(const char *l,int baud)
{
  h = CreateFile(l,GENERIC_READ|GENERIC_WRITE,0,NULL,
        OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);

  if (h == INVALID_HANDLE_VALUE) {
    h=NULL;
    throw errors("port not opened");
  }

  COMMTIMEOUTS cto = { 2, 1, 1, 0, 0 };
  DCB dcb;
  if(!SetCommTimeouts(h,&cto)){
    CloseHandle(h);
    h=NULL;
    throw errors("SetCommTimeouts error");
  }

  memset(&dcb,0,sizeof(dcb));
  dcb.DCBlength = sizeof(dcb);
  dcb.BaudRate = baud;
  dcb.fBinary = 1;
  dcb.fDtrControl = DTR_CONTROL_ENABLE;

  dcb.fOutxCtsFlow = 1;
  dcb.fRtsControl = DTR_CONTROL_HANDSHAKE;

  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.ByteSize = 8;

  if(!SetCommState(h,&dcb)){
    CloseHandle(h);
    h=NULL;
    throw errors("SetCommState error");
  }

  ZeroMemory(&ovr,sizeof(ovr));
  ovr.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
  if(ovr.hEvent == INVALID_HANDLE_VALUE){
    CloseHandle(h);
    h=NULL;
    throw errors("CreateEvent error");
  }

  ZeroMemory(&ovw,sizeof(ovw));
  ovw.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
  if(ovw.hEvent == INVALID_HANDLE_VALUE){
    CloseHandle(ovr.hEvent);CloseHandle(h);h=NULL;
    throw errors("CreateEvent error");
  }

  if(!SetCommMask(h,EV_RXCHAR|EV_TXEMPTY|EV_RLSD|EV_DSR|EV_CTS)){
    CloseHandle(h);CloseHandle(ovr.hEvent);h=NULL;
    throw errors("SetCommMask error");
  }

  txenable = CreateEvent(NULL,FALSE,TRUE,NULL);
  if (txenable == NULL){
    CloseHandle(h);CloseHandle(ovr.hEvent);h=NULL;
    throw errors("CreateEvent error");
  }

  SetEvent(txenable);

  unsigned int tid;
  threadalive = (HANDLE)_beginthreadex(NULL,0,mthread,this,0,&tid);
  if( threadalive == NULL ) {
    CloseHandle(h);CloseHandle(ovr.hEvent);h=NULL;
    throw errors("_beginthread error");
  }

  return 0;
}

void CComm::deinit()
{
  if (h){
    SetCommMask(h,0);
    WaitForSingleObject(threadalive,INFINITE);
    CloseHandle(threadalive);
  }
}

int CComm::transmit(const char *buf, DWORD size)
{
  const char *l = buf;
  DWORD sz = size,i;

  while ( sz ) {
    WaitForSingleObject(txenable,INFINITE);

    i = (sz>transmit_buffer_size) ? transmit_buffer_size : sz;

    if( !WriteFile(h,l,i,&i,&ovw) ) {
       if(GetLastError() == ERROR_IO_PENDING) {
          if( !GetOverlappedResult(h,&ovw,&i,TRUE) ) {
             throw errors("GetOverlappedResult failed");
             return 0;
          }
       }
    } else {
       throw errors("WriteFile failed");
       return 0;
    }
    sz-=i;l+=i;
  }
  return 1;
}

unsigned int WINAPI mthread(void *t)
{
  DWORD mask = 0;
  register CComm *cm = (CComm *)t;

  GetCommModemStatus(cm->h,&cm->mstat);
  cm->status=1;
  while(1){
     if(!WaitCommEvent(cm->h,&mask,&cm->ovr) ) {
         DWORD e=GetLastError();
         if( e == ERROR_IO_PENDING ) {
            DWORD r;
            if( !GetOverlappedResult(cm->h,&cm->ovr,&r,TRUE) ) {
               throw errors("GetOverlappedResult failed");
               break;
            }
         } else {
            throw errors("WaitCommEvent failed");
            break;
         }
     }

     if( mask == 0 ) {
         break;
     }

     if( mask & EV_RXCHAR) {
         char buf[10];
         DWORD read;
         do {
            read = 0;
            if( !ReadFile(cm->h,buf,sizeof(buf),&read,&cm->ovr) ) {
               if( GetLastError() == ERROR_IO_PENDING ){
                  if( !GetOverlappedResult(cm->h,&cm->ovr,&read,TRUE)){
                     throw errors("GetOverlappedResult failed");
                     break;
                  }
               } else {
                  throw errors("ReadFile failed");
                  break;
               }
            }
            if(read){
              cm->receive(buf,read);
            }
         } while(read);
     }
     if( mask & EV_TXEMPTY) {
       SetEvent(cm->txenable);
     }
     if ( mask & (EV_RLSD|EV_DSR|EV_CTS)) {
       GetCommModemStatus(cm->h,&cm->mstat);
       cm->status=1;
     }
     mask = 0;
  }
  CloseHandle(cm->ovr.hEvent);
  CloseHandle(cm->h);cm->h=NULL;
  return 0;
}

void CComm::clrdtr()
{
  EscapeCommFunction(h,CLRDTR);
}

void CComm::setdtr()
{
  EscapeCommFunction(h,SETDTR);
}

void CComm::receive(char *buf,int read)
{
  if (read+bhead<=receive_buffer_size){
    memcpy(bm_in+bhead,buf,read);
    bhead=(read+bhead)%receive_buffer_size;
  } else {
    int left=read+bhead-receive_buffer_size;
    memcpy(bm_in+bhead,buf,read-left);
    memcpy(bm_in,buf+read-left,left);
    bhead=left%receive_buffer_size;
  }
}
