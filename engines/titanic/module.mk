MODULE := engines/titanic

MODULE_OBJS := \
	detection.o \
	direct_draw.o \
	font.o \
	image.o \
	list.o \
	main_game_window.o \
	saveable_object.o \
	screen_manager.o \
	titanic.o \
	video_surface.o

# This module can be built as a plugin
ifeq ($(ENABLE_TITANIC), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
