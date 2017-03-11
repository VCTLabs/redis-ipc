SUBDIRS = src
#SUBDIRS = src inc test

AM_TESTS_ENVIRONMENT    = top_srcdir="$(top_srcdir)"

ACLOCAL_AMFLAGS         = -I m4 --install

EXTRA_DIST = autogen.sh README.rst debian inc/json.hh

AM_CPPFLAGS =
AM_LDFLAGS =

BUILT_SOURCES =
CLEANFILES =
MOSTLYCLEANFILES =

include_HEADERS =
noinst_LTLIBRARIES =
lib_LTLIBRARIES =
EXTRA_LTLIBRARIES =

include libltdl/ltdl.mk

#TEST_CFLAGS             = -I$(top_srcdir)/test

#TEST_LDADD              = -L$(top_srcdir)/src -lredis_ipc

#test_ldflags            = -no-install

export top_srcdir

#CLEAN_SUBDIRS           =

#${check_PROGRAMS}: LDFLAGS+="${test_ldflags}"

#noinst_LIBRARIES        =
#noinst_DATA             =
#noinst_LTLIBRARIES      =
#check_PROGRAMS          =
#TESTS                   = $(check_PROGRAMS)
#bin_PROGRAMS            =

#INSTALL_EXEC_HOOKS      =
#UNINSTALL_HOOKS         =
#CLEAN_HOOKS             =
AUTOMAKE_OPTIONS        = subdir-objects

${top_srcdir}/ChangeLog:
	(cd $(top_srcdir); git log) >> $@

#install-exec-hook: ${INSTALL_EXEC_HOOKS}
#uninstall-hook: ${UNINSTALL_HOOKS}

# Clear dependency_libs for multiarch
all-local:
	sed -i "/dependency_libs/ s/'.*'/''/" `find . -name '*.la' -o -name '*.lai'`

##	sed -i "/dependency_libs/ s/'.*'/''/" `find . -name '*.lai'`


# include test/Makefile.am
#include src/Makefile.am

