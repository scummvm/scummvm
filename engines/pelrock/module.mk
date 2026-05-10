MODULE := engines/pelrock

MODULE_OBJS = \
	actions.o \
	backgroundbook.o \
	cdplayer.o \
	chrono.o \
	computer.o \
	console.o \
	dialog.o \
	events.o \
	fonts/small_font.o \
	fonts/large_font.o \
	fonts/small_font_double.o \
	graphics.o \
	menu.o \
	metaengine.o \
	pathfinding.o \
	pelrock.o \
	resources.o \
	room.o \
	saveload.o \
	slidingpuzzle.o \
	sound.o \
	spellbook.o \
	util.o \
	video.o

# This module can be built as a plugin
ifeq ($(ENABLE_PELROCK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
