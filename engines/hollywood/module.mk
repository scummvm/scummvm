MODULE := engines/hollywood

MODULE_OBJS = \
	console.o \
	font.o \
	gameplay/actor_renderer.o \
	gameplay/cursor.o \
	gameplay/dialogue_menu.o \
	gameplay/game_loop.o \
	gameplay/hotspots.o \
	gameplay/inventory_actions.o \
	gameplay/options_menu.o \
	gameplay/panel_art.o \
	graphics.o \
	hollywood.o \
	metaengine.o \
	music.o \
	resource.o \
	saveload.o \
	scenes/intro/scene1000.o \
	scenes/intro/scene9000.o \
	scenes/intro/scene9010.o \
	scenes/intro/scene9050.o \
	scenes/intro/scene9100.o \
	scenes/intro/scene9110.o \
	scenes/intro/scene9120.o \
	scenes/playable/scene1010.o \
	scenes/playable/scene1020.o \
	scenes/playable/scene1030.o \
	scenes/playable/scene1040.o \
	scenes/playable/scene1050.o \
	scenes/playable/scene1060.o \
	scenes/playable/scene1070.o \
	scenes/playable/scene1080.o \
	scenes/playable/scene1090.o \
	scenes/playable/scene7000.o \
	scenes/playable/scene7010.o \
	scenes/playable/scene7020.o \
	scenes/playable/scene7030.o \
	scenes/playable/scene7040.o \
	scenes/playable/scene7050.o \
	scenes/playable/scene7060.o \
	scenes/playable/scene7070.o \
	scenes/playable/scene7080.o \
	scenes/playable/scene7090.o \
	scenes/playable/scene7100.o \
	scenes/playable/playable_scene.o

# This module can be built as a plugin
ifeq ($(ENABLE_HOLLYWOOD), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
