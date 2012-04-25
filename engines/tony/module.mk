MODULE := engines/tony

MODULE_OBJS := \
	detection.o \
	tony.o

# This module can be built as a plugin
ifeq ($(ENABLE_TONY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
