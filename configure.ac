#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([redis-ipc], [0.0.1], [sjl@vctlabs.com])
AC_CONFIG_SRCDIR([src/redis_ipc.c])
AC_CONFIG_MACRO_DIR([m4])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AM_MAINTAINER_MODE

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_AR
LT_PREREQ([2.2.6])
LT_INIT

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([memory.h stdint.h stdlib.h string.h sys/types.h sys/syscall.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strdup])

# Check for hiredis and json-c
PKG_CHECK_MODULES(HIREDIS, libhiredis,
  AC_CHECK_HEADER(hiredis/hiredis.h, [HIREDIS_LIBS="-lhiredis"], [hiredis=no])
  AC_CHECK_HEADER(hiredis/hiredis.h, [HIREDIS_CFLAGS="-I$includedir"
                                      HIREDIS_LIBS="-L$libdir -lhiredis"], [hiredis=no])
  AC_SUBST([HIREDIS_CFLAGS])
  AC_SUBST([HIREDIS_LIBS]) ,
[
  AC_MSG_ERROR(libHIREDIS not found)
])



AC_CONFIG_FILES([Makefile
                 src/Makefile])
##                 test/Makfile])
AC_OUTPUT

