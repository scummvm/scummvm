MODULE := engines/kingdom

MODULE_OBJS = \
	kingdom.o \
	console.o \
	detection.o

# This module can be built as a plugin
ifeq ($(ENABLE_KINGDOM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
