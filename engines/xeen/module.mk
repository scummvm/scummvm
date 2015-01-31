MODULE := engines/xeen

MODULE_OBJS := \
	clouds\clouds_game.o \
	darkside\darkside_game.o \
	worldofxeen\worldofxeen_game.o \
	combat.o \
	debugger.o \
	detection.o \
	dialogs.o \
	automap.o \
	dialogs_automap.o \
	dialogs_confirm.o \
	dialogs_error.o \
	dialogs_options.o \
	dialogs_input.o \
	dialogs_spells.o \
	dialogs_whowill.o \
	dialogs_yesno.o \
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
