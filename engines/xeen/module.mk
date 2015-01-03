MODULE := engines/xeen

MODULE_OBJS := \
	clouds\clouds_game.o \
	darkside\darkside_game.o \
	worldofxeen\worldofxeen_game.o \
	debugger.o \
	detection.o \
	dialogs.o \
	dialogs_options.o \
	events.o \
	files.o \
	font.o \
	interface.o \
	items.o \
	party.o \
	resdata.o \
	resources.o \
	saves.o \
	screen.o \
	sound.o \
	sprites.o \
	xeen.o \
	xsurface.o

# This module can be built as a plugin
ifeq ($(ENABLE_XEEN), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
