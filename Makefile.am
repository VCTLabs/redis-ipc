SUBDIRS = src test

#AM_TESTS_ENVIRONMENT    = top_srcdir="$(top_srcdir)"

ACLOCAL_AMFLAGS         = -I m4 --install

EXTRA_DIST = autogen.sh README.rst debian inc/json.hh

export top_srcdir

${top_srcdir}/ChangeLog:
	(cd $(top_srcdir); git log) >> $@

# Clear dependency_libs for multiarch
all-local:
	sed -i "/dependency_libs/ s/'.*'/''/" `find . -name '*.la' -o -name '*.lai'`


