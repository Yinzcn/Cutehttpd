
del /q *.o
del /q libchtd.a
del /q libchtd.dll.a
del /q chtd.dll

gcc -Wall %* -c cutehttpd.c -o cutehttpd.o -I..\dep
ar rs libchtd.a cutehttpd.o
gcc cutehttpd.o -L..\dep -lpthreadGC2s -lpcre -lws2_32 -s -shared -Wl,--out-implib=libchtd.dll.a,--export-all-symbols -o chtd.dll
