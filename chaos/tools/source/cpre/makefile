all : cpre.exe

objs\error.obj : error.c cc.h
  wcc386 error.c -fo=objs\error.obj

objs\ident.obj : ident.c cc.h
  wcc386 ident.c -fo=objs\ident.obj

objs\file.obj : file.c cc.h
  wcc386 file.c -fo=objs\file.obj

objs\directiv.obj : directiv.c cc.h
  wcc386 directiv.c -fo=objs\directiv.obj

objs\defs.obj : defs.c cc.h
  wcc386 defs.c -fo=objs\defs.obj

objs\cc.obj : cc.c cc.h
  wcc386 cc.c -fo=objs\cc.obj

objs = objs\error.obj objs\file.obj objs\directiv.obj objs\defs.obj objs\ident.obj objs\cc.obj

cpre.exe : $(objs)
  wlink @objs\cpre.lnk
