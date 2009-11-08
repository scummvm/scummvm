MODULE := engines/teenagent

MODULE_OBJS := \
	detection.o \
	teenagent.o \
	resources.o \
	pack.o \
	segment.o \
	scene.o \
	animation.o \
	font.o \
	surface.o \
	actor.o \
	callbacks.o \
	inventory.o \
	objects.o \
	music.o \
	console.o \
	dialog.o 

# This module can be built as a plugin
ifeq ($(ENABLE_TEENAGENT), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
