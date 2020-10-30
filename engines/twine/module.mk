MODULE := engines/twine

MODULE_OBJS := \
	actor.o \
	animations.o \
	collision.o \
	console.o \
	debug.o \
	debug_grid.o \
	debug_scene.o \
	detection.o \
	extra.o \
	flamovies.o \
	gamestate.o \
	grid.o \
	holomap.o \
	hqr.o \
	interface.o \
	input.o \
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
	script_life_v1.o \
	script_move_v1.o \
	sound.o \
	text.o \
	twine.o

# This module can be built as a plugin
ifeq ($(ENABLE_TWINE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
