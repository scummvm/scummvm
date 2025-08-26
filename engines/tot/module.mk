MODULE := engines/tot

MODULE_OBJS = \
	tot.o \
	console.o \
	metaengine.o \
	forest.o \
	util.o \
	anims.o \
	engine.o \
	dialog.o \
	font/bgifont.o \
	font/biosfont.o \
	decoder/TotFlicDecoder.o \
	sound.o \
	graphics.o \
	mouse.o \
	chrono.o \
	saveload.o \
	midi.o \
	debug.o \
	cutscenes.o \
	resources.o

# This module can be built as a plugin
ifeq ($(ENABLE_TOT), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
