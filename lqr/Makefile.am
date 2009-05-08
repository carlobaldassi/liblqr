## Process this file with automake to produce Makefile.in

lqr_LTLIBRARIES = liblqr-1.la

lqrdir = $(libdir)

liblqr_1_la_SOURCES = \
	lqr.h		\
	lqr_all.h	\
	lqr_base.h      \
	lqr_gradient.c  \
	lqr_gradient.h  \
	lqr_gradient_pub.h  \
	lqr_gradient_priv.h  \
	lqr_rwindow.c  \
	lqr_rwindow.h  \
	lqr_rwindow_pub.h  \
	lqr_rwindow_priv.h  \
	lqr_energy.c  \
	lqr_energy.h  \
	lqr_energy_pub.h  \
	lqr_energy_priv.h  \
	lqr_cursor.c    \
	lqr_cursor.h    \
	lqr_cursor_pub.h    \
	lqr_cursor_priv.h    \
	lqr_carver.c    \
	lqr_carver.h    \
	lqr_carver_pub.h    \
	lqr_carver_priv.h    \
	lqr_carver_macros_priv.h    \
	lqr_carver_list.c    \
	lqr_carver_list.h    \
	lqr_carver_list_pub.h    \
	lqr_carver_list_priv.h    \
	lqr_carver_bias.c    \
	lqr_carver_bias.h    \
	lqr_carver_bias_pub.h    \
	lqr_carver_bias_priv.h    \
	lqr_carver_rigmask.c    \
	lqr_carver_rigmask.h    \
	lqr_carver_rigmask_pub.h    \
	lqr_carver_rigmask_priv.h    \
	lqr_vmap.c \
	lqr_vmap.h \
	lqr_vmap_pub.h \
	lqr_vmap_priv.h \
	lqr_vmap_list.c \
	lqr_vmap_list.h \
	lqr_vmap_list_pub.h \
	lqr_vmap_list_priv.h \
	lqr_progress.c  \
	lqr_progress.h  \
	lqr_progress_pub.h  \
	lqr_progress_priv.h

liblqr_1_la_LDFLAGS = -version-info @LTVERSION@ @LT_NOUNDEF_FLAG@

liblqr_1_la_LIBADD = $(GLIB_LIBS) -lm

AM_CPPFLAGS = \
	-DDATADIR=\""$(DATADIR)"\"		\
	-I$(top_srcdir)		\
	@GLIB_CFLAGS@		\
	-I$(includedir)		\
	@FVISIBILITY_FLAG@

lqr_include0_HEADERS = lqr.h

lqr_include0dir = $(includedir)/@LIBRARY_SNAME@

lqr_include_HEADERS = lqr_base.h \
		      lqr_gradient_pub.h \
		      lqr_rwindow_pub.h \
		      lqr_energy_pub.h \
		      lqr_cursor_pub.h \
		      lqr_carver_pub.h \
		      lqr_carver_list_pub.h \
		      lqr_carver_bias_pub.h    \
		      lqr_carver_rigmask_pub.h    \
		      lqr_vmap_pub.h \
		      lqr_vmap_list_pub.h \
		      lqr_progress_pub.h

lqr_includedir = $(includedir)/@LIBRARY_SNAME@/@LIBRARY_BASE_NAME@

