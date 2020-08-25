MODULE := engines/cryo

MODULE_OBJS = \
	cryo.o \
	cryolib.o \
	debugger.o \
	eden.o \
	eden_graphics.o \
	metaengine.o \
	resource.o \
	sound.o \
	video.o

# This module can be built as a plugin
ifeq ($(ENABLE_CRYO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
