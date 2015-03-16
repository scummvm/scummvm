MODULE := engines/sherlock

MODULE_OBJS = \
	scalpel/scalpel.o \
	tattoo/tattoo.o \
	animation.o \
	decompress.o \
	debugger.o \
	detection.o \
	events.o \
	graphics.o \
	journal.o \
	resources.o \
	room.o \
	sherlock.o \
	sound.o \
	sprite.o \
	talk.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
