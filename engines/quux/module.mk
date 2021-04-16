MODULE := engines/quux

MODULE_OBJS := \
	metaengine.o \
	quux.o

MODULE_DIRS += \
	engines/quux

# This module can be built as a plugin
ifeq ($(ENABLE_QUUX), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
