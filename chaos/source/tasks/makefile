all : demo

demo.obj : demo.c
  cpre demo.c makeprep
  ccomp makeprep demo.obj
  $remove makeprep
  $fadd

demo : demo.obj
  $link
  xasm makexasm demo
  $end
  $remove demo.obj
