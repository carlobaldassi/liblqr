dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.54)

dnl remember to edit the .pc file name
dnl and lqr/Makefile.am if you change these 
m4_define([library_base_name], [lqr])
m4_define([api_version], [1])

dnl Name of library
m4_define([library_sname], [library_base_name-api_version])
m4_define([library_name], [liblqr-api_version])


dnl These three define the library version number
m4_define([package_major_version], [0])
m4_define([package_minor_version], [1])
m4_define([package_micro_version], [0])

m4_define([package_version],
          [package_major_version.package_minor_version.package_micro_version])

m4_define([ltversion_current], [0])
m4_define([ltversion_release], [0])
m4_define([ltversion_age], [0])

AC_INIT([library_name], [package_version])

AC_DEFINE(LIBRARY_NAME, PACKAGE_NAME, [Library name])

AC_DEFINE(PACKAGE_MAJOR_VERSION, package_major_version, [Package major version])
AC_DEFINE(PACKAGE_MINOR_VERSION, package_minor_version, [Package minor version])
AC_DEFINE(PACKAGE_MICRO_VERSION, package_micro_version, [Package micro version])

AC_DEFINE(LTVERSION_CURRENT, ltversion_current, [Library current version])
AC_DEFINE(LTVERSION_RELEASE, ltversion_release, [Library release])
AC_DEFINE(LTVERSION_AGE, ltversion_age, [Library age])

LIBRARY_BASE_NAME=library_base_name
AC_SUBST(LIBRARY_BASE_NAME)
API_VERSION=api_version
AC_SUBST(API_VERSION)
LIBRARY_NAME=library_name
AC_SUBST(LIBRARY_NAME)
LIBRARY_SNAME=library_sname
AC_SUBST(LIBRARY_SNAME)

PACKAGE_VERSION=package_major_version.package_minor_version.package_micro_version
AC_SUBST(PACKAGE_VERSION)

LTVERSION=ltversion_current:ltversion_release:ltversion_age
AC_SUBST(LTVERSION)

AC_CONFIG_SRCDIR([lqr/lqr_carver.c])
AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE([-Wall -Werror])

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC

AC_DISABLE_STATIC
AC_PROG_LIBTOOL

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

DATADIR="${datadir}/library_name"
AC_SUBST(DATADIR)


AC_CONFIG_FILES([
Makefile
lqr/Makefile
lqr-1.pc
])


AC_OUTPUT
