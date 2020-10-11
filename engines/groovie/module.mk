MODULE := engines/groovie

MODULE_OBJS := \
	cell.o \
	cursor.o \
	debug.o \
	font.o \
	graphics.o \
	groovie.o \
	lzss.o \
	metaengine.o \
	music.o \
	player.o \
	resource.o \
	saveload.o \
	script.o \
	vdx.o

ifdef ENABLE_GROOVIE2
MODULE_OBJS += \
	roq.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_GROOVIE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
