CROSS_COMPILE ?= arm-none-linux-gnueabi-
PREFIX  = $(CROSS_COMPILE)
CC      = $(PREFIX)gcc
CXX     = $(PREFIX)g++
CPP     = $(PREFIX)cpp
LD      = $(PREFIX)gcc
AR      = $(PREFIX)ar
RANLIB  = $(PREFIX)ranlib
PWD     = $(shell pwd)

SYSROOT ?= /usr/local/arago/arm-2009q1/arm-none-linux-gnueabi
RPATH_ARG ?= $(SYSROOT)/usr/lib

INCFLAG = -I./include -I$(SYSROOT)/usr/include
DEBUGFLAG = -g -O2 -std=gnu99
CFLAGS  = $(DEBUGFLAG) $(INCFLAG)
LDPATH = -L$(SYSROOT)/usr/lib -L$(SYSROOT)/lib -Wl,-rpath-link,$(RPATH_ARG)

LIBNAME=libredis_ipc
API_VERSION = 1
MINOR_VERSION = 0
PATCH_VERSION = 0

FULL_VERSION = $(API_VERSION).$(MINOR_VERSION).$(PATCH_VERSION)

SHARED = $(LIBNAME).$(FULL_VERSION).so
SONAME = $(LIBNAME).$(API_VERSION).so
LDNAME = $(LIBNAME).so
STATIC = $(LIBNAME).$(FULL_VERSION).a
OBJS = redis_ipc.o
LIBS = -lhiredis -ljson -Wl,--hash-style=gnu

all: $(SHARED) $(STATIC)

# -std=c99 allows json_object_object_foreach() macro from libjson to compile

$(OBJS) : %.o : %.c %.h
	$(CC) -c $(CFLAGS) -fPIC $<

$(SHARED) : $(OBJS)
	$(CC) -o $@ $< $(LIBS) $(LDPATH) -shared -Wl,-soname,$(SONAME)
	ln -sf $(SHARED) $(LDNAME)
	ln -sf $(SHARED) $(SONAME)

$(STATIC) : $(OBJS)
	ar rcs $@ $<

%_test : %_test.c $(SHARED)
	$(CC) $(CFLAGS) $(LIBS) -lredis_ipc -lpthread $(LDPATH) -o $@ $<

install:
	mkdir -p $(DESTDIR)/usr/include
	install *.h *.hh $(DESTDIR)/usr/include
	mkdir -p $(DESTDIR)/usr/lib
	cp -a lib* $(DESTDIR)/usr/lib

clean: 
	rm -f *.o *_test $(LIBNAME)*
