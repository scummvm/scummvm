MODULE := engines/tot

MODULE_OBJS = \
	decoder/TotFlicDecoder.o \
	font/bgifont.o \
	font/biosfont.o \
	anims.o \
	chrono.o \
	console.o \
	cutscenes.o \
	debug.o \
	dialog.o \
	engine.o \
	forest.o \
	graphics.o \
	metaengine.o \
	midi.o \
	mouse.o \
	resources.o \
	saveload.o \
	sound.o \
	tot.o \
	util.o

# This module can be built as a plugin
ifeq ($(ENABLE_TOT), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
