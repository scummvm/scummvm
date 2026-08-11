MODULE := engines/gamos

MODULE_OBJS = \
	blit.o \
	file.o \
	gamos.o \
	keycodes.o \
	metaengine.o \
	movie.o \
	music.o \
	proc.o \
	saveload.o \
	video.o \
	vm.o

# This module can be built as a plugin
ifeq ($(ENABLE_GAMOS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
