AM_CPPFLAGS             = -I$(top_srcdir)/src

lib_LTLIBRARIES = libredis_ipc.la

libredis_ipc_la_SOURCES = redis_ipc.c
include_HEADERS = redis_ipc.h

#pkginclude_HEADERS = $(top_srcdir)inc/json.hh

#libredis_ipc_la_SOURCES = redis_ipc.h \
#	redis_ipc.c

libredis_ipc_la_CPPFLAGS = -I$(includedir)
libredis_ipc_la_CFLAGS = -Wall -std=gnu99 $(HIREDIS_CFLAGS) $(JSONC_CFLAGS)
libredis_ipc_la_LIBADD = $(HIREDIS_LIBS) $(JSONC_LIBS)
#libredis_ipc_la_LDFLAGS = $(HIREDIS_LDFLAGS) $(JSONC_LDFLAGS)
#libredis_ipc_la_LIBTOOLFLAGS = --tag=disable-static


