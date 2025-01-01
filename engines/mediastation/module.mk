MODULE := engines/mediastation

MODULE_OBJS = \
	mediastation.o \
	asset.o \
	assetheader.o \
	chunk.o \
	context.o \
	contextparameters.o \
	bitmap.o \
	assets/image.o \
	assets/palette.o \
	assets/sound.o \
	assets/movie.o \
	assets/sprite.o \
	assets/path.o \
	assets/hotspot.o \
	assets/timer.o \
	assets/canvas.o \
	mediascript/eventhandler.o \
	mediascript/codechunk.o \
	mediascript/function.o \
	mediascript/variable.o \
	mediascript/operand.o \
	subfile.o \
	boot.o \
	datum.o \
	datafile.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_MEDIASTATION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
