MODULE := engines/asylum

MODULE_OBJS := \
	resources/actionlist.o \
	resources/actor.o \
	resources/object.o \
	resources/encounters.o \
	resources/polygons.o \
	resources/special.o \
	resources/worldstats.o \
	system/config.o \
	system/cursor.o \
	system/graphics.o \
	system/screen.o \
	system/sound.o \
	system/speech.o \
	system/text.o \
	system/video.o \
	views/blowuppuzzle.o \
	views/menu.o \
	views/scene.o \
	asylum.o \
	console.o \
	detection.o \
	respack.o

# This module can be built as a plugin
ifeq ($(ENABLE_ASYLUM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
