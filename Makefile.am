SUBDIRS = src test

#AM_TESTS_ENVIRONMENT    = top_srcdir="$(top_srcdir)"

ACLOCAL_AMFLAGS         = -I m4 --install

EXTRA_DIST = autogen.sh README.rst debian inc/json.hh
pkgconfig_DATA = redis-ipc.pc

export top_srcdir

${top_srcdir}/ChangeLog:
	(cd $(top_srcdir); git log) >> $@

# Clear dependency_libs for multiarch
all-local:
	sed -i "/dependency_libs/ s/'.*'/''/" `find . -name '*.la' -o -name '*.lai'`


if WITH_COVERAGE
COV_INFO_FILE = $(top_builddir)/coverage.info
COV_DIR = $(top_builddir)/coverage
cov:
	$(MAKE) $(AM_MAKEFLAGS) \
		CFLAGS="$(CFLAGS) $(COVERAGE_CFLAGS) $(COVERAGE_OPTFLAGS)" \
		CXXFLAGS="$(CXXFLAGS) $(COVERAGE_CFLAGS) $(COVERAGE_OPTFLAGS)"
	$(MAKE) $(AM_MAKEFLAGS) check \
		CFLAGS="$(CFLAGS) $(COVERAGE_CFLAGS) $(COVERAGE_OPTFLAGS)" \
		CXXFLAGS="$(CXXFLAGS) $(COVERAGE_CFLAGS) $(COVERAGE_OPTFLAGS)"
	@echo "Generating coverage report..."
	$(LCOV) --config-file $(top_srcdir)/.lcovrc --capture \
		--directory "$${PWD}/src" \
		--directory "$${PWD}/test" \
		--include "$${PWD}/src/redis_ipc.c" \
		--include "$${PWD}/src/redis_ipc.h" \
		--include "$${PWD}/inc/json.hh" \
		--output-file $(COV_INFO_FILE) \
		--gcov-tool $(GCOV)
	$(GENHTML) --prefix "$(top_builddir)" \
		--output-directory $(COV_DIR) \
		--title $(PACKAGE_NAME) \
		--legend --show-details \
		$(COV_INFO_FILE)

clean-local:
	@echo "Cleaning lcov files."
	@find $(top_builddir) -name "*.gcno" -exec rm -v {} \;
	@find $(top_builddir) -name "*.gcda" -exec rm -v {} \;
	@rm -rf $(top_builddir)/coverage*
endif
