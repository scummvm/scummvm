MODULE := engines/prince

MODULE_OBJS = \
	animation.o \
	debugger.o \
	script.o \
	graphics.o \
	mhwanh.o \
	detection.o \
	font.o \
	mob.o \
	object.o \
	sound.o \
	flags.o \
	variatxt.o \
	prince.o \
	archive.o \
	decompress.o \
	hero.o \
	hero_set.o \
	cursor.o 

# This module can be built as a plugin
ifeq ($(ENABLE_PRINCE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
