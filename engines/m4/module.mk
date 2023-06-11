MODULE := engines/m4

MODULE_OBJS = \
	m4.o \
	console.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_M4), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
