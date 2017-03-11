AM_CPPFLAGS    = -I$(top_srcdir)/inc -I$(top_srcdir)/src -I$(includedir)

LIBREDISIPC = $(top_srcdir)/src/.libs/libredis_ipc.la

BUILT_TESTS = command_result_test \
              json_test \
              multithread_test\
              settings_status_test \
              pub_sub_test

#TESTS = $(BUILT_TESTS) tests/runtests.sh
TESTS = $(check_PROGRAMS)
check_PROGRAMS = $(BUILT_TESTS)
include_HEADERS = $(top_srcdir)/inc/json.hh

command_result_test_SOURCES = command_result_test.c
command_result_test_LDADD = -lhiredis -ljson-c $(LIBREDISIPC) -lpthread
command_result_test_LDFLAGS = -Wl,--hash-style=gnu -no-install

json_test_SOURCES = json_test.cpp
json_test_LDADD = -lhiredis -ljson-c $(LIBREDISIPC) -lpthread
json_test_LDFLAGS = -Wl,--hash-style=gnu -no-install

multithread_test_SOURCES = multithread_test.c
multithread_test_LDADD = -lhiredis -ljson-c $(LIBREDISIPC) -lpthread
multithread_test_LDFLAGS = -Wl,--hash-style=gnu -no-install

pub_sub_test_SOURCES = pub_sub_test.c
pub_sub_test_LDADD = -lhiredis -ljson-c $(LIBREDISIPC) -lpthread
pub_sub_test_LDFLAGS = -Wl,--hash-style=gnu -no-install

settings_status_test_SOURCES = settings_status_test.c
settings_status_test_LDADD = -lhiredis -ljson-c $(LIBREDISIPC) -lpthread
settings_status_test_LDFLAGS = -Wl,--hash-style=gnu -no-install

