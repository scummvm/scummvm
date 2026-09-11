MODULE := engines/igor

MODULE_OBJS = \
	igor.o \
	console.o \
	metaengine.o \
	static.o \
	part_85.o \
	part_90.o

# This module can be built as a plugin
ifeq ($(ENABLE_IGOR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
