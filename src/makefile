#
# Makefile for Cutehttpd
# http://PHPnow.org/Cutehttpd
#

CC = gcc
CFLAGS  = -Wall
#LDLIBS  = -lpcre -lpthread
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

all: libchtd.a

clean:
	$(RM) *.o
	$(RM) libchtd.a

$(OBJS):
	$(CC) $(CFLAGS) -c $*.c -o $@

libchtd.a: $(OBJS)
	$(AR) rs libchtd.a $?
