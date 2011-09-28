
del /q *.o
del /q libchtd.a
del /q libchtd.dll.a
del /q chtd.dll
@echo on
gcc -Wall %* -c _onefile_.c -o onefile.o -I..\dep
ar rs libchtd.a onefile.o
gcc onefile.o -L..\dep -lpthreadGC2s -lpcre -lws2_32 -s -shared -Wl,--out-implib=libchtd.dll.a,--export-all-symbols -o chtd.dll
        @echo off
