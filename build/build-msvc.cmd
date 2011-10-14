@echo off
setlocal

set VSPATH=D:\Projects\MSVC90ENU
set SDKPATH=D:\Projects\Win-SDK\v6.0A
set PATH=%VSPATH%\Common7\IDE;%VSPATH%\VC\bin;%PATH%

set INCLUDE=%VSPATH%\VC\include;%SDKPATH%\Include;
set LIB=%VSPATH%\VC\Lib;%SDKPATH%\Lib;

if "%*" == "" (set C= ) else (set C= %* )

set ccmd=cl /nologo main.c /Fechtd-msvc.exe /W2 /O2%C%/I../src /I../dep /link /libpath:"..\dep" ws2_32.lib pcre.lib
echo %ccmd%
%ccmd%
