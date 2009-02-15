SUBDIRS = resource drivers
INCLUDES = -I$(top_srcdir)/src/include @EXTRA_INCLUDES@
EXTRA_DIST = gfx_line.c gfx_crossblit.c gfx_pixmap_scale.c alpha_mvi_crossblit.c

noinst_LIBRARIES = libscigraphics.a
libscigraphics_a_LIBADD = alpha_mvi_crossblit_32.o alpha_mvi_crossblit_32_P.o drivers/libscidrivers.a resource/libsciresources.a
EXTRA_libscigraphics_a_SOURCES = gfx_crossblit.c gfx_pixmap_scale.c
libscigraphics_a_SOURCES = font.c  gfx_resource.c  gfx_support.c  gfx_tools.c  operations.c  resmgr.c  sbtree.c  widgets.c menubar.c sci_widgets.c antialias.c gfx_console.c gfx_res_options.c font-5x8.c font-6x10.c

check_PROGRAMS = gfx_test
gfx_test_LDADD = libscigraphics.a drivers/libscidrivers.a @X_LIBS@ @ac_glx_libraries@ @ac_ggi_libraries@ @ac_readline@ @ac_curses_libraries@ @ac_png_libraries@ -lm @SDL_LIBS@
gfx_test_SOURCES = gfx_test.c

alpha_mvi_crossblit_32.o:
	$(COMPILE) @ev6_cpu@ -I../.. -I../include -DHAVE_CONFIG_H -DFUNCT_NAME=alpha_mvi_crossblit_32 -c $(top_srcdir)/src/gfx/alpha_mvi_crossblit.c -o $(top_builddir)/src/gfx/alpha_mvi_crossblit_32.o

alpha_mvi_crossblit_32_P.o:
	$(COMPILE) @ev6_cpu@ -I../.. -I../include -DHAVE_CONFIG_H -DPRIORITY=1 -DFUNCT_NAME=alpha_mvi_crossblit_32_P -c $(top_srcdir)/src/gfx/alpha_mvi_crossblit.c -o $(top_builddir)/src/gfx/alpha_mvi_crossblit_32_P.o


