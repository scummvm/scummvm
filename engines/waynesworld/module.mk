MODULE := engines/waynesworld

MODULE_OBJS := \
	waynesworld.o \
	detection.o \
	gamelogic.o \
	gamemap.o \
	gxlarchive.o \
	graphics.o \
	intro.o \
	metaengine.o \
	pathfinding.o \
	sound.o \
	staticdata.o

# This module can be built as a plugin
ifeq ($(ENABLE_WAYNESWORLD), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
