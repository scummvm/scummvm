MODULE := engines/asylum

MODULE_OBJS := \
	asylum.o \
	console.o \
	detection.o \
	graphics.o \
	menu.o \
	respack.o \
	scene.o \
	barrier.o \
	sceneres.o \
	screen.o \
	sound.o \
	text.o \
	video.o \
	actor.o \
	encounters.o \
	scriptman.o \
	blowuppuzzle.o \
	shared.o \
	cursor.o \
	polygons.o \
	actionlist.o

# This module can be built as a plugin
ifeq ($(ENABLE_ASYLUM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
