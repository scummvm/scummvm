MODULE := engines/tsage

MODULE_OBJS := \
	converse.o \
	core.o \
	debugger.o \
	detection.o \
	dialogs.o \
	events.o \
	globals.o \
	graphics.o \
	resources.o \
	saveload.o \
	scene_logic.o \
	scenes.o \
	sound.o \
	staticres.o \
	tsage.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/rules.mk

