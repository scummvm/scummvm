EXTRA_DIST = dc_driver.c dd_driver.cpp dd_driver.h dd_driver_line.cpp dx_driver.cpp dx_driver.h
INCLUDES = -I$(top_srcdir)/src/include @ac_ggi_includes@ @ac_png_includes@ @X_CFLAGS@ @EXTRA_INCLUDES@
AM_CFLAGS = $(SDL_CFLAGS)
LDADD =  $(LDADD_ALL)

gfxdriverdir = $(libdir)/freesci/gfx

#EXTRA_LTLIBRARIES = xlib_driver.la ggi_driver.la sdl_driver.la
#gfxdriver_LTLIBRARIES = @fsci_xlib_dl@ @fsci_ggi_dl@ @fsci_sdl_dl@

noinst_LIBRARIES = libscidrivers.a
libscidrivers_a_SOURCES = gfx_drivers.c xlib_driver.c ggi_driver.c sdl_driver.c directfb_driver.c \
				null_driver.c
#libscidrivers_a_OBJECTS = gfx_drivers.o @fsci_xlib_st@ @fsci_ggi_st@ @fsci_sdl_st@


#xlib_driver_la_LDFLAGS = -rpath $(gfxdriverdir) -module -no-undefined \
#			 -export-symbols $(srcdir)/EXPORTS_xlib
#xlib_driver_la_LIBADD = @X_LIBS@
#xlib_driver_la_SOURCES = xlib_driver.c

#ggi_driver_la_LDFLAGS = -rpath $(gfxdriverdir) -module -no-undefined \
#			-export-symbols $(srcdir)/EXPORTS_ggi
#ggi_driver_la_LIBADD = @X_LIBS@ @ac_ggi_libraries@
#ggi_driver_la_SOURCES = ggi_driver.c

#sdl_driver_la_LDFLAGS = -rpath $(gfxdriverdir) -module -no-undefined \
#			-export-symbols $(srcdir)/EXPORTS_sdl
#sdl_driver_la_LIBADD =  @SDL_LIBS@
#3sdl_driver_la_SOURCES = sdl_driver.c


