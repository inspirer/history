@echo off
rem (Eugeniy Gryaznov, 2002, inspirer@inbox.ru)

set lalr_path=lalr\
set perl_path=lalr\

set skip_build=no
set show_debug=

:check_params
if not .%1. == .-. goto no_skip1
shift
set skip_build=yes
goto check_params
:no_skip1
if not .%1. == .d. goto no_skip2
shift
set show_debug=d
goto check_params
:no_skip2
if not .%1. == .e. goto no_skip3
shift
set show_debug=e
goto check_params
:no_skip3
if not .%1. == .?. goto no_skip4
echo  syntax:  %lalr_path%make [-] [d] [e] [-1] [input_grammar] [output_prefix]
echo  (Eugeniy Gryaznov, 2002, inspirer@inbox.ru)
echo      `-`         не перестраивать таблицы
echo      `d`,`e`     создать debug файл (d - состояния, e - сост. и аванцеп.)
echo      `-1`        вывести имена бесполезных нетерминалов
echo      остальное и так понятно
goto exit
:no_skip4

if not exist debug goto no_debug
del debug
:no_debug
if not exist errors goto no_errors
del errors
:no_errors

%perl_path%perl -w %lalr_path%gg\lalr.pl %1 %2 %3 %4 %5 %6 %7
if not exist grammar goto exit
if .%skip_build%.==.yes. goto delgr
%lalr_path%gg.exe %show_debug%
:delgr
if not exist grammar goto exit
del grammar
:exit
