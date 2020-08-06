MODULE := engines/hopkins

MODULE_OBJS := \
	anim.o \
	computer.o \
	debugger.o \
	dialogs.o \
	events.o \
	files.o \
	font.o \
	graphics.o \
	globals.o \
	hopkins.o \
	lines.o \
	menu.o \
	metaengine.o \
	objects.o \
	saveload.o \
	script.o \
	sound.o \
	talk.o

# This module can be built as a plugin
ifeq ($(ENABLE_HOPKINS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
