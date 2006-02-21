MODULE := engines/gob

MODULE_OBJS := \
	anim.o \
	cdrom.o \
	dataio.o \
	draw.o \
	driver_vga.o \
	game.o \
	global.o \
	gob.o \
	goblin.o \
	init.o \
	inter.o \
	inter_v1.o \
	inter_v2.o \
	map.o \
	mult.o \
	mult_v1.o \
	mult_v2.o \
	music.o \
	pack.o \
	palanim.o \
	parse.o \
	parse_v1.o \
	parse_v2.o \
	scenery.o \
	sound.o \
	timer.o \
	util.o \
	video.o

MODULE_DIRS += \
	engines/gob

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
