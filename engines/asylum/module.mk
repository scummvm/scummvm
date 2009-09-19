MODULE := engines/asylum

MODULE_OBJS := \
	actionlist.o \
	actor.o \
	asylum.o \
	barrier.o \
	blowuppuzzle.o \
	console.o \
	cursor.o \
	detection.o \
	encounters.o \
	graphics.o \
	menu.o \
	polygons.o \
	respack.o \
	scene.o \
	screen.o \
	shared.o \
	sound.o \
	text.o \
	video.o \
	worldstats.o

# This module can be built as a plugin
ifeq ($(ENABLE_ASYLUM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
