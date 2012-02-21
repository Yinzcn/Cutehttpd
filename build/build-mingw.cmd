@echo off
setlocal enableextensions

set FH=..\.git\refs\heads\master
if exist %FH% for /f "tokens=1" %%i in (%FH%) do set REVISION=%%i
if not "%REVISION%" == "" (set CF=-DREV_A=0x%REVISION:~0,8% -DREV_B=0x%REVISION:~8,8%)
if not "%*" == "" (set CF=%CF% %*)

set mgwdir=D:\MinGW
set path=%path%;%mgwdir%\bin

set cmdl=gcc -Os -s -Wall %CF% -static main.c -o chtd.exe -I. -I../src -L../src -lws2_32

echo cmdl='%cmdl%' & %cmdl%
