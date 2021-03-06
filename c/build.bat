@echo off

if (%1) == (clean) goto clean
if (%1) == (test)  goto test

save\lapg.exe
save\perl.exe il_conv.pl
del /q cc.exe cc.ilk cc.pdb vc60.pdb 2> nul
cl /nologo /Zi cc.cpp parse.cpp slab.cpp types.cpp ns.cpp sym.cpp expr.cpp conv.cpp debug.cpp bytecode.cpp il.cpp
del /q *.obj 2> nul
goto exit

:clean
del /q cc.exe errors parse.cpp cc.ilk cc.pdb vc60.pdb
goto exit

:test
cc.exe test\test.cpp

:exit