MODULE := engines/eem

MODULE_OBJS = \
	animation.o \
	audio.o \
	clues.o \
	eem.o \
	font.o \
	graphics.o \
	metaengine.o \
	music.o \
	mystery.o \
	resource.o \
	site.o \
	ui.o

# This module can be built as a plugin
ifeq ($(ENABLE_EEM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
