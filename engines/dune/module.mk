MODULE := engines/dune

MODULE_OBJS = \
	adlib.o \
	animation_clock.o \
	archive.o \
	battle.o \
	book.o \
	condit.o \
	console.o \
	defeat.o \
	dialogue.o \
	dune.o \
	event.o \
	font.o \
	flight.o \
	globe_animation.o \
	graphics.o \
	intro.o \
	map.o \
	map_cards.o \
	map_region.o \
	map_renderer.o \
	map_units.o \
	map_scenery.o \
	metaengine.o \
	music.o \
	notices.o \
	resources.o \
	game.o \
	room.o \
	sound_effect.o \
	sprite_sheet.o \
	video.o

MODULE_DIRS += \
	engines/dune

# This module can be built as a plugin
ifeq ($(ENABLE_DUNE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
