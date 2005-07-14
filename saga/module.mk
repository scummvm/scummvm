MODULE := saga

MODULE_OBJS := \
	saga/actor.o \
	saga/animation.o \
	saga/console.o \
	saga/events.o \
	saga/font.o \
	saga/font_map.o \
	saga/game.o \
	saga/gfx.o \
	saga/ihnm_introproc.o \
	saga/image.o \
	saga/interface.o \
	saga/isomap.o \
	saga/ite_introproc.o \
	saga/itedata.o \
	saga/objectmap.o \
	saga/puzzle.o \
	saga/palanim.o \
	saga/render.o \
	saga/rscfile.o \
	saga/saga.o \
	saga/saveload.o \
	saga/scene.o \
	saga/script.o \
	saga/sfuncs.o \
	saga/sndres.o \
	saga/sprite.o \
	saga/sthread.o \
	saga/input.o \
	saga/music.o \
	saga/sound.o

MODULE_DIRS += \
	saga

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
