MODULE := engines/gob

MODULE_OBJS := \
	engines/gob/anim.o \
	engines/gob/cdrom.o \
	engines/gob/dataio.o \
	engines/gob/draw.o \
	engines/gob/driver_vga.o \
	engines/gob/game.o \
	engines/gob/global.o \
	engines/gob/gob.o \
	engines/gob/goblin.o \
	engines/gob/init.o \
	engines/gob/inter.o \
	engines/gob/inter_v1.o \
	engines/gob/inter_v2.o \
	engines/gob/map.o \
	engines/gob/mult.o \
	engines/gob/music.o \
	engines/gob/pack.o \
	engines/gob/palanim.o \
	engines/gob/parse.o \
	engines/gob/parse_v1.o \
	engines/gob/parse_v2.o \
	engines/gob/scenery.o \
	engines/gob/sound.o \
	engines/gob/timer.o \
	engines/gob/util.o \
	engines/gob/video.o

MODULE_DIRS += \
	engines/gob

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
