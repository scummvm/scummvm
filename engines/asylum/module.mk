MODULE := engines/asylum

MODULE_OBJS := \
	asylum.o \
	detection.o \
	graphics.o \
	menu.o \
	resourcepack.o \
	scene.o \
	scene_resource.o \
	screen.o \
	sound.o \
	text.o \
	video.o

# This module can be built as a plugin
ifeq ($(ENABLE_ASYLUM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
