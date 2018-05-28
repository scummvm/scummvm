MODULE := engines/startrek

MODULE_OBJS = \
	awaymission.o \
	bitmap.o \
	common.o \
	detection.o \
	events.o \
	filestream.o \
	font.o \
	graphics.o \
	iwfile.o \
	lzss.o \
	menu.o \
	object.o \
	room.o \
	sound.o \
	sprite.o \
	startrek.o \
	text.o \
	rooms/demon0.o \
	rooms/demon1.o \
	rooms/demon2.o \
	rooms/demon3.o \
	rooms/demon4.o
	


# This module can be built as a plugin
ifeq ($(ENABLE_STARTREK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
