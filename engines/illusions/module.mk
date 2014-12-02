MODULE := engines/illusions

MODULE_OBJS := \
	actor.o \
	actorresource.o \
	backgroundresource.o \
	bbdou/bbdou_bubble.o \
	bbdou/bbdou_cursor.o \
	bbdou/bbdou_inventory.o \
	bbdou/bbdou_specialcode.o \
	camera.o \
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
	pathfinder.o \
	resourcesystem.o \
	screen.o \
	screentext.o \
	scriptstack.o \
	scriptopcodes.o \
	scriptopcodes_bbdou.o \
	scriptopcodes_duckman.o \
	scriptresource.o \
	sequenceopcodes.o \
	sound.o \
	soundresource.o \
	specialcode.o \
	talkresource.o \
	textdrawer.o \
	thread.o \
	threads/abortablethread.o \
	threads/causethread_duckman.o \
	threads/scriptthread.o \
	threads/talkthread.o \
	threads/talkthread_duckman.o \
	threads/timerthread.o \
	time.o \
	updatefunctions.o

# This module can be built as a plugin
ifeq ($(ENABLE_ILLUSIONS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
