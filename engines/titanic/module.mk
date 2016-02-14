MODULE := engines/titanic

MODULE_OBJS := \
	detection.o \
	direct_draw.o \
	files.o \
	font.o \
	image.o \
	main_game_window.o \
	saveable_object.o \
	screen_manager.o \
	string.o \
	titanic.o \
	video_surface.o

# This module can be built as a plugin
ifeq ($(ENABLE_TITANIC), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
