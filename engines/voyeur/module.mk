MODULE := engines/voyeur

MODULE_OBJS := \
	animation.o \
	data.o \
	debugger.o \
	events.o \
	files.o \
	files_threads.o \
	metaengine.o \
	screen.o \
	sound.o \
	staticres.o \
	voyeur.o \
	voyeur_game.o

# This module can be built as a plugin
ifeq ($(ENABLE_VOYEUR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
