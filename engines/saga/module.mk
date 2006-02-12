MODULE := engines/saga

MODULE_OBJS := \
	actor.o \
	animation.o \
	console.o \
	events.o \
	font.o \
	font_map.o \
	game.o \
	gfx.o \
	ihnm_introproc.o \
	image.o \
	interface.o \
	isomap.o \
	ite_introproc.o \
	itedata.o \
	objectmap.o \
	puzzle.o \
	palanim.o \
	render.o \
	rscfile.o \
	saga.o \
	saveload.o \
	scene.o \
	script.o \
	sfuncs.o \
	sndres.o \
	sprite.o \
	sthread.o \
	input.o \
	music.o \
	sound.o

MODULE_DIRS += \
	engines/saga

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
