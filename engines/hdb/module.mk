MODULE := engines/hdb

MODULE_OBJS := \
	ai-bots.o \
	ai-cinematic.o \
	ai-funcs.o \
	ai-init.o \
	ai-inventory.o \
	ai-lists.o \
	ai-player.o \
	ai-use.o \
	ai-waypoint.o \
	file-manager.o \
	gfx.o \
	hdb.o \
	input.o \
	lua-script.o \
	map.o \
	menu.o \
	metaengine.o \
	sound.o \
	saveload.o \
	window.o

MODULE_DIRS += \
	engines/hdb

# This module can be built as a plugin
ifeq ($(ENABLE_HDB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
