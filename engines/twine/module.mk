MODULE := engines/twine

MODULE_OBJS := \
	actor.o \
	animations.o \
	collision.o \
	debug.o \
	debug.grid.o \
	debug.scene.o \
	detection.o \
	extra.o \
	fcaseopen.o \
	filereader.o \
	flamovies.o \
	gamestate.o \
	grid.o \
	holomap.o \
	hqrdepack.o \
	interface.o \
	keyboard.o \
	lbaengine.o \
	main.o \
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
	script.life.o \
	script.move.o \
	sdlengine.o \
	sound.o \
	sys.o \
	text.o \
	xmidi.o

# This module can be built as a plugin
ifeq ($(ENABLE_TWINE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
