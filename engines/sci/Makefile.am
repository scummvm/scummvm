SUBDIRS = scicore engine include gfx sfx tools menu win32
EXTRA_DIST = README yywrap.c config.l sciv dc/3dutils.c dc/Makefile \
	dc/bkg.c dc/config.h.in dc/dc.h dc/dc_save.c dc/gamemenu.c dc/gp.h \
	dc/keyboard.c dc/keyboard.h dc/mouse1.h dc/options.h \
	dc/selectgame.c dc/selectgame.h dc/sintab.h dc/texture.c dc/aica.h \
	dc/aica_cmd_iface.h dc/aica_crt0.s dc/aica_main.c dc/aica_sup.c \
	dc/snd_stream.c dc/stream.h
INCLUDES = -I$(top_srcdir)/src/include @EXTRA_INCLUDES@
AM_CFLAGS = $(SDL_CFLAGS)
LDADD = engine/libsciengine.a gfx/libscigraphics.a \
	gfx/resource/libsciresources.a gfx/drivers/libscidrivers.a \
	sfx/libscisound.a scicore/libscicore.a \
	sfx/player/libsciplayer.a \
	sfx/seq/libsciseq.a sfx/timer/libscitimer.a \
	sfx/pcm_device/libscipcm.a sfx/mixer/libscimixer.a \
	sfx/softseq/libscisoftseq.a \
	sfx/libscisoundlib.a \
	sfx/device/libscisounddevice.a \
	win32/libsciwin32.a \
	menu/libscimenu.a \
	@ac_readline@ @ac_curses_libraries@ @ac_png_libraries@ @LIB_M@ @SDL_LIBS@
bin_PROGRAMS = freesci
freesci_SOURCES = config.l main.c
freesci_LDFLAGS = @SCIV_LDFLAGS@
