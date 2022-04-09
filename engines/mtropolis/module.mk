MODULE := engines/mtropolis

MODULE_OBJS = \
	console.o \
	data.o \
	detection.o \
	metaengine.o \
	miniscript.o \
	modifiers.o \
	modifier_factory.o \
	mtropolis.o \
	plugin_obsidian.o \
	plugin_obsidian_data.o \
	plugin_standard.o \
	plugin_standard_data.o \
	runtime.o \
	vthread.o

# This module can be built as a plugin
ifeq ($(ENABLE_MTROPOLIS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
