MODULE := engines/mads

MODULE_OBJS := \
	nebular/dialogs_nebular.o \
	nebular/game_nebular.o \
	nebular/sound_nebular.o \
	compression.o \
	detection.o \
	dialogs.o \
	events.o \
	font.o \
	game.o \
	graphics.o \
	mads.o \
	msprite.o \
	msurface.o \
	palette.o \
	resources.o \
	sound.o \
	user_interface.o

# This module can be built as a plugin
ifeq ($(ENABLE_MADS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
