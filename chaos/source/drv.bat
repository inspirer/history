@echo off

cd ..

echo.
echo  -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
echo.
echo Building drivers ...
echo.
:F:\SOFT\CHASM\chasm_w.exe source\drv\keyb.csm
f:\source\c\chasm_os.exe source\drv\keyb.csm
echo.
:F:\SOFT\CHASM\chasm_w.exe source\drv\display.csm
f:\source\c\chasm_os.exe source\drv\display.csm
echo.
:F:\SOFT\CHASM\chasm_w.exe source\drv\fsys.csm
f:\source\c\chasm_os.exe source\drv\fsys.csm
echo.
