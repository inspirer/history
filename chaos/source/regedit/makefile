all : regedit

regedit.obj : regedit.c
  cpre regedit.c makeprep
  ccomp makeprep regedit.obj
  $remove makeprep
  $fadd

regedit : regedit.obj
  $link
  xasm makexasm regedit
  $end
  $remove regedit.obj
