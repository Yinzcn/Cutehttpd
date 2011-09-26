# 
# Makefile for Cutehttpd
# http://PHPnow.org/Cutehttpd
#

RM      = del /q

DEP  = ..\dep

CC = gcc
#CFLAGS = -Wall -g -DDEBUG
CFLAGS  = -Wall
LDLIBS  = -L$(DEP) -lpcre -lws2_32
OBJS    = buff.o \
          base.o \
          namevalue.o \
          parse_header.o \
          log.o \
          mime_type.o \
          http_status_lines.o \
          http.o \
          http_list_dir.o \
          http_send_file.o \
          conn.o \
          reqs.o \
          uhook.o \
          vhost.o \
          squeue.o \
          wker.o \
          fcgi.o \
          status_info.o \
          cutehttpd.o \
          debug.o \
          test_ext.o

all:	chtd-static chtd-shared

clean:
	$(RM) *.o
	$(RM) libchtd.a
	$(RM) libchtd.dll.a
	$(RM) chtd.dll

$(OBJS):
	$(CC) $(CFLAGS) -I$(DEP) -c $*.c -o $@

chtd-static:	$(OBJS)
	$(AR) rs libchtd.a $?

chtd-shared:	$(OBJS)
	$(CC) $(OBJS) $(LDLIBS) -s -shared -Wl,--out-implib=libchtd.dll.a,--export-all-symbols -o chtd.dll
