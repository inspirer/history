// hydra.cpp (CHydra)

#include "StdAfx.h"

#define HYDRA_DEBUG

static uchar hhex[]="0123456789abcdef";

inline void CHydra::hydraCrc(uchar c,ushort *crc)
{
  (*crc)=((*crc)>>8)^crc16table[(((*crc)^c)&0xFF)];
}

inline void CHydra::hydraCrcData(uchar *c,int size,ushort *crc)
{
  uchar *l=c;
  for (int i=0;i<size;i++,l++)
    (*crc)=((*crc)>>8)^crc16table[(((*crc)^(*l))&0xFF)];
}

void CHydra::hydraBuildCrc()
{
  ushort crc,i,n;

  for (i=0;i<256;i++){
    crc=i;for (n=1;n<9;n++)
      if (crc&1) crc=(crc>>1)^_crc16_poly;else crc>>=1;
    crc16table[i]=crc;
  }
}

void CHydra::hydraSendPkt(uchar *pk,int psize,uchar ptype)
{
  ushort CRC;
  uchar *l,*h;
  int i,e,pkt_size;
  uchar pfmt,c;

  if (ptype==HPKT_START||ptype==HPKT_INIT||ptype==HPKT_INITACK
      ||ptype==HPKT_END||ptype==HPKT_IDLE) pfmt=HCHR_HEXPKT;
  else {
    if (rh.TxOptions&HOPT_HIGHBIT){
      if (rh.TxOptions&HOPT_CTLCHRS && rh.TxOptions&HOPT_CANUUE) pfmt=HCHR_UUEPKT;
      else if (rh.TxOptions&HOPT_CANASC) pfmt=HCHR_ASCPKT;
      else pfmt=HCHR_HEXPKT;
    } else pfmt=HCHR_BINPKT;
  }

  #ifdef HYDRA_DEBUG
    sprintf(tinfo,"send pkt type=%c format=%c size=%i",ptype,pfmt,psize);
    g->log->logstr((CString)tinfo);
  #endif

  CRC=0xFFFF;hydraCrcData(pk,psize,&CRC);
  hydraCrc(ptype,&CRC);CRC=0xFFFF-CRC;

  memcpy(t,pk,psize);l=t+psize;
  *l++=ptype;*l++=(CRC&0xFF);*l++=(CRC>>8);
  e=psize+3;pkt_size=2;l=t;h=pkt;*h++=H_DLE;*h++=pfmt;

  switch(pfmt){
    case HCHR_BINPKT:
      for (i=0;i<e;i++){        
        c=*l;
        if (rh.TxOptions&HOPT_HIGHCTL) c&=0x7f;
        if (c==H_DLE||
            ((rh.TxOptions&HOPT_CTLCHRS)&&(c==127||c<=31)) ||
            ((rh.TxOptions&HOPT_TELENET)&&c==13&&*(l-1)=='@') ||
            ((rh.TxOptions&HOPT_XONXOFF)&&(c==XON||c==XOFF))
        ){
          *h++=H_DLE;pkt_size++;(*l)^=0x40;
        };
        *h++=*l++;pkt_size++;        
      }
      break;
    case HCHR_HEXPKT:
      for (i=0;i<e;i++,l++){
        if (*l==127||*l<=31){
          *h++=H_DLE;*h++=(*l)^0x40;pkt_size+=2;
        } else if(*l&0x80){
          *h++='\\';*h++=hhex[(*l)>>4];*h++=hhex[(*l)&0xF];pkt_size+=3;
        } else if (*l=='\\'){
          *h++='\\';*h++='\\';pkt_size+=2;
        } else *h++=*l,pkt_size++;
      }
      break;
  }

  *h++=H_DLE;*h++=HCHR_PKTEND;pkt_size+=2;

  if (ptype!=HPKT_DATA&&pfmt!=HCHR_BINPKT){
    *h++=13;*h++=10;pkt_size+=2;
  }

  g->mdm->transmit((char *)pkt,pkt_size);
  pkt[pkt_size]=0;
}

int CHydra::hydraReceivePkt()
{
  register uchar c,n;
  int i,e;
  ushort CRC;

  while(g->mdm->bhead!=g->mdm->btail){
    c=g->mdm->bm_in[g->mdm->btail];
    if (rxPktStatus){
      if (c!=H_DLE){
        n=c;if (rh.RxOptions&HOPT_HIGHCTL) n=c&0x7f;
        if ((rh.RxOptions&HOPT_XONXOFF)&&(n==XON||n==XOFF));
        else if ((rh.RxOptions&HOPT_CTLCHRS)&&(n<32||n==127));
        else if (rxPrev==H_DLE) switch(c){
          case HCHR_BINPKT: case HCHR_HEXPKT:
            rxPktLost++;rxPktStatus=c;rxPktSize=0;
            break;
          case HCHR_PKTEND:
            rxPktStatus=0;if (rxPktSize<3){ rxPktLost++;break;} rxPktSize-=2;
            CRC=0xFFFF;hydraCrcData(rx,rxPktSize,&CRC);CRC=0xFFFF-CRC;
            if (((ushort *)(rx+rxPktSize))[0]!=CRC){ rxPktLost++;break;}
            rxPktSize--;
            #ifdef HYDRA_DEBUG
              sprintf(tinfo,"recv pkt format=%c size=%i",rx[rxPktSize],rxPktSize);
              g->log->logstr((CString)tinfo);
            #endif
            return rx[rxPktSize];

          default:
            rx[rxPktSize++]=c^0x40;
            break;

        } else switch(rxPktStatus){
          case HCHR_BINPKT:
            rx[rxPktSize++]=c;
            break;
          case HCHR_HEXPKT:
            if (c=='\\'&&rxPrev=='\\'){
              rx[rxPktSize++]=c;rxOld=rxPrev=0;
            } else if (c=='\\'||rxPrev=='\\');
            else if (rxOld=='\\'){
              if (rxPrev<'0'||rxPrev>'f'||(rxPrev>'9'&&rxPrev<'a')||c<'0'
                ||c>'f'||(c>'9'&&c<'a')){ rxPktStatus=0;rxPktLost++;break;}
              e=c-48;i=rxPrev-48;
              if (e>9) e-=39;if (i>9) i-=39;
              i<<=4;i+=e;
              rx[rxPktSize++]=i;
            } else rx[rxPktSize++]=c;
            break;
        }
      }
    } else if (rxPrev==H_DLE&&(c==HCHR_BINPKT||c==HCHR_HEXPKT)){
      rxPktSize=0;rxPktStatus=c;
    }
    g->mdm->btail++;g->mdm->btail%=receive_buffer_size;rxOld=rxPrev;rxPrev=c;
  }
  return 0;
}

void CHydra::hydraRefresh()
{
}

void CHydra::hydraSendRPOS()
{
  rx_file.last_retry++;
  ((ulong *)pkt)[0]=rx_file.Offset;
  ((ulong *)pkt)[1]=1024;
  ((ulong *)pkt)[2]=rx_file.last_retry;
  hydraSendPkt(pkt,12,HPKT_RPOS);
  if (rx_file.Offset==-2){
    sprintf(tinfo,"HR: refusing `%s'",rx_file.fname);
    if (tx_file.tries) sprintf(tinfo+strlen(tinfo)," (try #%u)",rx_file.tries);
    g->f->conprint(tinfo,0,0);
  } else {      
    sprintf(tinfo,"HR: resending `%s' from %u ",rx_file.fname,rx_file.Offset);
    if (tx_file.tries) sprintf(tinfo+strlen(tinfo)," (try #%u)",rx_file.tries);
    g->f->conprint(tinfo,0,0);
  }
  rx_file.status=CHydraFile::RECEIVE_RPOS;
  rx_file.tries++;
}

void CHydra::hydraSendFINFO()
{
  int i;

  sprintf((char *)pkt,"%08x%08x%08x%08x%08x",tx_file.Timestamp,tx_file.Filesize,0,0,tx_file.Count);
  i=40;strcpy((char *)(pkt+i),tx_file.sname);i+=1+tx_file.sname.GetLength();
  strcpy((char *)(pkt+i),tx_file.fname);i+=1+tx_file.fname.GetLength();
  hydraSendPkt(pkt,i,HPKT_FINFO);
  tx_file.status=CHydraFile::WFIACK;
}

void CHydra::hydraMain()
{
  long i,e;
  ulong o;
  time_t et;

  rx_files=tx_files=rx_bytes=tx_bytes=0;
  rx_file.f=tx_file.f=NULL;
  rx_file.status=CHydraFile::NOTH;tx_file.status=CHydraFile::NOTH;
  rx_file.et=0;tx_file.et=1;
  rx_file.last_retry=tx_file.last_retry=0;
  tx_file.pktSize=2048;

  g->f->conprint("hyd: link options: " + (CString)hydraLinkOptions(rh.TxOptions),1,1);
  g->f->conprint("hyd: remote is " + (CString)rh.hydra_mailer,1,1);

  if ((rh.Supported&HNEC_OPTIONS)!=HNEC_OPTIONS){
    g->f->conprint("hyd: incompatible options",1,1);
    return;
  }

  braindead=time(NULL)+H_BRAINDEAD;
  while(hStep==2){
    if (braindead<time(NULL)){ hStep=2;break;}
    if ((g->mdm->mstat&MS_RLSD_ON)==0){ hStep=3;break;}
    if (g->f->cn->p=g->f->cn->getkey())
      if (g->f->cn->p->EventType==KEY_EVENT&&g->f->cn->p->Event.KeyEvent.bKeyDown)
        if (g->f->cn->p->Event.KeyEvent.uChar.AsciiChar=='h'&&
            g->f->cn->p->Event.KeyEvent.dwControlKeyState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
        { hStep=4;break;}

    while(i=hydraReceivePkt()) switch(i){

      // receiver (Data)     NOTH > RECEIVE ( > RECEIVE_RPOS) > NOTH
      case HPKT_INIT: hydraSendPkt(pkt,0,HPKT_INITACK);break;
      case HPKT_FINFO:
        if (rx_file.status==CHydraFile::RECEIVE||rx_file.status==CHydraFile::RECEIVE_RPOS){
          ((ulong *)pkt)[0]=rx_file.Offset;
          hydraSendPkt(pkt,4,HPKT_FINFOACK);
        } else if (rx[0]==0){
          ((ulong *)pkt)[0]=0;
          hydraSendPkt(pkt,4,HPKT_FINFOACK);
          rx_file.status=CHydraFile::DOWN;rx_file.et=0;
          braindead=time(NULL)+H_BRAINDEAD;
        } else {
          GetFromFINFO(&rx_file);
          ((ulong *)pkt)[0]=rx_file.Offset;
          hydraSendPkt(pkt,4,HPKT_FINFOACK);
          rx_file.et=time(NULL)+H_MINTIMER;
          braindead=time(NULL)+H_BRAINDEAD;
        }
        break;
      case HPKT_DATA:
        if (rx_file.status==CHydraFile::RECEIVE||(rx_file.status==CHydraFile::RECEIVE_RPOS&&rx_file.Offset>=0)){
          o=((ulong *)rx)[0];
          if (o!=rx_file.Offset||rxPktSize<4){
            if (rx_file.status==CHydraFile::RECEIVE) hydraSendRPOS();
          } else {
            braindead=time(NULL)+H_BRAINDEAD;
            rx_file.tries=0;
            if(!rx_file.FileWrite(rx+4,rxPktSize-4)){
              sprintf(tinfo,"HR: file I/O error: %s (%u)",rx_file.fname,rx_file.Offset);
              g->f->conprint(tinfo,1,1);
              rx_file.Offset=-2;
              hydraSendRPOS();
              rx_file.et=time(NULL)+H_MINTIMER;
              break;
            }
            rx_file.Offset+=rxPktSize-4;rx_bytes+=rxPktSize-4;
            rx_file.status=CHydraFile::RECEIVE;
            ((long *)pkt)[0]=rx_file.Offset;
            hydraSendPkt(pkt,4,HPKT_DATAACK);
            hydraRefresh();
          }
          rx_file.et=time(NULL)+H_MINTIMER;
        }
        break;
      case HPKT_EOF:
        braindead=time(NULL)+H_BRAINDEAD;
        if (rx_file.status==CHydraFile::RECEIVE||(rx_file.status==CHydraFile::RECEIVE_RPOS&&rx_file.Offset<0)){
          o=((ulong *)rx)[0];
          if (o==rx_file.Offset&&o>=rx_file.Filesize){
            sprintf(tinfo,"HR: file `%s` %u",rx_file.fname,rx_file.Offset);
            if (rx_file.start_from) sprintf(tinfo+strlen(tinfo)," (from %u)",rx_file.start_from);
            g->f->conprint(tinfo,1,1);
            rx_file.FileClose(1);rx_files++;
            rx_file.status=CHydraFile::NOTH;
            rx_file.et=0;
            hydraSendPkt(pkt,0,HPKT_EOFACK);
            hydraRefresh();
          } else if (rx_file.Offset==-2||i<rx_file.Filesize){
            rx_file.FileClose(0);
            rx_file.status=CHydraFile::NOTH;
            rx_file.et=0;
            hydraSendPkt(pkt,0,HPKT_EOFACK);
            hydraRefresh();
          } else {
            if (rx_file.tries==H_RETRIES){ hStep=9;break;}
            hydraSendRPOS();
            rx_file.et=time(NULL)+H_MINTIMER;
          }
        }
        break;
      case HPKT_IDLE:
        braindead=time(NULL)+H_BRAINDEAD;
        break;

      // Sender (Data) NOTH > WFIACK > SENDING
      case HPKT_FINFOACK:
        switch(tx_file.status){
          case CHydraFile::WAITFORDOWN:
            braindead=time(NULL)+H_BRAINDEAD;
            tx_file.status=CHydraFile::DOWN;
            tx_file.et=time(NULL)+H_IDLE;
            break;
          case CHydraFile::WFIACK:
            braindead=time(NULL)+H_BRAINDEAD;
            o=((ulong *)rx)[0];
            switch(o){
              case -1:
                sprintf(tinfo,"HS: file `%s' %u (skipped)",tx_file.fname,tx_file.Filesize);
                g->f->conprint(tinfo,1,1);
                tx_file.status=CHydraFile::NOTH;
                tx_file.FileClose();ls->erase(tx_files);
                tx_files++;
                break;
              case -2:
                sprintf(tinfo,"HS: file `%s' %u (refused)",tx_file.fname,tx_file.Filesize);
                g->f->conprint(tinfo,1,1);
                tx_file.status=CHydraFile::NOTH;
                tx_file.FileClose();tx_files++;
                break;
              default:
                sprintf(tinfo,"HS: sending `%s' %u",tx_file.fname,tx_file.Filesize);
                if (o) sprintf(tinfo+strlen(tinfo)," (from %u)",o);
                g->f->conprint(tinfo,0,0);
                tx_file.Offset=o;tx_file.start_from=o;
                tx_file.FileSeek(o);
                tx_file.status=CHydraFile::SENDING;
                tx_file.et=time(NULL)+H_MINTIMER;
                hydraRefresh();
                break;
            }
            break;
        }        
        break;
      case HPKT_DATAACK:
        tx_file.et=time(NULL)+H_MINTIMER;
        break;
      case HPKT_RPOS:
        if (tx_file.status==CHydraFile::SENDING){
          o=((ulong *)rx)[2];
          if (o!=tx_file.last_retry){
            tx_file.last_retry=o;
            o=((ulong *)rx)[0];
            if (o==-1){
              tx_file.Offset=o;
              ((ulong *)pkt)[0]=tx_file.Offset;
              hydraSendPkt(pkt,4,HPKT_EOF);
              sprintf(tinfo,"HS: file `%s' %u (skipped)",tx_file.fname,tx_file.Filesize);
              g->f->conprint(tinfo,1,1);
              tx_file.status=CHydraFile::WEOFACK;
              tx_file.et=time(NULL)+H_MINTIMER;
              
            } else if (o==-2){
              tx_file.Offset=o;
              ((ulong *)pkt)[0]=tx_file.Offset;
              hydraSendPkt(pkt,4,HPKT_EOF);
              sprintf(tinfo,"HS: file `%s' %u (refused)",tx_file.fname,tx_file.Filesize);
              g->f->conprint(tinfo,1,1);
              tx_file.status=CHydraFile::WEOFACK;
              tx_file.et=time(NULL)+H_MINTIMER;

            } else {
              tx_file.Offset=o;tx_file.FileSeek(o);
              sprintf(tinfo,"HS: resending `%s' from %u",tx_file.fname,o);
              g->f->conprint(tinfo,0,0);
              tx_file.et=time(NULL)+H_MINTIMER;
            }
          }
        }
        break;
      case HPKT_EOFACK:
        tx_file.FileClose();tx_files++;
        if (tx_file.Offset!=-1&&tx_file.Offset!=-2){
          sprintf(tinfo,"HS: file `%s' %u",tx_file.fname,tx_file.Filesize);
          if (tx_file.start_from) sprintf(tinfo+strlen(tinfo)," (from %u)",tx_file.start_from);
          g->f->conprint(tinfo,1,1);
        }
        tx_file.et=1;tx_file.status=CHydraFile::NOTH;
        break;
    }

    if (tx_file.status==CHydraFile::SENDING&&g->mdm->enable()){
      g->mdm->clear();
      i=tx_file.FileRead(pkt+4,tx_file.pktSize);
      ((ulong *)pkt)[0]=tx_file.Offset;
      if (i){
        tx_file.Offset+=i;
        hydraSendPkt(pkt,i+4,HPKT_DATA);
        hydraRefresh();
      } else {
        hydraSendPkt(pkt,4,HPKT_EOF);
        tx_file.status=CHydraFile::WEOFACK;
        tx_file.et=time(NULL)+H_MINTIMER;
        tx_file.tries=1;
        hydraRefresh();
      }
    }

    if (tx_file.et&&tx_file.et<time(NULL)) switch(tx_file.status){
      case CHydraFile::SENDING:
        g->f->conprint("dataack timeout",0,0);
        break;
      case CHydraFile::WEOFACK:
        if (tx_file.tries==10){ hStep=9;break;}
        ((ulong *)pkt)[0]=tx_file.Offset;
        hydraSendPkt(pkt,4,HPKT_EOF);
        tx_file.et=time(NULL)+H_MINTIMER;
        tx_file.tries++;        
        break;
      case CHydraFile::WFIACK:
        if (tx_file.tries==10){ hStep=9;break;}
        hydraSendFINFO();
        tx_file.et=time(NULL)+H_MINTIMER/2;
        tx_file.tries++;
        break;
      case CHydraFile::NOTH:
        send_next_file:
        if (tx_files<ls->size()){
          tx_file.realName=ls->get(tx_files);
          if (tx_file.OpenFileForRead()){
            if (tx_files) tx_file.Count=tx_files+1;else tx_file.Count=ls->size();
            hydraSendFINFO();
            tx_file.et=time(NULL)+H_MINTIMER;
            tx_file.tries=1;tx_file.start_from=0;
            sprintf(tinfo,"HS: sending fileinfo: %s %u",tx_file.fname,tx_file.Filesize);
            g->f->conprint(tinfo,0,0);
            hydraRefresh();
          } else {
            g->f->conprint("HS: (not found) " + tx_file.fname,1,1);
            ls->erase(tx_files);tx_files++;goto send_next_file;
          }
          break;
        }
      case CHydraFile::GODOWN:
        pkt[0]=0;hydraSendPkt(pkt,1,HPKT_FINFO);
        tx_file.status=CHydraFile::WAITFORDOWN;
        tx_file.et=time(NULL)+H_MINTIMER;
        tx_file.tries=0;
        break;
      case CHydraFile::WAITFORDOWN:
        if (tx_file.tries==10){ hStep=9;break;}
        pkt[0]=0;hydraSendPkt(pkt,1,HPKT_FINFO);
        tx_file.et=time(NULL)+H_MINTIMER/2;
        tx_file.tries++;
        break;
      case CHydraFile::DOWN:
        hydraSendPkt(pkt,1,HPKT_IDLE);
        tx_file.et=time(NULL)+H_IDLE;
        break;
    }

    if (rx_file.et&&rx_file.et<time(NULL)) switch(rx_file.status){
      case CHydraFile::RECEIVE_RPOS:
        if (rx_file.Offset==-2){
          if (rx_file.tries==H_RETRIES){ hStep=9;break;}
          hydraSendRPOS();
          rx_file.et=time(NULL)+H_MINTIMER/2;
        }
      case CHydraFile::RECEIVE:
        if (rx_file.tries==H_RETRIES){ hStep=9;break;}
        sprintf(tinfo,"HR: timeout #%i",rx_file.tries+1);
        g->f->conprint(tinfo,0,0);
        hydraSendRPOS();
        rx_file.et=time(NULL)+H_MINTIMER/2;
        break;
    }

    if (rx_file.status==CHydraFile::DOWN&&tx_file.status==CHydraFile::DOWN){ hStep=7;break;}

  }

  if (rx_file.status==CHydraFile::RECEIVE||rx_file.status==CHydraFile::RECEIVE_RPOS){
    rx_file.FileClose();
  }

  if (tx_file.status==CHydraFile::WFIACK||tx_file.status==CHydraFile::SENDING||tx_file.status==CHydraFile::WEOFACK){
    tx_file.FileClose();
  }

  if (hStep==7){
    i=0;while(hStep==7&&i<10){
      hydraSendPkt(pkt,0,HPKT_END);

      et=time(NULL)+H_START;
      do {
        if (g->f->cn->p=g->f->cn->getkey()) 
          if (g->f->cn->p->EventType==KEY_EVENT&&g->f->cn->p->Event.KeyEvent.bKeyDown)
            if (g->f->cn->p->Event.KeyEvent.uChar.AsciiChar=='h'&&
                g->f->cn->p->Event.KeyEvent.dwControlKeyState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
            { hStep=4;break;}

        if ((g->mdm->mstat&MS_RLSD_ON)==0)
          { hStep=3;break;}

        if (braindead<time(NULL)){ hStep=2;break;}

        e=hydraReceivePkt();
        if (e==HPKT_END){ hStep=8;break; }

      } while(et>=time(NULL));
      i++;
    }
  }

  switch(hStep){
    case 2: case 7: g->f->conprint("hyd: timeout",1,1);break;
    case 3: g->f->conprint("hyd: carrier lost",1,1);break;
    case 4: g->f->conprint("hyd: cancelled by operator",1,1);break;
    case 8: 
      for (i=0;i<3;i++) hydraSendPkt(pkt,0,HPKT_END);
      g->f->conprint("hyd: finished",1,1);break;
    case 9: g->f->conprint("hyd: too many errors",1,1);break;
  }
}

void CHydra::Start()
{
  int i,e,k;
  time_t et;

  hydraBuildCrc();
  rh.inited=rxPktStatus=rxPktLost=rxOld=rxPrev=0;
  hydra_start=time(NULL);
  braindead=time(NULL)+H_BRAINDEAD;
  rh.TxOptions=HNEC_OPTIONS;hStep=0;

  g->f->conprint("hyd: synchronizing protocol",0,0);

  i=0;while(hStep==0&&i<10){
    g->mdm->transmit("hydra\r",6);
    hydraSendPkt(pkt,0,HPKT_START);

    et=time(NULL)+H_START;
    do {
      if (g->f->cn->p=g->f->cn->getkey()) 
        if (g->f->cn->p->EventType==KEY_EVENT&&g->f->cn->p->Event.KeyEvent.bKeyDown)
          if (g->f->cn->p->Event.KeyEvent.uChar.AsciiChar=='h'&&
              g->f->cn->p->Event.KeyEvent.dwControlKeyState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
          { hStep=-1;break;}

      if ((g->mdm->mstat&MS_RLSD_ON)==0)
        { hStep=-2;break;}

      if (braindead<time(NULL)){ hStep=-3;break;}

      e=hydraReceivePkt();
      if (e==HPKT_START){
        hStep=1;
        break;
          
      } else if (e==HPKT_INIT){
        rh.getfromrx(rx);
        hydraSendPkt(NULL,0,HPKT_INITACK);
        hStep=1;
        break;
      }

    } while(et>=time(NULL));
    i++;
  }

  switch(hStep){
    case -2: g->f->conresult(15,"carrier lost");g->f->left_on_screen=1;return;
    case -1: g->f->conresult(15,"(Cancelled)");g->f->left_on_screen=1;return;
    case  0: case -3: g->f->conresult(15,"timeout");g->f->left_on_screen=1;return;
  }

  k=i=0;while(i<10&&hStep==1){
    e=hydraPrepeareINITpkt();
    hydraSendPkt(pkt,e,HPKT_INIT);

    et=time(NULL)+H_START;
    do {
      if (g->f->cn->p=g->f->cn->getkey())
        if (g->f->cn->p->EventType==KEY_EVENT&&g->f->cn->p->Event.KeyEvent.bKeyDown)
          if (g->f->cn->p->Event.KeyEvent.uChar.AsciiChar=='h'&&
              g->f->cn->p->Event.KeyEvent.dwControlKeyState&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
          { hStep=-1;break;}

      if ((g->mdm->mstat&MS_RLSD_ON)==0)
        { hStep=-2;break;}

      if (braindead<time(NULL)){ hStep=-3;break;}
          
      e=hydraReceivePkt();
      if (e==HPKT_INIT){
        rh.getfromrx(rx);
        hydraSendPkt(NULL,0,HPKT_INITACK);
        break;
      } else if (e==HPKT_INITACK) k=1;

      if (rh.inited&&k){ hStep=2;break;}
    } while(et>=time(NULL));
  }

  switch(hStep){
    case -2: g->f->conresult(15,"carrier lost");g->f->left_on_screen=1;return;
    case -1: g->f->conresult(15,"(Cancelled)");g->f->left_on_screen=1;return;
    case  1: case -3: g->f->conresult(15,"timeout");g->f->left_on_screen=1;return;
  }

  hydraMain();
}

static const char *HydOpt[] = 
{ "XON","TLN","CTL","HIC","HI8","BRK","ASC","UUE","C32","DEV","FPT","" };
static const int HydInt[] =
{ HOPT_XONXOFF, HOPT_TELENET, HOPT_CTLCHRS, HOPT_HIGHCTL, HOPT_HIGHBIT, 
HOPT_CANBRK, HOPT_CANASC, HOPT_CANUUE, HOPT_CRC32, HOPT_DEVICE, HOPT_FPT,0 };

int CRemoteHydra::hydraGetInitOptions(uchar *l)
{
  for (int i=0;*HydOpt[i];i++) if (!strcmp((char *)l,HydOpt[i])) return HydInt[i];
  return 0;
}

void CRemoteHydra::getfromrx(uchar *rx)
{
  uchar *l = rx, *h = hydra_mailer,c;

  Supported=Desired=0;
  c=*(l+8);*(l+8)=0;sscanf((char *)l,"%X",&_hydra_doc);*(l+8)=c;l+=8;
  while(*h++=*l++);
  c=1;h=l;while(*h&&c){
    while(*h&&*h!=',') h++;c=*h;*h=0;
    Supported|=hydraGetInitOptions(l);
    l=++h;
  }
  c=1;while(*h&&c){
    while(*h&&*h!=',') h++;c=*h;*h=0;
    Desired|=hydraGetInitOptions(l);
    l=++h;
  }
  l=h+8;c=*l;*l=0;sscanf((char *)h,"%X",&rxSize);*l=c;
  h=l+8;c=*h;*h=0;sscanf((char *)l,"%X",&txSize);*h=c;
  while(*h++);l=pktprefix;while(*l++=*h++);
  inited=1;RxOptions=TxOptions=Supported&Desired;
}

int CHydra::hydraPrepeareINITpkt()
{
  int e;

  strcpy((char *)pkt,APPL_ID);e=strlen((char *)pkt)+1;
  strcpy((char *)(pkt+e),hydraLinkOptions(SUPP_OPT));e+=strlen((char *)(pkt+e))+1;
  strcpy((char *)(pkt+e),hydraLinkOptions(DESIRED_OPT));e+=strlen((char *)(pkt+e))+1;
  strcpy((char *)(pkt+e),"0000000000000000");e+=strlen((char *)(pkt+e))+1;
  pkt[e++]=0;
  return e;
}

char * CHydra::hydraLinkOptions(int LinkOpt)
{
  tinfo[0]=tinfo[1]=0;
  if (LinkOpt&HOPT_XONXOFF) strcat(tinfo,",XON");
  if (LinkOpt&HOPT_TELENET) strcat(tinfo,",TLN");
  if (LinkOpt&HOPT_CTLCHRS) strcat(tinfo,",CTL");
  if (LinkOpt&HOPT_HIGHCTL) strcat(tinfo,",HIC");
  if (LinkOpt&HOPT_HIGHBIT) strcat(tinfo,",HI8");
  if (LinkOpt&HOPT_CANBRK) strcat(tinfo,",BRK");
  if (LinkOpt&HOPT_CANASC) strcat(tinfo,",ASC");
  if (LinkOpt&HOPT_CANUUE) strcat(tinfo,",UUE");
  if (LinkOpt&HOPT_CRC32) strcat(tinfo,",C32");
  if (LinkOpt&HOPT_DEVICE) strcat(tinfo,",DEV");
  if (LinkOpt&HOPT_FPT) strcat(tinfo,",FPT");

  return tinfo+1;
}

void CHydra::GetFromFINFO(CHydraFile *hf)
{
  char *l = (char *)rx, c;
  static CString x;

  c=*(l+8);*(l+8)=0;sscanf(l,"%X",&hf->Timestamp);*(l+8)=c;l+=8;
  c=*(l+8);*(l+8)=0;sscanf(l,"%X",&hf->Filesize);*(l+8)=c;l+=16;
  c=*(l+8);*(l+8)=0;sscanf(l,"%X",&hf->Transaction);*(l+8)=c;l+=8;
  c=*(l+8);*(l+8)=0;sscanf(l,"%X",&hf->Count);*(l+8)=c;l+=8;
  hf->sname=l;while(*l++);hf->fname=l;
  if (!hf->fname.GetLength()) hf->fname=hf->sname;hf->tries=0;
  hf->Offset=0;hf->Start=time(NULL);hf->status=CHydraFile::NOTH;
  if (hf->OpenFileForWrite()) hf->status=CHydraFile::RECEIVE;
  sprintf(tinfo,"HR: file %s %u",(LPCSTR)hf->fname,hf->Filesize);
  hf->start_from=hf->Offset;
  switch(hf->Offset){
    case -1: 
      strcat(tinfo," (skipped)");
      g->f->conprint(tinfo,1,1);
      break;
    case -2: 
      strcat(tinfo," (refused)");
      g->f->conprint(tinfo,1,1);
      break;
    default: 
      if (hf->Offset>0) sprintf(tinfo+strlen(tinfo)," (from %u)",hf->Offset);
      g->f->conprint(tinfo,0,0);
      break;
  }
  hydraRefresh();
}
