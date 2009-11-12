MODULE := engines/m4

MODULE_OBJS = \
	actor.o \
	animation.o \
	assets.o \
	compression.o \
	console.o \
	converse.o \
	detection.o \
	events.o \
	font.o \
	globals.o \
	graphics.o \
	gui.o \
	hotspot.o \
	m4.o \
	m4_menus.o \
	m4_views.o \
	mads_anim.o \
	mads_menus.o \
	midi.o \
	rails.o \
	resource.o \
	saveload.o \
	scene.o \
	script.o \
	sound.o \
	sprite.o \
	staticres.o \
	viewmgr.o \
	woodscript.o \
	ws_machine.o \
	ws_sequence.o


# This module can be built as a plugin
ifeq ($(ENABLE_M4), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
