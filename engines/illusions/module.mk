MODULE := engines/illusions

MODULE_OBJS := \
	actor.o \
	actorresource.o \
	backgroundresource.o \
	camera.o \
	detection.o \
	fixedpoint.o \
	graphics.o \
	illusions.o \
	input.o \
	resourcesystem.o \
	scriptman.o \
	scriptopcodes.o \
	scriptresource.o \
	scriptthread.o \
	spritedecompressqueue.o \
	spritedrawqueue.o \
	thread.o \
	time.o \
	updatefunctions.o

# This module can be built as a plugin
ifeq ($(ENABLE_ILLUSIONS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
