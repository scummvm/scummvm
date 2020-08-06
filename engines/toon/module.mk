MODULE := engines/toon

MODULE_OBJS := \
	anim.o \
	audio.o \
	character.o \
	console.o \
	conversation.o \
	drew.o \
	flux.o \
	font.o \
	hotspot.o \
	metaengine.o \
	movie.o \
	path.o \
	picture.o \
	resource.o \
	script.o \
	script_func.o \
	state.o \
	subtitles.o \
	text.o \
	tools.o \
	toon.o

# This module can be built as a plugin
ifeq ($(ENABLE_TOON), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
