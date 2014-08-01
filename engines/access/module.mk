MODULE := engines/access

MODULE_OBJS := \
	access.o \
	debugger.o \
	detection.o \
	events.o \
	resources.o \

# This module can be built as a plugin
ifeq ($(ENABLE_ACCESS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
