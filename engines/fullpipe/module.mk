MODULE := engines/fullpipe

MODULE_OBJS = \
	anihandler.o \
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
	statesaver.o \
	statics.o \
	utils.o \
	scenes/sceneIntro.o \
	scenes/sceneIntroDemo.o \
	scenes/scene01.o \
	scenes/scene02.o \
	scenes/scene03.o \
	scenes/scene04.o \
	scenes/scene05.o \
	scenes/scene06.o \
	scenes/scene07.o \
	scenes/scene08.o \
	scenes/scene09.o \
	scenes/scene10.o \
	scenes/scene11.o \
	scenes/scene12.o \
	scenes/scene13.o \
	scenes/scene14.o \
	scenes/scene15.o \
	scenes/scene16.o \
	scenes/scene17.o \
	scenes/scene18and19.o \
	scenes/scene20.o \
	scenes/scene21.o \
	scenes/scene22.o \
	scenes/scene23.o \
	scenes/scene24.o \
	scenes/scene25.o \
	scenes/scene26.o \
	scenes/scene27.o \
	scenes/scene28.o \
	scenes/scene29.o \
	scenes/scene30.o \
	scenes/scene31.o \
	scenes/scene32.o \
	scenes/scene33.o \
	scenes/scene34.o \
	scenes/scene35.o \
	scenes/scene36.o \
	scenes/scene37.o \
	scenes/scene38.o \
	scenes/sceneFinal.o \
	scenes/sceneDbg.o

# This module can be built as a plugin
ifeq ($(ENABLE_FULLPIPE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
