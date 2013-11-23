MODULE := engines/buried

MODULE_OBJS = \
	avi_frames.o \
	biochip_right.o \
	biochip_view.o \
	buried.o \
	complete.o \
	credits.o \
	database.o \
	death.o \
	detection.o \
	frame_window.o \
	gameui.o \
	graphics.o \
	inventory_info.o \
	inventory_window.o \
	livetext.o \
	main_menu.o \
	navarrow.o \
	overview.o \
	scene_view.o \
	sound.o \
	title_sequence.o \
	video_window.o \
	window.o \
	demo/demo_menu.o \
	demo/features.o \
	demo/movie_scene.o \
	environ/scene_base.o


# This module can be built as a plugin
ifeq ($(ENABLE_BURIED), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
