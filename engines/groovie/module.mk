MODULE := engines/groovie

MODULE_OBJS := \
	logic/cell.o \
	video/player.o \
	video/vdx.o \
	cursor.o \
	debug.o \
	font.o \
	graphics.o \
	groovie.o \
	lzss.o \
	metaengine.o \
	music.o \
	resource.o \
	saveload.o \
	script.o

ifdef ENABLE_GROOVIE2
MODULE_OBJS += \
	logic/beehive.o \
	logic/cake.o \
	logic/gallery.o \
	logic/mousetrap.o \
	logic/othello.o \
	logic/pente.o \
	logic/tlcgame.o \
	logic/triangle.o \
	logic/winerack.o \
	video/roq.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_GROOVIE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
