MODULE := engines/xyzzy

MODULE_OBJS = \
	xyzzy.o \
	console.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_XYZZY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
