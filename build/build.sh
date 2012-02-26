#make -C ../src CFLAGS="-Wall -DDEBUG" clean all
gcc -Wall -s -DDEBUG main.c -o chtd -I. -I../src -lpthread
