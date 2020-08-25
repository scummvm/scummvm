MODULE := engines/chewy

MODULE_OBJS = \
	chewy.o \
	cursor.o \
	console.o \
	events.o \
	graphics.o \
	metaengine.o \
	resource.o \
	scene.o \
	sound.o \
	text.o \
	video/cfo_decoder.o

# This module can be built as a plugin
ifeq ($(ENABLE_CHEWY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
