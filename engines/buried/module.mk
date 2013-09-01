MODULE := engines/buried

MODULE_OBJS = \
	avi_frames.o \
	buried.o \
	database.o \
	detection.o \
	graphics.o \
	livetext.o \
	sound.o \
	video_window.o \
	window.o


# This module can be built as a plugin
ifeq ($(ENABLE_BURIED), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
