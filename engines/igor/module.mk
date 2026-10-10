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
	graphics.o \
	resource.o \
	input.o \
	static_walk.o \
	static_cursor.o \
	options.o \
	parts/part_0.o \
	parts/part_1.o \
	parts/part_2.o \
	parts/part_4.o \
	parts/part_5.o \
	parts/part_6.o \
	parts/part_7.o \
	parts/part_8.o \
	parts/part_9.o \
	parts/part_10.o \
	parts/part_11.o \
	parts/part_12.o \
	parts/part_13.o \
	parts/part_14.o \
	parts/part_15.o \
	parts/part_16.o \
	parts/part_17.o \
	parts/part_18.o \
	parts/part_19.o \
	parts/part_21.o \
	parts/part_22.o \
	parts/part_23.o \
	parts/part_24.o \
	parts/part_25.o \
	parts/part_26.o \
	parts/part_27.o \
	parts/part_28.o \
	parts/part_30.o \
	parts/part_31.o \
	parts/part_32.o \
	parts/part_33.o \
	parts/part_34.o \
	parts/part_35.o \
	parts/part_36.o \
	parts/part_37.o \
	parts/part_50.o \
	parts/part_68.o \
	parts/part_69.o \
	parts/part_70.o \
	parts/part_71.o \
	parts/part_72.o \
	parts/part_73.o \
	parts/part_74.o \
	parts/part_75.o \
	parts/part_77.o \
	parts/part_78.o \
	parts/part_80.o \
	parts/part_81.o \
	parts/part_85.o \
	parts/part_90.o \
	parts/part_maze.o \
	parts/part_maze_data.o \
	parts/part_margaret.o \
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
