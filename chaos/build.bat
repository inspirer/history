 : ChaOS makefile :

@echo off

md bin
cls

echo.
echo Building kernel loader ...
echo.
F:\SOFT\CHASM\chasm_w.exe chaos.csm

echo.
echo Building kernel ...
echo.
F:\SOFT\CHASM\chasm_w.exe kernel.csm

echo.
choice Run ChaOS now
if errorlevel 2 goto no_chaos
echo.
cd bin
chaos
echo.
pause > nul

:no_chaos
echo.
