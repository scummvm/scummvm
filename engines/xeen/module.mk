MODULE := engines/xeen

MODULE_OBJS := \
	worldofxeen/clouds_cutscenes.o \
	worldofxeen/darkside_cutscenes.o \
	worldofxeen/worldofxeen_cutscenes.o \
	worldofxeen/worldofxeen_menu.o \
	worldofxeen/worldofxeen.o \
	swordsofxeen/swordsofxeen.o \
	swordsofxeen/swordsofxeen_menu.o \
	dialogs/credits_screen.o \
	dialogs/dialogs.o \
	dialogs/dialogs_awards.o \
	dialogs/dialogs_char_info.o \
	dialogs/dialogs_control_panel.o \
	dialogs/dialogs_copy_protection.o \
	dialogs/dialogs_create_char.o \
	dialogs/dialogs_difficulty.o \
	dialogs/dialogs_dismiss.o \
	dialogs/dialogs_exchange.o \
	dialogs/dialogs_info.o \
	dialogs/dialogs_input.o \
	dialogs/dialogs_items.o \
	dialogs/dialogs_map.o \
	dialogs/dialogs_message.o \
	dialogs/dialogs_party.o \
	dialogs/dialogs_query.o \
	dialogs/dialogs_quests.o \
	dialogs/dialogs_quick_fight.o \
	dialogs/dialogs_quick_ref.o \
	dialogs/dialogs_spells.o \
	dialogs/dialogs_whowill.o \
	dialogs/please_wait.o \
	character.o \
	combat.o \
	cutscenes.o \
	debugger.o \
	events.o \
	files.o \
	font.o \
	interface.o \
	interface_minimap.o \
	interface_scene.o \
	item.o \
	locations.o \
	map.o \
	metaengine.o \
	party.o \
	patcher.o \
	resources.o \
	saves.o \
	screen.o \
	scripts.o \
	sound.o \
	sound_driver.o \
	sound_driver_adlib.o \
	spells.o \
	sprites.o \
	subtitles.o \
	window.o \
	xeen.o \
	xsurface.o

# This module can be built as a plugin
ifeq ($(ENABLE_XEEN), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
