all : shell

shell.obj : shell.c
  cpre shell.c makeprep
  ccomp makeprep shell.obj
  $remove makeprep
  $fadd

shell : shell.obj
  $link
  xasm makexasm shell
  $end
  $remove shell.obj
