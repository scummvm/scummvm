MODULE := engines/mads

MODULE_OBJS := \
	nebular/dialogs_nebular.o \
	nebular/game_nebular.o \
	nebular/globals_nebular.o \
	nebular/sound_nebular.o \
	nebular/nebular_scenes.o \
	nebular/nebular_scenes1.o \
	nebular/nebular_scenes8.o \
	action.o \
	animation.o \
	assets.o \
	compression.o \
	debugger.o \
	detection.o \
	dialogs.o \
	events.o \
	font.o \
	game.o \
	game_data.o \
	hotspots.o \
	interface.o \
	inventory.o \
	mads.o \
	messages.o \
	msurface.o \
	palette.o \
	player.o \
	resources.o \
	scene.o \
	scene_data.o \
	screen.o \
	sequence.o \
	sound.o \
	sprites.o \
	user_interface.o

# This module can be built as a plugin
ifeq ($(ENABLE_MADS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
