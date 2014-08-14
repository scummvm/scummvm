MODULE := engines/access

MODULE_OBJS := \
	animation.o \
	asurface.o \
	access.o \
	data.o \
	debugger.o \
	decompress.o \
	detection.o \
	events.o \
	files.o \
	inventory.o \
	player.o \
	resources.o \
	room.o \
	screen.o \
	scripts.o \
	sound.o \
	amazon/amazon_game.o \
	amazon/amazon_resources.o \
	amazon/amazon_room.o \
	amazon/amazon_scripts.o

# This module can be built as a plugin
ifeq ($(ENABLE_ACCESS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
