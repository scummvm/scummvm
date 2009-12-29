MODULE := engines/mohawk

MODULE_OBJS = \
	bitmap.o \
	console.o \
	detection.o \
	dialogs.o \
	file.o \
	graphics.o \
	livingbooks.o \
	mohawk.o \
	myst.o \
	myst_jpeg.o \
	myst_pict.o \
	myst_vars.o \
	myst_saveload.o \
	myst_scripts.o \
	riven.o \
	riven_external.o \
	riven_saveload.o \
	riven_scripts.o \
	riven_vars.o \
	sound.o \
	video/cinepak.o \
	video/qdm2.o \
	video/qtrle.o \
	video/qt_player.o \
	video/rpza.o \
	video/smc.o \
	video/video.o


# This module can be built as a plugin
ifeq ($(ENABLE_MOHAWK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
