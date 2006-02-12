MODULE := engines/queen

MODULE_OBJS := \
	bankman.o \
	command.o \
	credits.o \
	cutaway.o \
	debug.o \
	display.o \
	graphics.o \
	grid.o \
	input.o \
	journal.o \
	logic.o \
	music.o \
	musicdata.o \
	queen.o \
	resource.o \
	restables.o \
	sound.o \
	state.o \
	talk.o \
	walk.o

MODULE_DIRS += \
	engines/queen

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
