MODULE := engines/fullpipe

MODULE_OBJS = \
	behavior.o \
	console.o \
	detection.o \
	floaters.o \
	fullpipe.o \
	gameloader.o \
	gfx.o \
	init.o \
	input.o \
	interaction.o \
	inventory.o \
	lift.o \
	messagehandlers.o \
	messages.o \
	modal.o \
	motion.o \
	ngiarchive.o \
	scene.o \
	scenes.o \
	sound.o \
	stateloader.o \
	statics.o \
	utils.o \
	scenes/scene01.o \
	scenes/scene02.o \
	scenes/scene03.o \
	scenes/scene04.o \
	scenes/scene05.o \
	scenes/scene06.o \
	scenes/scene07.o \
	scenes/scene08.o \
	scenes/scene10.o \
	scenes/scene11.o \
	scenes/scene12.o \
	scenes/scene15.o \
	scenes/scene24.o \
	scenes/sceneDbg.o \
	scenes/sceneIntro.o

# This module can be built as a plugin
ifeq ($(ENABLE_FULLPIPE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
