MODULE := engines/prisoner

MODULE_OBJS = \
	actor.o \
	actorframesound.o \
	background.o \
	clickbox.o \
	cursor.o \
	detection.o \
	dialog.o \
	font.o \
	inventory.o \
	kroarchive.o \
	midi.o \
	muxplayer.o \
	palette.o \
	path.o \
	prisoner.o \
	resource.o \
	resourcemgr.o \
	scene.o \
	sceneitem.o \
	screen.o \
	screentext.o \
	script.o \
	scriptops.o \
	speech.o \
	zone.o


# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
