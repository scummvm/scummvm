MODULE := engines/illusions

MODULE_OBJS := \
    abortablethread.o \
	actor.o \
	actorresource.o \
	backgroundresource.o \
	camera.o \
	cursor.o \
	detection.o \
	dictionary.o \
	fixedpoint.o \
	fontresource.o \
	graphics.o \
	illusions.o \
	input.o \
	resourcesystem.o \
	screen.o \
	scriptman.o \
	scriptopcodes.o \
	scriptresource.o \
	scriptthread.o \
	sequenceopcodes.o \
	soundresource.o \
	spritedecompressqueue.o \
	spritedrawqueue.o \
	talkresource.o \
	talkthread.o \
	thread.o \
	time.o \
	timerthread.o \
	updatefunctions.o

# This module can be built as a plugin
ifeq ($(ENABLE_ILLUSIONS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
