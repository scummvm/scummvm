MODULE := engines/asylum

MODULE_OBJS := \
	puzzles/board.o \
	puzzles/boardkeyhidesto.o \
	puzzles/boardsalvation.o \
	puzzles/boardyouth.o \
	puzzles/clock.o \
	puzzles/fisherman.o \
	puzzles/hivecontrol.o \
	puzzles/hivemachine.o \
	puzzles/lock.o \
	puzzles/morguedoor.o \
	puzzles/pipes.o \
	puzzles/puzzle.o \
	puzzles/puzzles.o \
	puzzles/tictactoe.o \
	puzzles/timemachine.o \
	puzzles/vcr.o \
	puzzles/wheel.o \
	puzzles/writings.o \
	resources/actor.o \
	resources/data.o \
	resources/object.o \
	resources/encounters.o \
	resources/inventory.o \
	resources/polygons.o \
	resources/script.o \
	resources/special.o \
	resources/worldstats.o \
	system/config.o \
	system/cursor.o \
	system/graphics.o \
	system/savegame.o \
	system/screen.o \
	system/sound.o \
	system/speech.o \
	system/text.o \
	views/insertdisc.o \
	views/menu.o \
	views/resviewer.o \
	views/scene.o \
	views/scenetitle.o \
	views/video.o \
	asylum.o \
	console.o \
	metaengine.o \
	respack.o

# This module can be built as a plugin
ifeq ($(ENABLE_ASYLUM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
