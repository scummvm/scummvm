MODULE := queen

MODULE_OBJS = \
	queen/command.o \
	queen/cutaway.o \
	queen/debug.o \
	queen/display.o \
	queen/graphics.o \
	queen/input.o \
	queen/logic.o \
	queen/queen.o \
	queen/resource.o \
	queen/restables.o \
	queen/sound.o \
	queen/talk.o \
	queen/walk.o

MODULE_DIRS += \
	queen

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include common.rules
