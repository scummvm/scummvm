MODULE := engines/gargoyle

MODULE_OBJS := \
	detection.o \
	gargoyle.o

# This module can be built as a plugin
ifeq ($(ENABLE_GARGOYLE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
