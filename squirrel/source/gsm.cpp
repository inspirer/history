
// WAV structure
struct THeader{
  unsigned long RIFF,fsize,WAVE,fmt,hex10;
  unsigned int PCM,channel;
  unsigned long waveHz,PlaySp;
  unsigned int _bytes,sndBIT;
  unsigned long buff_data,fdatasize;
} head;


// GSM structure
struct gsm_state {

  short dp0[ 280 ];
  short z1;                    // preprocessing.c, Offset_com.
  long  L_z2;                  //                  Offset_com.
  int   mp;                    //                  Preemphasis
  short u[8];                  // short_term_aly_filter.c
  short LARpp[2][8];           //
  short j;                     //
  short ltp_cut;               // long_term.c, LTP crosscorr.
  short nrp;                   // long_term.c, synthesis    40
  short v[9];                  // short_term.c, synthesis
  short msr;                   // decoder.c,   Postprocessing
  char  verbose;               // only used if !NDEBUG
  char  fast;                  // only used if FAST
  char  wav_fmt;               // only used if WAV49 defined
  unsigned char frame_index;   //            odd/even chaining
  unsigned char frame_chain;   //   half-byte to carry forward
};

// Tables
short gsm_FAC[8] = { 18431, 20479, 22527, 24575, 26623, 28671, 30719, 32767 };
short gsm_QLB[4] = { 3277, 11469, 21299, 32767 };

// Defines
#define MIN_WORD        (-32767 - 1)
#define MAX_WORD          32767

#define SASR(x, by) ((x)>>(by))
#define saturate(x) ((x) < MIN_WORD ? MIN_WORD : (x) > MAX_WORD ? MAX_WORD: (x))
#define GSM_ADD(a,b) ((ltmp=(long)(a) + (long)(b)) >= MAX_WORD ? MAX_WORD : ltmp <= MIN_WORD ? MIN_WORD : ltmp)
#define GSM_SUB(a,b) ((ltmp=(long)(a) - (long)(b)) >= MAX_WORD ? MAX_WORD : ltmp <= MIN_WORD ? MIN_WORD : ltmp)
#define GSM_MULT_R(a, b) (SASR( ((long)(a) * (long)(b) + 16384), 15 ))

// Functions
short gsm_add(short a,short b){long sum =(long)a+(long)b;return saturate(sum);}
short gsm_sub(short a,short b){long diff=(long)a-(long)b;return saturate(diff);}
short gsm_asr(short a,int n){ if (n>=16) return -(a < 0);
  if (n<=-16) return 0;if (n<0) return a << -n;return a >> n;}
short gsm_asl(short a,int n){ if (n >= 16) return 0;
  if (n <= -16) return -(a < 0);if (n < 0) return gsm_asr(a, -n);return a << n;}

// Postprocessor
static void Postprocessing(struct gsm_state *S,register short *s)
{
  register int k;
  register short msr = S->msr;
  register long ltmp;   // for GSM_ADD
  register short tmp;

  for (k = 160; k--; s++) {
     tmp = GSM_MULT_R( msr, 28180 );
     msr = GSM_ADD(*s, tmp);            // Deemphasis
     *s  = GSM_ADD(msr, msr) & 0xFFF8;  // Truncation & Upscaling
  }
  S->msr = msr;
}

// Gsm Long Term Synthesis Filtering
void Gsm_Long_Term_Synthesis_Filtering(struct gsm_state *S,short Ncr,
                             short bcr,register short *erp,register short *drp)
{
  register long ltmp;
  register int k;
  short brp, drpp, Nr;

  Nr = Ncr < 40 || Ncr > 120 ? S->nrp : Ncr;
  S->nrp = Nr;

  brp = gsm_QLB[ bcr ];

  for (k=0; k<=39; k++) {
          drpp   = GSM_MULT_R( brp, drp[ k - Nr ] );
          drp[k] = GSM_ADD( erp[k], drpp );
  }

  for (k=0; k<=119; k++) drp[ -120 + k ] = drp[ -80 + k ];
}

//
static void Decoding_of_the_coded_Log_Area_Ratios(short *LARc,short *LARpp)
{
  register short temp1;
  register long  ltmp;

#define STEP( B, MIC, INVA )    \
                temp1    = GSM_ADD( *LARc++, MIC ) << 10;       \
                temp1    = GSM_SUB( temp1, B << 1 );            \
                temp1    = GSM_MULT_R( INVA, temp1 );           \
                *LARpp++ = GSM_ADD( temp1, temp1 );

        STEP(      0,  -32,  13107 );
        STEP(      0,  -32,  13107 );
        STEP(   2048,  -16,  13107 );
        STEP(  -2560,  -16,  13107 );

        STEP(     94,   -8,  19223 );
        STEP(  -1792,   -8,  17476 );
        STEP(   -341,   -4,  31454 );
        STEP(  -1144,   -4,  29708 );
}

// Coefficients 0-12
static void Coefficients_0_12(register short *LARpp_j_1,
         register short *LARpp_j,register short *LARp)
{
  register int i;
  register long ltmp;

  for (i=1; i<=8; i++, LARp++, LARpp_j_1++, LARpp_j++) {
    *LARp = GSM_ADD( SASR( *LARpp_j_1, 2 ), SASR( *LARpp_j, 2 ));
    *LARp = GSM_ADD( *LARp,  SASR( *LARpp_j_1, 1));
  }
}

// Coefficients 13-26
static void Coefficients_13_26(register short *LARpp_j_1,
         register short *LARpp_j,register short *LARp)
{
  register int i;
  register long ltmp;
  for (i=1; i<=8; i++, LARpp_j_1++, LARpp_j++, LARp++) {
    *LARp = GSM_ADD( SASR( *LARpp_j_1, 1), SASR( *LARpp_j, 1 ));
  }
}

// Coefficients 27-39
static void Coefficients_27_39(register short *LARpp_j_1,
         register short *LARpp_j,register short *LARp)
{
  register int i;
  register long ltmp;

  for (i=1; i<=8; i++, LARpp_j_1++, LARpp_j++, LARp++) {
    *LARp = GSM_ADD( SASR( *LARpp_j_1, 2 ), SASR( *LARpp_j, 2 ));
    *LARp = GSM_ADD( *LARp, SASR( *LARpp_j, 1 ));
  }
}

// Coefficients 40-159
static void Coefficients_40_159(register short *LARpp_j,register short * LARp)
{
  register int i;
  for (i=1; i<=8; i++, LARp++, LARpp_j++) *LARp=*LARpp_j;
}

// LARp to rp
static void LARp_to_rp(register short *LARp)
{
  register int i;
  register short temp;
  register long ltmp;

  for (i = 1; i <= 8; i++, LARp++) {

    if (*LARp < 0) {
        temp = *LARp == MIN_WORD ? MAX_WORD : -(*LARp);
        *LARp = - ((temp < 11059) ? temp << 1
          : ((temp < 20070) ? temp + 11059
          :  GSM_ADD( temp >> 2, 26112 )));
    } else {
        temp  = *LARp;
        *LARp =    (temp < 11059) ? temp << 1
          : ((temp < 20070) ? temp + 11059
          :  GSM_ADD( temp >> 2, 26112 ));
    }
  }
}

static void Short_term_analysis_filtering(struct gsm_state *S,
            register short *rp,register int k_n,register short *s)
{
  register short *u = S->u;
  register int i;
  register short di, zzz, ui, sav, rpi;
  register long ltmp;

  for (; k_n--; s++) {

          di = sav = *s;

          for (i = 0; i < 8; i++) {

                  ui    = u[i];
                  rpi   = rp[i];
                  u[i]  = sav;

                  zzz   = GSM_MULT_R(rpi, di);
                  sav   = GSM_ADD(   ui,  zzz);

                  zzz   = GSM_MULT_R(rpi, ui);
                  di    = GSM_ADD(   di,  zzz );
          }

          *s = di;
  }
}

static void Short_term_synthesis_filtering(struct gsm_state *S,
  register short *rrp,register int k,register short *wt,register short *sr)
{
  register short *v = S->v;
  register int i;
  register short sri,tmp1,tmp2;
  register long ltmp;

  while (k--) {
          sri = *wt++;
          for (i = 8; i--;) {

                  tmp1 = rrp[i];
                  tmp2 = v[i];
                  tmp2 =  ( tmp1 == MIN_WORD && tmp2 == MIN_WORD
                          ? MAX_WORD
                          : 0x0FFFF & (( (long)tmp1 * (long)tmp2
                                       + 16384) >> 15)) ;

                  sri  = GSM_SUB( sri, tmp2 );

                  tmp1  = ( tmp1 == MIN_WORD && sri == MIN_WORD
                          ? MAX_WORD
                          : 0x0FFFF & (( (long)tmp1 * (long)sri
                                       + 16384) >> 15)) ;

                  v[i+1] = GSM_ADD( v[i], tmp1);
          }
          *sr++ = v[0] = sri;
  }
}

// Gsm Short Term Synthesis Filter
void Gsm_Short_Term_Synthesis_Filter(struct gsm_state * S,short *LARcr,
                                                      short *wt,short *s)
{
  short *LARpp_j= S->LARpp[ S->j ];
  short *LARpp_j_1= S->LARpp[ S->j ^=1 ];
  short LARp[8];

#define  FILTER  Short_term_synthesis_filtering

  Decoding_of_the_coded_Log_Area_Ratios( LARcr, LARpp_j );

  Coefficients_0_12( LARpp_j_1, LARpp_j, LARp );
  LARp_to_rp( LARp );
  FILTER( S, LARp, 13, wt, s );

  Coefficients_13_26( LARpp_j_1, LARpp_j, LARp);
  LARp_to_rp( LARp );
  FILTER( S, LARp, 14, wt + 13, s + 13 );

  Coefficients_27_39( LARpp_j_1, LARpp_j, LARp);
  LARp_to_rp( LARp );
  FILTER( S, LARp, 13, wt + 27, s + 27 );

  Coefficients_40_159( LARpp_j, LARp );
  LARp_to_rp( LARp );
  FILTER(S, LARp, 120, wt + 40, s + 40);
}


// RPE grid positioning
static void RPE_grid_positioning(
  short Mc,            // grid position:IN
  register short *xMp, // [0..12]      :IN
  register short *ep)  // [0..39]      :OUT
{
  int i=13;

  switch (Mc) {
     case 3: *ep++ = 0;
     case 2:  do {
                     *ep++ = 0;
     case 1:         *ep++ = 0;
     case 0:         *ep++ = *xMp++;
              } while (--i);
  }
  while (++Mc < 4) *ep++ = 0;
}

// APCM inverse quantization
static void APCM_inverse_quantization(register short *xMc,// [0..12] : IN
   short mant,short exp, register short *xMp)  // [0..12] : OUT
{
  int i;
  short temp, temp1, temp2, temp3;
  long ltmp;

  temp1 = gsm_FAC[mant];
  temp2 = gsm_sub(6,exp);
  temp3 = gsm_asl(1,gsm_sub(temp2,1));

  for (i=13;i--;) {
          temp = (*xMc++ << 1) - 7;               // restore sign
          temp <<= 12;                            // 16 bit signed
          temp = GSM_MULT_R( temp1, temp );
          temp = GSM_ADD( temp, temp3 );
          *xMp++ = gsm_asr( temp, temp2 );
  }
}

// APCM quantization
static void APCM_quantization_xmaxc_to_exp_mant(
  short xmaxc,                    // IN
  short *exp_out,short *mant_out) // OUT
{
  short exp,mant;
  exp=0;if (xmaxc>15) exp=SASR(xmaxc,3)-1;
  mant=xmaxc-(exp<<3);

  if (mant==0){ exp=-4;mant=7;} else
  {
    while(mant<=7){mant=mant<<1|1;exp--;}
    mant-=8;
  }
  *exp_out=exp;
  *mant_out=mant;
}


// Gsm_RPE_Decoding
void Gsm_RPE_Decoding(
  struct gsm_state *S,
  short xmaxcr,short Mcr,
  short *xMcr,  // [0..12], 3bits : IN
  short *erp)   // [0..39] : OUT
{
  short exp, mant;
  short xMp[13];

  APCM_quantization_xmaxc_to_exp_mant( xmaxcr, &exp, &mant );
  APCM_inverse_quantization( xMcr, mant, exp, xMp );
  RPE_grid_positioning( Mcr, xMp, erp );
}

// GSM decoder
void Gsm_Decoder(struct gsm_state *S,
 short *LARcr,                                   // [0..7]:IN
 short *Ncr,short *bcr,short *Mcr,short *xmaxcr, // [0..3]:IN
 short *xMcr,                                    // [0..13*4]:IN
 short *s)                                       // [0..159]:OUT
{
  int  j,k;
  short erp[40], wt[160];
  short *drp = S->dp0 + 120;

  for (j=0; j<=3; j++, xmaxcr++, bcr++, Ncr++, Mcr++, xMcr+=13){

          Gsm_RPE_Decoding(S, *xmaxcr, *Mcr, xMcr, erp );
          Gsm_Long_Term_Synthesis_Filtering(S, *Ncr, *bcr, erp, drp );

          for (k=0; k<=39; k++) wt[j*40 + k] = drp[k];
  }

  Gsm_Short_Term_Synthesis_Filter(S, LARcr, wt, s );
  Postprocessing(S, s);
}

#define GSM_MAGIC 0xD

int gsm_decode(struct gsm_state *gsm,unsigned char *c, short *target)
{

  short LARc[8], Nc[4], Mc[4], bc[4], xmaxc[4], xmc[13*4];


  if (((*c >> 4) & 0x0F) != GSM_MAGIC) return -1;

  LARc[0]  = (*c++ & 0xF) << 2;           // 1
  LARc[0] |= (*c >> 6) & 0x3;
  LARc[1]  = *c++ & 0x3F;
  LARc[2]  = (*c >> 3) & 0x1F;
  LARc[3]  = (*c++ & 0x7) << 2;
  LARc[3] |= (*c >> 6) & 0x3;
  LARc[4]  = (*c >> 2) & 0xF;
  LARc[5]  = (*c++ & 0x3) << 2;
  LARc[5] |= (*c >> 6) & 0x3;
  LARc[6]  = (*c >> 3) & 0x7;
  LARc[7]  = *c++ & 0x7;
  Nc[0]  = (*c >> 1) & 0x7F;
  bc[0]  = (*c++ & 0x1) << 1;
  bc[0] |= (*c >> 7) & 0x1;
  Mc[0]  = (*c >> 5) & 0x3;
  xmaxc[0]  = (*c++ & 0x1F) << 1;
  xmaxc[0] |= (*c >> 7) & 0x1;
  xmc[0]  = (*c >> 4) & 0x7;
  xmc[1]  = (*c >> 1) & 0x7;
  xmc[2]  = (*c++ & 0x1) << 2;
  xmc[2] |= (*c >> 6) & 0x3;
  xmc[3]  = (*c >> 3) & 0x7;
  xmc[4]  = *c++ & 0x7;
  xmc[5]  = (*c >> 5) & 0x7;
  xmc[6]  = (*c >> 2) & 0x7;
  xmc[7]  = (*c++ & 0x3) << 1;            // 10
  xmc[7] |= (*c >> 7) & 0x1;
  xmc[8]  = (*c >> 4) & 0x7;
  xmc[9]  = (*c >> 1) & 0x7;
  xmc[10]  = (*c++ & 0x1) << 2;
  xmc[10] |= (*c >> 6) & 0x3;
  xmc[11]  = (*c >> 3) & 0x7;
  xmc[12]  = *c++ & 0x7;
  Nc[1]  = (*c >> 1) & 0x7F;
  bc[1]  = (*c++ & 0x1) << 1;
  bc[1] |= (*c >> 7) & 0x1;
  Mc[1]  = (*c >> 5) & 0x3;
  xmaxc[1]  = (*c++ & 0x1F) << 1;
  xmaxc[1] |= (*c >> 7) & 0x1;
  xmc[13]  = (*c >> 4) & 0x7;
  xmc[14]  = (*c >> 1) & 0x7;
  xmc[15]  = (*c++ & 0x1) << 2;
  xmc[15] |= (*c >> 6) & 0x3;
  xmc[16]  = (*c >> 3) & 0x7;
  xmc[17]  = *c++ & 0x7;
  xmc[18]  = (*c >> 5) & 0x7;
  xmc[19]  = (*c >> 2) & 0x7;
  xmc[20]  = (*c++ & 0x3) << 1;
  xmc[20] |= (*c >> 7) & 0x1;
  xmc[21]  = (*c >> 4) & 0x7;
  xmc[22]  = (*c >> 1) & 0x7;
  xmc[23]  = (*c++ & 0x1) << 2;
  xmc[23] |= (*c >> 6) & 0x3;
  xmc[24]  = (*c >> 3) & 0x7;
  xmc[25]  = *c++ & 0x7;
  Nc[2]  = (*c >> 1) & 0x7F;
  bc[2]  = (*c++ & 0x1) << 1;             // 20
  bc[2] |= (*c >> 7) & 0x1;
  Mc[2]  = (*c >> 5) & 0x3;
  xmaxc[2]  = (*c++ & 0x1F) << 1;
  xmaxc[2] |= (*c >> 7) & 0x1;
  xmc[26]  = (*c >> 4) & 0x7;
  xmc[27]  = (*c >> 1) & 0x7;
  xmc[28]  = (*c++ & 0x1) << 2;
  xmc[28] |= (*c >> 6) & 0x3;
  xmc[29]  = (*c >> 3) & 0x7;
  xmc[30]  = *c++ & 0x7;
  xmc[31]  = (*c >> 5) & 0x7;
  xmc[32]  = (*c >> 2) & 0x7;
  xmc[33]  = (*c++ & 0x3) << 1;
  xmc[33] |= (*c >> 7) & 0x1;
  xmc[34]  = (*c >> 4) & 0x7;
  xmc[35]  = (*c >> 1) & 0x7;
  xmc[36]  = (*c++ & 0x1) << 2;
  xmc[36] |= (*c >> 6) & 0x3;
  xmc[37]  = (*c >> 3) & 0x7;
  xmc[38]  = *c++ & 0x7;
  Nc[3]  = (*c >> 1) & 0x7F;
  bc[3]  = (*c++ & 0x1) << 1;
  bc[3] |= (*c >> 7) & 0x1;
  Mc[3]  = (*c >> 5) & 0x3;
  xmaxc[3]  = (*c++ & 0x1F) << 1;
  xmaxc[3] |= (*c >> 7) & 0x1;
  xmc[39]  = (*c >> 4) & 0x7;
  xmc[40]  = (*c >> 1) & 0x7;
  xmc[41]  = (*c++ & 0x1) << 2;
  xmc[41] |= (*c >> 6) & 0x3;
  xmc[42]  = (*c >> 3) & 0x7;
  xmc[43]  = *c++ & 0x7;                  // 30
  xmc[44]  = (*c >> 5) & 0x7;
  xmc[45]  = (*c >> 2) & 0x7;
  xmc[46]  = (*c++ & 0x3) << 1;
  xmc[46] |= (*c >> 7) & 0x1;
  xmc[47]  = (*c >> 4) & 0x7;
  xmc[48]  = (*c >> 1) & 0x7;
  xmc[49]  = (*c++ & 0x1) << 2;
  xmc[49] |= (*c >> 6) & 0x3;
  xmc[50]  = (*c >> 3) & 0x7;
  xmc[51]  = *c & 0x7;                    // 33

  Gsm_Decoder(gsm, LARc, Nc, bc, Mc, xmaxc, xmc, target);

  return 0;
}

 // Variables
struct gsm_state gsm;
FILE *wav_out;
short wavb[160];

// Start WAV file
int Start_GSM_WAV(char *wav_name,int _8bit)
{
  // Create file
  wav_out=fopen(wav_name,"wb");
  if (wav_out==NULL) return 0;

  // Create header
  memcpy(&head.RIFF,"RIFF",4);
  memcpy(&head.WAVE,"WAVE",4);
  memcpy(&head.fmt,"fmt ",4);
  head.hex10=16;
  head.PCM=1;
  head.channel=1;
  head.waveHz=8000;
  head.PlaySp=16000;
  head._bytes=2;
  head.sndBIT=16;
  head.fdatasize=0;
  if (_8bit){
    head._bytes=1;
    head.sndBIT=8;
    head.PlaySp=8000;
  }
  memcpy(&head.buff_data,"data",4);
  fwrite(&head,1,44,wav_out);
  memset(&gsm,0,647);gsm.nrp=40;
  return 1;
}

void decode_block(unsigned char *gsm_block,int _8bit)
{
  gsm_decode(&gsm,gsm_block,wavb);
  if (_8bit){
    for (int wrb=0;wrb<160;wrb++){
      short _a;
      _a=wavb[wrb];
      _a=((long)_a/256)+128;
      fputc(_a&0xFF,wav_out);
    }
    head.fdatasize+=160;
  } else {
    fwrite(wavb,1,320,wav_out);
    head.fdatasize+=320;
  }
}

void Close_GSM_WAV()
{
  // Fix header
  head.fsize=ftell(wav_out);
  fseek(wav_out,4,SEEK_SET);
  fwrite(&head.fsize,1,4,wav_out);
  fseek(wav_out,40,SEEK_SET);
  fwrite(&head.fdatasize,1,4,wav_out);

  // close files
  fclose(wav_out);
}
