MODULE := engines/asylum

MODULE_OBJS := \
	asylum.o \
	console.o \
	detection.o \
	graphics.o \
	menu.o \
	respack.o \
	scene.o \
	sceneres.o \
	screen.o \
	sound.o \
	text.o \
	video.o \
	actor.o \
	encounters.o \
	scriptman.o \
	blowuppuzzle.o

# This module can be built as a plugin
ifeq ($(ENABLE_ASYLUM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
