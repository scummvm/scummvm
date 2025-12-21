MODULE := engines/pelrock

MODULE_OBJS = \
	pelrock.o \
	actions.o \
	chrono.o \
	console.o \
	metaengine.o \
	room.o \
	fonts/small_font.o \
	fonts/large_font.o \
	fonts/small_font_double.o \
	util.o \
	resources.o\
	sound.o \
	video/video.o \
	pathfinding.o \
	events.o \
	dialog.o \
	menu.o \
	graphics.o

# This module can be built as a plugin
ifeq ($(ENABLE_PELROCK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
