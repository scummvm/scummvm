MODULE := engines/parallaction

MODULE_OBJS := \
	animation.o \
	archive.o \
	callables.o \
	commands.o \
	debug.o \
	detection.o \
	dialogue.o \
	disk.o \
	font.o \
	graphics.o \
	intro.o \
	inventory.o \
	location.o \
	menu.o \
	parser.o \
	parallaction.o \
	saveload.o \
	sound.o \
	staticres.o \
	walk.o \
	zone.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
