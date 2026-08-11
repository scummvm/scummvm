MODULE := engines/dgds

MODULE_OBJS := \
	ads.o \
	clock.o \
	console.o \
	decompress.o \
	dgds.o \
	dgds_rect.o \
	dialog.o \
	dragon_native.o \
	drawing.o \
	font.o \
	game_palettes.o \
	globals.o \
	head.o \
	hoc_intro.o \
	image.o \
	inventory.o \
	menu.o \
	metaengine.o \
	parser.o \
	request.o \
	resource.o \
	scene.o \
	scene_condition.o \
	scene_op.o \
	scripts.o \
	sound.o \
	sound_raw.o \
	ttm.o \
	minigames/china_tank.o \
	minigames/china_train.o \
	minigames/dragon_arcade.o \
	minigames/dragon_arcade_ttm.o \
	minigames/shell_game.o \
	sound/midiparser_sci.o \
	sound/music.o \
	sound/drivers/adlib.o \
	sound/drivers/amigamac1.o \
	sound/drivers/cms.o \
	sound/drivers/midi.o \
	sound/drivers/midipatch.o \
	sound/resource/sci_resource.o \
	sound/resource/resource_audio.o \

# This module can be built as a plugin
ifeq ($(ENABLE_DGDS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
