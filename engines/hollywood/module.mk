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
	gameplay/travel_screen.o \
	graphics.o \
	hollywood.o \
	metaengine.o \
	music.o \
	resource.o \
	saveload.o \
	scenes/chapter_intro_scene.o \
	scenes/intro/intro_resource_set.o \
	scenes/intro/intro_scene.o \
	scenes/intro/scene1000.o \
	scenes/intro/scene9000.o \
	scenes/intro/scene9010.o \
	scenes/intro/scene9050.o \
	scenes/intro/scene9100.o \
	scenes/intro/scene9110.o \
	scenes/intro/scene9120.o \
	scenes/playable/actor_path_controller.o \
	scenes/playable/action_overlay_player.o \
	scenes/playable/scene1010.o \
	scenes/playable/scene1020.o \
	scenes/playable/scene1030.o \
	scenes/playable/scene1040.o \
	scenes/playable/scene1050.o \
	scenes/playable/scene1060.o \
	scenes/playable/scene1070.o \
	scenes/playable/scene1080.o \
	scenes/playable/scene1090.o \
	scenes/playable/scene2000.o \
	scenes/playable/scene2010.o \
	scenes/playable/scene2020.o \
	scenes/playable/scene2030.o \
	scenes/playable/scene2040.o \
	scenes/playable/scene2050.o \
	scenes/playable/scene2060.o \
	scenes/playable/scene2070.o \
	scenes/playable/scene2080.o \
	scenes/playable/scene3000.o \
	scenes/playable/scene3010.o \
	scenes/playable/scene3020.o \
	scenes/playable/scene3030.o \
	scenes/playable/scene3040.o \
	scenes/playable/scene3050.o \
	scenes/playable/scene3060.o \
	scenes/playable/scene3070.o \
	scenes/playable/scene3080.o \
	scenes/playable/scene3090.o \
	scenes/playable/scene3100.o \
	scenes/playable/scene3110.o \
	scenes/playable/scene4000.o \
	scenes/playable/scene4010.o \
	scenes/playable/scene4020.o \
	scenes/playable/scene4030.o \
	scenes/playable/scene4040.o \
	scenes/playable/scene4050.o \
	scenes/playable/scene4060.o \
	scenes/playable/scene4070.o \
	scenes/playable/scene4080.o \
	scenes/playable/scene4090.o \
	scenes/playable/scene4100.o \
	scenes/playable/scene4110.o \
	scenes/playable/scene5000.o \
	scenes/playable/scene5010.o \
	scenes/playable/scene6000.o \
	scenes/playable/scene6010.o \
	scenes/playable/scene6020.o \
	scenes/playable/scene6030.o \
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
	scenes/playable/scene8000.o \
	scenes/playable/scene8010.o \
	scenes/playable/scene8020.o \
	scenes/playable/scene_resources.o \
	scenes/playable/scene_surface_state.o \
	scenes/playable/scene_text_store.o \
	scenes/playable/speech_controller.o \
	scenes/playable/playable_scene.o \
	scenes/playable/playable_scene_actions.o

# This module can be built as a plugin
ifeq ($(ENABLE_HOLLYWOOD), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
