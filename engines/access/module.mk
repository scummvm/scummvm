MODULE := engines/access

MODULE_OBJS := \
	access.o \
	animation.o \
	asurface.o \
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
	polygon.o \
	resources.o \
	room.o \
	screen.o \
	scripts.o \
	sound.o \
	video.o \
	amazon/amazon_game.o \
	amazon/amazon_inventory.o \
	amazon/amazon_logic.o \
	amazon/amazon_player.o \
	amazon/amazon_resources.o \
	amazon/amazon_room.o \
	amazon/amazon_scripts.o \
	martian/martian_duct.o \
	martian/martian_game.o \
	martian/martian_inventory.o \
	martian/martian_player.o \
	martian/martian_resources.o \
	martian/martian_room.o \
	martian/martian_scripts.o \
	noctropolis/noctropolis_comicviewer.o \
	noctropolis/noctropolis_font.o \
	noctropolis/noctropolis_game.o \
	noctropolis/noctropolis_inventory.o \
	noctropolis/noctropolis_last_comic.o \
	noctropolis/noctropolis_player.o \
	noctropolis/noctropolis_resources.o \
	noctropolis/noctropolis_room.o \
	noctropolis/noctropolis_scripts.o \
	noctropolis/noctropolis_special_comic.o \
	martian/midiparser_bemd.o \
	video/movie_decoder.o

# This module can be built as a plugin
ifeq ($(ENABLE_ACCESS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
