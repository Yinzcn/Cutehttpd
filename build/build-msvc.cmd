@echo off
setlocal enableextensions

set FH=..\.git\refs\heads\master
if exist %FH% for /f "tokens=1" %%i in (%FH%) do set REVISION=%%i
if not "%REVISION%" == "" set CF=-DREV_A=0x%REVISION:~0,8% -DREV_B=0x%REVISION:~8,8%
if not "%CFLAGS%" == "" set CF=%CF% %CFLAGS%
if not "%*" == "" set CF=%CF% %*

set VSPATH=D:\Projects\MSVC90ENU
set SDKPATH=D:\Projects\Win-SDK\v7.1
set PATH=%VSPATH%\Common7\IDE;%VSPATH%\VC\bin;%PATH%

set INCLUDE=%VSPATH%\VC\include;%SDKPATH%\Include;
set LIB=%VSPATH%\VC\Lib;%SDKPATH%\Lib;

set ccmd=cl /nologo main.c /Fechtd.exe /W2 /O2 %CF% /I../src /link ws2_32.lib

echo ccmd='%ccmd%' & %ccmd%
