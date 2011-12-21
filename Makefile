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
LIBS = -lhiredis -ljson
DEBUG = -g -O0

all: $(SHARED) $(STATIC)

# -std=c99 allows json_object_object_foreach() macro from libjson to compile
$(OBJS) : %.o : %.c %.h
	$(CC) -c $(CFLAGS) -std=c99 -fPIC $(DEBUG) $<

$(SHARED) : $(OBJS)
	$(CC) -o $@ $< $(LIBS) $(LDFLAGS) -shared -Wl,-soname,$(SONAME)
	# create links used by static linker and dynamic linker
	ln -s $(SHARED) $(LDNAME)
	ln -s $(SHARED) $(SONAME)

$(STATIC) : $(OBJS)
	ar rcs $@ $<

%_test : %_test.c $(SHARED)
	$(CC) $(CFLAGS) $(DEBUG) $(LIBS) -lredis_ipc -o $@ $<

install:
	mkdir -p $(DESTDIR)/usr/include
	install *.h $(DESTDIR)/usr/include
	mkdir -p $(DESTDIR)/usr/lib
	cp -a lib* $(DESTDIR)/usr/lib

clean: 
	rm -f *.o *_test $(LIBNAME)*
