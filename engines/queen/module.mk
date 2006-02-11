MODULE := engines/queen

MODULE_OBJS := \
	engines/queen/bankman.o \
	engines/queen/command.o \
	engines/queen/credits.o \
	engines/queen/cutaway.o \
	engines/queen/debug.o \
	engines/queen/display.o \
	engines/queen/graphics.o \
	engines/queen/grid.o \
	engines/queen/input.o \
	engines/queen/journal.o \
	engines/queen/logic.o \
	engines/queen/music.o \
	engines/queen/musicdata.o \
	engines/queen/queen.o \
	engines/queen/resource.o \
	engines/queen/restables.o \
	engines/queen/sound.o \
	engines/queen/state.o \
	engines/queen/talk.o \
	engines/queen/walk.o

MODULE_DIRS += \
	engines/queen

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
