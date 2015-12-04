MODULE := engines/lab

MODULE_OBJS := \
	anim.o \
	detection.o \
	engine.o \
	eventman.o \
	graphics.o \
	image.o \
	interface.o \
	intro.o \
	lab.o \
	labsets.o \
	map.o \
	music.o \
	processroom.o \
	resource.o \
	savegame.o \
	special.o \
	text.o \
	timing.o \
	vga.o


# This module can be built as a plugin
ifeq ($(ENABLE_LAB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
