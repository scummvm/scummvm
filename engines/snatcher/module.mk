MODULE := engines/snatcher

MODULE_OBJS := \
	graphics.o \
	metaengine.o \
	render_scd.o \
	resource.o \
	snatcher.o \
	sound.o \
	sound_device_null.o \
	sound_device_scd.o

# This module can be built as a plugin
ifeq ($(ENABLE_SNATCHER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
