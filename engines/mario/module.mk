MODULE := engines/mario

MODULE_OBJS = \
	mario.o \
	detection.o

# This module can be built as a plugin
ifeq ($(ENABLE_MARIO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
