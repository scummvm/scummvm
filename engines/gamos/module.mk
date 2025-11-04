MODULE := engines/gamos

MODULE_OBJS = \
	blit.o \
	gamos.o \
	file.o \
	console.o \
	metaengine.o \
	keycodes.o \
	music.o \
	proc.o \
	movie.o \
	saveload.o \
	vm.o \
	video.o

# This module can be built as a plugin
ifeq ($(ENABLE_GAMOS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
