MODULE := engines/hdb

MODULE_OBJS := \
	ai-init.o \
	ai-inventory.o \
	ai-cinematic.o \
	ai-funcs.o \
	ai-player.o \
	ai-waypoint.o \
	console.o \
	draw-manager.o \
	detection.o \
	file-manager.o \
	hdb.o \
	lua-script.o \
	map-loader.o

MODULE_DIRS += \
	engines/hdb

# This module can be built as a plugin
ifeq ($(ENABLE_HDB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
