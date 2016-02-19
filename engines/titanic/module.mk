MODULE := engines/titanic

MODULE_OBJS := \
	compressed_file.o \
	detection.o \
	direct_draw.o \
	font.o \
	image.o \
	main_game_window.o \
	screen_manager.o \
	simple_file.o \
	string.o \
	titanic.o \
	video_surface.o \
	objects/file_item.o \
	objects/list.o \
	objects/message_target.o \
	objects/project_item.o \
	objects/saveable_object.o \
	objects/tree_item.o

# This module can be built as a plugin
ifeq ($(ENABLE_TITANIC), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
