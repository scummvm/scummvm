MODULE := engines/buried

MODULE_OBJS = \
	agent_evaluation.o \
	avi_frames.o \
	biochip_right.o \
	biochip_view.o \
	buried.o \
	complete.o \
	console.o \
	credits.o \
	death.o \
	frame_window.o \
	gameui.o \
	graphics.o \
	inventory_info.o \
	inventory_window.o \
	livetext.o \
	main_menu.o \
	metaengine.o \
	navarrow.o \
	overview.o \
	saveload.o \
	scene_view.o \
	sound.o \
	title_sequence.o \
	video_window.o \
	window.o \
	demo/demo_menu.o \
	demo/features.o \
	demo/movie_scene.o \
	environ/agent3_lair.o \
	environ/ai_lab.o \
	environ/alien.o \
	environ/castle.o \
	environ/da_vinci.o \
	environ/future_apartment.o \
	environ/mayan.o \
	environ/scene_base.o \
	environ/scene_common.o \
	environ/scene_factory.o


# This module can be built as a plugin
ifeq ($(ENABLE_BURIED), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
