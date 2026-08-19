MODULE := engines/dune

MODULE_OBJS = \
	archive.o \
	dune.o \
	graphics.o \
	intro.o \
	metaengine.o \
	resources.o \
	room.o \
	sprite_sheet.o \
	video.o

MODULE_DIRS += \
	engines/dune

# This module can be built as a plugin
ifeq ($(ENABLE_DUNE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
