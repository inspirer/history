// -------------------------------[ Intel386 floating point instructions ]---

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

// --------------------------------------------------------------------------
