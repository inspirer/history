@echo off

call D:\Far\ClearPluginsCache.bat
cl /nologo /ML /O2 /G5 /LD /GX ssbrowser.cpp registry.cpp ss.cpp /link /noentry /nologo /opt:nowin98 /def:ssbrowser.def kernel32.lib advapi32.lib
del /q ssbrowser.exp registry.obj ss.obj ssbrowser.lib ssbrowser.obj 2> nul
