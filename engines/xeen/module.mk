MODULE := engines/xeen

MODULE_OBJS := \
	clouds\clouds_game.o \
	darkside\darkside_game.o \
	worldofxeen\worldofxeen_game.o \
	character.o \
	combat.o \
	debugger.o \
	detection.o \
	dialogs.o \
	automap.o \
	dialogs_automap.o \
	dialogs_char_info.o \
	dialogs_control_panel.o \
	dialogs_dismiss.o \
	dialogs_error.o \
	dialogs_exchange.o \
	dialogs_fight_options.o \
	dialogs_options.o \
	dialogs_info.o \
	dialogs_input.o \
	dialogs_items.o \
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
	interface_map.o \
	items.o \
	map.o \
	party.o \
	resources.o \
	saves.o \
	screen.o \
	scripts.o \
	sound.o \
	spells.o \
	sprites.o \
	town.o \
	xeen.o \
	xsurface.o

# This module can be built as a plugin
ifeq ($(ENABLE_XEEN), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
