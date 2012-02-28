@echo off
setlocal enableextensions

set FH=..\.git\refs\heads\master
if exist %FH% for /f "tokens=1" %%i in (%FH%) do set REVISION=%%i
if not "%REVISION%" == "" set CF=-DREV_A=0x%REVISION:~0,8% -DREV_B=0x%REVISION:~8,8%
if not "%CFLAGS%" == "" set CF=%CF% %CFLAGS%
if not "%*" == "" set CF=%CF% %*

set pccdir=D:\PellesC

set cmdl=%pccdir%\bin\cc -MT -Os %CF% -W1 -Gd -Ze -Zx -Go main.c ws2_32.lib /ochtd.exe

echo cmdl='%cmdl%' & %cmdl%
