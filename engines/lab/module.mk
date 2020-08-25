MODULE := engines/lab

MODULE_OBJS := \
	anim.o \
	console.o \
	dispman.o \
	engine.o \
	eventman.o \
	image.o \
	interface.o \
	intro.o \
	lab.o \
	labsets.o \
	map.o \
	metaengine.o \
	music.o \
	processroom.o \
	resource.o \
	savegame.o \
	special.o \
	speciallocks.o \
	utils.o

# This module can be built as a plugin
ifeq ($(ENABLE_LAB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
