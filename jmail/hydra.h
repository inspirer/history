// hydra.h

#define _crc16_poly 0x8408
#define _crc16_init 0xFFFF
#define _crc16_test 0xF0B8

#define max_pkt_size 32768
#define rx_buff_size 32768
        
// revision : 2b1aab00 (1,01 dec 1992)
#define APPL_ID       "2b1aab00jMail,1-4.nt"

//What we can do
#define SUPP_OPT      (HOPT_XONXOFF|HOPT_TELENET|HOPT_CTLCHRS|HOPT_HIGHCTL|HOPT_HIGHBIT|HOPT_CANBRK)
#define DESIRED_OPT   (HOPT_XONXOFF|HOPT_TELENET)
#define HCAN_OPTIONS  (HOPT_XONXOFF|HOPT_TELENET|HOPT_CTLCHRS|HOPT_HIGHCTL|HOPT_HIGHBIT|HOPT_CANBRK)

//Vital options if we ask for any; abort if other side doesn't support them
#define HNEC_OPTIONS  (HOPT_XONXOFF|HOPT_TELENET|HOPT_CTLCHRS|HOPT_HIGHCTL|HOPT_HIGHBIT|HOPT_CANBRK)

// Ctrl-Q (^Q) Xmit-On character & Ctrl-S (^S) Xmit-Off character
#define XON           17
#define XOFF          19

// Ctrl-X (^X) HYDRA DataLinkEscape
#define H_DLE         24
#define H_BRAINDEAD   120      // Braindead in 2 mins (120 secs)
#define H_MINBLKLEN   64       // Min. length of a HYDRA data block
#define H_MAXBLKLEN   2048     // Max. length of a HYDRA data block

#define H_OVERHEAD    8        // Max. no. control bytes in a pkt
#define H_MAXPKTLEN   ((H_MAXBLKLEN + H_OVERHEAD + 5) * 3)  // Encoded pkt
#define H_BUFLEN      (H_MAXPKTLEN + 16);   // Buffer sizes: max.enc.pkt + slack
#define H_PKTPREFIX   31       // Max length of pkt prefix string
#define H_FLAGLEN     3        // Length of a flag field
#define H_RETRIES     10       // No. retries in case of an error
#define H_MINTIMER    10       // Minimum timeout period
#define H_MAXTIMER    60       // Maximum timeout period
#define H_START       5        // Timeout for re-sending startstuff
#define H_IDLE        20       // Idle? TX IDLE pkt every 20 secs

//HYDRA Packet Types
#define HPKT_START    'A'      // Startup sequence
#define HPKT_INIT     'B'      // Session initialisation
#define HPKT_INITACK  'C'      // Response to INIT pkt
#define HPKT_FINFO    'D'      // File info (name, size, time)
#define HPKT_FINFOACK 'E'      // Response to FINFO pkt
#define HPKT_DATA     'F'      // File data packet
#define HPKT_DATAACK  'G'      // File data position ACK packet
#define HPKT_RPOS     'H'      // Transmitter reposition packet
#define HPKT_EOF      'I'      // End of file packet
#define HPKT_EOFACK   'J'      // Response to EOF packet
#define HPKT_END      'K'      // End of session
#define HPKT_IDLE     'L'      // Idle - just saying I'm alive
#define HPKT_DEVDATA  'M'      // Data to specified device
#define HPKT_DEVDACK  'N'      // Response to DEVDATA pkt

//HYDRA Packet Format: START[<data>]<type><crc>END
#define HCHR_PKTEND   'a'      // End of packet (any format)
#define HCHR_BINPKT   'b'      // Start of binary packet
#define HCHR_HEXPKT   'c'      // Start of hex encoded packet
#define HCHR_ASCPKT   'd'      // Start of shifted 7bit encoded pkt
#define HCHR_UUEPKT   'e'      // Start of uuencoded packet

//HYDRA Local Storage of INIT Options (Bitmapped)
#define HOPT_XONXOFF  0x0001   // Escape XON/XOFF
#define HOPT_TELENET  0x0002   // Escape CR-'@'-CR (Telenet escape)
#define HOPT_CTLCHRS  0x0004   // Escape ASCII 0-31 and 127
#define HOPT_HIGHCTL  0x0008   // Escape above 3 with 8th bit too
#define HOPT_HIGHBIT  0x0010   // Escape ASCII 128-255 + strip high
#define HOPT_CANBRK   0x0020   // Can transmit a break signal
#define HOPT_CANASC   0x0040   // Can transmit/handle ASC packets
#define HOPT_CANUUE   0x0080   // Can transmit/handle UUE packets
#define HOPT_CRC32    0x0100   // Packets with CRC-32 allowed
#define HOPT_DEVICE   0x0200   // DEVICE packets allowed
#define HOPT_FPT      0x0400   // Can handle filenames with paths

class CRemoteHydra {
public:
  uchar hydra_mailer[80],pktprefix[30];
  long TxOptions,RxOptions,rxSize,txSize,Supported,Desired;
  time_t _hydra_doc;
  int inited;

  void getfromrx(uchar *rx);
  int hydraGetInitOptions(uchar *l);
};

class CHydraFile : public CJFile {
public:
  enum { NOTH, WFIACK, SENDING, WEOFACK,
    GODOWN, WAITFORDOWN, DOWN, RECEIVE, RECEIVE_RPOS };
  ulong Count,Transaction,last_retry,tries,start_from,pktSize;
  time_t et;
};

class CHydra {
private:
  ushort crc16table[256];
  uchar pkt[max_pkt_size],rx[rx_buff_size],t[max_pkt_size];
  int hStep,rxPktStatus,rxPktSize,rxPktLost,rxPrev,rxOld;
  long rx_files,tx_files,rx_bytes,tx_bytes;
  char tinfo[256];
  CRemoteHydra rh;
  CHydraFile rx_file,tx_file;
  time_t braindead,hydra_start;

  void hydraBuildCrc();
  inline void hydraCrc(uchar c,ushort *crc);
  inline void hydraCrcData(uchar *c,int size,ushort *crc);
  void hydraSendPkt(uchar *pk,int psize,uchar ptype);
  int hydraReceivePkt();
  void hydraMessage(const char *w);
  void hydraRefresh();
  void hydraSendRPOS();
  void hydraMain();
  int hydraPrepeareINITpkt();
  void hydraSendFINFO();
  char *hydraLinkOptions(int LinkOpt);
  void GetFromFINFO(CHydraFile *hf);

public:
  CFileList *ls;
  void Start();
};
