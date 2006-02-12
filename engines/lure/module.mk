MODULE := engines/lure

MODULE_OBJS := \
	animseq.o \
	debug-input.o \
	debug-methods.o \
	decode.o \
	disk.o \
	events.o \
	game.o \
	hotspots.o \
	intro.o \
	lure.o \
	memory.o \
	menu.o \
	palette.o \
	res.o \
	res_struct.o \
	room.o \
	screen.o \
	scripts.o \
	strings.o \
	surface.o \
	system.o

MODULE_DIRS += \
	engines/lure

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules

