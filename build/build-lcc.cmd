@echo off
setlocal enableextensions

set FH=..\.git\refs\heads\master
if exist %FH% for /f "tokens=1" %%i in (%FH%) do set REVISION=%%i
if not "%REVISION%" == "" set CF=-DREV_A=0x%REVISION:~0,8% -DREV_B=0x%REVISION:~8,8%
if not "%CFLAGS%" == "" set CF=%CF% %CFLAGS%
if not "%*" == "" set CF=%CF% %*

set lccdir=D:\lcc

set cmdl=%lccdir%\bin\lc -O %CF% -DHAVE_STRNDUP main.c -o chtd.exe

echo cmdl='%cmdl%' & %cmdl%
