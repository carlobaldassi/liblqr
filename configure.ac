dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.54)

dnl Name of library
m4_define([library_name], [liblqr])

dnl These three define the library version number
m4_define([library_major_version], [0])
m4_define([library_minor_version], [1])
m4_define([library_micro_version], [0])

m4_define([library_version],
          [library_major_version.library_minor_version.library_micro_version])

AC_INIT([library_name], [library_version])

AC_DEFINE(LIBRARY_NAME, PACKAGE_NAME, [Library name])
AC_DEFINE(LIBRARY_VERSION, PACKAGE_VERSION, [Library version])

AC_DEFINE(LIBRARY_MAJOR_VERSION, library_major_version, [Library major version])
AC_DEFINE(LIBRARY_MINOR_VERSION, library_minor_version, [Library minor version])
AC_DEFINE(LIBRARY_MICRO_VERSION, library_micro_version, [Library micro version])

AC_CONFIG_SRCDIR([lqr/lqr_raster.c])
AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE([-Wall -Werror])

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_CC_STDC
AC_HEADER_STDC


ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

AM_MAINTAINER_MODE


dnl Use -Wall if we have gcc.
changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl


GLIB_REQUIRED_VERSION=2.0.0

PKG_CHECK_MODULES(GLIB,
  glib-2.0 >= $GLIB_REQUIRED_VERSION)

GLIB_CFLAGS=`$PKG_CONFIG --cflags glib-2.0`
AC_SUBST(GLIB_CFLAGS)

GLIB_LIBS=`$PKG_CONFIG --libs glib-2.0`
AC_SUBST(GLIB_LIBS)

GLIB_DIR=`$PKG_CONFIG --variable=libdir glib-2.0`
AC_SUBST(GLIB_DIR)

AC_CHECK_FUNCS(bind_textdomain_codeset)

LOCALEDIR="${datadir}/locale"
if test "$DATADIRNAME" = "lib"; then
   LOCALEDIR="$libdir/locale"
fi
AC_SUBST(LOCALEDIR)

DATADIR="${datadir}/library_name"
AC_SUBST(DATADIR)


AC_CONFIG_FILES([
Makefile
lqr/Makefile
])


AC_OUTPUT
