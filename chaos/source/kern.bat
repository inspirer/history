 : ChaOS kernel makefile :

@echo off

cd ..

echo.
echo  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
echo.
echo Building kernel ...
echo.
f:\source\c\chasm_os.exe source\kernel\kernel.csm
:f:\soft\chasm\chasm_w.exe source\kernel\kernel.csm

echo.
echo Building kernel loader ...
echo.
f:\source\c\chasm_os.exe source\kernel\chaos.csm
:f:\soft\chasm\chasm_w.exe source\kernel\chaos.csm

echo.
choice Run ChaOS now
if errorlevel 2 goto no_chaos
echo.
chaos
echo.
:no_chaos
echo.
