MODULE := engines/access

MODULE_OBJS := \
	animation.o \
	asurface.o \
	access.o \
	bubble_box.o \
	char.o \
	data.o \
	debugger.o \
	decompress.o \
	events.o \
	files.o \
	font.o \
	inventory.o \
	metaengine.o \
	player.o \
	resources.o \
	room.o \
	screen.o \
	scripts.o \
	sound.o \
	video.o \
	video/movie_decoder.o \
	amazon/amazon_game.o \
	amazon/amazon_logic.o \
	amazon/amazon_player.o \
	amazon/amazon_resources.o \
	amazon/amazon_room.o \
	amazon/amazon_scripts.o \
	martian/martian_game.o \
	martian/martian_player.o \
	martian/martian_resources.o \
	martian/martian_room.o \
	martian/martian_scripts.o

# This module can be built as a plugin
ifeq ($(ENABLE_ACCESS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
