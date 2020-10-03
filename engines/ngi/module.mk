MODULE := engines/ngi

MODULE_OBJS = \
	anihandler.o \
	behavior.o \
	console.o \
	floaters.o \
	gameloader.o \
	gfx.o \
	init.o \
	input.o \
	interaction.o \
	inventory.o \
	lift.o \
	messagehandlers.o \
	messages.o \
	metaengine.o \
	modal.o \
	motion.o \
	ngi.o \
	ngiarchive.o \
	scene.o \
	scenes.o \
	sound.o \
	stateloader.o \
	statesaver.o \
	statics.o \
	utils.o \
	fullpipe/sceneIntro.o \
	fullpipe/sceneIntroDemo.o \
	fullpipe/scene01.o \
	fullpipe/scene02.o \
	fullpipe/scene03.o \
	fullpipe/scene04.o \
	fullpipe/scene05.o \
	fullpipe/scene06.o \
	fullpipe/scene07.o \
	fullpipe/scene08.o \
	fullpipe/scene09.o \
	fullpipe/scene10.o \
	fullpipe/scene11.o \
	fullpipe/scene12.o \
	fullpipe/scene13.o \
	fullpipe/scene14.o \
	fullpipe/scene15.o \
	fullpipe/scene16.o \
	fullpipe/scene17.o \
	fullpipe/scene18and19.o \
	fullpipe/scene20.o \
	fullpipe/scene21.o \
	fullpipe/scene22.o \
	fullpipe/scene23.o \
	fullpipe/scene24.o \
	fullpipe/scene25.o \
	fullpipe/scene26.o \
	fullpipe/scene27.o \
	fullpipe/scene28.o \
	fullpipe/scene29.o \
	fullpipe/scene30.o \
	fullpipe/scene31.o \
	fullpipe/scene32.o \
	fullpipe/scene33.o \
	fullpipe/scene34.o \
	fullpipe/scene35.o \
	fullpipe/scene36.o \
	fullpipe/scene37.o \
	fullpipe/scene38.o \
	fullpipe/sceneFinal.o \
	fullpipe/sceneDbg.o

# This module can be built as a plugin
ifeq ($(ENABLE_NGI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
