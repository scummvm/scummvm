MODULE := engines/buried

MODULE_OBJS = \
	avi_frames.o \
	buried.o \
	credits.o \
	database.o \
	detection.o \
	graphics.o \
	livetext.o \
	main_menu.o \
	overview.o \
	sound.o \
	title_sequence.o \
	video_window.o \
	window.o \
	demo/demo_menu.o


# This module can be built as a plugin
ifeq ($(ENABLE_BURIED), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
