// IA32.h

//	This file is part of the Linker project.
//	AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#ifndef _IA32_H__INCLUDED_
#define _IA32_H__INCLUDED_

#include "defs.h"
#include "expr.h"

#define DT_ARRAY  128

struct IA32data
{
	int type;
	IA32data *next;
	Expr *expr;
	union {
		int fill;
		char data[10];
		ulong number;
		char *string;
	};

	IA32data(){ expr = NULL; next = NULL; type = 0; }
	~IA32data();
	static void DeleteList(IA32data *list);
};

CLASS_SEQ(IA32data)

struct IA32param 
{
	ulong type;
	int place;
	union {
		struct {
			ulong index_present:1;
			ulong base_present:1;
			ulong disp_style:2;
			ulong BX:1,BP:1,SI:1,DI:1;

			ulong index:3, base:3, mult:2;
			ulong reg:3;
		} flags;
		int all_flags;
	};
	Expr *expr;
};

struct IA32struct
{
	char *name;
	int place;
	IA32param p[3];

	union{
		struct {
			ulong params:3;
			ulong prefix:4;
			ulong prefix_present:1;
			ulong segment:4;
			ulong segment_present:1;
			ulong mem_addr_size:2;
		};
		int all_flags;
	};
};

struct IA32opcode
{
	uchar size,code[8];
	Expr *dispe,*imme;
	ulong disp_size:4,imm_size:4,relative:1;

	~IA32opcode();
};

struct IA32info
{
	IA32info *prev;
	ulong code;
};

class IA32 {
public:
	enum {
		Noth = 0, D = 0x01, ShortForm = 0x02, Seg2ShortForm = 0x04, 
		Seg3ShortForm = 0x08, Modrm = 0x10,	_C16 = 0x20, _C32 = 0x80,
	};
	enum { None = 0xff };
	enum {
		Reg8 = 0x01, Reg16 = 0x02, Reg32 = 0x04,
		Imm8 = 0x08, Imm8S = 0x10, Imm16 = 0x20, Imm32 = 0x40, ImmXX = 0x80,
		Imm = (Imm32|Imm16|Imm8S|Imm8|ImmXX),
		Mem8 = 0x100, Mem16 = 0x200, Mem32 = 0x400, MemFL = 0x800, Mem = 0x1000,
		MemXX = (Mem|MemFL|Mem32|Mem16|Mem8),
		Disp8 = 0x2000, Disp16 = 0x4000, Disp32 = 0x8000, DispXX = 0x10000,
		Disp = (Disp8|Disp16|Disp32|DispXX),
		InOutPortReg = 0x20000, ShiftCount = 0x40000,
		Control = 0x80000, Debug = 0x100000, Test = 0x200000,
		SReg2 = 0x400000, SReg3 = 0x800000,
		Acc8 = 0x1000000, Acc16 = 0x2000000, Acc32 = 0x4000000,
		FloatReg = 0x8000000, FloatAcc = 0x10000000,
		MemReg = 0x20000000, MMXReg = 0x40000000
	};

	struct reg_entry {
		char *reg_name;
		ulong reg_type;
		ulong reg_num;
		ulong hashval;
	};

	struct prefix_entry {
		char *prefix_name;
		uchar prefix_code;
		ulong hashval;
	};

	struct opcode_entry {
		char *name;
		ulong  operands;
		ulong base_opcode;
		uchar extension_opcode;
		ulong opcode_modifier;
		ulong operand_types[3];
		ulong hashval;
	};

	union modrm_byte {
		struct {
			ulong rm:3, reg:3, mod:2;
		};
		uchar binary;
	};

	union sib_byte {
		struct {
			ulong base:3,index:3,ss:2;
		};
		uchar binary;
	};

	union flags_byte {
		struct {
			ulong dispsize:4;
			ulong immsize:4;
			ulong modrm_present:1;
			ulong sib_present:1;
			ulong invert_direction:1;
			ulong relative:1;
		};
		ulong binary;
	};

	static opcode_entry	optab[];
	static reg_entry	regtab[];
	static prefix_entry	prefixtab[];
	static const char * paramtype[];

	int code;
	ulong offset,fileoffset;
	IA32info *info_stack;

	static void inithash();
	static int isregisterorprefix(char *,ulong&);
	IA32opcode * AcceptOpcode(IA32struct *op);

	IA32(){ code = 1; offset = fileoffset = 0; info_stack = NULL; }
};

#endif
