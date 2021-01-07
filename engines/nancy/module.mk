MODULE := engines/nancy

MODULE_OBJS = \
  action/recordtypes.o \
  action/arfactory_v1.o \
  audio.o \
  console.o \
  datatypes.o \
  decompress.o \
  graphics.o \
  iff.o \
  input.o \
  logic.o \
  logo.o \
  metaengine.o \
  nancy.o \
  resource.o \
  scene.o \
  video.o

# This module can be built as a plugin
ifeq ($(ENABLE_NANCY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
