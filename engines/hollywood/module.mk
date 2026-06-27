MODULE := engines/hollywood

MODULE_OBJS = \
	hollywood.o \
	metaengine.o \
	music.o \
	resource.o \
	scenes/intro/scene9000.o \
	scenes/intro/scene9010.o \
	scenes/intro/scene9100.o

# This module can be built as a plugin
ifeq ($(ENABLE_HOLLYWOOD), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
