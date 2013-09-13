MODULE := engines/buried

MODULE_OBJS = \
	avi_frames.o \
	biochip_right.o \
	buried.o \
	credits.o \
	database.o \
	detection.o \
	frame_window.o \
	gameui.o \
	graphics.o \
	livetext.o \
	main_menu.o \
	navarrow.o \
	overview.o \
	sound.o \
	title_sequence.o \
	video_window.o \
	window.o \
	demo/demo_menu.o \
	demo/features.o \
	demo/movie_scene.o


# This module can be built as a plugin
ifeq ($(ENABLE_BURIED), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
