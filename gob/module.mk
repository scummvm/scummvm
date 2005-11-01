MODULE := gob

MODULE_OBJS := \
	gob/anim.o \
	gob/cdrom.o \
	gob/dataio.o \
	gob/draw.o \
	gob/driver_vga.o \
	gob/game.o \
	gob/global.o \
	gob/gob.o \
	gob/goblin.o \
	gob/init.o \
	gob/inter.o \
	gob/map.o \
	gob/mult.o \
	gob/pack.o \
	gob/palanim.o \
	gob/parse.o \
	gob/scenery.o \
	gob/sound.o \
	gob/timer.o \
	gob/util.o \
	gob/video.o

MODULE_DIRS += \
	gob

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
