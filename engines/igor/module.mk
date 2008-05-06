MODULE := engines/igor

MODULE_OBJS := \
	detection.o \
	igor.o \
	menu.o \
	midi.o \
	saveload.o \
	staticres.o \
	parts/part_04.o \
	parts/part_05.o \
	parts/part_06.o \
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
	parts/part_33.o \
	parts/part_36.o \
	parts/part_37.o \
	parts/part_75.o \
	parts/part_85.o \
	parts/part_90.o \
	parts/part_95.o \
	parts/part_main.o

# This module can be built as a plugin
ifeq ($(ENABLE_IGOR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
