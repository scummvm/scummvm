MODULE := engines/touche

MODULE_OBJS := \
	console.o \
	graphics.o \
	menu.o \
	metaengine.o \
	midi.o \
	opcodes.o \
	resource.o \
	saveload.o \
	staticres.o \
	touche.o

# This module can be built as a plugin
ifeq ($(ENABLE_TOUCHE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
