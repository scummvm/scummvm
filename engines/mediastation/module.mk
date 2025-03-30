MODULE := engines/mediastation

MODULE_OBJS = \
	asset.o \
	assetheader.o \
	assets/canvas.o \
	assets/font.o \
	assets/hotspot.o \
	assets/image.o \
	assets/movie.o \
	assets/palette.o \
	assets/path.o \
	assets/screen.o \
	assets/sound.o \
	assets/sprite.o \
	assets/text.o \
	assets/timer.o \
	bitmap.o \
	boot.o \
	context.o \
	cursors.o \
	datafile.o \
	datum.o \
	mediascript/codechunk.o \
	mediascript/collection.o \
	mediascript/eventhandler.o \
	mediascript/function.o \
	mediascript/scriptconstants.o \
	mediascript/scriptvalue.o \
	mediastation.o \
	metaengine.o \
	transitions.o

# This module can be built as a plugin
ifeq ($(ENABLE_MEDIASTATION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
