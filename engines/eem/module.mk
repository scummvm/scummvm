MODULE := engines/eem

MODULE_OBJS = \
	console.o \
	eem.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_EEM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
