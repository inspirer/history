// i386ins.c

#include "i386.h"
#include "cc.h"

const Template i386_optab[] = {

{"_addr16",0, 0x67,   None, Noth, 0, 0, 0},
{"_cs",    0, 0x2e,   None, Noth, 0, 0, 0},
{"_data16",0, 0x66,   None, Noth, 0, 0, 0},
{"_ds",    0, 0x3e,   None, Noth, 0, 0, 0},
{"_es",    0, 0x26,   None, Noth, 0, 0, 0},
{"_fs",    0, 0x64,   None, Noth, 0, 0, 0},
{"_gs",    0, 0x65,   None, Noth, 0, 0, 0},
{"_lock",  0, 0xf0,   None, Noth, 0, 0, 0},
{"_rep",   0, 0xf3,   None, Noth, 0, 0, 0},
{"_repe",  0, 0xf3,   None, Noth, 0, 0, 0},
{"_repne", 0, 0xf2,   None, Noth, 0, 0, 0},
{"_repnz", 0, 0xf2,   None, Noth, 0, 0, 0},
{"_repz",  0, 0xf3,   None, Noth, 0, 0, 0},
{"_ss",    0, 0x36,   None, Noth, 0, 0, 0},
{"_ja",    1, 0x0f87, None, Noth, DispXX, 0, 0},
{"_jae",   1, 0x0f83, None, Noth, DispXX, 0, 0},
{"_jb",    1, 0x0f82, None, Noth, DispXX, 0, 0},
{"_jbe",   1, 0x0f86, None, Noth, DispXX, 0, 0},
{"_jc",    1, 0x0f82, None, Noth, DispXX, 0, 0},
{"_je",    1, 0x0f84, None, Noth, DispXX, 0, 0},
{"_jg",    1, 0x0f8f, None, Noth, DispXX, 0, 0},
{"_jge",   1, 0x0f8d, None, Noth, DispXX, 0, 0},
{"_jl",    1, 0x0f8c, None, Noth, DispXX,  0, 0},
{"_jle",   1, 0x0f8e, None, Noth, DispXX, 0, 0},
{"_jna",   1, 0x0f86, None, Noth, DispXX, 0, 0},
{"_jnae",  1, 0x0f82, None, Noth, DispXX, 0, 0},
{"_jnb",   1, 0x0f83, None, Noth, DispXX, 0, 0},
{"_jnbe",  1, 0x0f87, None, Noth, DispXX, 0, 0},
{"_jnc",   1, 0x0f83, None, Noth, DispXX, 0, 0},
{"_jne",   1, 0x0f85, None, Noth, DispXX, 0, 0},
{"_jng",   1, 0x0f8e, None, Noth, DispXX, 0, 0},
{"_jnge",  1, 0x0f8c, None, Noth, DispXX,  0, 0},
{"_jnl",   1, 0x0f8d, None, Noth, DispXX, 0, 0},
{"_jnle",  1, 0x0f8f, None, Noth, DispXX, 0, 0},
{"_jno",   1, 0x0f81, None, Noth, DispXX, 0, 0},
{"_jnp",   1, 0x0f8b, None, Noth, DispXX, 0, 0},
{"_jns",   1, 0x0f89, None, Noth, DispXX, 0, 0},
{"_jnz",   1, 0x0f85, None, Noth, DispXX, 0, 0},
{"_jo",    1, 0x0f80, None, Noth, DispXX, 0, 0},
{"_jp",    1, 0x0f8a, None, Noth, DispXX, 0, 0},
{"_jpe",   1, 0x0f8a, None, Noth, DispXX, 0, 0},
{"_jpo",   1, 0x0f8b, None, Noth, DispXX, 0, 0},
{"_js",    1, 0x0f88, None, Noth, DispXX, 0, 0},
{"_jz",    1, 0x0f84, None, Noth, DispXX, 0, 0},
{"aaa",    0, 0x37,   None, Noth, 0, 0, 0},
{"aad",    0, 0xd50a, None, Noth, 0, 0, 0},
{"aam",    0, 0xd40a, None, Noth, 0, 0, 0},
{"aas",    0, 0x3f,   None, Noth, 0, 0, 0},
{"adc",    2, 0x10,   None, D|Modrm,      Reg8|Mem8,   Reg8, 0},
{"adc",    2, 0x11,   None, _C16|D|Modrm, Reg16|Mem16, Reg16, 0},
{"adc",    2, 0x11,   None, _C32|D|Modrm, Reg32|Mem32, Reg32, 0},
{"adc",    2, 0x14,   None, Noth,         Acc8,        Imm8,  0},
{"adc",    2, 0x15,   None, _C16,         Acc16,       Imm16, 0},
{"adc",    2, 0x15,   None, _C32,         Acc32,       Imm32, 0},
{"adc",    2, 0x80,   2,    Modrm,        Reg8|Mem8,   Imm8, 0},
{"adc",    2, 0x81,   2,    _C16|Modrm,   Reg16|Mem16, Imm16, 0},
{"adc",    2, 0x81,   2,    _C32|Modrm,   Reg32|Mem32, Imm32, 0},
{"add",    2, 0x0,    None, D|Modrm,      Reg8|Mem8,   Reg8, 0},
{"add",    2, 0x1,    None, _C16|D|Modrm, Reg16|Mem16, Reg16, 0},
{"add",    2, 0x1,    None, _C32|D|Modrm, Reg32|Mem32, Reg32, 0},
{"add",    2, 0x4,    None, Noth,         Acc8,        Imm8,  0},
{"add",    2, 0x5,    None, _C16,         Acc16,       Imm16, 0},
{"add",    2, 0x5,    None, _C32,         Acc32,       Imm32, 0},
{"add",    2, 0x80,   0,    Modrm,        Reg8|Mem8,   Imm8, 0},
{"add",    2, 0x81,   0,    _C16|Modrm,   Reg16|Mem16, Imm16, 0},
{"add",    2, 0x81,   0,    _C32|Modrm,   Reg32|Mem32, Imm32, 0},
{"and",    2, 0x20,   None, D|Modrm,      Reg8|Mem8,   Reg8,  0},
{"and",    2, 0x21,   None, _C16|D|Modrm, Reg16|Mem16, Reg16, 0},
{"and",    2, 0x21,   None, _C32|D|Modrm, Reg32|Mem32, Reg32, 0},
{"and",    2, 0x24,   None, Noth,         Acc8,        Imm8,  0},
{"and",    2, 0x25,   None, _C16,         Acc16,       Imm16, 0},
{"and",    2, 0x25,   None, _C32,         Acc32,       Imm32, 0},
{"and",    2, 0x80,   4,    Modrm,        Reg8|Mem8,   Imm8,  0},
{"and",    2, 0x81,   4,    _C16|Modrm,   Reg16|Mem16, Imm16, 0},
{"and",    2, 0x81,   4,    _C32|Modrm,   Reg32|Mem32, Imm32, 0},
{"bound",  2, 0x62,   None, _C16|Modrm, Reg16, Mem16, 0},
{"bound",  2, 0x62,   None, _C32|Modrm, Reg32, Mem32, 0},
{"bsf",    2, 0x0fbc, None, _C16|Modrm, Reg16,       Reg16|Mem16, 0},
{"bsf",    2, 0x0fbc, None, _C32|Modrm, Reg32,       Reg32|Mem32, 0},
{"bsr",    2, 0x0fbd, None, _C16|Modrm, Reg16,       Reg16|Mem16, 0},
{"bsr",    2, 0x0fbd, None, _C32|Modrm, Reg32,       Reg32|Mem32, 0},
{"bt",     2, 0x0fa3, None, _C16|Modrm, Reg16|Mem16, Reg16,   0},
{"bt",     2, 0x0fa3, None, _C32|Modrm, Reg32|Mem32, Reg32,   0},
{"bt",     2, 0x0fba, 4,    _C16|Modrm, Reg16|Mem16, Imm8,    0},
{"bt",     2, 0x0fba, 4,    _C32|Modrm, Reg32|Mem32, Imm8,    0},
{"btc",    2, 0x0fba, 7,    _C16|Modrm, Reg16|Mem16, Imm8,    0},
{"btc",    2, 0x0fba, 7,    _C32|Modrm, Reg32|Mem32, Imm8,    0},
{"btc",    2, 0x0fbb, None, _C16|Modrm, Reg16|Mem16, Reg16,   0},
{"btc",    2, 0x0fbb, None, _C32|Modrm, Reg32|Mem32, Reg32,   0},
{"btr",    2, 0x0fb3, None, _C16|Modrm, Reg16|Mem16, Reg16,   0},
{"btr",    2, 0x0fb3, None, _C32|Modrm, Reg32|Mem32, Reg32,   0},
{"btr",    2, 0x0fba, 6,    _C16|Modrm, Reg16|Mem16, Imm8,    0},
{"btr",    2, 0x0fba, 6,    _C32|Modrm, Reg32|Mem32, Imm8,    0},
{"bts",    2, 0x0fab, None, _C16|Modrm, Reg16|Mem16, Reg16,   0},
{"bts",    2, 0x0fab, None, _C32|Modrm, Reg32|Mem32, Reg32,   0},
{"bts",    2, 0x0fba, 5,    _C16|Modrm, Reg16|Mem16, Imm8,    0},
{"bts",    2, 0x0fba, 5,    _C32|Modrm, Reg32|Mem32, Imm8,    0},
{"call",   1, 0xe8,   None, Noth, DispXX, 0, 0},
{"call",   1, 0xff,   2,    _C16|Modrm, Reg16|Mem16, 0, 0},
{"call",   1, 0xff,   2,    _C32|Modrm, Reg32|Mem32, 0, 0},
{"callf",  1, 0xff,   3,    Modrm, Reg32|Mem32, 0, 0},
{"callf",  2, 0x9a,   None, Noth, ImmXX, Imm16, 0},
{"cbw",    0, 0x98,   None, _C16, 0, 0, 0},
{"cdq",    0, 0x99,   None, _C32, 0, 0, 0},
{"clc",    0, 0xf8,   None, Noth, 0, 0, 0},
{"cld",    0, 0xfc,   None, Noth, 0, 0, 0},
{"cli",    0, 0xfa,   None, Noth, 0, 0, 0},
{"clts",   0, 0x0f06, None, Noth, 0, 0, 0},
{"cmc",    0, 0xf5,   None, Noth, 0, 0, 0},
{"cmp",    2, 0x38,   None, D|Modrm, Reg8|Mem8, Reg8, 0},
{"cmp",    2, 0x39,   None, _C16|D|Modrm, Reg16|Mem16, Reg16, 0},
{"cmp",    2, 0x39,   None, _C32|D|Modrm, Reg32|Mem32, Reg32, 0},
{"cmp",    2, 0x3c,   None, Noth, Acc8, Imm8, 0},
{"cmp",    2, 0x3d,   None, _C16, Acc16, Imm16, 0},
{"cmp",    2, 0x3d,   None, _C32, Acc32, Imm32, 0},
{"cmp",    2, 0x80,   7,    Modrm, Reg8|Mem8,  Imm8, 0},
{"cmp",    2, 0x81,   7,    _C16|Modrm, Reg16|Mem16, Imm16, 0},
{"cmp",    2, 0x81,   7,    _C32|Modrm, Reg32|Mem32, Imm32, 0},
{"cmpsb",  0, 0xa6,   None, Noth, 0, 0, 0},
{"cmpsd",  0, 0xa7,   None, _C32, 0, 0, 0},
{"cmpsw",  0, 0xa7,   None, _C16, 0, 0, 0},
{"cwd",    0, 0x99,   None, _C16, 0, 0, 0},
{"cwde",   0, 0x98,   None, _C32, 0, 0, 0},
{"daa",    0, 0x27,   None, Noth, 0, 0, 0},
{"das",    0, 0x2f,   None, Noth, 0, 0, 0},
{"dec",    1, 0x48,   None, _C16|ShortForm, Reg16,       0, 0},
{"dec",    1, 0x48,   None, _C32|ShortForm, Reg32,       0, 0},
{"dec",    1, 0xfe,   1,    Modrm,          Reg8|Mem8,   0, 0},
{"dec",    1, 0xff,   1,    _C16|Modrm,     Reg16|Mem16, 0, 0},
{"dec",    1, 0xff,   1,    _C32|Modrm,     Reg32|Mem32, 0, 0},
{"div",    1, 0xf6,   6,    Modrm,      Reg8|Mem8,   0, 0},
{"div",    1, 0xf7,   6,    _C16|Modrm, Reg16|Mem16, 0, 0},
{"div",    1, 0xf7,   6,    _C32|Modrm, Reg32|Mem32, 0, 0},
{"enter",  2, 0xc8,   None, Noth, Imm16, Imm8, 0},
{"hlt",    0, 0xf4,   None, Noth, 0, 0, 0},
{"idiv",   1, 0xf6,   7,    Modrm,      Reg8|Mem8,   0, 0},
{"idiv",   1, 0xf7,   7,    _C16|Modrm, Reg16|Mem16, 0, 0},
{"idiv",   1, 0xf7,   7,    _C32|Modrm, Reg32|Mem32, 0, 0},
{"imul",   1, 0xf6,   5,    Modrm,      Reg8|Mem8,   0, 0},
{"imul",   1, 0xf7,   5,    _C16|Modrm, Reg16|Mem16, 0, 0},
{"imul",   1, 0xf7,   5,    _C32|Modrm, Reg32|Mem32, 0, 0},
{"imul",   2, 0x0faf, None, _C16|Modrm, Reg16, Reg16|Mem16, 0},
{"imul",   2, 0x0faf, None, _C32|Modrm, Reg32, Reg32|Mem32, 0},
{"imul",   3, 0x69,   None, _C16|Modrm, Reg16, Reg16|Mem16, Imm16},
{"imul",   3, 0x69,   None, _C32|Modrm, Reg32, Reg32|Mem32, Imm32},
{"in",     2, 0xe4,   None, Noth, Acc8, Imm8, 0 },
{"in",     2, 0xe5,   None, _C16, Acc16, Imm8, 0 },
{"in",     2, 0xe5,   None, _C32, Acc32, Imm8, 0 },
{"in",     2, 0xec,   None, Noth, Acc8,  InOutPortReg, 0 },
{"in",     2, 0xed,   None, _C16, Acc16, InOutPortReg, 0 },
{"in",     2, 0xed,   None, _C32, Acc32, InOutPortReg, 0 },
{"inc",    1, 0x40,   None, _C16|ShortForm, Reg16,       0, 0},
{"inc",    1, 0x40,   None, _C32|ShortForm, Reg32,       0, 0},
{"inc",    1, 0xfe,   0,    Modrm, Reg8|Mem8,   0, 0},
{"inc",    1, 0xff,   0,    _C16|Modrm, Reg16|Mem16, 0, 0},
{"inc",    1, 0xff,   0,    _C32|Modrm, Reg32|Mem32, 0, 0},
{"insb",   0, 0x6c,   None, Noth, 0, 0, 0},
{"insd",   0, 0x6d,   None, _C32, 0, 0, 0},
{"insw",   0, 0x6d,   None, _C16, 0, 0, 0},
{"int",    1, 0xcd,   None, Noth, Imm8, 0, 0},
{"int3",   0, 0xcc,   None, Noth, 0, 0, 0},
{"into",   0, 0xce,   None, Noth, 0, 0, 0},
{"iret",   0, 0xcf,   None, Noth, 0, 0, 0},
{"ja",     1, 0x77,   None, Noth, Disp8, 0, 0},
{"jae",    1, 0x73,   None, Noth, Disp8, 0, 0},
{"jb",     1, 0x72,   None, Noth, Disp8, 0, 0},
{"jbe",    1, 0x76,   None, Noth, Disp8, 0, 0},
{"jc",     1, 0x72,   None, Noth, Disp8, 0, 0},
{"jcxz",   1, 0x67e3, None, Noth, Disp8, 0, 0},
{"je",     1, 0x74,   None, Noth, Disp8, 0, 0},
{"jecxz",  1, 0xe3,   None, Noth, Disp8, 0, 0},
{"jg",     1, 0x7f,   None, Noth, Disp8, 0, 0},
{"jge",    1, 0x7d,   None, Noth, Disp8, 0, 0},
{"jl",     1, 0x7c,   None, Noth, Disp8, 0, 0},
{"jle",    1, 0x7e,   None, Noth, Disp8, 0, 0},
{"jmp",    1, 0xe9,   None, Noth, DispXX, 0, 0},
{"jmp",    1, 0xff,   4,    _C16|Modrm,Reg16|Mem16, 0, 0},
{"jmp",    1, 0xff,   4,    _C32|Modrm,Reg32|Mem32, 0, 0},
{"jmpf",   1, 0xff,   5,    Modrm,Reg32|Mem32, 0, 0},
{"jmpf",   2, 0xea,   None, Noth, ImmXX,  Imm16, 0},
{"jmps",   1, 0xeb,   None, Noth, Disp8,  0, 0},
{"jna",    1, 0x76,   None, Noth, Disp8,  0, 0},
{"jnae",   1, 0x72,   None, Noth, Disp8,  0, 0},
{"jnb",    1, 0x73,   None, Noth, Disp8,  0, 0},
{"jnbe",   1, 0x77,   None, Noth, Disp8,  0, 0},
{"jnc",    1, 0x73,   None, Noth, Disp8,  0, 0},
{"jne",    1, 0x75,   None, Noth, Disp8,  0, 0},
{"jng",    1, 0x7e,   None, Noth, Disp8,  0, 0},
{"jnge",   1, 0x7c,   None, Noth, Disp8,  0, 0},
{"jnl",    1, 0x7d,   None, Noth, Disp8,  0, 0},
{"jnle",   1, 0x7f,   None, Noth, Disp8,  0, 0},
{"jno",    1, 0x71,   None, Noth, Disp8,  0, 0},
{"jnp",    1, 0x7b,   None, Noth, Disp8,  0, 0},
{"jns",    1, 0x79,   None, Noth, Disp8,  0, 0},
{"jnz",    1, 0x75,   None, Noth, Disp8,  0, 0},
{"jo",     1, 0x70,   None, Noth, Disp8,  0, 0},
{"jp",     1, 0x7a,   None, Noth, Disp8,  0, 0},
{"jpe",    1, 0x7a,   None, Noth, Disp8,  0, 0},
{"jpo",    1, 0x7b,   None, Noth, Disp8,  0, 0},
{"js",     1, 0x78,   None, Noth, Disp8,  0, 0},
{"jz",     1, 0x74,   None, Noth, Disp8,  0, 0},
{"lahf",   0, 0x9f,   None, Noth, 0, 0, 0},
{"lds",    2, 0xc5,   None, _C16|Modrm, Reg16, Mem, 0},
{"lds",    2, 0xc5,   None, _C32|Modrm, Reg32, Mem, 0},
{"lea",    2, 0x8d,   None, _C16|Modrm, Reg16, Mem, 0 },
{"lea",    2, 0x8d,   None, _C32|Modrm, Reg32, Mem, 0 },
{"leave",  0, 0xc9,   None, Noth, 0,     0,    0},
{"les",    2, 0xc4,   None, _C16|Modrm, Reg16, Mem, 0},
{"les",    2, 0xc4,   None, _C32|Modrm, Reg32, Mem, 0},
{"lfs",    2, 0x0fb4, None, _C16|Modrm, Reg16, Mem, 0},
{"lfs",    2, 0x0fb4, None, _C32|Modrm, Reg32, Mem, 0},
{"lgdt",   1, 0x0f01, 2, Modrm, Mem, 0, 0},
{"lgs",    2, 0x0fb5, None, _C16|Modrm, Reg16, Mem, 0},
{"lgs",    2, 0x0fb5, None, _C32|Modrm, Reg32, Mem, 0},
{"lidt",   1, 0x0f01, 3,    Modrm, Mem, 0, 0},
{"lldt",   1, 0x0f00, 2,    Modrm, Reg16|Mem, 0, 0},
{"lmsw",   1, 0x0f01, 6,    Modrm, Reg16|Mem, 0, 0},
{"lodsb",  0, 0xac,   None, Noth, 0, 0, 0},
{"lodsd",  0, 0xad,   None, _C32, 0, 0, 0},
{"lodsw",  0, 0xad,   None, _C16, 0, 0, 0},
{"loop",   1, 0xe2,   None, Noth, Disp8, 0, 0},
{"loope",  1, 0xe1,   None, Noth, Disp8, 0, 0},
{"loopne", 1, 0xe0,   None, Noth, Disp8, 0, 0},
{"loopnz", 1, 0xe0,   None, Noth, Disp8, 0, 0},
{"loopz",  1, 0xe1,   None, Noth, Disp8, 0, 0},
{"lss",    2, 0x0fb2, None, _C16|Modrm, Reg16, Mem, 0},
{"lss",    2, 0x0fb2, None, _C32|Modrm, Reg32, Mem, 0},
{"ltr",    1, 0x0f00, 3,    Modrm, Reg16|Mem, 0, 0},
{"mov",    2, 0x0f20, None, D|Modrm, Reg32|MemReg, Control, 0},
{"mov",    2, 0x0f21, None, D|Modrm, Reg32|MemReg, Debug, 0},
{"mov",    2, 0x0f24, None, D|Modrm, Reg32|MemReg, Test, 0},
{"mov",    2, 0x88,   None, Modrm|D,       Reg8|Mem8,   Reg8,       0 },
{"mov",    2, 0x89,   None, _C16|Modrm|D,  Reg16|Mem16, Reg16,      0 },
{"mov",    2, 0x89,   None, _C32|Modrm|D,  Reg32|Mem32, Reg32,      0 },
{"mov",    2, 0x8c,   None, D|Modrm,       Reg16|Mem16, SReg3|SReg2,0 },
{"mov",    2, 0xb0,   None, ShortForm,     Reg8,        Imm8,       0 },
{"mov",    2, 0xb8,   None, _C16|ShortForm,Reg16,       Imm16,      0 },
{"mov",    2, 0xb8,   None, _C32|ShortForm,Reg32,       Imm32,      0 },
{"mov",    2, 0xc6,   None, Modrm,         Mem8,        Imm8,       0 },
{"mov",    2, 0xc7,   None, _C16|Modrm,    Mem16,       Imm16,      0 },
{"mov",    2, 0xc7,   None, _C32|Modrm,    Mem32,       Imm32,      0 },
{"movm",   2, 0xa0,   None, D,             Acc8,        Imm16,      0 },
{"movm",   2, 0xa1,   None, _C16|D,        Acc16,       Imm16,      0 },
{"movm",   2, 0xa1,   None, _C32|D,        Acc32,       Imm32,      0 },
{"movsb",  0, 0xa4,   None, Noth, 0, 0, 0},
{"movsd",  0, 0xa5,   None, _C32, 0, 0, 0},
{"movsw",  0, 0xa5,   None, _C16, 0, 0, 0},
{"movsx",  2, 0x0fbe, None, _C32|Modrm,   Reg32, Reg8|Mem8,  0},
{"movsx",  2, 0x0fbf, None, _C32|Modrm,   Reg32, Reg16|Mem16,0},
{"movzx",  2, 0x0fb6, None, _C32|Modrm,   Reg32, Reg8|Mem8,  0},
{"movzx",  2, 0x0fb7, None, _C32|Modrm,   Reg32, Reg16|Mem16,0},
{"mul",    1, 0xf6,   4,    Modrm,      Reg8|Mem8,   0, 0},
{"mul",    1, 0xf7,   4,    _C16|Modrm, Reg16|Mem16, 0, 0},
{"mul",    1, 0xf7,   4,    _C32|Modrm, Reg32|Mem32, 0, 0},
{"neg",    1, 0xf6,   3,    Modrm,          Reg8|Mem8,   0, 0},
{"neg",    1, 0xf7,   3,    _C16|Modrm,     Reg16|Mem16, 0, 0},
{"neg",    1, 0xf7,   3,    _C32|Modrm,     Reg32|Mem32, 0, 0},
{"nop",    0, 0x90,   None, Noth, 0, 0, 0},
{"not",    1, 0xf6,   2,    Modrm,          Reg8|Mem8,   0, 0},
{"not",    1, 0xf7,   2,    _C16|Modrm,     Reg16|Mem16, 0, 0},
{"not",    1, 0xf7,   2,    _C32|Modrm,     Reg32|Mem32, 0, 0},
{"or",     2, 0x08,   None, D|Modrm,      Reg8|Mem8,   Reg8,  0},
{"or",     2, 0x09,   None, _C16|D|Modrm, Reg16|Mem16, Reg16, 0},
{"or",     2, 0x09,   None, _C32|D|Modrm, Reg32|Mem32, Reg32, 0},
{"or",     2, 0x0c,   None, Noth,         Acc8,        Imm8,  0},
{"or",     2, 0x0d,   None, _C16,         Acc16,       Imm16, 0},
{"or",     2, 0x0d,   None, _C32,         Acc32,       Imm32, 0},
{"or",     2, 0x80,   1,    Modrm,        Reg8|Mem8,   Imm8,  0},
{"or",     2, 0x81,   1,    _C16|Modrm,   Reg16|Mem16, Imm16, 0},
{"or",     2, 0x81,   1,    _C32|Modrm,   Reg32|Mem32, Imm32, 0},
{"out",    2, 0xe6,   None, Noth, Imm8,         Acc8,         0 },
{"out",    2, 0xe7,   None, _C16, Imm8,         Acc16,        0 },
{"out",    2, 0xe7,   None, _C32, Imm8,         Acc32,        0 },
{"out",    2, 0xee,   None, Noth, InOutPortReg, Acc8,         0 },
{"out",    2, 0xef,   None, _C16, InOutPortReg, Acc16,        0 },
{"out",    2, 0xef,   None, _C32, InOutPortReg, Acc32,        0 },
{"outsb",  0, 0x6e,   None, Noth, 0, 0, 0},
{"outsd",  0, 0x6f,   None, _C32, 0, 0, 0},
{"outsw",  0, 0x6f,   None, _C16, 0, 0, 0},
{"pop",    1, 0x07,   None, Seg2ShortForm,  SReg2,       0, 0 },
{"pop",    1, 0x0fa1, None, Seg3ShortForm,  SReg3,       0, 0 },
{"pop",    1, 0x58,   None, _C16|ShortForm, Reg16,       0, 0 },
{"pop",    1, 0x58,   None, _C32|ShortForm, Reg32,       0, 0 },
{"pop",    1, 0x8f,   0x0,  _C16|Modrm,     Reg16|Mem16, 0, 0 },
{"pop",    1, 0x8f,   0x0,  _C32|Modrm,     Reg32|Mem32, 0, 0 },
{"popa",   0, 0x61,   None, Noth,           0,           0, 0 },
{"popf",   0, 0x9d,   None, Noth, 0, 0, 0},
{"push",   1, 0x06,   None, Seg2ShortForm,  SReg2,       0, 0 },
{"push",   1, 0x0fa0, None, Seg3ShortForm,  SReg3,       0, 0 },
{"push",   1, 0x50,   None, _C16|ShortForm, Reg16,       0, 0 },
{"push",   1, 0x50,   None, _C32|ShortForm, Reg32,       0, 0 },
{"push",   1, 0x68,   None, Noth,           ImmXX,       0, 0 },
{"push",   1, 0xff,   0x6,  _C16|Modrm,     Reg16|Mem16, 0, 0 },
{"push",   1, 0xff,   0x6,  _C32|Modrm,     Reg32|Mem32, 0, 0 },
{"pusha",  0, 0x60,   None, Noth,           0,           0, 0 },
{"pushd",  1, 0x68,   None, _C32,           Imm32,       0, 0 },
{"pushf",  0, 0x9c,   None, Noth, 0, 0, 0},
{"pushw",  1, 0x68,   None, _C16,           Imm16,       0, 0 },
{"rcl",    1, 0xd0,   2,    Modrm,      Reg8|Mem8,   0,    0},
{"rcl",    1, 0xd1,   2,    _C16|Modrm, Reg16|Mem16, 0,    0},
{"rcl",    1, 0xd1,   2,    _C32|Modrm, Reg32|Mem32, 0,    0},
{"rcl",    2, 0xc0,   2,    Modrm,      Reg8|Mem8,   Imm8, 0},
{"rcl",    2, 0xc1,   2,    _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"rcl",    2, 0xc1,   2,    _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"rcl",    2, 0xd2,   2,    Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"rcl",    2, 0xd3,   2,    _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"rcl",    2, 0xd3,   2,    _C32|Modrm, Reg32|Mem32, ShiftCount, 0},
{"rcr",    1, 0xd0,   3,    Modrm,      Reg8|Mem8,   0,    0},
{"rcr",    1, 0xd1,   3,    _C16|Modrm, Reg16|Mem16, 0,    0},
{"rcr",    1, 0xd1,   3,    _C32|Modrm, Reg32|Mem32, 0,    0},
{"rcr",    2, 0xc0,   3,    Modrm,      Reg8|Mem8,   Imm8, 0},
{"rcr",    2, 0xc1,   3,    _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"rcr",    2, 0xc1,   3,    _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"rcr",    2, 0xd2,   3,    Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"rcr",    2, 0xd3,   3,    _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"rcr",    2, 0xd3,   3,    _C32|Modrm, Reg32|Mem32, ShiftCount, 0},
{"ret",    0, 0xc3,   None, Noth, 0,     0,    0},
{"ret",    1, 0xc2,   None, Noth, Imm16, 0,    0},
{"retf",   0, 0xcb,   None, Noth, 0,     0,    0},
{"retf",   1, 0xca,   None, Noth, Imm16, 0,    0},
{"rol",    1, 0xd0,   0,    Modrm,      Reg8|Mem8,   0,    0},
{"rol",    1, 0xd1,   0,    _C16|Modrm, Reg16|Mem16, 0,    0},
{"rol",    1, 0xd1,   0,    _C32|Modrm, Reg32|Mem32, 0,    0},
{"rol",    2, 0xc0,   0,    Modrm,      Reg8|Mem8,   Imm8, 0},
{"rol",    2, 0xc1,   0,    _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"rol",    2, 0xc1,   0,    _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"rol",    2, 0xd2,   0,    Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"rol",    2, 0xd3,   0,    _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"rol",    2, 0xd3,   0,    _C32|Modrm, Reg32|Mem32, ShiftCount, 0},
{"ror",    1, 0xd0,   1,    Modrm,      Reg8|Mem8,   0,    0},
{"ror",    1, 0xd1,   1,    _C16|Modrm, Reg16|Mem16, 0,    0},
{"ror",    1, 0xd1,   1,    _C32|Modrm, Reg32|Mem32, 0,    0},
{"ror",    2, 0xc0,   1,    Modrm,      Reg8|Mem8,   Imm8, 0},
{"ror",    2, 0xc1,   1,    _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"ror",    2, 0xc1,   1,    _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"ror",    2, 0xd2,   1,    Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"ror",    2, 0xd3,   1,    _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"ror",    2, 0xd3,   1,    _C32|Modrm, Reg32|Mem32, ShiftCount, 0},
{"sahf",   0, 0x9e,   None, Noth, 0, 0, 0},
{"sal",    1, 0xd0,   4,    Modrm,      Reg8|Mem8,   0,    0},
{"sal",    1, 0xd1,   4,    _C16|Modrm, Reg16|Mem16, 0,    0},
{"sal",    1, 0xd1,   4,    _C32|Modrm, Reg32|Mem32, 0,    0},
{"sal",    2, 0xc0,   4,    Modrm,      Reg8|Mem8,   Imm8, 0},
{"sal",    2, 0xc1,   4,    _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"sal",    2, 0xc1,   4,    _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"sal",    2, 0xd2,   4,    Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"sal",    2, 0xd3,   4,    _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"sal",    2, 0xd3,   4,    _C32|Modrm, Reg32|Mem32, ShiftCount, 0},
{"sar",    1, 0xd0,   7,    Modrm,      Reg8|Mem8,   0,    0},
{"sar",    1, 0xd1,   7,    _C16|Modrm, Reg16|Mem16, 0,    0},
{"sar",    1, 0xd1,   7,    _C32|Modrm, Reg32|Mem32, 0,    0},
{"sar",    2, 0xc0,   7,    Modrm,      Reg8|Mem8,   Imm8, 0},
{"sar",    2, 0xc1,   7,    _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"sar",    2, 0xc1,   7,    _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"sar",    2, 0xd2,   7,    Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"sar",    2, 0xd3,   7,    _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"sar",    2, 0xd3,   7,    _C32|Modrm, Reg32|Mem32, ShiftCount, 0},
{"sbb",    2, 0x18,   None, D|Modrm,       Reg8|Mem8,   Reg8,  0},
{"sbb",    2, 0x19,   None, _C16|D|Modrm,  Reg16|Mem16, Reg16, 0},
{"sbb",    2, 0x19,   None, _C32|D|Modrm,  Reg32|Mem32, Reg32, 0},
{"sbb",    2, 0x1c,   None, Noth,          Acc8,        Imm8,  0},
{"sbb",    2, 0x1d,   None, _C16,          Acc16,       Imm16, 0},
{"sbb",    2, 0x1d,   None, _C32,          Acc32,       Imm32, 0},
{"sbb",    2, 0x80,   3,    Modrm,         Reg8|Mem8,   Imm8,  0},
{"sbb",    2, 0x81,   3,    _C16|Modrm,    Reg16|Mem16, Imm16, 0},
{"sbb",    2, 0x81,   3,    _C32|Modrm,    Reg32|Mem32, Imm32, 0},
{"scasb",  0, 0xae,   None, Noth, 0, 0, 0},
{"scasd",  0, 0xaf,   None, _C32, 0, 0, 0},
{"scasw",  0, 0xaf,   None, _C16, 0, 0, 0},
{"seta",   1, 0x0f97, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setae",  1, 0x0f93, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setb",   1, 0x0f92, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setbe",  1, 0x0f96, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setc",   1, 0x0f92, 0,    Modrm, Reg8|Mem8, 0, 0},
{"sete",   1, 0x0f94, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setg",   1, 0x0f9f, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setge",  1, 0x0f9d, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setl",   1, 0x0f9c, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setle",  1, 0x0f9e, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setna",  1, 0x0f96, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setnae", 1, 0x0f92, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setnb",  1, 0x0f93, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setnbe", 1, 0x0f97, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setnc",  1, 0x0f93, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setne",  1, 0x0f95, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setng",  1, 0x0f9e, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setnge", 1, 0x0f9c, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setnl",  1, 0x0f9d, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setnle", 1, 0x0f9f, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setno",  1, 0x0f91, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setnp",  1, 0x0f9b, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setns",  1, 0x0f99, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setnz",  1, 0x0f95, 0,    Modrm, Reg8|Mem8, 0, 0},
{"seto",   1, 0x0f90, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setp",   1, 0x0f9a, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setpe",  1, 0x0f9a, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setpo",  1, 0x0f9b, 0,    Modrm, Reg8|Mem8, 0, 0},
{"sets",   1, 0x0f98, 0,    Modrm, Reg8|Mem8, 0, 0},
{"setz",   1, 0x0f94, 0,    Modrm, Reg8|Mem8, 0, 0},
{"sgdt",   1, 0x0f01, 0,    Modrm, Mem, 0, 0},
{"shl",    1, 0xd0,   4,    Modrm,      Reg8|Mem8,   0,    0},
{"shl",    1, 0xd1,   4,    _C16|Modrm, Reg16|Mem16, 0,    0},
{"shl",    1, 0xd1,   4,    _C32|Modrm, Reg32|Mem32, 0,    0},
{"shl",    2, 0xc0,   4,    Modrm,      Reg8|Mem8,   Imm8, 0},
{"shl",    2, 0xc1,   4,    _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"shl",    2, 0xc1,   4,    _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"shl",    2, 0xd2,   4,    Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"shl",    2, 0xd3,   4,    _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"shl",    2, 0xd3,   4,    _C32|Modrm, Reg32|Mem32, ShiftCount, 0},
{"shr",    1, 0xd0,   5,    Modrm,      Reg8|Mem8,   0,    0},
{"shr",    1, 0xd1,   5,    _C16|Modrm, Reg16|Mem16, 0,    0},
{"shr",    1, 0xd1,   5,    _C32|Modrm, Reg32|Mem32, 0,    0},
{"shr",    2, 0xc0,   5,    Modrm,      Reg8|Mem8,   Imm8, 0},
{"shr",    2, 0xc1,   5,    _C16|Modrm, Reg16|Mem16, Imm8, 0},
{"shr",    2, 0xc1,   5,    _C32|Modrm, Reg32|Mem32, Imm8, 0},
{"shr",    2, 0xd2,   5,    Modrm,      Reg8|Mem8,   ShiftCount, 0},
{"shr",    2, 0xd3,   5,    _C16|Modrm, Reg16|Mem16, ShiftCount, 0},
{"shr",    2, 0xd3,   5,    _C32|Modrm, Reg32|Mem32, ShiftCount, 0},
{"sidt",   1, 0x0f01, 1,    Modrm, Mem, 0, 0},
{"sldt",   1, 0x0f00, 0,    Modrm, Reg16|Mem, 0, 0},
{"smsw",   1, 0x0f01, 4,    Modrm, Reg16|Mem, 0, 0},
{"stc",    0, 0xf9,   None, Noth, 0, 0, 0},
{"std",    0, 0xfd,   None, Noth, 0, 0, 0},
{"sti",    0, 0xfb,   None, Noth, 0, 0, 0},
{"stosb",  0, 0xaa,   None, Noth, 0, 0, 0},
{"stosd",  0, 0xab,   None, _C32, 0, 0, 0},
{"stosw",  0, 0xab,   None, _C16, 0, 0, 0},
{"str",    1, 0x0f00, 1,    Modrm, Reg16|Mem, 0, 0},
{"sub",    2, 0x28,   None, D|Modrm,       Reg8|Mem8,   Reg8,  0},
{"sub",    2, 0x29,   None, _C16|D|Modrm,  Reg16|Mem16, Reg16, 0},
{"sub",    2, 0x29,   None, _C32|D|Modrm,  Reg32|Mem32, Reg32, 0},
{"sub",    2, 0x2c,   None, Noth,          Acc8,        Imm8,  0},
{"sub",    2, 0x2d,   None, _C16,          Acc16,       Imm16, 0},
{"sub",    2, 0x2d,   None, _C32,          Acc32,       Imm32, 0},
{"sub",    2, 0x80,   5,    Modrm,         Reg8|Mem8,   Imm8,  0},
{"sub",    2, 0x81,   5,    _C16|Modrm,    Reg16|Mem16, Imm16, 0},
{"sub",    2, 0x81,   5,    _C32|Modrm,    Reg32|Mem32, Imm32, 0},
{"test",   2, 0x84,   None, Modrm,      Reg8|Mem8,   Reg8,  0},
{"test",   2, 0x85,   None, _C16|Modrm, Reg16|Mem16, Reg16, 0},
{"test",   2, 0x85,   None, _C32|Modrm, Reg32|Mem32, Reg32, 0},
{"test",   2, 0xa8,   None, Noth,       Acc8,        Imm8,  0},
{"test",   2, 0xa9,   None, _C16,       Acc16,       Imm16, 0},
{"test",   2, 0xa9,   None, _C32,       Acc32,       Imm32, 0},
{"test",   2, 0xf6,   0,    Modrm,      Reg8|Mem8,   Imm8,  0},
{"test",   2, 0xf7,   0,    _C16|Modrm, Reg16|Mem16, Imm16, 0},
{"test",   2, 0xf7,   0,    _C32|Modrm, Reg32|Mem32, Imm32, 0},
{"verr",   1, 0x0f00, 4,    Modrm, Reg16|Mem, 0, 0},
{"verw",   1, 0x0f00, 5,    Modrm, Reg16|Mem, 0, 0},
{"wait",   0, 0x9b,   None, Noth, 0, 0, 0},
{"xchg",   2, 0x86,   None, Modrm,      Reg8,  Reg8|Mem8,   0 },
{"xchg",   2, 0x87,   None, _C16|Modrm, Reg16, Reg16|Mem16, 0 },
{"xchg",   2, 0x87,   None, _C32|Modrm, Reg32, Reg32|Mem32, 0 },
{"xchg",   2, 0x90,   None, ShortForm,  Acc32, Reg32,       0 },
{"xlat",   0, 0xd7,   None, Noth, 0, 0, 0},
{"xor",    2, 0x30,   None, D|Modrm,      Reg8|Mem8,   Reg8,  0},
{"xor",    2, 0x31,   None, _C16|D|Modrm, Reg16|Mem16, Reg16, 0},
{"xor",    2, 0x31,   None, _C32|D|Modrm, Reg32|Mem32, Reg32, 0},
{"xor",    2, 0x34,   None, Noth,         Acc8,        Imm8,  0},
{"xor",    2, 0x35,   None, _C16,         Acc16,       Imm16, 0},
{"xor",    2, 0x35,   None, _C32,         Acc32,       Imm32, 0},
{"xor",    2, 0x80,   6,    Modrm,        Reg8|Mem8,   Imm8,  0},
{"xor",    2, 0x81,   6,    _C16|Modrm,   Reg16|Mem16, Imm16, 0},
{"xor",    2, 0x81,   6,    _C32|Modrm,   Reg32|Mem32, Imm32, 0},

// 486 extensions
{"bswap",  1, 0x0fc8, None, ShortForm, Reg32,   0,     0 },
{"invd",   0, 0x0f08, None, Noth,  0,           0,     0 },
{"wbinvd", 0, 0x0f09, None, Noth,  0,           0,     0 },

// Pentium and late-model 486 extensions
{"cpuid",  0, 0x0fa2, None, Noth, 0, 0, 0},

// Pentium extensions
{"wrmsr",  0, 0x0f30, None, Noth, 0, 0, 0},
{"rdtsc",  0, 0x0f31, None, Noth, 0, 0, 0},
{"rdmsr",  0, 0x0f32, None, Noth, 0, 0, 0},

// FPU
{"f2xm1",  0, 0xd9f0, None, Noth, 0, 0, 0}, // st0 = 2^st0 - 1
{"fabs",   0, 0xd9e1, None, Noth, 0, 0, 0}, // st0 = |st0|
{"fadd",   1, 0xd8,   0,    Modrm, Mem32, 0, 0}, // st0 += mem32
{"fadd",   1, 0xdc,   0,    Modrm, MemFL, 0, 0}, // st0 += mem64
{"fadd",   0, 0xdcc1, None, Noth, 0, 0, 0}, // st0 += st1
{"fadd",   1, 0xd8c0, None, ShortForm, FloatReg, 0, 0}, // st0 += stX
{"fadd",   2, 0xd8c0, None, ShortForm, FloatAcc, FloatReg, 0}, // st0 += stX
{"fadd",   2, 0xdcc0, None, ShortForm, FloatReg, FloatAcc, 0}, // stX += st0
{"faddp",  0, 0xdec1, None, Noth, 0, 0, 0}, // st0 += st1 ; pop
{"faddp",  1, 0xdec0, None, ShortForm, FloatReg, 0, 0}, // st0 += stX ; pop
{"fiadd",  1, 0xde,   0,    Modrm, Mem16, 0, 0}, // st0 += m16int
{"fiadd",  1, 0xda,   0,    Modrm, Mem32, 0, 0}, // st0 += m32int
{"fbld",   1, 0xdf,   4,    Modrm, Mem, 0, 0}, // push ; st0 = bcd
{"fbstp",  1, 0xdf,   6,    Modrm, Mem, 0, 0}, // bcd = st0 ; pop
{"fchs",   0, 0xd9e0, None, Noth,  0, 0, 0}, // st0 = -st0;
{"fclex",  0, 0x9bdbe2,None,Noth,  0, 0, 0}, // fwait ; sw[0..7]=sw[15]=0
{"fnclex", 0, 0xdbe2, None, Noth,  0, 0, 0}, // sw[0..7]=sw[15]=0
{"fcmovb", 2, 0xdac0, None, ShortForm, FloatAcc, FloatReg, 0}, // < : st0=stX
{"fcmove", 2, 0xdac8, None, ShortForm, FloatAcc, FloatReg, 0}, // = : st0=stX
{"fcmovbe",2, 0xdad0, None, ShortForm, FloatAcc, FloatReg, 0}, // <=: st0=stX
{"fcmovu", 2, 0xdad8, None, ShortForm, FloatAcc, FloatReg, 0}, // PF: st0=stX
{"fcmovnb",2, 0xdbc0, None, ShortForm, FloatAcc, FloatReg, 0}, // >=: st0=stX
{"fcmovne",2, 0xdbc8, None, ShortForm, FloatAcc, FloatReg, 0}, // !=: st0=stX
{"fcmovnbe",2,0xdbd0, None, ShortForm, FloatAcc, FloatReg, 0}, // > : st0=stX
{"fcmovnu",2, 0xdbd8, None, ShortForm, FloatAcc, FloatReg, 0}, // PF=0: --"--
{"fcom",   1, 0xd8,   2,    Modrm, Mem32, 0, 0}, // cmp st0,m64real
{"fcom",   1, 0xdc,   2,    Modrm, MemFL, 0, 0}, // cmp st0,m64real
{"fcom",   1, 0xd8d0, None, ShortForm, FloatReg, 0, 0}, // cmp st0,stX
{"fcom",   0, 0xd8d1, None, Noth, 0, 0, 0},      // cmp st0,st1
{"fcomp",  1, 0xd8,   3,    Modrm, Mem32, 0, 0}, // cmp st0,m32real ; pop
{"fcomp",  1, 0xdc,   3,    Modrm, MemFL, 0, 0}, // cmp st0,m64real ; pop
{"fcomp",  1, 0xd8d8, None, ShortForm, FloatReg, 0, 0}, // cmp st0,stX ; pop
{"fcomp",  0, 0xd8d9, None, Noth, 0, 0, 0}, // cmp st0,st1 ; pop
{"fcompp", 0, 0xded9, None, Noth, 0, 0, 0}, // cmp st0,st1 ; pop ; pop
{"fcomi",  2, 0xdbf0, None, ShortForm, FloatAcc, FloatReg, 0}, // ?PPro? cmp st0,stX
{"fcomip", 2, 0xdff0, None, ShortForm, FloatAcc, FloatReg, 0}, // ?PPro? cmp st0,stX ; pop
{"fucomi", 2, 0xdbe8, None, ShortForm, FloatAcc, FloatReg, 0}, // ?PPro? cmp st0,stX
{"fucomip",2, 0xdfe8, None, ShortForm, FloatAcc, FloatReg, 0}, // ?PPro? cmp st0,stX ; pop
{"fcos",   0, 0xd9ff, None, Noth, 0, 0, 0}, // ?387? st0 = cos(st0)
{"fdecstp",0, 0xd9f6, None, Noth, 0, 0, 0}, // sw.top--
{"fdisi",  0, 0xdbe1, None, Noth, 0, 0, 0}, // sw.iem = 1 (8087 only)
{"fdiv",   1, 0xd8,   6,    Modrm, Mem32, 0, 0}, // st0 /=m32real
{"fdiv",   1, 0xdc,   6,    Modrm, MemFL, 0, 0}, // st0 /=m64real
{"fdiv",   2, 0xd8f0, None, ShortForm, FloatAcc, FloatReg, 0}, // st0 /= stX
{"fdiv",   2, 0xdcf8, None, ShortForm, FloatReg, FloatAcc, 0}, // stX /= st0
{"fdivp",  2, 0xdef8, None, ShortForm, FloatReg, FloatAcc, 0}, // stX /= st0 ; pop
{"fdivp",  0, 0xdef9, None, Noth, 0, 0, 0}, // st1 /= st0 ; pop
{"fidiv",  1, 0xde,   6,    Modrm, Mem16, 0, 0}, // st0 /= m16int
{"fidiv",  1, 0xda,   6,    Modrm, Mem32, 0, 0}, // st0 /= m32int
{"fdivr",  1, 0xd8,   7,    Modrm, Mem32, 0, 0}, // st0 = m32real/st0
{"fdivr",  1, 0xdc,   7,    Modrm, MemFL, 0, 0}, // st0 = m64real/st0
{"fdivr",  2, 0xd8f8, None, ShortForm, FloatAcc, FloatReg, 0}, // st0 = stX/st0
{"fdivr",  2, 0xdcf0, None, ShortForm, FloatReg, FloatAcc, 0}, // stX = st0/stX
{"fdivrp", 2, 0xdef0, None, ShortForm, FloatReg, FloatAcc, 0}, // stX = st0/stX ; pop
{"fdivrp", 0, 0xdef1, None, Noth, 0, 0, 0}, // st1 = st0/st1 ; pop
{"fidivr", 1, 0xde,   7,    Modrm, Mem16, 0, 0}, // st0 = m16int/st0
{"fidivr", 1, 0xda,   7,    Modrm, Mem32, 0, 0}, // st0 = m32int/st0
{"feni",   0, 0xdbe0, None, Noth, 0, 0, 0}, // sw.iem = 0 (8087 only)
{"ffree",  1, 0xddc0, None, ShortForm, FloatReg, 0, 0}, // TAG(X) = 11b (free)
{"ficom",  1, 0xde,   2,    Modrm, Mem16, 0, 0}, // cmp st0, m16int
{"ficom",  1, 0xda,   2,    Modrm, Mem32, 0, 0}, // cmp st0, m32int
{"ficomp", 1, 0xde,   3,    Modrm, Mem16, 0, 0}, // cmp st0, m16int ; pop
{"ficomp", 1, 0xda,   3,    Modrm, Mem32, 0, 0}, // cmp st0, m32int ; pop
{"fild",   1, 0xdf,   0,    Modrm, Mem16, 0, 0}, // push ; st0 = m16int
{"fild",   1, 0xdb,   0,    Modrm, Mem32, 0, 0}, // push ; st0 = m32int
{"fild",   1, 0xdf,   5,    Modrm, MemFL, 0, 0}, // push ; st0 = m64int
{"fincstp",0, 0xd9f7, None, Noth,  0, 0, 0}, // sw.top++
{"finit",  0, 0x9bdbe3,None,Noth,  0, 0, 0}, // init FPU
{"fninit", 0, 0xdbe3, None, Noth,  0, 0, 0}, // fwait ; init FPU
{"fist",   1, 0xdf,   2,    Modrm, Mem16, 0, 0}, // m16int = st0
{"fist",   1, 0xdb,   2,    Modrm, Mem32, 0, 0}, // m32int = st0
{"fistp",  1, 0xdf,   3,    Modrm, Mem16, 0, 0}, // m16int = st0 ; pop
{"fistp",  1, 0xdb,   3,    Modrm, Mem32, 0, 0}, // m32int = st0 ; pop
{"fistp",  1, 0xdf,   7,    Modrm, MemFL, 0, 0}, // m64int = st0 ; pop
{"fld",    1, 0xd9,   0,    Modrm, Mem32, 0, 0}, // push ; st0 = m32real
{"fld",    1, 0xdd,   0,    Modrm, MemFL, 0, 0}, // push ; st0 = m64real
{"fldt",   1, 0xdb,   5,    Modrm, Mem, 0, 0},   // push ; st0 = m80real
{"fld",    1, 0xd9c0, None, ShortForm, FloatReg, 0, 0}, // push ; st0 = stX
{"fld1",   0, 0xd9e8, None, Noth, 0, 0, 0}, // push ; st0 = 1.0
{"fldl2t", 0, 0xd9e9, None, Noth, 0, 0, 0}, // push ; st0 = log2(10)
{"fldl2e", 0, 0xd9ea, None, Noth, 0, 0, 0}, // push ; st0 = log2(e)
{"fldpi",  0, 0xd9eb, None, Noth, 0, 0, 0}, // push ; st0 = pi
{"fldlg2", 0, 0xd9ec, None, Noth, 0, 0, 0}, // push ; st0 = log10(2)
{"fldln2", 0, 0xd9ed, None, Noth, 0, 0, 0}, // push ; st0 = ln(2)
{"fldz",   0, 0xd9ee, None, Noth, 0, 0, 0}, // push ; st0 = 0.0
{"fldcw",  1, 0xd9,   5,    Modrm, Mem, 0, 0}, // cw = m2byte (16bit)
{"fldenv", 1, 0xd9,   4,    Modrm, Mem, 0, 0}, // cw, sw, tw, etc.. = m14/28byte
{"fmul",   1, 0xd8,   1,    Modrm, Mem32, 0, 0}, // st0 *= m32real
{"fmul",   1, 0xdc,   1,    Modrm, MemFL, 0, 0}, // st0 *= m64real
{"fmul",   1, 0xd8c0, None, ShortForm, FloatAcc, FloatReg, 0}, // st0 *= stX
{"fmul",   1, 0xdcc8, None, ShortForm, FloatReg, FloatAcc, 0}, // stX *= st0
{"fmulp",  1, 0xdec8, None, ShortForm, FloatReg, FloatAcc, 0}, // stX *= st0 ; pop
{"fmulp",  0, 0xdec9, None, Noth, 0, 0, 0}, // st1 *= st0 ; pop
{"fimul",  1, 0xde,   1,    Modrm, Mem16, 0, 0}, // st0 *= m16int
{"fimul",  1, 0xda,   1,    Modrm, Mem32, 0, 0}, // st0 *= m32int
{"fnop",   0, 0xd9d0, None, Noth, 0, 0, 0}, // nop
{"fpatan", 0, 0xd9f3, None, Noth,  0, 0, 0}, // st1 = arctg(st1/st0) ; pop
{"fprem",  0, 0xd9f8, None, Noth,  0, 0, 0}, // st0 = st0 % st1
{"fprem1", 0, 0xd9f5, None, Noth,  0, 0, 0}, // ?387? st0 = st0 % st1 (IEEE 754)
{"fptan",  0, 0xd9f2, None, Noth,  0, 0, 0}, // st0 = tg(st0) ; push(1.0)
{"frndint",0, 0xd9fc, None, Noth,  0, 0, 0}, // st0 = round(st0)
{"frstor", 1, 0xdd,   4,    Modrm, Mem, 0, 0}, // FPU = m94/108byte
{"fsave",  1, 0x9bdd, 6,    Modrm, Mem, 0, 0}, // fwait; m94/108byte = FPU
{"fnsave", 1, 0xdd,   6,    Modrm, Mem, 0, 0}, // m94/108byte = FPU
{"fscale", 0, 0xd9fd, None, Noth,  0, 0, 0}, // st0 *= 2^st1
{"fsetpm", 0, 0xdbe4, None, Noth,  0, 0, 0}, // switch FPU to PM (287 only)
{"fsin",   0, 0xd9fe, None, Noth,  0, 0, 0}, // ?387? st0 = sin(st0)
{"fsincos",0, 0xd9fb, None, Noth,  0, 0, 0}, // ?387? push (cos(st0));st1=sin(st1)
{"fsqrt",  0, 0xd9fa, None, Noth,  0, 0, 0}, // st0 = sqrt(st0)
{"fst",    1, 0xd9,   2,    Modrm, Mem32, 0, 0}, // m32real = st0
{"fst",    1, 0xdd,   2,    Modrm, MemFL, 0, 0}, // m64real = st0
{"fst",    1, 0xddd0, None, ShortForm, FloatReg, 0, 0}, // stX = st0
{"fstp",   1, 0xd9,   3,    Modrm, Mem32, 0, 0}, // m32real = st0 ; pop
{"fstp",   1, 0xdd,   3,    Modrm, MemFL, 0, 0}, // m64real = st0 ; pop
{"fstpt",  1, 0xdb,   7,    Modrm, Mem, 0, 0},   // m80real = st0 ; pop
{"fstp",   1, 0xddd8, None, ShortForm, FloatReg, 0, 0}, // stX = st0 ; pop
{"fstcw",  1, 0x9bd9, 7,    Modrm, Mem, 0, 0}, // fwait ; m2byte = cw
{"fnstcw", 1, 0xd9,   7,    Modrm, Mem, 0, 0}, // m2byte = cw
{"fstenv", 1, 0x9bd9, 6,    Modrm, Mem, 0, 0}, // fwait ; m14/28byte = FPU
{"fnstenv",1, 0xd9,   6,    Modrm, Mem, 0, 0}, // m14/28byte = FPU
{"fstsw",  1, 0x9bdd, 7,    Modrm, Mem, 0, 0}, // fwait ; m2byte = sw
{"fstsw",  1, 0x9bdfe0,None,Noth,  Acc16,0, 0}, // ax = sw
{"fnstsw", 1, 0xdd,   7,    Modrm, Mem, 0, 0}, // fwait ; m2byte = sw
{"fnstsw", 1, 0xdfe0, None, Noth,  Acc16, 0, 0},  // ax = sw
{"fsub",   1, 0xd8,   4,    Modrm, Mem32, 0, 0}, // st0 -= m32real
{"fsub",   1, 0xdc,   4,    Modrm, MemFL, 0, 0}, // st0 -= m64real
{"fsub",   2, 0xd8e0, None, ShortForm, FloatAcc, FloatReg, 0}, // st0 -= stX
{"fsub",   2, 0xdce8, None, ShortForm, FloatReg, FloatAcc, 0}, // stX -= st0
{"fsubp",  2, 0xdee8, None, ShortForm, FloatReg, FloatAcc, 0}, // stX -= st0 ;pop
{"fsubp",  0, 0xdee9, None, Noth, 0, 0, 0}, // st1 -= st0 ; pop
{"fisub",  1, 0xde,   4,    Modrm, Mem16, 0, 0}, // st0 -= m16int
{"fisub",  1, 0xda,   4,    Modrm, Mem32, 0, 0}, // st0 -= m32int
{"fsubr",  1, 0xd8,   5,    Modrm, Mem32, 0, 0}, // st0 = m32real - st0
{"fsubr",  1, 0xdc,   5,    Modrm, MemFL, 0, 0}, // st0 = m64real - st0
{"fsubr",  2, 0xd8e8, None, ShortForm, FloatAcc, FloatReg, 0}, // st0 = stX - st0
{"fsubr",  2, 0xdce0, None, ShortForm, FloatReg, FloatAcc, 0}, // stX = st0 - stX
{"fsubrp", 2, 0xdee0, None, ShortForm, FloatReg, FloatAcc, 0}, // stX = st0 - stX ; pop
{"fsubrp", 0, 0xdee1, None, Noth,  0, 0, 0}, // st1 = st0 - st1 ; pop
{"fisubr", 1, 0xde,   5,    Modrm, Mem16, 0, 0}, // st0 = m16int - st0
{"fisubr", 1, 0xda,   5,    Modrm, Mem32, 0, 0}, // st0 = m32int - st0
{"ftst",   0, 0xd9e4, None, Noth, 0, 0, 0}, // cmp st0,0.0
{"fucom",  1, 0xdde0, None, ShortForm, FloatReg, 0, 0}, // ucmp st0,stX
{"fucom",  0, 0xdde1, None, Noth, 0, 0, 0}, // ucmp st0,st1
{"fucomp", 1, 0xdde8, None, ShortForm, FloatReg, 0, 0}, // ucmp st0,stX ; pop
{"fucomp", 0, 0xdde9, None, Noth, 0, 0, 0}, // ucmp st0,st1 ; pop
{"fucompp",0, 0xdae9, None, Noth, 0, 0, 0}, // ucmp st0,st1 ; pop ; pop
{"fwait",  0, 0x9b,   None, Noth, 0, 0, 0}, // fwait
{"fxam",   0, 0xd9e5, None, Noth, 0, 0, 0}, // st0 type
{"fxch",   1, 0xd9c8, None, ShortForm, FloatReg, 0, 0}, // xchg st0, stX
{"fxch",   0, 0xd9c9, None, Noth, 0, 0, 0}, // xchg st0, st1
{"fxtract",0, 0xd9f4, None, Noth, 0, 0, 0}, // st0 = p(st0) ; push(mant(st0))
{"fyl2x",  0, 0xd9f1, None, Noth, 0, 0, 0}, // st1 *= log2(st0)
{"fyl2xp1",0, 0xd9f9, None, Noth, 0, 0, 0}, // st1 *= log2(st0+1.0)

// Pentium Pro extensions
{"rdpmc", 0, 0x0f33, None, Noth, 0, 0, 0},

{"ud2",   0, 0x0fff, None, Noth, 0, 0, 0}, // official undefined instr.

// "cmov*" instr skipped
// Underfined instructions
 //{"cmpxchg8b",1,0x0fc7, 1, Modrm, Mem, 0, 0},
 //{"invlpg",1, 0x0f01,   7,    Modrm, Mem,         0,     0 },
 //{"xadd",  2, 0x0fc0,   None, Modrm,      Reg8|Mem8,   Reg8,  0 },
 //{"xadd",  2, 0x0fc1,   None, _C16|Modrm, Reg16|Mem16, Reg16, 0 },
 //{"xadd",  2, 0x0fc1,   None, _C32|Modrm, Reg32|Mem32, Reg32, 0 },
 //{"cmpxchg",2,0x0fb0,   None, Modrm,      Reg8|Mem8,   Reg8,  0 },
 //{"cmpxchg",2,0x0fb1,   None, _C16|Modrm, Reg16|Mem16, Reg16, 0 },
 //{"cmpxchg",2,0x0fb1,   None, _C32|Modrm, Reg32|Mem32, Reg32, 0 },
 //{"arpl",  2, 0x63, None, Modrm, Reg16, Reg16|Mem, 0},
 //{"lar",   2, 0x0f02, None, Modrm|ReverseRegRegmem, WordReg|Mem, WordReg, 0},
 //{"lsl",   2, 0x0f03, None, Modrm|ReverseRegRegmem, WordReg|Mem, WordReg, 0},
 //{"imul",  3, 0x6b, None, Modrm, Imm8S, WordReg|Mem, WordReg},
 //{"imul",  2, 0x6b, None, Modrm|imulKludge, Imm8S, WordReg, 0},
 //{"shrd",  3, 0x0fac, None, Modrm, Imm8, WordReg, WordReg|Mem},
 //{"shrd",  3, 0x0fad, None, Modrm, ShiftCount, WordReg, WordReg|Mem},
 //{"shld",  3, 0x0fa4, None, Modrm, Imm8, WordReg, WordReg|Mem},
 //{"shld",  3, 0x0fa5, None, Modrm, ShiftCount, WordReg, WordReg|Mem},


{"", 0, 0, 0, 0, 0, 0, 0}       // sentinal (end)
};

// 386 register table
const reg_entry i386_regtab[] = {

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
const prefix_entry i386_prefixtab[] = {
  {"addr16",0x67},{"data16",0x66},{"lock", 0xf0},
  {"wait",  0x9b},{"cs",    0x2e},{"ds",   0x3e},
  {"es",    0x26},{"fs",    0x64},{"gs",   0x65},
  {"ss",    0x36},{"rep",   0xf3},{"repe", 0xf3},
  {"repz",  0xf3},{"repne", 0xf2},{"repnz",0xf2},
  {"",0}
};

int code32=1;
int tnum;
int _index[256];

// --------------------------------------------------------------------------

const optimize opti[] = {

{"add", 2, Reg32, Imm32, 0, "", "0", "", ""},
{"mov", 2, Reg32, Imm32, 0, "", "0", "", "xor %0,%0"},
{"shl", 2, Reg32, Imm,   0, "", "1", "", "shl %0"},
{"shr", 2, Reg32, Imm,   0, "", "1", "", "shr %0"},
{"",    0, 0,0,0,           "","","",       ""}

};

char *_type[] = {
 "Reg8", "Reg16", "Reg32", "Imm8", "Imm8S", "Imm16", "Imm32", "ImmXX",
 "Mem8", "Mem16", "Mem32", "Float", "Mem", "Disp8", "Disp16", "Disp32",
 "DispXX", "I/OPortReg", "Count", "Control", "Debug", "Test", "SegReg2",
 "SegReg3", "Acc8", "Acc16", "Acc32", "FloatReg", "FloatAcc", "MemReg", ""
};

void generate_list()
{
  int a,b,c,d,e;

  if (freopen("xasm.txt","wt",stdout)==0){
    fprintf(stderr,"fatal: error creating 'xasm.txt'\n");
    exit(0);
  }
  printf("\n\txasm x86/watcom/build of Sa 18-11-2000\n\n");
  printf("1. Supported registers\n\n");
  c=1;
  for(a=0;*_type[a];a++){
    d=0;
    for (b=0;*i386_regtab[b].reg_name;b++){
      if (i386_regtab[b].reg_type&c){
        if (!d) printf("  %s: %s",_type[a],i386_regtab[b].reg_name);
           else printf(",%s",i386_regtab[b].reg_name);
        d=1;
      }
    }
    if (d) printf("\n");
    c<<=1;
  }
  printf("\n2. Prefixes\n\n");
  for (a=0;*i386_prefixtab[a].prefix_name;a++){
    printf("  %s\n",i386_prefixtab[a].prefix_name);
  }
  printf("\n3. Opcodes\n\n");
  for (a=0;*i386_optab[a].name;a++){
    if(i386_optab[a].opcode_modifier&D) printf("d"); else printf(" ");
    printf(" %8s",i386_optab[a].name);
    if (i386_optab[a].operands) printf("  ");
    for (b=0;b<i386_optab[a].operands;b++){
      if (b) printf(",");
      printf("<");
      d=0;e=0;c=i386_optab[a].operand_types[b];
      while(c>0){
        if (c&1){
          if(d) printf("|");d=1;
          printf("%s",_type[e]);
        }
        e++;c>>=1;
      }
      printf(">");
    }
    printf("\n");
  }
}
