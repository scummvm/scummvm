MODULE := engines/mediastation

MODULE_OBJS = \
	actor.o \
	actors/camera.o \
	actors/canvas.o \
	actors/document.o \
	actors/font.o \
	actors/hotspot.o \
	actors/image.o \
	actors/movie.o \
	actors/palette.o \
	actors/path.o \
	actors/screen.o \
	actors/sound.o \
	actors/sprite.o \
	actors/stage.o \
	actors/text.o \
	actors/timer.o \
	audio.o \
	bitmap.o \
	boot.o \
	clients.o \
	context.o \
	cursors.o \
	datafile.o \
	graphics.o \
	mediascript/codechunk.o \
	mediascript/collection.o \
	mediascript/eventhandler.o \
	mediascript/function.o \
	mediascript/scriptconstants.o \
	mediascript/scriptvalue.o \
	mediastation.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_MEDIASTATION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
