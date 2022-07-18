MODULE := engines/mtropolis

MODULE_OBJS = \
	asset_factory.o \
	assets.o \
	audio_player.o \
	boot.o \
	core.o \
	data.o \
	debug.o \
	element_factory.o \
	elements.o \
	hacks.o \
	metaengine.o \
	miniscript.o \
	modifiers.o \
	modifier_factory.o \
	mtropolis.o \
	plugin/obsidian.o \
	plugin/obsidian_data.o \
	plugin/standard.o \
	plugin/standard_data.o \
	render.o \
	runtime.o \
	saveload.o \
	vthread.o

# This module can be built as a plugin
ifeq ($(ENABLE_MTROPOLIS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
