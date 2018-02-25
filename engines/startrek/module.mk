MODULE := engines/startrek

MODULE_OBJS = \
	bitmap.o \
	common.o \
	detection.o \
	filestream.o \
	font.o \
	lzss.o \
	graphics.o \
	sound.o \
	sprite.o \
	startrek.o \
	text.o
	


# This module can be built as a plugin
ifeq ($(ENABLE_STARTREK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
