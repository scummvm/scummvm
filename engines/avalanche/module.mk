MODULE := engines/avalanche

MODULE_OBJS = \
	avalanche.o \
	console.o \
	detection.o

# This module can be built as a plugin
ifeq ($(ENABLE_AVALANCHE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
