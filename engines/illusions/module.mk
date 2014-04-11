MODULE := engines/illusions

MODULE_OBJS := \
	abortablethread.o \
	actor.o \
	actorresource.o \
	backgroundresource.o \
	bbdou/bbdou_bubble.o \
	bbdou/bbdou_cursor.o \
	bbdou/bbdou_inventory.o \
	bbdou/bbdou_specialcode.o \
	camera.o \
	causethread_duckman.o \
	cursor.o \
	detection.o \
	dictionary.o \
	fixedpoint.o \
	fontresource.o \
	graphics.o \
	illusions.o \
	illusions_bbdou.o \
	illusions_duckman.o \
	input.o \
	midiresource.o \
	resourcesystem.o \
	screen.o \
	screentext.o \
	scriptman.o \
	scriptopcodes.o \
	scriptopcodes_bbdou.o \
	scriptopcodes_duckman.o \
	scriptresource.o \
	scriptthread.o \
	sequenceopcodes.o \
	soundresource.o \
	specialcode.o \
	talkresource.o \
	talkthread.o \
	talkthread_duckman.o \
	textdrawer.o \
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
