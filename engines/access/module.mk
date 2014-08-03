MODULE := engines/access

MODULE_OBJS := \
	access.o \
	debugger.o \
	decompress.o \
	detection.o \
	events.o \
	files.o \
	graphics.o \
	resources.o \
	screen.o \
	sound.o \
	amazon\amazon_game.o

# This module can be built as a plugin
ifeq ($(ENABLE_ACCESS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
