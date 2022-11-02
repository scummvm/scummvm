MODULE := engines/prince

MODULE_OBJS = \
	animation.o \
	archive.o \
	cursor.o \
	debugger.o \
	decompress.o \
	draw.o \
	flags.o \
	font.o \
	graphics.o \
	hero.o \
	inventory.o \
	metaengine.o \
	mhwanh.o \
	music.o \
	mob.o \
	object.o \
	prince.o \
	pscr.o \
	resource.o \
	saveload.o \
	script.o \
	sound.o \
	variatxt.o \
	videoplayer.o \
	walk.o

# This module can be built as a plugin
ifeq ($(ENABLE_PRINCE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
