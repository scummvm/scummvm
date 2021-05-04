MODULE := engines/trecision

MODULE_OBJS = \
	console.o \
	actor.o \
	dialog.o \
	fastfile.o \
	graphics.o \
	inventory.o \
	logic.o \
	metaengine.o \
	sound.o \
	script.o \
	trecision.o \
	utils.o \
	3d.o \
	video.o \
	nl/do.o \
	nl/regen.o \
	nl/schedule.o \
	nl/string.o \
	nl/3d/3dvar.o \
	nl/3d/3dwalk.o \
	nl/ll/llmouse.o \
	nl/ll/llscreen.o

# This module can be built as a plugin
ifeq ($(ENABLE_TRECISION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
