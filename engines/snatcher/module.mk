MODULE := engines/snatcher

MODULE_OBJS := \
	graphics.o \
	metaengine.o \
	palette_scd.o \
	render_scd.o \
	resource.o \
	scene/scene_d0.o \
	scene/scene_d1.o \
	scene/scene_d2.o \
	snatcher.o \
	sound.o \
	sound_device_scd.o \
	statires.o \
	transition_scd.o \
	util.o

# This module can be built as a plugin
ifeq ($(ENABLE_SNATCHER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
