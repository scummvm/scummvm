MODULE := engines/harvester

MODULE_OBJS = \
	art.o \
	bitmap.o \
	cft_font.o \
	cheats.o \
	console.o \
	dialogue.o \
	flow.o \
	fst_player.o \
	harvester.o \
	inventory.o \
	media_manager.o \
	menu.o \
	menu_config.o \
	menu_navigation.o \
	menu_options.o \
	menu_prompts.o \
	menu_render.o \
	menu_save.o \
	metaengine.o \
	monster.o \
	player.o \
	player_movement.o \
	player_target.o \
	resources.o \
	combat.o \
	combat_contact.o \
	combat_effects.o \
	combat_geometry.o \
	combat_hud.o \
	combat_monsters.o \
	combat_reactions.o \
	combat_targeting.o \
	interaction.o \
	input.o \
	room_presentation.o \
	scene_entities.o \
	scene.o \
	entity.o \
	saveload.o \
	script.o \
	script_actors.o \
	script_commands.o \
	script_records.o \
	script_room.o \
	script_runtime.o \
	script_save.o \
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
