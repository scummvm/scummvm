MODULE := engines/gnap

MODULE_OBJS := \
	character.o \
	datarchive.o \
	debugger.o \
	gamesys.o \
	gnap.o \
	grid.o \
	menu.o \
	metaengine.o \
	music.o \
	resource.o \
	sound.o \
	scenes/arcade.o \
	scenes/groupcs.o \
	scenes/group0.o \
	scenes/group1.o \
	scenes/group2.o \
	scenes/group3.o \
	scenes/group4.o \
	scenes/group5.o \
	scenes/intro.o \
	scenes/scenecore.o

# This module can be built as a plugin
ifeq ($(ENABLE_GNAP), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
