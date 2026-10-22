MODULE := engines/harvester

MODULE_OBJS = \
	art.o \
	bitmap.o \
	cft_font.o \
	cheats.o \
	console.o \
	dialogue/dialogue.o \
	dialogue/dialogue_menu.o \
	dialogue/dialogue_playback.o \
	dialogue/dialogue_render.o \
	dialogue/dialogue_session.o \
	flow/flow.o \
	flow/flow_debug.o \
	flow/flow_map.o \
	flow/flow_scene.o \
	flow/flow_startup.o \
	flow/flow_transition.o \
	fst_player.o \
	harvester.o \
	inventory.o \
	media/audio_decoder.o \
	media/media_manager.o \
	media/media_playback.o \
	menu/menu.o \
	menu/menu_config.o \
	menu/menu_navigation.o \
	menu/menu_options.o \
	menu/menu_prompts.o \
	menu/menu_render.o \
	menu/menu_save.o \
	metaengine.o \
	monster.o \
	patches.o \
	player/player.o \
	player/player_animation.o \
	player/player_attack.o \
	player/player_idle.o \
	player/player_keyboard.o \
	player/player_movement.o \
	player/player_reactions.o \
	player/player_target.o \
	resources.o \
	combat/combat.o \
	combat/combat_contact.o \
	combat/combat_effects.o \
	combat/combat_geometry.o \
	combat/combat_hud.o \
	combat/combat_monsters.o \
	combat/combat_reactions.o \
	combat/combat_targeting.o \
	scene/interaction.o \
	scene/interaction_effects.o \
	scene/interaction_outcomes.o \
	scene/interaction_transitions.o \
	scene/input.o \
	scene/input_inventory.o \
	scene/input_keyboard.o \
	scene/room_presentation.o \
	scene/scene_entities.o \
	scene/scene.o \
	entity.o \
	saveload.o \
	script/script.o \
	script/script_actors.o \
	script/script_commands.o \
	script/script_records.o \
	script/script_room.o \
	script/script_runtime.o \
	script/script_save.o \
	text.o \
	xfile_archive.o \
	npc/authority_dialogue.o \
	npc/beggar_dialogue.o \
	npc/boyle_dialogue.o \
	npc/buster_dialogue.o \
	npc/butcher_dialogue.o \
	npc/chessmaster_dialogue.o \
	npc/cloak_atnd_dialogue.o \
	npc/curator_dialogue.o \
	npc/dad_dialogue.o \
	npc/dark_woman_dialogue.o \
	npc/dwayne_dialogue.o \
	npc/edna_dialogue.o \
	npc/fireman1_dialogue.o \
	npc/fireman2_dialogue.o \
	npc/gladiator_dialogue.o \
	npc/hank_dialogue.o \
	npc/herrill_dialogue.o \
	npc/inquisitor_dialogue.o \
	npc/jimmy_dialogue.o \
	npc/johnson_dialogue.o \
	npc/karin_dialogue.o \
	npc/librarian_dialogue.o \
	npc/lodge_chef_dialogue.o \
	npc/loomis_dialogue.o \
	npc/madam_dialogue.o \
	npc/maint_man_dialogue.o \
	npc/mcknight_dialogue.o \
	npc/memb_dir_dialogue.o \
	npc/mom_dialogue.o \
	npc/mother_dialogue.o \
	npc/moynahan_dialogue.o \
	npc/mr_potts_dialogue.o \
	npc/mrs_potts_dialogue.o \
	npc/nude_man_dialogue.o \
	npc/parsons_dialogue.o \
	npc/pastorelli_dialogue.o \
	npc/phelps_dialogue.o \
	npc/priest_dialogue.o \
	npc/pta_mom_dialogue.o \
	npc/ryder_dialogue.o \
	npc/sergeant_dialogue.o \
	npc/sparky_dialogue.o \
	npc/stephanie_dialogue.o \
	npc/stub_dialogue.o \
	npc/swell_dialogue.o \
	npc/valet_dialogue.o \
	npc/vet_dialogue.o \
	npc/wasp_woman_dialogue.o \
	npc/whaley_dialogue.o

# This module can be built as a plugin
ifeq ($(ENABLE_HARVESTER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
