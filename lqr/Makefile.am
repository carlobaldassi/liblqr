## Process this file with automake to produce Makefile.in

lqr_LTLIBRARIES = liblqr-1.la

lqrdir = $(libdir)

liblqr_1_la_SOURCES = \
	lqr.h		\
	lqr_base.h      \
	lqr_gradient.c  \
	lqr_gradient.h  \
	lqr_cursor.c    \
	lqr_cursor.h    \
	lqr_carver.c    \
	lqr_carver.h    \
	lqr_seams_buffer.c \
	lqr_seams_buffer.h \
	lqr_seams_buffer_list.c \
	lqr_seams_buffer_list.h \
	lqr_colour.c    \
	lqr_colour.h	\
	lqr_progress.c  \
	lqr_progress.h

liblqr_1_la_LDFLAGS = -version-info @LTVERSION@

AM_CPPFLAGS = \
	-DDATADIR=\""$(DATADIR)"\"		\
	-I$(top_srcdir)		\
	@GLIB_CFLAGS@		\
	-I$(includedir)

LDADD = $(GLIB_LIBS) -lm

lqr_include_HEADERS = lqr.h \
		      lqr_base.h \
		      lqr_gradient.h \
		      lqr_cursor.h \
		      lqr_carver.h \
		      lqr_seams_buffer.h \
		      lqr_seams_buffer_list.h \
		      lqr_colour.h \
		      lqr_progress.h

lqr_includedir = $(includedir)/@LIBRARY_SNAME@/@LIBRARY_BASE_NAME@

