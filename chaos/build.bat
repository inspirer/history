 : ChaOS makefile :

@echo off

echo.
echo Building kernel loader ...
echo.
F:\SOFT\CHASM\chasm_w.exe chaos.csm

echo.
echo Building kernel ...
echo.
F:\SOFT\CHASM\chasm_w.exe kernel.csm

echo.
echo Building system device driver ...
echo.
F:\SOFT\CHASM\chasm_w.exe sysdev.csm

echo.
echo Building task ...
echo.
F:\SOFT\CHASM\chasm_w.exe startup.csm

echo.
choice Run ChaOS now
if errorlevel 2 goto no_chaos
echo.
chaos
echo.
pause > nul

:no_chaos
echo.
