MODULE := engines/twine

MODULE_OBJS := \
	actor.o \
	animations.o \
	collision.o \
	debug.o \
	debug_grid.o \
	debug_scene.o \
	detection.o \
	extra.o \
	flamovies.o \
	gamestate.o \
	grid.o \
	holomap.o \
	hqrdepack.o \
	interface.o \
	menu.o \
	menuoptions.o \
	metaengine.o \
	movements.o \
	music.o \
	redraw.o \
	renderer.o \
	resources.o \
	scene.o \
	screens.o \
	script_life.o \
	script_move.o \
	sound.o \
	text.o \
	twine.o \
	xmidi.o

# This module can be built as a plugin
ifeq ($(ENABLE_TWINE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
