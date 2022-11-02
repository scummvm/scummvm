MODULE := engines/mads

MODULE_OBJS := \
	dragonsphere/game_dragonsphere.o \
	dragonsphere/dragonsphere_scenes.o \
	dragonsphere/dragonsphere_scenes1.o \
	dragonsphere/globals_dragonsphere.o \
	phantom/game_phantom.o \
	phantom/globals_phantom.o \
	phantom/phantom_scenes.o \
	phantom/phantom_scenes1.o \
	phantom/phantom_scenes2.o \
	phantom/phantom_scenes3.o \
	phantom/phantom_scenes4.o \
	phantom/phantom_scenes5.o \
	nebular/dialogs_nebular.o \
	nebular/game_nebular.o \
	nebular/globals_nebular.o \
	nebular/menu_nebular.o \
	nebular/sound_nebular.o \
	nebular/nebular_scenes.o \
	nebular/nebular_scenes1.o \
	nebular/nebular_scenes2.o \
	nebular/nebular_scenes3.o \
	nebular/nebular_scenes4.o \
	nebular/nebular_scenes5.o \
	nebular/nebular_scenes6.o \
	nebular/nebular_scenes7.o \
	nebular/nebular_scenes8.o \
	action.o \
	animation.o \
	assets.o \
	audio.o \
	camera.o \
	compression.o \
	conversations.o \
	debugger.o \
	dialogs.o \
	events.o \
	font.o \
	game.o \
	game_data.o \
	globals.o \
	hotspots.o \
	inventory.o \
	mads.o \
	menu_views.o \
	messages.o \
	msurface.o \
	metaengine.o \
	palette.o \
	player.o \
	rails.o \
	resources.o \
	scene.o \
	scene_data.o \
	screen.o \
	sequence.o \
	sound.o \
	sprites.o \
	staticres.o \
	user_interface.o

# This module can be built as a plugin
ifeq ($(ENABLE_MADS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
