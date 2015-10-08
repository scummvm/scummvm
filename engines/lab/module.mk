MODULE := engines/lab

MODULE_OBJS := \
	allocroom.o \
	detection.o \
	engine.o \
	graphics.o \
	interface.o \
	intro.o \
	lab.o \
	labfile.o \
	labmusic.o \
	labsets.o \
	labtext.o \
	machine.o \
	map.o \
	mouse.o \
	processroom.o \
	readdiff.o \
	readparse.o \
	resource.o \
	savegame.o \
	special.o \
	text.o \
	timing.o \
	undiff.o \
	vga.o


# This module can be built as a plugin
ifeq ($(ENABLE_LAB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

