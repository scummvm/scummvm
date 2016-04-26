MODULE := engines/dm

MODULE_OBJS := \
	detection.o \
	dm.o

MODULE_DIRS += \
	engines/dm

# This module can be built as a plugin
ifeq ($(ENABLE_DM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

