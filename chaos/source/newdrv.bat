@echo off
cd \SOURCE\CHAOS\source\newdrv
xasm.exe filesys.sm ..\..\filesys.drv
xasm.exe dosfs.sm ..\..\dosfs.drv
