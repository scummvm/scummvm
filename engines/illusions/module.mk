MODULE := engines/illusions

MODULE_OBJS := \
	backgroundresource.o \
	illusions.o \
	detection.o \
	resourcesystem.o

# This module can be built as a plugin
ifeq ($(ENABLE_ILLUSIONS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
