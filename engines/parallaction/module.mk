MODULE := engines/parallaction

MODULE_OBJS := \
	animation.o \
	archive.o \
	callables.o \
	commands.o \
	debug.o \
	detection.o \
	dialogue.o \
	graphics.o \
	intro.o \
	inventory.o \
	location.o \
	menu.o \
	music.o \
	parser.o \
	parallaction.o \
	saveload.o \
	staticres.o \
	table.o \
	walk.o \
	zone.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
