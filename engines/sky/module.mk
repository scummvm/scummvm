MODULE := engines/sky

MODULE_OBJS := \
	autoroute.o \
	compact.o \
	control.o \
	debug.o \
	disk.o \
	grid.o \
	hufftext.o \
	intro.o \
	logic.o \
	metaengine.o \
	mouse.o \
	rnc_deco.o \
	screen.o \
	sky.o \
	sound.o \
	text.o \
	music/adlibchannel.o \
	music/adlibmusic.o \
	music/gmchannel.o \
	music/gmmusic.o \
	music/mt32music.o \
	music/musicbase.o

# This module can be built as a plugin
ifeq ($(ENABLE_SKY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
