MODULE := engines/made

MODULE_OBJS := \
	console.o \
	database.o \
	graphics.o \
	made.o \
	metaengine.o \
	music.o \
	pmvplayer.o \
	redreader.o \
	resource.o \
	screen.o \
	screenfx.o \
	script.o \
	scriptfuncs.o \
	sound.o


# This module can be built as a plugin
ifeq ($(ENABLE_MADE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
