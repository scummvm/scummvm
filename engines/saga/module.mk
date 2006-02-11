MODULE := engines/saga

MODULE_OBJS := \
	engines/saga/actor.o \
	engines/saga/animation.o \
	engines/saga/console.o \
	engines/saga/events.o \
	engines/saga/font.o \
	engines/saga/font_map.o \
	engines/saga/game.o \
	engines/saga/gfx.o \
	engines/saga/ihnm_introproc.o \
	engines/saga/image.o \
	engines/saga/interface.o \
	engines/saga/isomap.o \
	engines/saga/ite_introproc.o \
	engines/saga/itedata.o \
	engines/saga/objectmap.o \
	engines/saga/puzzle.o \
	engines/saga/palanim.o \
	engines/saga/render.o \
	engines/saga/rscfile.o \
	engines/saga/saga.o \
	engines/saga/saveload.o \
	engines/saga/scene.o \
	engines/saga/script.o \
	engines/saga/sfuncs.o \
	engines/saga/sndres.o \
	engines/saga/sprite.o \
	engines/saga/sthread.o \
	engines/saga/input.o \
	engines/saga/music.o \
	engines/saga/sound.o

MODULE_DIRS += \
	engines/saga

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
