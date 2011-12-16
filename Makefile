LIBNAME=libredis_ipc
API_VERSION = 1
MINOR_VERSION = 0
PATCH_VERSION = 0

FULL_VERSION = $(API_VERSION).$(MINOR_VERSION).$(PATCH_VERSION)

SHARED = $(LIBNAME).$(FULL_VERSION).so
SONAME = $(LIBNAME).$(API_VERSION).so
STATIC = $(LIBNAME).$(FULL_VERSION).a
OBJS = redis_ipc.o
LIBS = -lhiredis -ljson

all: $(SHARED) $(STATIC)

$(OBJS) : %.o : %.c %.h
	$(CC) -c $(CFLAGS) $(INCLUDE) $(DEBUG) $<

$(SHARED) : $(OBJS)
	$(CC) -o $@ $< $(LIBS) $(LDFLAGS) -shared -Wl,-soname,$(SONAME)

$(STATIC) : $(OBJS)
	ar rcs $@ $<

clean: 
	rm -f *.o $(SHARED) $(STATIC)
