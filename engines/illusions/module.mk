MODULE := engines/illusions

MODULE_OBJS := \
	actor.o \
	bbdou/bbdou_bubble.o \
	bbdou/bbdou_cursor.o \
	bbdou/bbdou_inventory.o \
	bbdou/bbdou_specialcode.o \
	bbdou/illusions_bbdou.o \
	camera.o \
	cursor.o \
	detection.o \
	dictionary.o \
	duckman/duckman_dialog.o \
	duckman/duckman_inventory.o \
	duckman/duckman_specialcode.o \
	duckman/illusions_duckman.o \
	duckman/propertytimers.o \
	fixedpoint.o \
	graphics.o \
	illusions.o \
	input.o \
	pathfinder.o \
	resources/actorresource.o \
	resources/backgroundresource.o \
	resources/fontresource.o \
	resources/midiresource.o \
	resources/scriptresource.o \
	resources/soundresource.o \
	resources/talkresource.o \
	resourcesystem.o \
	screen.o \
	screentext.o \
	scriptstack.o \
	scriptopcodes.o \
	scriptopcodes_bbdou.o \
	scriptopcodes_duckman.o \
	sequenceopcodes.o \
	sound.o \
	specialcode.o \
	textdrawer.o \
	threads/abortablethread.o \
	threads/causethread_duckman.o \
	threads/scriptthread.o \
	threads/talkthread.o \
	threads/talkthread_duckman.o \
	threads/timerthread.o \
	thread.o \
	time.o \
	updatefunctions.o

# This module can be built as a plugin
ifeq ($(ENABLE_ILLUSIONS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
