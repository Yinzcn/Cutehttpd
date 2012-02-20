@echo off
setlocal enableextensions

set mgwdir=D:\MinGW
set path=%path%;%mgwdir%\bin

if "%*" == "" (set C= ) else (set C= %* )

set cmdl=gcc -Os -s -Wall%C%-static main.c -o chtd.exe -I. -I../src -L../src -lws2_32

echo cmdl='%cmdl%' & %cmdl%
