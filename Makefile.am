## Process this file with automake to produce Makefile.in

SUBDIRS = lqr man

EXTRA_DIST = \
	docs/liblqr_manual.docbook \
	docs/lqr_style.xsl \
	docs/redir-html \
	docs/Makefile \
	docs/README \
	examples/liquidrescale.cpp \
	examples/liquidrescale.h   \
	examples/liquidrescale-basic.cpp \
	examples/liquidrescale-basic.h   \
	examples/Makefile \
	examples/README \
	BUGS		\
	README.Windows	\
	autogen.sh	

DISTCLEANFILES = \
		 autom4te.cache

ACLOCAL_AMFLAGS = -I m4


lqr_pkgconfig_DATA = @LIBRARY_SNAME@.pc

lqr_pkgconfigdir = $(libdir)/pkgconfig
