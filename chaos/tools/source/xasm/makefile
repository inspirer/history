all : xasm.exe

objs\error.obj : error.c cc.h i386.h
  wcc386 error.c -fo=objs\error.obj

objs\ident.obj : ident.c cc.h i386.h
  wcc386 ident.c -fo=objs\ident.obj

objs\file.obj : file.c cc.h
  wcc386 file.c -fo=objs\file.obj

objs\directiv.obj : directiv.c cc.h i386.h
  wcc386 directiv.c -fo=objs\directiv.obj

objs\defs.obj : defs.c cc.h i386.h
  wcc386 defs.c -fo=objs\defs.obj

objs\i386.obj : i386.c cc.h i386.h
  wcc386 i386.c -fo=objs\i386.obj

objs\xasm.obj : xasm.c cc.h i386.h
  wcc386 xasm.c -fo=objs\xasm.obj

objs\i386ins.obj : i386ins.c cc.h i386.h
  wcc386 i386ins.c -fo=objs\i386ins.obj

objs = objs\error.obj objs\file.obj objs\directiv.obj objs\defs.obj objs\ident.obj objs\xasm.obj objs\i386ins.obj objs\i386.obj

xasm.exe : $(objs)
  wlink @objs\cc.lnk
