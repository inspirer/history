all : xasm

objs\error.obj : error.c cc.h i386.h
  cpre error.c makeprep
  ccomp makeprep objs\error.obj
  $remove makeprep
  $fadd

objs\ident.obj : ident.c cc.h i386.h
  cpre ident.c makeprep
  ccomp makeprep objs\ident.obj
  $remove makeprep
  $add

objs\file.obj : file.c cc.h
  cpre file.c makeprep
  ccomp makeprep objs\file.obj
  $remove makeprep
  $add

objs\directiv.obj : directiv.c cc.h i386.h
  cpre directiv.c makeprep
  ccomp makeprep objs\directiv.obj
  $remove makeprep
  $add

objs\defs.obj : defs.c cc.h i386.h
  cpre defs.c makeprep
  ccomp makeprep objs\defs.obj
  $remove makeprep
  $add

objs\i386.obj : i386.c cc.h i386.h
  cpre i386.c makeprep
  ccomp makeprep objs\i386.obj
  $remove makeprep
  $add

objs\xasm.obj : xasm.c cc.h i386.h
  cpre xasm.c makeprep
  ccomp makeprep objs\xasm.obj
  $remove makeprep
  $add

objs\i386ins.obj : i386ins.c cc.h i386.h
  cpre i386ins.c makeprep
  ccomp makeprep objs\i386ins.obj
  $remove makeprep
  $add

xasm : objs\error.obj objs\file.obj objs\directiv.obj objs\defs.obj objs\ident.obj objs\xasm.obj objs\i386ins.obj objs\i386.obj
  $link
  xasm makexasm xasm
  $end
  