@echo off
setlocal enableextensions

set FH=..\.git\refs\heads\master
if exist %FH% for /f "tokens=1" %%i in (%FH%) do set REVISION=%%i
if not "%REVISION%" == "" (set CF=-DREV_A=0x%REVISION:~0,8% -DREV_B=0x%REVISION:~8,8%)
if not "%*" == "" (set CF=%CF% %*)

set tccdir=D:\Projects\tinycc.git\win32
set winsdk=D:\MinGW


call :impdef ws2_32.dll

set cmdl=%tccdir%\tcc -v %CF% -Wall -I%tccdir%\include -I%tccdir%\include\winapi -I%winsdk%\include -lws2_32 main.c -o chtd.exe

echo cmdl='%cmdl%' & %cmdl%

goto :eof


:impdef
if not exist %tccdir%\lib\%~n1.def %tccdir%\tiny_impdef %1 -o %tccdir%\lib\%~n1.def
