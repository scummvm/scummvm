MODULE := saga

MODULE_OBJS = \
	saga/actionmap.o \
	saga/actor.o \
	saga/actordata.o \
	saga/animation.o \
	saga/console.o \
	saga/cvar.o \
	saga/events.o \
	saga/expr.o \
	saga/font.o \
	saga/font_map.o \
	saga/game.o \
	saga/gfx.o \
	saga/ihnm_introproc.o \
	saga/image.o \
	saga/interface.o \
	saga/isomap.o \
	saga/ite_introproc.o \
	saga/objectmap.o \
	saga/palanim.o \
	saga/render.o \
	saga/rscfile.o \
	saga/saga.o \
	saga/scene.o \
	saga/sceneproc.o \
	saga/script.o \
	saga/sdata.o \
	saga/sdebug.o \
	saga/sfuncs.o \
	saga/sndres.o \
	saga/sprite.o \
	saga/sstack.o \
	saga/sthread.o \
	saga/text.o \
	saga/transitions.o \
	saga/ys_dl_list.o \
	saga/input.o \
	saga/timer.o \
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
