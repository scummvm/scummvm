MODULE := engines/igor

MODULE_OBJS = \
	igor.o \
	console.o \
	metaengine.o \
	static.o

# This module can be built as a plugin
ifeq ($(ENABLE_IGOR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
