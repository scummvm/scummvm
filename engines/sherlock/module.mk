MODULE := engines/sherlock

MODULE_OBJS = \
	scalpel/scalpel.o \
	tattoo/tattoo.o \
	decompress.o \
	detection.o \
	graphics.o \
	journal.o \
	resources.o \
	room.o \
	sherlock.o \
	sprite.o \
	talk.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
