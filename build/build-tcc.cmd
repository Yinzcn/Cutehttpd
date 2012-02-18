@echo off

setlocal enableextensions

set tccdir=D:\Projects\tinycc.git\win32
set winsdk=D:\MinGW

if "%*" == "" (set C= ) else (set C= %* )

call :impdef ws2_32.dll

set s_cmdl=%tccdir%\tcc -v -Wall%C%-DTCC_TARGET_PE -DTCC_TARGET_I386 main.c -I%tccdir%\include -I%tccdir%\include\winapi -I%winsdk%\include -lws2_32

echo cmdl=%s_cmdl% & %s_cmdl%

goto :eof


:impdef
if not exist %tccdir%\lib\%~n1.def %tccdir%\tiny_impdef %1 -o %tccdir%\lib\%~n1.def
