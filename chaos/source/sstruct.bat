@echo off
cd F:\SOURCE\CHAOS
if exist source\sstruct\ssmaker.exe goto next
cd source\sstruct
wcl386 ssmaker.c /l=DOS4G
del *.obj
cd ..\..
:next
F:\SOURCE\c\xasm.exe source\minidrv\drvload.sm source\minidrv\drvload.out
F:\SOURCE\c\xasm.exe source\minidrv\vmm.sm source\minidrv\vmm.out
source\SSTRUCT\ssmaker source\sstruct.lst
