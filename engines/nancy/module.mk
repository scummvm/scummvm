MODULE := engines/nancy

MODULE_OBJS = \
  console.o \
  nancy.o \
  detection.o \
  resource.o

# This module can be built as a plugin
ifeq ($(ENABLE_NANCY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
