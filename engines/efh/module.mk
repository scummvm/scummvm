MODULE := engines/efh

MODULE_OBJS = \
	efh.o \
	metaengine.o

MODULE_DIRS += \
	engines/efh

# This module can be built as a plugin
ifeq ($(ENABLE_EFH), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
