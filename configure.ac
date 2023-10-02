#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([redis-ipc], [0.2.1], [sjl@vctlabs.com])
AC_CONFIG_SRCDIR([src/redis_ipc.c])
AC_CONFIG_MACRO_DIR([m4])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AM_INIT_AUTOMAKE([foreign subdir-objects])

if test "x$prefix" = "xNONE"; then
        prefix=$ac_default_prefix
fi
if test "x$exec_prefix" = "xNONE"; then
        exec_prefix='${prefix}'
fi

AC_CONFIG_HEADERS([config.h])

AM_PROG_AR
LT_PREREQ([2.2.6])
LT_CONFIG_LTDL_DIR([libltdl])
LT_INIT([dlopen])
LTDL_INIT([subproject])

AM_MAINTAINER_MODE([enable])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([memory.h stdint.h stdlib.h string.h sys/types.h sys/syscall.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strdup])

AC_CHECK_LIB(pthread, pthread_create, [LIBS="-lpthread ${LIBS}"], [
  echo "pthreads required, failing"
  exit -1
])

# Check for hiredis and json-c
JSON_C_MIN_VERSION="0.9"

PKG_CHECK_MODULES(JSONC, libjson-c >= $JSON_C_MIN_VERSION, ,
  AC_CHECK_HEADER(json-c/json.h, [JSONC_LIBS="-ljson-c"], [json-c=no])
  AC_CHECK_HEADER(json-c/json.h, [JSONC_CFLAGS="-I$includedir"
                                      JSONC_LIBS="-L$libdir -ljson-c"], [json-c=no])
  AC_SUBST([JSONC_CFLAGS])
  AC_SUBST([JSONC_LIBS])
)

PKG_CHECK_MODULES(HIREDIS, libhiredis, ,
  AC_CHECK_HEADER(hiredis/hiredis.h, [HIREDIS_LIBS="-lhiredis"], [hiredis=no])
  AC_CHECK_HEADER(hiredis/hiredis.h, [HIREDIS_CFLAGS="-I$includedir"
                                      HIREDIS_LIBS="-L$libdir -lhiredis"], [hiredis=no])
  AC_SUBST([HIREDIS_CFLAGS])
  AC_SUBST([HIREDIS_LIBS])
)
#[
#  AC_MSG_ERROR(libHIREDIS not found)
#])

AC_ARG_ENABLE(debug,
              [  --enable-debug      Enable debugging code.],, enable_debug="no")

AC_ARG_ENABLE(gprof,
              [  --enable-gprof      Enable gcc profiling.],, enable_gprof="no")

# Test coverage generate. Flag: --with-coverage
AC_ARG_WITH([coverage],
            AS_HELP_STRING([--with-coverage], [Generate test covearge report with lcov.]),
            [with_cov=true], []
            )
AM_CONDITIONAL([WITH_COVERAGE], [test x$with_cov = xtrue])
if test x$with_cov = xtrue; then
  AC_PATH_PROG(PWD, pwd)
  AC_PATH_PROG(LCOV, lcov)
  AC_PATH_PROG(GCOV, gcov)
  AC_PATH_PROG(GENHTML, genhtml)
  COVERAGE_CFLAGS="--coverage"
  COVERAGE_OPTFLAGS="-pg -g -O0"
  AC_SUBST([PWD])
  AC_SUBST([GCOV])
  AC_SUBST([LCOV])
  AC_SUBST([GENHTML])
  AC_SUBST([COVERAGE_CFLAGS])
  AC_SUBST([COVERAGE_OPTFLAGS])
fi

enable_value()
{
        if test "x$1" = "xyes" ; then
                echo 1
        else
                echo 0
        fi
}

AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [package name])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [version number])
AC_DEFINE_UNQUOTED(PATH_PREFIX, "$prefix", [prefix directory])
AC_DEFINE_UNQUOTED(PATH_EXECPREFIX, "$exec_prefix", [execprefix directory])
AC_DEFINE_UNQUOTED(PATH_LOCALSTATEDIR, "$localstatedir", [local state directory])
AC_DEFINE_UNQUOTED(PATH_DATAROOTDIR, "$datarootdir", [data root directory])
AC_DEFINE_UNQUOTED(PATH_DATADIR, "$datadir", [data directory])
AC_DEFINE_UNQUOTED(PATH_LIBEXECDIR, "$libexecdir", [libexec directory])
if test -n "$env_ld_library_path"; then
        AC_DEFINE_UNQUOTED(ENV_LD_LIBRARY_PATH, "$env_ld_library_path", [set LD_LIBRARY_PATH to this value])
fi
AC_DEFINE_UNQUOTED(ENABLE_DEBUG, `enable_value $enable_debug`, [Enable debugging])
AC_DEFINE_UNQUOTED(ENABLE_GPROF, `enable_value $enable_gprof`, [Enable gcc profiling])

AC_ARG_VAR(RIPC_RUNTIME_DIR, [Supply default path to directory containing redis server socket])
AM_CONDITIONAL([RIPC_RUNTIME_DIR], [test x$RIPC_RUNTIME_DIR != x])

PKG_INSTALLDIR

AC_CONFIG_FILES([Makefile \
                 src/Makefile \
                 test/Makefile \
                 redis-ipc.pc])

AC_OUTPUT
