lib_LTLIBRARIES = libredis_ipc.la

#pkginclude_HEADERS = inc/json.hh

libredis_ipc_la_SOURCES = redis_ipc.c
include_HEADERS = redis_ipc.h inc/json.hh

#libredis_ipc_la_SOURCES = redis_ipc.h \
#	redis_ipc.c

libredis_ipc_la_CPPFLAGS = -I$(includedir)
libredis_ipc_la_CFLAGS = -std=gnu99 $(HIREDIS_CFLAGS)
libredis_ipc_la_LIBADD = $(HIREDIS_LIBS)
libredis_ipc_la_LDFLAGS = $(HIREDIS_LDFLAGS)
#libredis_ipc_la_LIBTOOLFLAGS = --tag=disable-static


