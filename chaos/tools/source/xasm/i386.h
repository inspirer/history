// i386.h
#ifndef _INCLUDED_
 #define _INCLUDED_
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
#endif

#define CodePrefix    0x66
#define MemoryPrefix  0x67

// extension opcode
#define None          0xff

// opcode_modifier bits
#define Noth          0x00
#define D             0x01
#define ShortForm     0x02           // register is in low 3 bits of opcode
#define Seg2ShortForm 0x04           // encoding of load segment reg insns
#define Seg3ShortForm 0x08           // fs/gs segment register insns.
#define Modrm         0x10
#define _C16          0x20
#define _C32          0x80

// operand_types
#define Unknown      0x00
#define Reg8         0x01            // Registers
#define Reg16        0x02
#define Reg32        0x04
#define Imm8         0x08            // Immediate
#define Imm8S        0x10
#define Imm16        0x20
#define Imm32        0x40
#define ImmXX        0x80
#define Imm     (Imm32|Imm16|Imm8S|Imm8|ImmXX)
#define Mem8         0x100           // Memory
#define Mem16        0x200
#define Mem32        0x400
#define MemFL        0x800
#define Mem          0x1000
#define Disp8        0x2000          // Jumps
#define Disp16       0x4000
#define Disp32       0x8000
#define DispXX       0x10000
#define InOutPortReg 0x20000         // dx
#define ShiftCount   0x40000         // cl
#define Control      0x80000         // Control
#define Debug        0x100000        // Debug
#define Test         0x200000        // Test
#define SReg2        0x400000        // 2 bit segment register (cs,es,ds,ss)
#define SReg3        0x800000        // 3 bit segment register (..+fs,gs)
#define Acc8         0x1000000       // al
#define Acc16        0x2000000       // ax
#define Acc32        0x4000000       // eax
#define FloatReg     0x8000000       // st(i)
#define FloatAcc     0x10000000      // st(0)
#define MemReg       0x20000000      // Store register in Mem field

// internal
#define MemIndex 0x01
#define MemAddr  0x02
#define MemBase  0x04
#define TinyBase 0x08
#define BX_used  0x10
#define BP_used  0x20
#define SI_used  0x40
#define DI_used  0x80

// register name
typedef struct {
   char *reg_name;
   unsigned long reg_type;
   unsigned int reg_num;
} reg_entry;

// prefix name
typedef struct {
   char *prefix_name;
   unsigned char prefix_code;
} prefix_entry;

// segment reg name
typedef struct {
   char *seg_name;
   unsigned int seg_prefix;
} seg_entry;

// Template
typedef struct {
   char *name;
   unsigned int operands;
   unsigned long base_opcode;
   unsigned char extension_opcode;
   unsigned long opcode_modifier;
   unsigned long operand_types[3];
} Template;

extern const prefix_entry i386_prefixtab[];
extern const reg_entry i386_regtab[];
extern const Template i386_optab[];
extern int code32,tnum;
extern int _index[256];

// Instruction
struct TInstruction
{
  char *prefix,*command,*params[3];     // prefix/command/parameters
  int   cmdnum;                         // numofcommand
  short pnum,direction;                 // numofparams, direction
  unsigned long offs,ptype[3],sline;    // offset32/paramtypes/sourceline
  struct TInstruction *next,*from;      // next_structure, from_which_take_offs
  char code[15],*fname;                 // code/sourcefile
  long size;                            // code_size
  long def_area;                        // definition area
};

extern struct TInstruction *ins,*pi,*_pi,*s_del,*c_del; // ..,start_del,curr_del
extern long instr;

int i386reg(char *rg);
int i386prefix(char *rg);
long defdata(struct TInstruction *,FILE *);
int i386(struct TInstruction *);

typedef struct TOptimizer
{
  char *command;
  int  args;
  unsigned long t[3];
  char *a[3];
  char *change;
} optimize;

extern const optimize opti[];
void generate_list();
