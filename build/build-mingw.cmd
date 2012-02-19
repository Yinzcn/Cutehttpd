@echo off
setlocal enableextensions

set mgwdir=D:\MinGW
set path=%path%;%mgwdir%\bin

if "%*" == "" (set C= ) else (set C= %* )

set cmdl=gcc main.c -o chtd-static.exe -Os -s -Wall%C%-static -I. -I../src -L../src -lws2_32

echo cmdl='%cmdl%' & %cmdl%
