MODULE := engines/access

MODULE_OBJS := \
	asurface.o \
	access.o \
	debugger.o \
	decompress.o \
	detection.o \
	events.o \
	files.o \
	globals.o \
	resources.o \
	screen.o \
	sound.o \
	amazon\amazon_game.o \
	amazon\amazon_globals.o

# This module can be built as a plugin
ifeq ($(ENABLE_ACCESS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
