// --------------------------------------------------------------------------
// ReadFile()                    Read next character from binary file
// FGetPos()                     Get current file position
// ErrorExit(code)               Exit with error - code
// OpenFile(fname,pos,size)      Open file at pos as size (if size=0 => full)
// CloseFile()                   Close last file entry
// --------------------------------------------------------------------------
// tfs->FileName                 Name of current file
// tfs->lastline                 Current line
// tfs->lines                    lines in current file
// FileOpened                    Number of opened files
// ch                            Character, returned by ReadFile() func
// --------------------------------------------------------------------------

#define prebuf 1000

// Main structure
struct TFileStream {
 struct TFileStream *next,*prev;
 char FileName[128],swp[prebuf],lastline[128],_old;
 unsigned long lines,sz,ps;
 int i,e;
 FILE *fp;
};

// Variables for files
struct TFileStream *tfs,*tt;
int FileOpened=0;
char ch,old;

 // ReadFile
int ReadFile(){
  if (FileOpened>0){

    // If needed => read next block
    if (tfs->e==tfs->i){
     tfs->i=fread(tfs->swp,1,prebuf,tfs->fp);
     tfs->e=0;if (tfs->i==0) return 0;
    }old=ch;

    // Save character from block
    tfs->e++;tfs->ps++;
    ch=tfs->swp[tfs->e-1];
    if (tfs->sz==0) return 0;
    tfs->sz--;
    if (ch==9) ch=' ';

    // Save character to lastline
    if (tfs->_old==13){
      tfs->lines++;
      tfs->lastline[0]=0;
    }
    if (ch!=10&&ch!=13){
      tfs->lastline[strlen(tfs->lastline)+1]=0;
      tfs->lastline[strlen(tfs->lastline)]=ch;
    }
    tfs->_old=ch;

    return 1;
  } else return 0;
}

 // Get File Position
unsigned long FGetPos(){
   if (FileOpened>0)
      return (ftell(tfs->fp)-tfs->i+tfs->e);
   else
      return 0;
}

 // Close file
int CloseFile(){
  if (FileOpened>0){

    if (!strcmp(tfs->FileName,datf)) ch=1; else ch=0;
    fclose(tfs->fp);
    tt=tfs->prev;
    free(tfs);
    FileOpened--;
    if (FileOpened>0) tfs=tt;
    if (ch) remove(datf);
    return 1;
  } else return 0;
}

 // If error found => exit
void ErrorExit(int rescode){

  // Show info
  if (FileOpened>0){
    cout << "ERROR in File: " << tfs->FileName << " at line " << tfs->lines << " col " << strlen(tfs->lastline) << endl;
    while (ch!=13&&ReadFile());
    cout << " " << tfs->lastline << endl;
  }
  switch(rescode){

   // Assembly errors
   case  1: cout << "Invalid opcode" << endl;break;
   case  2: cout << "Prefix error" << endl;break;
   case  3: cout << "Parameter of unknown type" << endl;break;
   case  4: cout << "Immediate operand is out of range" << endl;break;
   case  5: cout << "Disp 8/16 error" << endl;break;
   case  6: cout << "Internal opcode/register error" << endl;break;
   case  7: cout << "Using two index registers is unavailable" << endl;break;
   case  8: cout << "Addressing mode error" << endl;break;
   case  9: cout << "ss error" << endl;break;
   case 10: cout << "Index error" << endl;break;
   case 11: cout << "Using two base registers is unavaible" << endl;break;
   case 12: cout << "ESP register cannot be used as index" << endl;break;
   case 13: cout << "Number error" << endl;break;
   case 14: cout << "Number is out of range" << endl;break;
   case 15: cout << "Reference overriding" << endl;break;
   case 16: cout << "Expression error" << endl;break;
   case 17: cout << "Use 32-bit register" << endl;break;
   case 18: cout << "Use 16-bit register" << endl;break;
   case 19: cout << "Do not use register twice" << endl;break;
   case 20: cout << "Use 32-bit register as index" << endl;break;
   case 21: cout << "Use 32-bit register as base" << endl;break;

   // File
   case 30: cout << "File not found" << endl;break;
   case 31: cout << "Error creating output" << endl;break;
   case 32: cout << "Error writing output" << endl;break;
   case 33: cout << "Error creating data" << endl;break;
   case 34: cout << "Error writing data" << endl;break;
   case 35: cout << "DATA presents: use #data directive" << endl;break;

   // Constructions
   case 40: cout << ">'< not found" << endl;break;
   case 41: cout << ">""< not found" << endl;break;
   case 42: cout << "Too big string" << endl;break;
   case 43: cout << "'' construction may be from 1 up to 4 char length" << endl;break;
   case 44: cout << ">]< not found" << endl;break;
   case 45: cout << ">)< not found" << endl;break;
   case 51: cout << "Too much '{'" << endl;break;
   case 52: cout << "Too much '}'" << endl;break;
   case 53: cout << "'}' not found" << endl;break;
   case 56: cout << "Directive error" << endl;break;
   case 57: cout << "Unknown directive" << endl;break;
   case 58: cout << "Code must be 16-bit or 32-bit" << endl;break;
   case 59: cout << "Construction not finished" << endl;break;
   case 60: cout << "Construction error" << endl;break;
   case 61: cout << "Construction error: else not found" << endl;break;
   case 62: cout << "Too much ')'" << endl;break;
   case 63: cout << "ident after + - / * >> > < << ^ | & absent" << endl;break;
   case 64: cout << "ident after ++ -- present" << endl;break;
   case 65: cout << "constr error : use brackets" << endl;break;
   case 66: cout << "ifdef buffer overflow" << endl;break;
   case 67: cout << "Calling macros from func/if/while etc. unavail" << endl;break;

   // Label
   case 70: cout << "Extra characters in label name" << endl;break;
   case 71: cout << "First character of label cannot be a number" << endl;break;
   case 72: cout << "Second use of the same label" << endl;break;
   case 73: cout << "Internal buffer overflow" << endl;break;
   case 74: cout << "Invalid label" << endl;break;
   case 75: cout << "?(V/^)x error" << endl;break;
   case 76: cout << "Unknown identificator" << endl;break;

   // Other
   case 80: cout << "put >,< between definitions" << endl;break;
   case 81: cout << "expression is absent" << endl;break;
   case 82: cout << "unknown type: AND or OR" << endl;break;
   case 83: cout << "#endm found, but #macro absent" << endl;break;
   case 84: cout << "You can not call macros from macros" << endl;break;

   // func
   case 90: cout << "function error: >(< not found" << endl;break;
   case 91: cout << "function error: parameter is absent" << endl;break;
   case 92: cout << "function error: parameter error >'< or >""< is absent" << endl;break;
   case 93: cout << "ERROR: ()&&()||() not available" << endl;break;
   case 94: cout << "function definition error: function name:dest()" << endl;break;
   case 95: cout << "function error: params count wrong" << endl;break;
   case 96: cout << "char/uint/ulong error: ,, unavailable" << endl;break;
   case 97: cout << "char/uint/ulong error: syntax" << endl;break;
   case 98: cout << "char/uint/ulong error: >]< is absent" << endl;break;
   case 99: cout << "function destination is not defined yet" << endl;break;

  }

  // Remove files
  while(CloseFile());remove(datf);if (second) remove(outfile);
  exit(0);}

 // Open file for reading
void OpenFile(char *fname,unsigned long pos,unsigned long size){

  // Create new entry
  if (FileOpened==0){
     tfs=(TFileStream *)malloc(sizeof(*tfs));
     if (tfs==NULL) ErrorExit(90);
  } else {
     tfs->next=(TFileStream *)malloc(sizeof(*tfs));
     if (tfs->next==NULL) ErrorExit(90);
     tt=tfs;tfs=tfs->next;tfs->prev=tt;
  }
  FileOpened++;strcpy(tfs->FileName,fname);tfs->lines=1;

  // Open File
  tfs->fp=fopen(tfs->FileName,"rb");
  if (tfs->fp==NULL){
     tt=tfs->prev;free(tfs);FileOpened--;
     if (FileOpened>0) tfs=tt;
     ErrorExit(30);
  }

  // Prepearing entries
  tfs->sz=size;tfs->ps=pos;tfs->i=0;tfs->e=0;tfs->lastline[0]=0;
  if (size==0){fseek(tfs->fp,0,SEEK_END);tfs->sz=ftell(tfs->fp)-pos;}
  fseek(tfs->fp,pos,SEEK_SET);

}

// --------------------------------------------------------------------------
