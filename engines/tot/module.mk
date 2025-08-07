MODULE := engines/tot

MODULE_OBJS = \
	tot.o \
	console.o \
	metaengine.o \
	forest.o \
	util.o \
	playanim.o \
	graphics.o \
	anims.o \
	routines2.o \
	routines.o \
	dialog.o \
	game.o \
	font/bgifont.o \
	font/biosfont.o \
	decoder/TotFlicDecoder.o \
	soundman.o \
	graphman.o \
	mouseman.o \
	chrono.o \
	saveload.o \
	midi.o

# This module can be built as a plugin
ifeq ($(ENABLE_TOT), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
