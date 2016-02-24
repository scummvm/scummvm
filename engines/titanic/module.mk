MODULE := engines/titanic

MODULE_OBJS := \
	compressed_file.o \
	compression.o \
	detection.o \
	direct_draw.o \
	font.o \
	game_manager.o \
	game_view.o \
	image.o \
	main_game_window.o \
	screen_manager.o \
	simple_file.o \
	string.o \
	titanic.o \
	video_surface.o \
	objects/dont_save_file_item.o \
	objects/file_item.o \
	objects/game_object.o \
	objects/list.o \
	objects/message_target.o \
	objects/named_item.o \
	objects/pet_control.o \
	objects/project_item.o \
	objects/saveable_object.o \
	objects/tree_item.o

# This module can be built as a plugin
ifeq ($(ENABLE_TITANIC), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
