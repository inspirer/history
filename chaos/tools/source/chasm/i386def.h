// ---------------------------------------------[ Intel386 Assembler ]-------

#define CodePrefix   0x66
#define MemoryPrefix 0x67

#define Unknown  0x0
#define Reg8     0x1                 // Registers
#define Reg16    0x2
#define Reg32    0x4
#define Imm8     0x8                 // Immediate
#define Imm8S    0x10
#define Imm16    0x20
#define Imm32    0x40
#define Mem8     0x80                // Memory
#define Mem16    0x100
#define Mem32    0x200
#define MemFL    0x10000000
#define Mem      0x20000000
#define Disp8    0x400               // Jumps
#define Disp16   0x800
#define Disp32   0x1000
// specials
#define InOutPortReg 0x2000         // dx
#define ShiftCount 0x4000           // cl
#define Control  0x8000              // Control
#define Debug    0x10000             // Debug
#define Test     0x20000             // Test
#define SReg2    0x40000             // 2 bit segment register (cs,es,ds,ss)
#define SReg3    0x80000             // 3 bit segment register (..+fs,gs)
#define Acc8     0x100000            // al
#define Acc16    0x200000            // ax
#define Acc32    0x400000            // eax
#define FloatReg 0x800000           // st(i)
#define FloatAcc 0x1000000          // st(0)
#define MemReg   0x2000000           // Store register in Mem field
#define ImmXX    0x4000000
#define DispXX   0x8000000
#define Imm     (Imm32|Imm16|Imm8S|Imm8|ImmXX)

#define MemIndex 1
#define MemAddr  2
#define MemBase  4
#define TinyBase 8
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

/* 386 operand encoding bytes:  see 386 book for details of this. */
typedef struct {
        unsigned regmem:3;           /* codes register or memory operand */
        unsigned reg:3;      /* codes register operand (or extended opcode) */
        unsigned mode:2;             /* how to interpret regmem & reg */
} modrm_byte;

/* 386 opcode byte to code indirect addressing. */
typedef struct {
        unsigned base:3;
        unsigned index:3;
        unsigned scale:2;
} base_index_byte;

// Template
typedef struct {

   char *name;
   unsigned int operands;
   unsigned long base_opcode;
   unsigned char extension_opcode;
#define None 0xff

   unsigned long opcode_modifier;

   // opcode_modifier bits:
#define Noth          0
#define D             1
#define ShortForm     2       // register is in low 3 bits of opcode
#define Seg2ShortForm 4       // encoding of load segment reg insns
#define Seg3ShortForm 8       // fs/gs segment register insns.
#define Modrm         0x10
#define _C16          0x20
#define _C32          0x80
//#define FloatD 0x400            // Float direction flag

   unsigned long operand_types[3];
} Template;

// --------------------------------------------------------------------------
