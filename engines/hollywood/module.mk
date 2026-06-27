MODULE := engines/hollywood

MODULE_OBJS = \
	font.o \
	gameplay/cursor.o \
	gameplay/hotspots.o \
	graphics.o \
	hollywood.o \
	metaengine.o \
	music.o \
	resource.o \
	scenes/intro/scene9000.o \
	scenes/intro/scene9010.o \
	scenes/intro/scene9050.o \
	scenes/intro/scene9100.o \
	scenes/intro/scene9110.o \
	scenes/intro/scene9120.o \
	scenes/playable/scene7000.o \
	scenes/playable/scene7010.o

# This module can be built as a plugin
ifeq ($(ENABLE_HOLLYWOOD), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
