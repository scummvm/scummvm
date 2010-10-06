MODULE := engines/asylum

MODULE_OBJS := \
	resources/actionarea.o \
	resources/actionlist.o \
	resources/actor.o \
	resources/barrier.o \
	resources/polygons.o \
	resources/worldstats.o \
	system/config.o \
	system/cursor.o \
	system/graphics.o \
	system/screen.o \
	system/sound.o \
	system/speech.o \
	system/text.o \
	system/video.o \
	views/menu.o \
	views/scene.o \
	asylum.o \
	detection.o \
	console.o \
	respack.o 

# This module can be built as a plugin
ifeq ($(ENABLE_ASYLUM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
