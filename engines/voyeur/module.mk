MODULE := engines/voyeur

MODULE_OBJS := \
	animation.o \
	debugger.o \
	detection.o \
	events.o \
	game.o \
	files.o \
	graphics.o \
	sound.o \
	staticres.o \
	utils.o \
	voyeur.o

# This module can be built as a plugin
ifeq ($(ENABLE_VOYEUR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
