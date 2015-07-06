MODULE := engines/nancy

MODULE_OBJS = \
  audio.o \
  console.o \
  decompress.o \
  detection.o \
  iff.o \
  logo.o \
  nancy.o \
  resource.o \
  video.o

# This module can be built as a plugin
ifeq ($(ENABLE_NANCY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
