@echo off
set ansi_c_path=f:\Projects\ANSI\

if .%1. == .lib. goto libs
if .%1. == .-add. goto add
%ansi_c_path%cpre.exe %1.c | %ansi_c_path%ccomp.exe - %1.jy
if .%2. == ./c. goto finish
%ansi_c_path%jy.exe %1.jy %ansi_c_path%libs %ansi_c_path%stub %ansi_c_path%import %2 %3 %4 %5 %6
del %1.jy
goto finish

:libs
if not exist %ansi_c_path%libs goto not_del
del %ansi_c_path%libs
:not_del
for %%a in (*.jy) DO call %0 -add %%a %ansi_c_path%libs
goto finish

:add
echo { >> %3
type %2 >> %3
echo } >> %3

:finish
