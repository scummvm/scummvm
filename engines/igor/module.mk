MODULE := engines/igor

MODULE_OBJS = \
	igor.o \
	console.o \
	metaengine.o \
	walk.o \
	palette.o \
	font.o \
	room.o \
	resource.o \
	input.o \
	static_walk.o \
	static_cursor.o \
	part_5.o \
	part_85.o \
	part_90.o \
	text.o \
	part_main.o \

# This module can be built as a plugin
ifeq ($(ENABLE_IGOR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
