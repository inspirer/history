// ia32.cpp

//	This file is part of the Linker project.
//	AUTHORS: Eugeniy Gryaznov, 2002, inspirer@inbox.ru

#include "ia32.h"
#include "linker.h"

// ======== IA32data ========

void IA32data::DeleteList(IA32data *list)
{
	IA32data *i = list,*e;
	while(i){ e = i; i = i->next; delete e; }
}

IA32data::~IA32data()
{
	if( expr ) delete expr;
	if( type == -1) delete string;
}

// ======== IA32opcode ========

IA32opcode::~IA32opcode()
{
	if( dispe ) delete dispe;
	if( imme ) delete imme;
}

// ======== IA32 ========

void IA32::inithash()
{
	int i;

	for( i=0; *regtab[i].reg_name; i++)
		regtab[i].hashval = Lex::hash_value(regtab[i].reg_name);
	for( i=0; *prefixtab[i].prefix_name; i++)
		prefixtab[i].hashval = Lex::hash_value(prefixtab[i].prefix_name);
	for( i=0; *optab[i].name; i++)
		optab[i].hashval = Lex::hash_value(optab[i].name);
}

int IA32::isregisterorprefix(char *s,ulong& hashcode_or_type)
{
	int i;

	for( i=0; *regtab[i].reg_name; i++)	if( regtab[i].hashval == hashcode_or_type )
		if( !strcmp(regtab[i].reg_name,s) ){
			if( regtab[i].reg_type == SReg2 || regtab[i].reg_type == SReg3 ) hashcode_or_type = 0;
				else hashcode_or_type = 1;
			return i;
		}

	for( i=0; *prefixtab[i].prefix_name; i++) if( prefixtab[i].hashval == hashcode_or_type )
		if( !strcmp(prefixtab[i].prefix_name,s) ){
			hashcode_or_type = 2;
			return i;
		}

	return -1;
}

IA32opcode * IA32::AcceptOpcode(IA32struct *op)
{
	uchar code[8];
	Expr *dispe = NULL,*imme = NULL;
	ulong i,e,k,params = op->params, pcode, pmodrm, psib, cmdparam, opcode, ophash;
	flags_byte flags;
	modrm_byte modrm;
	sib_byte sib;

	ophash = Lex::hash_value(op->name);
	if( !op->mem_addr_size ) op->mem_addr_size = IA32::code+1;

	flags.binary = 0;
	opcode = -1;
	for( i=0; *optab[i].name; i++ ) 
		if( optab[i].hashval == ophash && optab[i].operands == params && strcmp(optab[i].name,op->name) == 0 ){

		e = 1;
		for( k=0; k<params; k++)
			if( (op->p[k].type & optab[i].operand_types[k]) == 0 ) e = 0;

		if( !e && params == 2 && optab[i].opcode_modifier&D ){
			e = 1;
			if( (op->p[0].type & optab[i].operand_types[1]) == 0 ) e = 0;
			if( (op->p[1].type & optab[i].operand_types[0]) == 0 ) e = 0;
			if( e ) flags.invert_direction = 1;
		}
		if( e ){ opcode = i; break; }
	}

	if( opcode == -1){
		lnk.error(E_INVALIDOPC,op->place,op->name);
		return NULL;
	}

	ulong cmdsize = 0;

	if( optab[opcode].opcode_modifier&_C32 && !IA32::code
	 || optab[opcode].opcode_modifier&_C16 && IA32::code)
			code[cmdsize++]=0x66;

	if( op->mem_addr_size == 2 && !IA32::code
	 || op->mem_addr_size == 1 && IA32::code)
			code[cmdsize++]=0x67;

	if( op->prefix_present )
		code[cmdsize++]=prefixtab[op->prefix].prefix_code;

	if( op->segment_present ) switch(op->segment){
		case 0: code[cmdsize++]=0x26;break;
		case 1: code[cmdsize++]=0x2e;break;
		case 2: code[cmdsize++]=0x36;break;
		case 3: code[cmdsize++]=0x3e;break;
		case 4: code[cmdsize++]=0x64;break;
		case 5:	code[cmdsize++]=0x65;break;
	}

	i = optab[opcode].base_opcode; e = 0;
	while( i!=0 ) i>>=8, e++;
	i = optab[opcode].base_opcode; if( !e ) e++;
	for( k=e; k>0; k--,i>>=8 ) code[cmdsize+k-1]=(uchar)(i&0xFF);
	cmdsize += e;
	pcode = cmdsize-1;

	if( flags.invert_direction )
		code[pcode]|=2;

	if( optab[opcode].opcode_modifier & Modrm ){
		flags.modrm_present = 1;
		pmodrm = cmdsize++;
		modrm.binary = 0;
		sib.binary = 0;

		if( optab[opcode].extension_opcode!=None)
			modrm.reg = optab[opcode].extension_opcode;
	}

	for( i=0; i<params; i++){

		IA32param *curr = &op->p[i];

		if( flags.invert_direction ) cmdparam = optab[opcode].operand_types[1-i];
			else cmdparam = optab[opcode].operand_types[i];

		if( cmdparam == DispXX ) if( IA32::code ) cmdparam = Disp32; else cmdparam = Disp16;
		if( cmdparam == ImmXX )  if( IA32::code ) cmdparam = Imm32; else cmdparam = Imm16;

		switch( cmdparam & curr->type ){

			case Acc8: case Acc16: case Acc32: case FloatAcc: 
			case InOutPortReg: case ShiftCount:
				break;

			case Reg8: case Reg16: case Reg32:
			case FloatReg: case SReg2: case SReg3: case Debug:
			case Control: case Test: case MMXReg:

				if( optab[opcode].opcode_modifier & ShortForm )
					code[pcode]|=curr->flags.reg;
				else if( optab[opcode].opcode_modifier & (Seg2ShortForm|Seg3ShortForm))
					code[pcode]|=curr->flags.reg<<3;
				else {
					if( !flags.modrm_present )
						lnk.error(E_INTERNAL,op->place);

					if( (Mem|Mem8|Mem16|Mem32|MemReg) & cmdparam )
						modrm.rm = curr->flags.reg, modrm.mod=3;
					else
						modrm.reg = curr->flags.reg;
				}
				break;

			case Disp8:
				dispe = curr->expr;
				flags.dispsize = 1;
				flags.relative = 1;
				break;
			case Disp16:
				dispe = curr->expr;
				flags.dispsize = 2;
				flags.relative = 1;
				break;
			case Disp32:
				dispe = curr->expr;
				flags.dispsize = 4;
				flags.relative = 1;
				break;

			case Imm8:
				imme = curr->expr;
				flags.immsize = 1;
				break;
			case Imm16:
				imme = curr->expr;
				flags.immsize = 2;
				break;
			case Imm32:
				imme = curr->expr;
				flags.immsize = 4;
				break;

			case Mem: case Mem8: case Mem16: case Mem32: case MemFL:
				if( !flags.modrm_present ) 
					lnk.error(E_INTERNAL,op->place);

				if( op->mem_addr_size == 2){  // 32bit

					if( curr->flags.index_present && curr->flags.mult == 0 && !curr->flags.base_present ){
						curr->flags.index_present = 0;
						curr->flags.base_present = 1;
						curr->flags.base = curr->flags.index;
					}

					if( curr->flags.index_present ){
						flags.sib_present = 1;
						psib = cmdsize++;

						if( curr->flags.index == 4 && curr->flags.mult == 0){
							k = curr->flags.index; curr->flags.index = curr->flags.base;
							curr->flags.base = k;
						}

						if( curr->flags.index == 4){			// [!esp]
							lnk.error(E_ESPINDEX, curr->place);
							return NULL;
						}
						sib.index = curr->flags.index;
						sib.ss = curr->flags.mult;
						modrm.rm = 4;

						if( curr->flags.base_present ){			// [m*index+base]
							sib.base = curr->flags.base;
							if( curr->expr ){
								modrm.mod = curr->flags.disp_style;
								if( curr->flags.disp_style == 2)
									flags.dispsize = 4;
								else flags.dispsize = 1;
								dispe = curr->expr;
							} else {
								if( curr->flags.base != 5 )		// [!ebp]
									modrm.mod = 0;
								else {
									modrm.mod = 1;
									flags.dispsize = 1;
								}
							}

						} else {								// [m*index+disp]
							modrm.mod = 0;
							sib.base = 5;
							flags.dispsize = 4;
							dispe = curr->expr;
						}

					} else {									// [base+disp]
						if( curr->flags.base_present ){
							if( curr->flags.base != 4 ){		// [!esp]
								modrm.rm = curr->flags.base;
								if( curr->expr ){
									modrm.mod = curr->flags.disp_style;
									dispe = curr->expr;
									if( curr->flags.disp_style == 2)
										flags.dispsize = 4;
									else flags.dispsize = 1;
								} else {
									modrm.mod = 0;
									if( curr->flags.base == 5){ // [ebp] => [ebp+d8]
										modrm.mod = 1;
										flags.dispsize = 1;
									}
								}

							} else {							// [esp+disp]
								flags.sib_present = 1;
								psib = cmdsize++;
								modrm.rm = 4;
								sib.ss = 0;
								sib.index = 4;
								sib.base = 4;
								if( curr->expr ){
									modrm.mod = curr->flags.disp_style;
									dispe = curr->expr;
									if( curr->flags.disp_style == 2)
										flags.dispsize = 4;
									else flags.dispsize = 1;
								} else modrm.mod = 0;
							}

						} else {
							if( curr->expr == NULL ){
								lnk.error(E_NULLREF,curr->place);
								return NULL;
							}
							modrm.mod = 0;
							modrm.rm = 5;
							dispe = curr->expr;
							flags.dispsize = 4;
						}
					}

				} else { // 16bit
					lnk.error(E_NOTIMPLEMENTED,curr->place,"16bit memory addressing");
					return NULL;
				}  
		}
		lnk.info.free_place(op->p[i].place);
	}

	if( flags.modrm_present ) code[pmodrm] = modrm.binary;
	if( flags.sib_present ) code[psib] = sib.binary;

    #ifdef DEBUG_object
	printf("accepted: ");
	for( i=0; i<cmdsize; i++) printf(" %02x",code[i]);
	if( flags.dispsize ){ printf(" [disp%i( ",flags.dispsize*8); dispe->Debug(); printf(" )]"); }
	if( flags.immsize ){ printf(" [imm%i( ",flags.immsize*8); imme->Debug(); printf(" )]"); }
	printf("\n");
    #endif

	IA32opcode *r = new IA32opcode;
	for( i=0; i<8; i++) r->code[i] = code[i];
	r->disp_size = flags.dispsize;
	r->imm_size = flags.immsize;
	r->relative = flags.relative;
	lnk.info.free_place(op->place);
	r->size = (uchar)cmdsize;
	r->dispe = dispe;
	r->imme = imme;
	return r;
}
