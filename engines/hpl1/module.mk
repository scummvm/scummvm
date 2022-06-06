MODULE := engines/hpl1

MODULE_OBJS := \
	hpl1.o \
	console.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_HPL1), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
