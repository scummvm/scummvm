MODULE := engines/illusions

MODULE_OBJS := \
	actor.o \
	bbdou/bbdou_bubble.o \
	bbdou/bbdou_cursor.o \
	bbdou/bbdou_credits.o \
	bbdou/bbdou_credits_staticdata.o \
	bbdou/bbdou_foodctl.o \
	bbdou/bbdou_inventory.o \
	bbdou/bbdou_menukeys.o \
	bbdou/bbdou_specialcode.o \
	bbdou/bbdou_videoplayer.o \
	bbdou/gamestate_bbdou.o \
	bbdou/bbdou_triggerfunctions.o \
	bbdou/illusions_bbdou.o \
	bbdou/menusystem_bbdou.o \
	bbdou/scriptopcodes_bbdou.o \
	camera.o \
	cursor.o \
	dictionary.o \
	duckman/duckman_credits.o \
	duckman/duckman_dialog.o \
	duckman/duckman_inventory.o \
	duckman/duckman_screenshakereffects.o \
	duckman/duckman_specialcode.o \
	duckman/duckman_videoplayer.o \
	duckman/gamestate_duckman.o \
	duckman/illusions_duckman.o \
	duckman/menusystem_duckman.o \
	duckman/propertytimers.o \
	duckman/scriptopcodes_duckman.o \
	fileresourcereader.o \
	fixedpoint.o \
	gamarchive.o \
	gamestate.o \
	gamresourcereader.o \
	graphics.o \
	illusions.o \
	input.o \
	metaengine.o \
	menusystem.o \
	pathfinder.o \
	resources/actorresource.o \
	resources/backgroundresource.o \
	resources/fontresource.o \
	resources/genericresource.o \
	resources/midiresource.o \
	resources/scriptresource.o \
	resources/soundresource.o \
	resources/talkresource.o \
	resourcesystem.o \
	saveload.o \
	screen.o \
	screentext.o \
	scriptstack.o \
	scriptopcodes.o \
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

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
