MODULE := engines/xeen

MODULE_OBJS := \
	clouds\clouds_game.o \
	darkside\darkside_game.o \
	worldofxeen\worldofxeen_game.o \
	debugger.o \
	detection.o \
	events.o \
	resources.o \
	screen.o \
	xeen.o

# This module can be built as a plugin
ifeq ($(ENABLE_XEEN), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
