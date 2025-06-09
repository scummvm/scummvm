MODULE := engines/snatcher

MODULE_OBJS := \
	animator_scd.o \
	graphics.o \
	memory.o \
	metaengine.o \
	palette_scd.o \
	resource.o \
	saveload.o \
	scene/scene_d0.o \
	scene/scene_d1.o \
	scene/scene_d2.o \
	script.o \
	snatcher.o \
	sound.o \
	sound_device_scd.o \
	staticres.o \
	text_scd.o \
	transition_scd.o \
	ui.o \
	util.o

# This module can be built as a plugin
ifeq ($(ENABLE_SNATCHER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
