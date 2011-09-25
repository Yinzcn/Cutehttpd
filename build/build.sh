make -C ../src CFLAGS="-Wall -DDEBUG" clean all
gcc -DDEBUG main.c -o chtd -Wall -s -I. -I../src -L../src -lchtd -lpcre -lpthread
