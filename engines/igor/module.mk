MODULE := engines/igor

MODULE_OBJS = \
	igor.o \
	console.o \
	debug.o \
	metaengine.o \
	walk.o \
	palette.o \
	font.o \
	room.o \
	resource.o \
	input.o \
	static_walk.o \
	static_cursor.o \
	part_4.o \
	part_5.o \
	part_85.o \
	part_90.o \
	part_6.o \
	part_7.o \
	part_8.o \
	part_9.o \
	part_10.o \
	part_11.o \
	part_12.o \
	part_17.o \
	part_23.o \
	part_34.o \
	part_35.o \
	text.o \
	part_main.o \
	sound.o

# This module can be built as a plugin
ifeq ($(ENABLE_IGOR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
