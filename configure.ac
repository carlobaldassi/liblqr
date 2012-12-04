dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.63)

dnl remember to edit the .pc file name
dnl and lqr/Makefile.am if you change these 
m4_define([library_base_name], [lqr])
m4_define([api_version], [1])

dnl Name of library
m4_define([library_sname], [library_base_name-api_version])
m4_define([library_name], [liblqr-api_version])


dnl These three define the library version number
m4_define([package_major_version], [0])
m4_define([package_minor_version], [4])
m4_define([package_micro_version], [2])

m4_define([package_version],
          [package_major_version.package_minor_version.package_micro_version])

m4_define([ltversion_current], [3])
m4_define([ltversion_revision], [2])
m4_define([ltversion_age], [3])

AC_INIT([library_name], [package_version])

AC_PREFIX_DEFAULT([/usr])

AC_DEFINE(LIBRARY_NAME, PACKAGE_NAME, [Library name])

AC_DEFINE(PACKAGE_MAJOR_VERSION, package_major_version, [Package major version])
AC_DEFINE(PACKAGE_MINOR_VERSION, package_minor_version, [Package minor version])
AC_DEFINE(PACKAGE_MICRO_VERSION, package_micro_version, [Package micro version])

AC_DEFINE(LTVERSION_CURRENT, ltversion_current, [Library current version])
AC_DEFINE(LTVERSION_REVISON, ltversion_revision, [Library revision])
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

LTVERSION=ltversion_current:ltversion_revision:ltversion_age
AC_SUBST(LTVERSION)

AC_CONFIG_SRCDIR([lqr/lqr_carver.c])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror])

AC_ISC_POSIX
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_HEADER_STDC

AC_C_INLINE
AC_C_VOLATILE

AC_DISABLE_STATIC
LT_INIT([win32-dll])

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

GLIB_REQUIRED_VERSION=2.8.0

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

AC_ARG_ENABLE([install-man],
     [AS_HELP_STRING([--enable-install-man], [Install man pages])],
     [AS_CASE(["${enableval}"],
	  [yes], [install_man=true],
	  [no], [install_man=false],
	  [AC_MSG_ERROR([bad value ${enableval} for --enable-install-man])]
	  )],
     [install_man=false])
AM_CONDITIONAL([INSTMAN], [test x$install_man = xtrue])

AC_ARG_ENABLE([legacy-macros],
     [AS_HELP_STRING([--disable-legacy-macros], [Disable legacy macros])],
     [AS_CASE([["${enableval}"]],
	  [yes], [legacy_macros=true],
	  [no], [legacy_macros=false],
	  [AC_MSG_ERROR([bad value ${enableval} for --enable-legacy-macros])]
	  )],
     [legacy_macros=true])
AS_IF([test "x$legacy_macros" = "xfalse"],
      [AC_DEFINE([LQR_DISABLE_LEGACY_MACROS],[],[Disable legacy macros])],
      [:]
      )

AC_ARG_ENABLE([declspec],
     [AS_HELP_STRING([--disable-declspec], [Disable Win32 __declspec directives (only meaningful for building static library on MinGW)])],
     [AS_CASE([["${enableval}"]],
	  [yes], [declspec=true],
	  [no], [declspec=false],
	  [AC_MSG_ERROR([bad value ${enableval} for --enable-declspec])]
	  )],
     [declspec=true])
AS_IF([test "x$declspec" = "xfalse"],
      [AC_DEFINE([LQR_DISABLE_DECLSPEC],[],[Disable __declspec directives])],
      [:]
      )


AC_MSG_CHECKING([[if we are on Win32]])
AC_LANG([C])
AC_PREPROC_IFELSE([[
		   #ifndef WIN32
		   #errrrrrrrrorrrrr
		   #endif
		   ]],
		   [[is_win32=true]],
		   [[is_win32=false]]
		   )
AS_IF([test "x$is_win32" = "xtrue"],
      [AC_MSG_RESULT([yes]); LT_NOUNDEF_FLAG="-no-undefined"; FVISIBILITY_FLAG=""],
      [AC_MSG_RESULT([no]); LT_NOUNDEF_FLAG=""; FVISIBILITY_FLAG="-fvisibility=\"hidden\""],
      )

AC_SUBST(LT_NOUNDEF_FLAG)
AC_SUBST(FVISIBILITY_FLAG)

AC_DEFINE([LQR_EXPORTS],[],[Flag for Windows DLL creation])

AC_CONFIG_FILES([
Makefile
lqr/Makefile
man/Makefile
lqr-1.pc
])


AC_OUTPUT
