MODULE := engines/fullpipe

MODULE_OBJS = \
	detection.o \
	fullpipe.o \
	gfx.o \
	inventory.o \
	messagequeue.o \
	motion.o \
	ngiarchive.o \
	scene.o \
	sound.o \
	stateloader.o \
	statics.o \
	utils.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
