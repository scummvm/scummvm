MODULE := engines/myst3

MODULE_OBJS := \
	ambient.o \
	archive.o \
	console.o \
	cursor.o \
	database.o \
	detection.o \
	directoryentry.o \
	directorysubentry.o \
	gfx.o \
	hotspot.o \
	inventory.o \
	menu.o \
	movie.o \
	myst3.o \
	node.o \
	nodecube.o \
	nodeframe.o \
	puzzles.o \
	scene.o \
	script.o \
	sound.o \
	state.o \
	subtitles.o

# This module can be built as a plugin
ifeq ($(ENABLE_MYST3), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
