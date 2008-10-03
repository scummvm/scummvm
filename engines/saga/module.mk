MODULE := engines/saga

MODULE_OBJS := \
	actor.o \
	actor_path.o \
	actor_walk.o \
	animation.o \
	console.o \
	detection.o \
	events.o \
	font.o \
	font_map.o \
	gfx.o \
	image.o \
	input.o \
	interface.o \
	introproc_ihnm.o \
	introproc_ite.o \
	isomap.o \
	itedata.o \
	music.o \
	objectmap.o \
	palanim.o \
	puzzle.o \
	render.o \
	rscfile.o \
	saga.o \
	saveload.o \
	scene.o \
	script.o \
	sfuncs.o \
	sndres.o \
	sound.o \
	sprite.o \
	sthread.o

# This module can be built as a plugin
ifeq ($(ENABLE_SAGA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
