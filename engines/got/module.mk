MODULE := engines/got

MODULE_OBJS = \
	got.o \
	console.o \
	events.o \
	messages.o \
	metaengine.o \
	sound.o \
	vars.o \
	data/actor.o \
	data/level.o \
	data/sd_data.o \
	data/setup.o \
	data/thor_info.o \
	game/boss1.o \
	game/back.o \
	game/init.o \
	game/main.o \
	game/move.o \
	game/move_patterns.o \
	game/object.o \
	game/panel.o \
	game/script.o \
	game/shot_movement.o \
	game/shot_pattern.o \
	game/special_tile.o \
	game/status.o \
	gfx/font.o \
	gfx/gfx_chunks.o \
	gfx/gfx_pics.o \
	gfx/gfx_surface.o \
	gfx/image.o \
	gfx/palette.o \
	utils/compression.o \
	utils/file.o \
	utils/res_archive.o \
	views/view.o \
	views/game.o \
	views/game_content.o \
	views/game_status.o \
	views/part_title.o \
	views/story.o \
	views/title.o \
	views/dialogs/dialog.o \
	views/dialogs/select_option.o \
	views/dialogs/ask.o \
	views/dialogs/main_menu.o \
	views/dialogs/options_menu.o \
	views/dialogs/play_game.o \
	views/dialogs/quit.o \
	views/dialogs/quit_game.o \
	views/dialogs/say.o \
	views/dialogs/select_game.o \
	views/dialogs/select_item.o \
	views/dialogs/set_sound.o

# This module can be built as a plugin
ifeq ($(ENABLE_GOT), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
