MODULE := engines/lab

MODULE_OBJS := \
	allocroom.o \
	anim.o \
	detection.o \
	engine.o \
	graphics.o \
	image.o \
	interface.o \
	intro.o \
	lab.o \
	labfile.o \
	labsets.o \
	machine.o \
	map.o \
	mouse.o \
	music.o \
	processroom.o \
	readdiff.o \
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
