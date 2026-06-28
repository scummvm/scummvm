MODULE := engines/comfy

MODULE_OBJS = \
	comfy.o \
	console.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_COMFY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
