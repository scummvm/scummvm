MODULE := engines/sherlock

MODULE_OBJS = \
	scalpel/chess.o \
	scalpel/darts.o \
	scalpel/scalpel.o \
	tattoo/tattoo.o \
	animation.o \
	decompress.o \
	debugger.o \
	detection.o \
	events.o \
	graphics.o \
	journal.o \
	people.o \
	resources.o \
	scene.o \
	screen.o \
	sherlock.o \
	sound.o \
	sprites.o \
	talk.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
