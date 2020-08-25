MODULE := engines/pink

MODULE_OBJS = \
	archive.o \
	audio_info_mgr.o \
	cel_decoder.o \
	console.o \
	cursor_mgr.o \
	director.o \
	file.o \
	gui.o \
	metaengine.o \
	pda_mgr.o \
	pink.o \
	resource_mgr.o \
	saveload.o \
	sound.o \
	objects/object.o \
	objects/module.o \
	objects/inventory.o \
	objects/side_effect.o \
	objects/condition.o \
	objects/actions/action.o \
	objects/actions/action_cel.o \
	objects/actions/action_hide.o \
	objects/actions/action_loop.o \
	objects/actions/action_play.o \
	objects/actions/action_play_with_sfx.o \
	objects/actions/action_sound.o \
	objects/actions/action_still.o \
	objects/actions/action_talk.o \
	objects/actions/action_text.o \
	objects/actions/walk_action.o \
	objects/actors/actor.o \
	objects/actors/audio_info_pda_button.o \
	objects/actors/lead_actor.o \
	objects/actors/pda_button_actor.o \
	objects/actors/supporting_actor.o \
	objects/handlers/handler.o \
	objects/handlers/handler_mgr.o \
	objects/pages/game_page.o \
	objects/pages/page.o \
	objects/pages/pda_page.o \
	objects/sequences/seq_timer.o \
	objects/sequences/sequence.o \
	objects/sequences/sequence_context.o \
	objects/sequences/sequence_item.o \
	objects/sequences/sequencer.o \
	objects/walk/walk_mgr.o \
	objects/walk/walk_location.o \
	objects/walk/walk_shortest_path.o

# This module can be built as a plugin
ifeq ($(ENABLE_PINK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
