// ----------------------------------------------[ Intel386 instructions ]---

#include "i386def.h"

static const Template i386_optab[] = {

// Float instructions
#include "i386flt.h"

//  opcode prefixes
{"aword", 0, 0x67,   None, Noth, 0, 0, 0},
{"addr16",0, 0x67,   None, Noth, 0, 0, 0},
{"word",  0, 0x66,   None, Noth, 0, 0, 0},
{"data16",0, 0x66,   None, Noth, 0, 0, 0},
{"lock",  0, 0xf0,   None, Noth, 0, 0, 0},
{"cs",    0, 0x2e,   None, Noth, 0, 0, 0},
{"ds",    0, 0x3e,   None, Noth, 0, 0, 0},
{"es",    0, 0x26,   None, Noth, 0, 0, 0},
{"fs",    0, 0x64,   None, Noth, 0, 0, 0},
{"gs",    0, 0x65,   None, Noth, 0, 0, 0},
{"ss",    0, 0x36,   None, Noth, 0, 0, 0},
{"rep",   0, 0xf3,   None, Noth, 0, 0, 0},
{"repe",  0, 0xf3,   None, Noth, 0, 0, 0},
{"repne", 0, 0xf2,   None, Noth, 0, 0, 0},
{"repz",  0, 0xf3,   None, Noth, 0, 0, 0},
{"repnz", 0, 0xf2,   None, Noth, 0, 0, 0},

// Byte instructions
{"nop",   0, 0x90,   None, Noth, 0, 0, 0},
{"wait",  0, 0x9b,   None, Noth, 0, 0, 0},
{"pushf", 0, 0x9c,   None, Noth, 0, 0, 0},
{"popf",  0, 0x9d,   None, Noth, 0, 0, 0},
{"sahf",  0, 0x9e,   None, Noth, 0, 0, 0},
{"lahf",  0, 0x9f,   None, Noth, 0, 0, 0},
{"hlt",   0, 0xf4,   None, Noth, 0, 0, 0},
{"cmc",   0, 0xf5,   None, Noth, 0, 0, 0},
{"clc",   0, 0xf8,   None, Noth, 0, 0, 0},
{"stc",   0, 0xf9,   None, Noth, 0, 0, 0},
{"cli",   0, 0xfa,   None, Noth, 0, 0, 0},
{"sti",   0, 0xfb,   None, Noth, 0, 0, 0},
{"cld",   0, 0xfc,   None, Noth, 0, 0, 0},
{"std",   0, 0xfd,   None, Noth, 0, 0, 0},
{"clts",  0, 0x0f06, None, Noth, 0, 0, 0},

// String manipulation
{"movsb", 0, 0xa4,   None, Noth, 0, 0, 0},
{"movsw", 0, 0xa5,   None, _C16, 0, 0, 0},
{"movsd", 0, 0xa5,   None, _C32, 0, 0, 0},
{"cmpsb", 0, 0xa6,   None, Noth, 0, 0, 0},
{"cmpsw", 0, 0xa7,   None, _C16, 0, 0, 0},
{"cmpsd", 0, 0xa7,   None, _C32, 0, 0, 0},
{"stosb", 0, 0xaa,   None, Noth, 0, 0, 0},
{"stosw", 0, 0xab,   None, _C16, 0, 0, 0},
{"stosd", 0, 0xab,   None, _C32, 0, 0, 0},
{"lodsb", 0, 0xac,   None, Noth, 0, 0, 0},
{"lodsw", 0, 0xad,   None, _C16, 0, 0, 0},
{"lodsd", 0, 0xad,   None, _C32, 0, 0, 0},
{"scasb", 0, 0xae,   None, Noth, 0, 0, 0},
{"scasw", 0, 0xaf,   None, _C16, 0, 0, 0},
{"scasd", 0, 0xaf,   None, _C32, 0, 0, 0},
{"xlat",  0, 0xd7,   None, Noth, 0, 0, 0},

{"insb",  0, 0x6c,   None, Noth, 0, 0, 0},
{"insw",  0, 0x6d,   None, _C16, 0, 0, 0},
{"insd",  0, 0x6d,   None, _C32, 0, 0, 0},
{"outsb", 0, 0x6e,   None, Noth, 0, 0, 0},
{"outsw", 0, 0x6f,   None, _C16, 0, 0, 0},
{"outsd", 0, 0x6f,   None, _C32, 0, 0, 0},

// conversion insns
{"cbw",   0, 0x98,   None, _C16, 0, 0, 0},
{"cwd",   0, 0x99,   None, _C16, 0, 0, 0},
{"cwde",  0, 0x98,   None, _C32, 0, 0, 0},
{"cdq",   0, 0x99,   None, _C32, 0, 0, 0},

// load segment registers from memory
{"lds",   2, 0xc5,   None, _C16|Modrm, Reg16, Mem, 0},
{"les",   2, 0xc4,   None, _C16|Modrm, Reg16, Mem, 0},
{"lfs",   2, 0x0fb4, None, _C16|Modrm, Reg16, Mem, 0},
{"lgs",   2, 0x0fb5, None, _C16|Modrm, Reg16, Mem, 0},
{"lss",   2, 0x0fb2, None, _C16|Modrm, Reg16, Mem, 0},
{"lds",   2, 0xc5,   None, _C32|Modrm, Reg32, Mem, 0},
{"les",   2, 0xc4,   None, _C32|Modrm, Reg32, Mem, 0},
{"lfs",   2, 0x0fb4, None, _C32|Modrm, Reg32, Mem, 0},
{"lgs",   2, 0x0fb5, None, _C32|Modrm, Reg32, Mem, 0},
{"lss",   2, 0x0fb2, None, _C32|Modrm, Reg32, Mem, 0},

// in/out from ports
{"in",    2, 0xe4,   None, Noth, Acc8,         Imm8,         0 },
{"in",    2, 0xe5,   None, _C16, Acc16,        Imm8,         0 },
{"in",    2, 0xe5,   None, _C32, Acc32,        Imm8,         0 },
{"out",   2, 0xe6,   None, Noth, Imm8,         Acc8,         0 },
{"out",   2, 0xe7,   None, _C16, Imm8,         Acc16,        0 },
{"out",   2, 0xe7,   None, _C32, Imm8,         Acc32,        0 },
{"in",    2, 0xec,   None, Noth, Acc8,         InOutPortReg, 0 },
{"in",    2, 0xed,   None, _C16, Acc16,        InOutPortReg, 0 },
{"in",    2, 0xed,   None, _C32, Acc32,        InOutPortReg, 0 },
{"out",   2, 0xee,   None, Noth, InOutPortReg, Acc8,         0 },
{"out",   2, 0xef,   None, _C16, InOutPortReg, Acc16,        0 },
{"out",   2, 0xef,   None, _C32, InOutPortReg, Acc32,        0 },

// move instructions
{"movm",  2, 0xa0,   None, D,             Acc8,        Imm16,      0 },
{"movm",  2, 0xa1,   None, _C16|D,        Acc16,       Imm16,      0 },
{"movm",  2, 0xa1,   None, _C32|D,        Acc32,       Imm32,      0 },
{"mov",   2, 0x88,   None, Modrm|D,       Reg8|Mem8,   Reg8,       0 },
{"mov",   2, 0x89,   None, _C16|Modrm|D,  Reg16|Mem16, Reg16,      0 },
{"mov",   2, 0x89,   None, _C32|Modrm|D,  Reg32|Mem32, Reg32,      0 },
{"mov",   2, 0xb0,   None, ShortForm,     Reg8,        Imm8,       0 },
{"mov",   2, 0xb8,   None, _C16|ShortForm,Reg16,       Imm16,      0 },
{"mov",   2, 0xb8,   None, _C32|ShortForm,Reg32,       Imm32,      0 },
{"mov",   2, 0xc6,   None, Modrm,         Mem8,        Imm8,       0 },
{"mov",   2, 0xc7,   None, _C16|Modrm,    Mem16,       Imm16,      0 },
{"mov",   2, 0xc7,   None, _C32|Modrm,    Mem32,       Imm32,      0 },
{"mov",   2, 0x8c,   None, D|Modrm,       Reg16|Mem16, SReg3|SReg2,0 },

// move to/from control debug registers
{"mov",   2, 0x0f20, None, D|Modrm, Reg32|MemReg, Control, 0},
{"mov",   2, 0x0f21, None, D|Modrm, Reg32|MemReg, Debug, 0},
{"mov",   2, 0x0f24, None, D|Modrm, Reg32|MemReg, Test, 0},

// move with sign/zero extend
{"movsx", 2, 0x0fbe, None, _C32|Modrm,   Reg32, Reg8|Mem8,  0},
{"movsx", 2, 0x0fbf, None, _C32|Modrm,   Reg32, Reg16|Mem16,0},
{"movzx", 2, 0x0fb6, None, _C32|Modrm,   Reg32, Reg8|Mem8,  0},
{"movzx", 2, 0x0fb7, None, _C32|Modrm,   Reg32, Reg16|Mem16,0},

// push instructions
{"pusha", 0, 0x60,   None, Noth,           0,           0, 0 },
{"push",  1, 0x50,   None, _C16|ShortForm, Reg16,       0, 0 },
{"push",  1, 0x50,   None, _C32|ShortForm, Reg32,       0, 0 },
{"push",  1, 0xff,   0x6,  _C16|Modrm,     Reg16|Mem16, 0, 0 },
{"push",  1, 0xff,   0x6,  _C32|Modrm,     Reg32|Mem32, 0, 0 },
//{"push",  1, 0x6a,   None, Noth,           Imm8S,       0, 0 },
{"push",  1, 0x68,   None, Noth,           ImmXX,       0, 0 },
{"pushw", 1, 0x68,   None, _C16,           Imm16,       0, 0 },
{"pushd", 1, 0x68,   None, _C32,           Imm32,       0, 0 },
{"push",  1, 0x06,   None, Seg2ShortForm,  SReg2,       0, 0 },
{"push",  1, 0x0fa0, None, Seg3ShortForm,  SReg3,       0, 0 },

// pop instructions
{"popa",  0, 0x61,   None, Noth,           0,           0, 0 },
{"pop",   1, 0x58,   None, _C16|ShortForm, Reg16,       0, 0 },
{"pop",   1, 0x58,   None, _C32|ShortForm, Reg32,       0, 0 },
{"pop",   1, 0x8f,   0x0,  _C16|Modrm,     Reg16|Mem16, 0, 0 },
{"pop",   1, 0x8f,   0x0,  _C32|Modrm,     Reg32|Mem32, 0, 0 },
{"pop",   1, 0x07,   None, Seg2ShortForm,  SReg2,       0, 0 },
{"pop",   1, 0x0fa1, None, Seg3ShortForm,  SReg3,       0, 0 },

// xchg instructions
{"xchg",  2, 0x90,   None, ShortForm,  Acc32, Reg32,       0 },
{"xchg",  2, 0x86,   None, Modrm,      Reg8,  Reg8|Mem8,   0 },
{"xchg",  2, 0x87,   None, _C16|Modrm, Reg16, Reg16|Mem16, 0 },
{"xchg",  2, 0x87,   None, _C32|Modrm, Reg32, Reg32|Mem32, 0 },

// load effective address
{"lea",   2, 0x8d,   None, _C16|Modrm, Reg16, Mem, 0 },
{"lea",   2, 0x8d,   None, _C32|Modrm, Reg32, Mem, 0 },

// Add
{"add",   2, 0x0,    None, D|Modrm,      Reg8|Mem8,   Reg8, 0},
{"add",   2, 0x1,    None, _C16|D|Modrm, Reg16|Mem16, Reg16, 0},
{"add",   2, 0x1,    None, _C32|D|Modrm, Reg32|Mem32, Reg32, 0},
{"add",   2, 0x4,    None, Noth,         Acc8,        Imm8,  0},
{"add",   2, 0x5,    None, _C16,         Acc16,       Imm16, 0},
{"add",   2, 0x5,    None, _C32,         Acc32,       Imm32, 0},
{"add",   2, 0x80,   0,    Modrm,        Reg8|Mem8,   Imm8, 0},
{"add",   2, 0x81,   0,    _C16|Modrm,   Reg16|Mem16, Imm16, 0},
{"add",   2, 0x81,   0,    _C32|Modrm,   Reg32|Mem32, Imm32, 0},
//{"add",   2, 0x83,   0,    Modrm,   Reg32|Mem32, Imm8S, 0},

// Adc
{"adc",   2, 0x10,   None, D|Modrm,      Reg8|Mem8,   Reg8, 0},
{"adc",   2, 0x11,   None, _C16|D|Modrm, Reg16|Mem16, Reg16, 0},
{"adc",   2, 0x11,   None, _C32|D|Modrm, Reg32|Mem32, Reg32, 0},
{"adc",   2, 0x14,   None, Noth,         Acc8,        Imm8,  0},
{"adc",   2, 0x15,   None, _C16,         Acc16,       Imm16, 0},
{"adc",   2, 0x15,   None, _C32,         Acc32,       Imm32, 0},
{"adc",   2, 0x80,   2,    Modrm,        Reg8|Mem8,   Imm8, 0},
{"adc",   2, 0x81,   2,    _C16|Modrm,   Reg16|Mem16, Imm16, 0},
{"adc",   2, 0x81,   2,    _C32|Modrm,   Reg32|Mem32, Imm32, 0},
//{"adc",   2, 0x83,   0,    _C16|Modrm,   Reg16|Mem16, Imm8S, 0},
//{"adc",   2, 0x83,   0,    _C32|Modrm,   Reg32|Mem32, Imm8S, 0},

// ...
{"aaa",   0, 0x37,   None, Noth, 0, 0, 0},
{"aas",   0, 0x3f,   None, Noth, 0, 0, 0},
{"daa",   0, 0x27,   None, Noth, 0, 0, 0},
{"das",   0, 0x2f,   None, Noth, 0, 0, 0},
{"aad",   0, 0xd50a, None, Noth, 0, 0, 0},
{"aam",   0, 0xd40a, None, Noth, 0, 0, 0},


// Sub
{"sub",   2, 0x28,   None, D|Modrm,       Reg8|Mem8,   Reg8,  0},
{"sub",   2, 0x29,   None, _C16|D|Modrm,  Reg16|Mem16, Reg16, 0},
{"sub",   2, 0x29,   None, _C32|D|Modrm,  Reg32|Mem32, Reg32, 0},
{"sub",   2, 0x2c,   None, Noth,          Acc8,        Imm8,  0},
{"sub",   2, 0x2d,   None, _C16,          Acc16,       Imm16, 0},
{"sub",   2, 0x2d,   None, _C32,          Acc32,       Imm32, 0},
{"sub",   2, 0x80,   5,    Modrm,         Reg8|Mem8,   Imm8,  0},
{"sub",   2, 0x81,   5,    _C16|Modrm,    Reg16|Mem16, Imm16, 0},
{"sub",   2, 0x81,   5,    _C32|Modrm,    Reg32|Mem32, Imm32, 0},
//{"sub",   2, 0x83,   5,    _C16|Modrm,    Reg16|Mem16, Imm8S, 0},
//{"sub",   2, 0x83,   5,    _C32|Modrm,    Reg32|Mem32, Imm8S, 0},

// Sbb
{"sbb",   2, 0x18,   None, D|Modrm,       Reg8|Mem8,   Reg8,  0},
{"sbb",   2, 0x19,   None, _C16|D|Modrm,  Reg16|Mem16, Reg16, 0},
{"sbb",   2, 0x19,   None, _C32|D|Modrm,  Reg32|Mem32, Reg32, 0},
{"sbb",   2, 0x1c,   None, Noth,          Acc8,        Imm8,  0},
{"sbb",   2, 0x1d,   None, _C16,          Acc16,       Imm16, 0},
{"sbb",   2, 0x1d,   None, _C32,          Acc32,       Imm32, 0},
{"sbb",   2, 0x80,   3,    Modrm,         Reg8|Mem8,   Imm8,  0},
{"sbb",   2, 0x81,   3,    _C16|Modrm,    Reg16|Mem16, Imm16, 0},
{"sbb",   2, 0x81,   3,    _C32|Modrm,    Reg32|Mem32, Imm32, 0},
//{"sbb",   2, 0x83,   3,    _C16|Modrm,    Reg16|Mem16, Imm8S, 0},
//{"sbb",   2, 0x83,   3,    _C32|Modrm,    Reg32|Mem32, Imm8S, 0},

// And
{"and",   2, 0x20,  None, D|Modrm,      Reg8|Mem8,   Reg8,  0},
{"and",   2, 0x21,  None, _C16|D|Modrm, Reg16|Mem16, Reg16, 0},
{"and",   2, 0x21,  None, _C32|D|Modrm, Reg32|Mem32, Reg32, 0},
{"and",   2, 0x24,  None, Noth,         Acc8,        Imm8,  0},
{"and",   2, 0x25,  None, _C16,         Acc16,       Imm16, 0},
{"and",   2, 0x25,  None, _C32,         Acc32,       Imm32, 0},
{"and",   2, 0x80,  4,    Modrm,        Reg8|Mem8,   Imm8,  0},
{"and",   2, 0x81,  4,    _C16|Modrm,   Reg16|Mem16, Imm16, 0},
{"and",   2, 0x81,  4,    _C32|Modrm,   Reg32|Mem32, Imm32, 0},
//{"and",   2, 0x83,  4,    Modrm,    WordReg|WordMem, Imm8S, 0},

// Or
{"or",    2, 0x08,  None, D|Modrm,      Reg8|Mem8,   Reg8,  0},
{"or",    2, 0x09,  None, _C16|D|Modrm, Reg16|Mem16, Reg16, 0},
{"or",    2, 0x09,  None, _C32|D|Modrm, Reg32|Mem32, Reg32, 0},
{"or",    2, 0x0c,  None, Noth,         Acc8,        Imm8,  0},
{"or",    2, 0x0d,  None, _C16,         Acc16,       Imm16, 0},
{"or",    2, 0x0d,  None, _C32,         Acc32,       Imm32, 0},
{"or",    2, 0x80,  1,    Modrm,        Reg8|Mem8,   Imm8,  0},
{"or",    2, 0x81,  1,    _C16|Modrm,   Reg16|Mem16, Imm16, 0},
{"or",    2, 0x81,  1,    _C32|Modrm,   Reg32|Mem32, Imm32, 0},
//{"or",    2, 0x83,  1,    Modrm,    WordReg|WordMem, Imm8S, 0},

// Xor
{"xor",   2, 0x30,  None, D|Modrm,      Reg8|Mem8,   Reg8,  0},
{"xor",   2, 0x31,  None, _C16|D|Modrm, Reg16|Mem16, Reg16, 0},
{"xor",   2, 0x31,  None, _C32|D|Modrm, Reg32|Mem32, Reg32, 0},
{"xor",   2, 0x34,  None, Noth,         Acc8,        Imm8,  0},
{"xor",   2, 0x35,  None, _C16,         Acc16,       Imm16, 0},
{"xor",   2, 0x35,  None, _C32,         Acc32,       Imm32, 0},
{"xor",   2, 0x80,  6,    Modrm,        Reg8|Mem8,   Imm8,  0},
{"xor",   2, 0x81,  6,    _C16|Modrm,   Reg16|Mem16, Imm16, 0},
{"xor",   2, 0x81,  6,    _C32|Modrm,   Reg32|Mem32, Imm32, 0},
//{"xor",   2, 0x83,  6,    Modrm,    WordReg|WordMem, Imm8S, 0},

// Compare
{"cmp",   2, 0x38,   None, D|Modrm,       Reg8|Mem8,   Reg8,  0},
{"cmp",   2, 0x39,   None, _C16|D|Modrm,  Reg16|Mem16, Reg16, 0},
{"cmp",   2, 0x39,   None, _C32|D|Modrm,  Reg32|Mem32, Reg32, 0},
{"cmp",   2, 0x3c,   None, Noth,          Acc8,        Imm8,  0},
{"cmp",   2, 0x3d,   None, _C16,          Acc16,       Imm16, 0},
{"cmp",   2, 0x3d,   None, _C32,          Acc32,       Imm32, 0},
{"cmp",   2, 0x80,   7,    Modrm,         Reg8|Mem8,   Imm8,  0},
{"cmp",   2, 0x81,   7,    _C16|Modrm,    Reg16|Mem16, Imm16, 0},
{"cmp",   2, 0x81,   7,    _C32|Modrm,    Reg32|Mem32, Imm32, 0},
//{"cmp",   2, 0x83,   7,    _C16|Modrm,    Reg16|Mem16, Imm8S, 0},
//{"cmp",   2, 0x83,   7,    _C32|Modrm,    Reg32|Mem32, Imm8S, 0},

// Test
{"test",  2, 0x84,   None, Modrm,      Reg8|Mem8,   Reg8,  0},
{"test",  2, 0x85,   None, _C16|Modrm, Reg16|Mem16, Reg16, 0},
{"test",  2, 0x85,   None, _C32|Modrm, Reg32|Mem32, Reg32, 0},
{"test",  2, 0xa8,   None, Noth,       Acc8,        Imm8,  0},
{"test",  2, 0xa9,   None, _C16,       Acc16,       Imm16, 0},
{"test",  2, 0xa9,   None, _C32,       Acc32,       Imm32, 0},
{"test",  2, 0xf6,   0,    Modrm,      Reg8|Mem8,   Imm8,  0},
{"test",  2, 0xf7,   0,    _C16|Modrm, Reg16|Mem16, Imm16, 0},
{"test",  2, 0xf7,   0,    _C32|Modrm, Reg32|Mem32, Imm32, 0},

// Rotate
{"rol",   1, 0xd0,   0, Modrm,      Reg8|Mem8,   0,    0},
{"rol",   1, 0xd1,   0, _C16|Modrm, Reg16|Mem16, 0,    0},
{"rol",   1, 0xd1,   0, _C32|Modrm, Reg32|Mem32, 0,    0},
{"rol",   2, 0xc0,   0, Modrm,      Reg8|Mem8,   Imm8, 0},
{"rol",   2, 0xc1,   0, _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"rol",   2, 0xc1,   0, _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"rol",   2, 0xd2,   0, Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"rol",   2, 0xd3,   0, _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"rol",   2, 0xd3,   0, _C32|Modrm, Reg32|Mem32, ShiftCount, 0},

{"ror",   1, 0xd0,   1, Modrm,      Reg8|Mem8,   0,    0},
{"ror",   1, 0xd1,   1, _C16|Modrm, Reg16|Mem16, 0,    0},
{"ror",   1, 0xd1,   1, _C32|Modrm, Reg32|Mem32, 0,    0},
{"ror",   2, 0xc0,   1, Modrm,      Reg8|Mem8,   Imm8, 0},
{"ror",   2, 0xc1,   1, _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"ror",   2, 0xc1,   1, _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"ror",   2, 0xd2,   1, Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"ror",   2, 0xd3,   1, _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"ror",   2, 0xd3,   1, _C32|Modrm, Reg32|Mem32, ShiftCount, 0},

{"rcl",   1, 0xd0,   2, Modrm,      Reg8|Mem8,   0,    0},
{"rcl",   1, 0xd1,   2, _C16|Modrm, Reg16|Mem16, 0,    0},
{"rcl",   1, 0xd1,   2, _C32|Modrm, Reg32|Mem32, 0,    0},
{"rcl",   2, 0xc0,   2, Modrm,      Reg8|Mem8,   Imm8, 0},
{"rcl",   2, 0xc1,   2, _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"rcl",   2, 0xc1,   2, _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"rcl",   2, 0xd2,   2, Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"rcl",   2, 0xd3,   2, _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"rcl",   2, 0xd3,   2, _C32|Modrm, Reg32|Mem32, ShiftCount, 0},

{"rcr",   1, 0xd0,   3, Modrm,      Reg8|Mem8,   0,    0},
{"rcr",   1, 0xd1,   3, _C16|Modrm, Reg16|Mem16, 0,    0},
{"rcr",   1, 0xd1,   3, _C32|Modrm, Reg32|Mem32, 0,    0},
{"rcr",   2, 0xc0,   3, Modrm,      Reg8|Mem8,   Imm8, 0},
{"rcr",   2, 0xc1,   3, _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"rcr",   2, 0xc1,   3, _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"rcr",   2, 0xd2,   3, Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"rcr",   2, 0xd3,   3, _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"rcr",   2, 0xd3,   3, _C32|Modrm, Reg32|Mem32, ShiftCount, 0},

// Shift
{"sal",   1, 0xd0,   4, Modrm,      Reg8|Mem8,   0,    0},
{"sal",   1, 0xd1,   4, _C16|Modrm, Reg16|Mem16, 0,    0},
{"sal",   1, 0xd1,   4, _C32|Modrm, Reg32|Mem32, 0,    0},
{"sal",   2, 0xc0,   4, Modrm,      Reg8|Mem8,   Imm8, 0},
{"sal",   2, 0xc1,   4, _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"sal",   2, 0xc1,   4, _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"sal",   2, 0xd2,   4, Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"sal",   2, 0xd3,   4, _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"sal",   2, 0xd3,   4, _C32|Modrm, Reg32|Mem32, ShiftCount, 0},

{"shl",   1, 0xd0,   4, Modrm,      Reg8|Mem8,   0,    0},
{"shl",   1, 0xd1,   4, _C16|Modrm, Reg16|Mem16, 0,    0},
{"shl",   1, 0xd1,   4, _C32|Modrm, Reg32|Mem32, 0,    0},
{"shl",   2, 0xc0,   4, Modrm,      Reg8|Mem8,   Imm8, 0},
{"shl",   2, 0xc1,   4, _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"shl",   2, 0xc1,   4, _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"shl",   2, 0xd2,   4, Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"shl",   2, 0xd3,   4, _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"shl",   2, 0xd3,   4, _C32|Modrm, Reg32|Mem32, ShiftCount, 0},

//{"shld",  3, 0x0fa4, None, Modrm, Imm8, WordReg, WordReg|Mem},
//{"shld",  3, 0x0fa5, None, Modrm, ShiftCount, WordReg, WordReg|Mem},

{"shr",   1, 0xd0,   5, Modrm,      Reg8|Mem8,   0,    0},
{"shr",   1, 0xd1,   5, _C16|Modrm, Reg16|Mem16, 0,    0},
{"shr",   1, 0xd1,   5, _C32|Modrm, Reg32|Mem32, 0,    0},
{"shr",   2, 0xc0,   5, Modrm,      Reg8|Mem8,   Imm8, 0},
{"shr",   2, 0xc1,   5, _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"shr",   2, 0xc1,   5, _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"shr",   2, 0xd2,   5, Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"shr",   2, 0xd3,   5, _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"shr",   2, 0xd3,   5, _C32|Modrm, Reg32|Mem32, ShiftCount, 0},

//{"shrd",  3, 0x0fac, None, Modrm, Imm8, WordReg, WordReg|Mem},
//{"shrd",  3, 0x0fad, None, Modrm, ShiftCount, WordReg, WordReg|Mem},

{"sar",   1, 0xd0,   7, Modrm,      Reg8|Mem8,   0,    0},
{"sar",   1, 0xd1,   7, _C16|Modrm, Reg16|Mem16, 0,    0},
{"sar",   1, 0xd1,   7, _C32|Modrm, Reg32|Mem32, 0,    0},
{"sar",   2, 0xc0,   7, Modrm,      Reg8|Mem8,   Imm8, 0},
{"sar",   2, 0xc1,   7, _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"sar",   2, 0xc1,   7, _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"sar",   2, 0xd2,   7, Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"sar",   2, 0xd3,   7, _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"sar",   2, 0xd3,   7, _C32|Modrm, Reg32|Mem32, ShiftCount, 0},

// Inc/Dec
{"inc",   1, 0x40,   None, _C16|ShortForm, Reg16,       0, 0},
{"inc",   1, 0x40,   None, _C32|ShortForm, Reg32,       0, 0},
{"inc",   1, 0xfe,   0,    Modrm,          Reg8|Mem8,   0, 0},
{"inc",   1, 0xff,   0,    _C16|Modrm,     Reg16|Mem16, 0, 0},
{"inc",   1, 0xff,   0,    _C32|Modrm,     Reg32|Mem32, 0, 0},
{"dec",   1, 0x48,   None, _C16|ShortForm, Reg16,       0, 0},
{"dec",   1, 0x48,   None, _C32|ShortForm, Reg32,       0, 0},
{"dec",   1, 0xfe,   1,    Modrm,          Reg8|Mem8,   0, 0},
{"dec",   1, 0xff,   1,    _C16|Modrm,     Reg16|Mem16, 0, 0},
{"dec",   1, 0xff,   1,    _C32|Modrm,     Reg32|Mem32, 0, 0},

{"neg",   1, 0xf6,   3,    Modrm,          Reg8|Mem8,   0, 0},
{"not",   1, 0xf6,   2,    Modrm,          Reg8|Mem8,   0, 0},
{"neg",   1, 0xf7,   3,    _C16|Modrm,     Reg16|Mem16, 0, 0},
{"neg",   1, 0xf7,   3,    _C32|Modrm,     Reg32|Mem32, 0, 0},
{"not",   1, 0xf7,   2,    _C16|Modrm,     Reg16|Mem16, 0, 0},
{"not",   1, 0xf7,   2,    _C32|Modrm,     Reg32|Mem32, 0, 0},

// Call
{"call",  1, 0xe8,   None, Noth, DispXX, 0, 0},
{"callf", 2, 0x9a,   None, Noth, ImmXX, Imm16, 0},
{"call",  1, 0xff,   2,    _C16|Modrm, Reg16|Mem16, 0, 0},
{"call",  1, 0xff,   2,    _C32|Modrm, Reg32|Mem32, 0, 0},
{"callf", 1, 0xff,   3,    Modrm, Reg32|Mem32, 0, 0},

// Jump
{"jmps",  1, 0xeb,   None, Noth, Disp8, 0, 0},
{"jmp",   1, 0xe9,   None, Noth, DispXX, 0, 0},
{"jmpf",  2, 0xea,   None, Noth, ImmXX, Imm16, 0},
{"jmp",   1, 0xff,   4,    _C16|Modrm, Reg16|Mem16, 0, 0},
{"jmp",   1, 0xff,   4,    _C32|Modrm, Reg32|Mem32, 0, 0},
{"jmpf",  1, 0xff,   5,    Modrm, Reg32|Mem32, 0, 0},

// Funct utils
{"ret",   0, 0xc3,   None, Noth, 0,     0,    0},
{"ret",   1, 0xc2,   None, Noth, Imm16, 0,    0},
{"retf",  0, 0xcb,   None, Noth, 0,     0,    0},
{"retf",  1, 0xca,   None, Noth, Imm16, 0,    0},
{"enter", 2, 0xc8,   None, Noth, Imm16, Imm8, 0},
{"leave", 0, 0xc9,   None, Noth, 0,     0,    0},

// bit manipulation
{"bsf",   2, 0x0fbc,   None, _C16|Modrm, Reg16,       Reg16|Mem16, 0},
{"bsf",   2, 0x0fbc,   None, _C32|Modrm, Reg32,       Reg32|Mem32, 0},
{"bsr",   2, 0x0fbd,   None, _C16|Modrm, Reg16,       Reg16|Mem16, 0},
{"bsr",   2, 0x0fbd,   None, _C32|Modrm, Reg32,       Reg32|Mem32, 0},
{"bt",    2, 0x0fa3,   None, _C16|Modrm, Reg16|Mem16, Reg16,   0},
{"bt",    2, 0x0fa3,   None, _C32|Modrm, Reg32|Mem32, Reg32,   0},
{"bt",    2, 0x0fba,   4,    _C16|Modrm, Reg16|Mem16, Imm8,    0},
{"bt",    2, 0x0fba,   4,    _C32|Modrm, Reg32|Mem32, Imm8,    0},

{"bts",   2, 0x0fab,   None, _C16|Modrm, Reg16|Mem16, Reg16,   0},
{"bts",   2, 0x0fab,   None, _C32|Modrm, Reg32|Mem32, Reg32,   0},
{"bts",   2, 0x0fba,   5,    _C16|Modrm, Reg16|Mem16, Imm8,    0},
{"bts",   2, 0x0fba,   5,    _C32|Modrm, Reg32|Mem32, Imm8,    0},

{"btc",   2, 0x0fbb,   None, _C16|Modrm, Reg16|Mem16, Reg16,   0},
{"btc",   2, 0x0fbb,   None, _C32|Modrm, Reg32|Mem32, Reg32,   0},
{"btc",   2, 0x0fba,   7,    _C16|Modrm, Reg16|Mem16, Imm8,    0},
{"btc",   2, 0x0fba,   7,    _C32|Modrm, Reg32|Mem32, Imm8,    0},

{"btr",   2, 0x0fb3,   None, _C16|Modrm, Reg16|Mem16, Reg16,   0},
{"btr",   2, 0x0fb3,   None, _C32|Modrm, Reg32|Mem32, Reg32,   0},
{"btr",   2, 0x0fba,   6,    _C16|Modrm, Reg16|Mem16, Imm8,    0},
{"btr",   2, 0x0fba,   6,    _C32|Modrm, Reg32|Mem32, Imm8,    0},

// these turn into pseudo operations when disp is larger than 8 bits
{"jcxz",  1, 0x67e3, None, Noth, Disp8, 0, 0},
{"jecxz", 1, 0xe3,   None, Noth, Disp8, 0, 0},

{"loop",  1, 0xe2,   None, Noth, Disp8, 0, 0},

{"loopz", 1, 0xe1,   None, Noth, Disp8, 0, 0},
{"loope", 1, 0xe1,   None, Noth, Disp8, 0, 0},

{"loopnz",1, 0xe0,   None, Noth, Disp8, 0, 0},
{"loopne",1, 0xe0,   None, Noth, Disp8, 0, 0},

// conditional jumps
{"jo",    1, 0x70,   None, Noth, Disp8,  0, 0},
{"_jo",   1, 0x0F80, None, Noth, DispXX, 0, 0},

{"jno",   1, 0x71,   None, Noth, Disp8,  0, 0},
{"_jno",  1, 0x0F81, None, Noth, DispXX, 0, 0},

{"jb",    1, 0x72,   None, Noth, Disp8,  0, 0},
{"_jb",   1, 0x0F82, None, Noth, DispXX, 0, 0},
{"jc",    1, 0x72,   None, Noth, Disp8,  0, 0},
{"_jc",   1, 0x0F82, None, Noth, DispXX, 0, 0},
{"jnae",  1, 0x72,   None, Noth, Disp8,  0, 0},
{"_jnae", 1, 0x0F82, None, Noth, DispXX, 0, 0},

{"jnb",   1, 0x73,   None, Noth, Disp8,  0, 0},
{"_jnb",  1, 0x0F83, None, Noth, DispXX, 0, 0},
{"jnc",   1, 0x73,   None, Noth, Disp8,  0, 0},
{"_jnc",  1, 0x0F83, None, Noth, DispXX, 0, 0},
{"jae",   1, 0x73,   None, Noth, Disp8,  0, 0},
{"_jae",  1, 0x0F83, None, Noth, DispXX, 0, 0},

{"je",    1, 0x74,   None, Noth, Disp8,  0, 0},
{"_je",   1, 0x0F84, None, Noth, DispXX, 0, 0},
{"jz",    1, 0x74,   None, Noth, Disp8,  0, 0},
{"_jz",   1, 0x0F84, None, Noth, DispXX, 0, 0},

{"jne",   1, 0x75,   None, Noth, Disp8,  0, 0},
{"_jne",  1, 0x0F85, None, Noth, DispXX, 0, 0},
{"jnz",   1, 0x75,   None, Noth, Disp8,  0, 0},
{"_jnz",  1, 0x0F85, None, Noth, DispXX, 0, 0},

{"jbe",   1, 0x76,   None, Noth, Disp8,  0, 0},
{"_jbe",  1, 0x0F86, None, Noth, DispXX, 0, 0},
{"jna",   1, 0x76,   None, Noth, Disp8,  0, 0},
{"_jna",  1, 0x0F86, None, Noth, DispXX, 0, 0},

{"jnbe",  1, 0x77,   None, Noth, Disp8,  0, 0},
{"_jnbe", 1, 0x0F87, None, Noth, DispXX, 0, 0},
{"ja",    1, 0x77,   None, Noth, Disp8,  0, 0},
{"_ja",   1, 0x0F87, None, Noth, DispXX, 0, 0},

{"js",    1, 0x78,   None, Noth, Disp8,  0, 0},
{"_js",   1, 0x0F88, None, Noth, DispXX, 0, 0},

{"jns",   1, 0x79,   None, Noth, Disp8,  0, 0},
{"_jns",  1, 0x0F89, None, Noth, DispXX, 0, 0},

{"jp",    1, 0x7a,   None, Noth, Disp8,  0, 0},
{"_jp",   1, 0x0F8a, None, Noth, DispXX, 0, 0},
{"jpe",   1, 0x7a,   None, Noth, Disp8,  0, 0},
{"_jpe",  1, 0x0F8a, None, Noth, DispXX, 0, 0},

{"jnp",   1, 0x7b,   None, Noth, Disp8,  0, 0},
{"_jnp",  1, 0x0F8b, None, Noth, DispXX, 0, 0},
{"jpo",   1, 0x7b,   None, Noth, Disp8,  0, 0},
{"_jpo",  1, 0x0F8b, None, Noth, DispXX, 0, 0},

{"jl",    1, 0x7c,   None, Noth, Disp8,  0, 0},
{"_jl",   1, 0x0F8c, None, Noth, DispXX,  0, 0},
{"jnge",  1, 0x7c,   None, Noth, Disp8,  0, 0},
{"_jnge", 1, 0x0F8c, None, Noth, DispXX,  0, 0},

{"jnl",   1, 0x7d,   None, Noth, Disp8,  0, 0},
{"_jnl",  1, 0x0F8d, None, Noth, DispXX, 0, 0},
{"jge",   1, 0x7d,   None, Noth, Disp8,  0, 0},
{"_jge",  1, 0x0F8d, None, Noth, DispXX, 0, 0},

{"jle",   1, 0x7e,   None, Noth, Disp8,  0, 0},
{"_jle",  1, 0x0F8e, None, Noth, DispXX, 0, 0},
{"jng",   1, 0x7e,   None, Noth, Disp8,  0, 0},
{"_jng",  1, 0x0F8e, None, Noth, DispXX, 0, 0},

{"jnle",  1, 0x7f,   None, Noth, Disp8,  0, 0},
{"_jnle", 1, 0x0F8f, None, Noth, DispXX, 0, 0},
{"jg",    1, 0x7f,   None, Noth, Disp8,  0, 0},
{"_jg",   1, 0x0F8f, None, Noth, DispXX, 0, 0},

// set byte on flag instructions
{"seto",  1, 0x0f90, 0, Modrm, Reg8|Mem8, 0, 0},

{"setno", 1, 0x0f91, 0, Modrm, Reg8|Mem8, 0, 0},

{"setc",  1, 0x0f92, 0, Modrm, Reg8|Mem8, 0, 0},
{"setb",  1, 0x0f92, 0, Modrm, Reg8|Mem8, 0, 0},
{"setnae",1, 0x0f92, 0, Modrm, Reg8|Mem8, 0, 0},

{"setnc", 1, 0x0f93, 0, Modrm, Reg8|Mem8, 0, 0},
{"setnb", 1, 0x0f93, 0, Modrm, Reg8|Mem8, 0, 0},
{"setae", 1, 0x0f93, 0, Modrm, Reg8|Mem8, 0, 0},

{"sete",  1, 0x0f94, 0, Modrm, Reg8|Mem8, 0, 0},
{"setz",  1, 0x0f94, 0, Modrm, Reg8|Mem8, 0, 0},

{"setne", 1, 0x0f95, 0, Modrm, Reg8|Mem8, 0, 0},
{"setnz", 1, 0x0f95, 0, Modrm, Reg8|Mem8, 0, 0},

{"setbe", 1, 0x0f96, 0, Modrm, Reg8|Mem8, 0, 0},
{"setna", 1, 0x0f96, 0, Modrm, Reg8|Mem8, 0, 0},

{"setnbe",1, 0x0f97, 0, Modrm, Reg8|Mem8, 0, 0},
{"seta",  1, 0x0f97, 0, Modrm, Reg8|Mem8, 0, 0},

{"sets",  1, 0x0f98, 0, Modrm, Reg8|Mem8, 0, 0},

{"setns", 1, 0x0f99, 0, Modrm, Reg8|Mem8, 0, 0},

{"setp",  1, 0x0f9a, 0, Modrm, Reg8|Mem8, 0, 0},
{"setpe", 1, 0x0f9a, 0, Modrm, Reg8|Mem8, 0, 0},

{"setnp", 1, 0x0f9b, 0, Modrm, Reg8|Mem8, 0, 0},
{"setpo", 1, 0x0f9b, 0, Modrm, Reg8|Mem8, 0, 0},

{"setl",  1, 0x0f9c, 0, Modrm, Reg8|Mem8, 0, 0},
{"setnge",1, 0x0f9c, 0, Modrm, Reg8|Mem8, 0, 0},

{"setnl", 1, 0x0f9d, 0, Modrm, Reg8|Mem8, 0, 0},
{"setge", 1, 0x0f9d, 0, Modrm, Reg8|Mem8, 0, 0},

{"setle", 1, 0x0f9e, 0, Modrm, Reg8|Mem8, 0, 0},
{"setng", 1, 0x0f9e, 0, Modrm, Reg8|Mem8, 0, 0},

{"setnle",1, 0x0f9f, 0, Modrm, Reg8|Mem8, 0, 0},
{"setg",  1, 0x0f9f, 0, Modrm, Reg8|Mem8, 0, 0},

{"bound", 2, 0x62,   None, _C16|Modrm, Reg16, Mem16, 0},
{"bound", 2, 0x62,   None, _C32|Modrm, Reg32, Mem32, 0},

// "*"
{"mul",   1, 0xf6,   4,    Modrm,      Reg8|Mem8,   0, 0},
{"mul",   1, 0xf7,   4,    _C16|Modrm, Reg16|Mem16, 0, 0},
{"mul",   1, 0xf7,   4,    _C32|Modrm, Reg32|Mem32, 0, 0},
{"imul",  1, 0xf6,   5,    Modrm,      Reg8|Mem8,   0, 0},
{"imul",  1, 0xf7,   5,    _C16|Modrm, Reg16|Mem16, 0, 0},
{"imul",  1, 0xf7,   5,    _C32|Modrm, Reg32|Mem32, 0, 0},
{"imul",  3, 0x69, None,   _C16|Modrm, Reg16, Reg16|Mem16, Imm16},
{"imul",  3, 0x69, None,   _C32|Modrm, Reg32, Reg32|Mem32, Imm32},
{"imul",  2, 0x0faf, None, _C16|Modrm, Reg16, Reg16|Mem16, 0},
{"imul",  2, 0x0faf, None, _C32|Modrm, Reg32, Reg32|Mem32, 0},
//{"imul",  3, 0x6b, None, Modrm, Imm8S, WordReg|Mem, WordReg},
//{"imul",  2, 0x6b, None, Modrm|imulKludge, Imm8S, WordReg, 0},


// "/"
{"div",   1, 0xf6,   6,    Modrm,      Reg8|Mem8,   0, 0},
{"div",   1, 0xf7,   6,    _C16|Modrm, Reg16|Mem16, 0, 0},
{"div",   1, 0xf7,   6,    _C32|Modrm, Reg32|Mem32, 0, 0},
{"idiv",  1, 0xf6,   7,    Modrm,      Reg8|Mem8,   0, 0},
{"idiv",  1, 0xf7,   7,    _C16|Modrm, Reg16|Mem16, 0, 0},
{"idiv",  1, 0xf7,   7,    _C32|Modrm, Reg32|Mem32, 0, 0},


// interrupts
{"int",   1, 0xcd,   None, Noth, Imm8, 0, 0},
{"int3",  0, 0xcc,   None, Noth, 0,    0, 0},
{"into",  0, 0xce,   None, Noth, 0,    0, 0},
{"iret",  0, 0xcf,   None, Noth, 0,    0, 0},

// protection control
{"lgdt",  1, 0x0f01, 2, Modrm, Mem, 0, 0},
{"lidt",  1, 0x0f01, 3, Modrm, Mem, 0, 0},
{"lldt",  1, 0x0f00, 2, Modrm, Reg16|Mem, 0, 0},
{"lmsw",  1, 0x0f01, 6, Modrm, Reg16|Mem, 0, 0},
{"ltr",   1, 0x0f00, 3, Modrm, Reg16|Mem, 0, 0},
 //{"arpl",  2, 0x63, None, Modrm, Reg16, Reg16|Mem, 0},
 //{"lar",   2, 0x0f02, None, Modrm|ReverseRegRegmem, WordReg|Mem, WordReg, 0},
 //{"lsl",   2, 0x0f03, None, Modrm|ReverseRegRegmem, WordReg|Mem, WordReg, 0},

{"sgdt",  1, 0x0f01, 0, Modrm, Mem, 0, 0},
{"sidt",  1, 0x0f01, 1, Modrm, Mem, 0, 0},
{"sldt",  1, 0x0f00, 0, Modrm, Reg16|Mem, 0, 0},
{"smsw",  1, 0x0f01, 4, Modrm, Reg16|Mem, 0, 0},
{"str",   1, 0x0f00, 1, Modrm, Reg16|Mem, 0, 0},

{"verr",  1, 0x0f00, 4, Modrm, Reg16|Mem, 0, 0},
{"verw",  1, 0x0f00, 5, Modrm, Reg16|Mem, 0, 0},

// 486 extensions
{"bswap", 1, 0x0fc8,   None, ShortForm, Reg32,   0,     0 },
//{"xadd",  2, 0x0fc0,   None, Modrm,      Reg8|Mem8,   Reg8,  0 },
//{"xadd",  2, 0x0fc1,   None, _C16|Modrm, Reg16|Mem16, Reg16, 0 },
//{"xadd",  2, 0x0fc1,   None, _C32|Modrm, Reg32|Mem32, Reg32, 0 },
//{"cmpxchg",2,0x0fb0,   None, Modrm,      Reg8|Mem8,   Reg8,  0 },
//{"cmpxchg",2,0x0fb1,   None, _C16|Modrm, Reg16|Mem16, Reg16, 0 },
//{"cmpxchg",2,0x0fb1,   None, _C32|Modrm, Reg32|Mem32, Reg32, 0 },
{"invd",  0, 0x0f08,   None, Noth,  0,           0,     0 },
{"wbinvd",0, 0x0f09,   None, Noth,  0,           0,     0 },
//{"invlpg",1, 0x0f01,   7,    Modrm, Mem,         0,     0 },

// Pentium and late-model 486 extensions
{"cpuid", 0, 0x0fa2, None, Noth, 0, 0, 0},

// Pentium extensions
{"wrmsr", 0, 0x0f30, None, Noth, 0, 0, 0},
{"rdtsc", 0, 0x0f31, None, Noth, 0, 0, 0},
{"rdmsr", 0, 0x0f32, None, Noth, 0, 0, 0},
//{"cmpxchg8b",1,0x0fc7, 1, Modrm, Mem, 0, 0},

// Pentium Pro extensions
{"rdpmc", 0, 0x0f33, None, Noth, 0, 0, 0},

{"ud2",   0, 0x0fff, None, Noth, 0, 0, 0}, // official undefined instr.

// "cmov*" instr skipped

{"", 0, 0, 0, 0, 0, 0, 0}       // sentinal (end)
};

// 386 register table
static const reg_entry i386_regtab[] = {

  // 8 bit regs
  {"al",Reg8|Acc8,0},{"cl",Reg8|ShiftCount,1},{"dl",Reg8,2},{"bl",Reg8,3},
  {"ah",Reg8,4},{"ch",Reg8,5},{"dh",Reg8,6},{"bh",Reg8,7},

  // 16 bit regs
  {"ax",Reg16|Acc16,0},{"cx",Reg16,1},{"dx",Reg16|InOutPortReg,2},{"bx",Reg16,3},
  {"sp",Reg16,4},{"bp",Reg16,5},{"si",Reg16,6},{"di",Reg16,7},

  // 32 bit regs
  {"eax",Reg32|Acc32,0},{"ecx",Reg32,1},{"edx",Reg32,2},{"ebx",Reg32,3},
  {"esp",Reg32,4},{"ebp",Reg32,5},{"esi",Reg32,6},{"edi",Reg32,7},

  // segment registers
  {"es", SReg2, 0}, {"cs", SReg2, 1}, {"ss", SReg2, 2},
  {"ds", SReg2, 3}, {"fs", SReg3, 4}, {"gs", SReg3, 5},

  // control registers
  {"cr0",Control,0},{"cr2",Control,2},{"cr3",Control,3},{"cr4",Control,4},

  // debug registers
  {"dr0",Debug,0},{"dr1",Debug,1},{"dr2",Debug,2},
  {"dr3",Debug,3},{"dr6",Debug,6},{"dr7",Debug,7},

  // test registers
  {"tr3",Test,3},{"tr4",Test,4},{"tr5",Test,5},{"tr6",Test,6},{"tr7",Test,7},

  // float registers
  {"st0",FloatReg|FloatAcc,0},{"st",FloatReg|FloatAcc,0},{"st1",FloatReg,1},
  {"st2",FloatReg,2},{"st3",FloatReg,3},{"st4",FloatReg,4},
  {"st5",FloatReg,5},{"st6",FloatReg,6},{"st7",FloatReg,7},

  // End
  {"",0,0}
};

 // Prefix table
static const prefix_entry i386_prefixtab[] = {
  {"addr16",0x67},{"data16",0x66 },{"lock",0xf0 },
  {"wait",0x9b },{"cs",0x2e },{"ds",0x3e },
  {"es",0x26 },{"fs",0x64 },{"gs",0x65 },
  {"ss",0x36 },{"rep",0xf3},{"repe",0xf3},
  {"repz",0xf3},{"repne",0xf2},{"repnz",0xf2},
  {"",0}
};

// --------------------------------------------------------------------------
