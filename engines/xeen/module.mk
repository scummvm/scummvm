MODULE := engines/xeen

MODULE_OBJS := \
	worldofxeen/clouds_cutscenes.o \
	worldofxeen/darkside_cutscenes.o \
	worldofxeen/worldofxeen_cutscenes.o \
	worldofxeen/worldofxeen_menu.o \
	worldofxeen/worldofxeen.o \
	worldofxeen/worldofxeen_resources.o \
	swordsofxeen/swordsofxeen.o \
	character.o \
	combat.o \
	cutscenes.o \
	debugger.o \
	detection.o \
	dialogs.o \
	dialogs_awards.o \
	dialogs_char_info.o \
	dialogs_control_panel.o \
	dialogs_create_char.o \
	dialogs_dismiss.o \
	dialogs_exchange.o \
	dialogs_fight_options.o \
	dialogs_info.o \
	dialogs_input.o \
	dialogs_items.o \
	dialogs_map.o \
	dialogs_message.o \
	dialogs_party.o \
	dialogs_query.o \
	dialogs_quests.o \
	dialogs_quick_ref.o \
	dialogs_spells.o \
	dialogs_whowill.o \
	events.o \
	files.o \
	font.o \
	interface.o \
	interface_minimap.o \
	interface_scene.o \
	locations.o \
	map.o \
	music.o \
	party.o \
	resources.o \
	saves.o \
	screen.o \
	scripts.o \
	sound.o \
	spells.o \
	sprites.o \
	window.o \
	xeen.o \
	xsurface.o

# This module can be built as a plugin
ifeq ($(ENABLE_XEEN), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
