 : ChaOS kernel makefile :

@echo off

cd ..

echo.
echo  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
echo.
echo Building kernel ...
echo.
chasm.exe source\kernel\kernel.csm

echo.
echo Building kernel loader ...
echo.
chasm.exe source\kernel\chaos.csm

echo.
choice Run ChaOS now
if errorlevel 2 goto no_chaos
echo.
chaos
echo.
:no_chaos
echo.
