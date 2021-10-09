if RIPC_RUNTIME_DIR
PATH_FLAG = -DRIPC_RUNTIME_DIR=\"$(RIPC_RUNTIME_DIR)\"
endif

AM_CPPFLAGS             = -I$(top_srcdir)/src $(PATH_FLAG)

lib_LTLIBRARIES = libredis_ipc.la

libredis_ipc_la_SOURCES = redis_ipc.c
include_HEADERS = redis_ipc.h

libredis_ipc_la_CPPFLAGS = -I$(includedir)
libredis_ipc_la_CFLAGS = -Wall -std=gnu99 $(HIREDIS_CFLAGS) $(JSONC_CFLAGS)
libredis_ipc_la_LIBADD = $(HIREDIS_LIBS) $(JSONC_LIBS)
