MODULE := engines/pelrock

MODULE_OBJS = \
	pelrock.o \
	chrono.o \
	console.o \
	metaengine.o \
	fonts/small_font.o \
	fonts/large_font.o \

# This module can be built as a plugin
ifeq ($(ENABLE_PELROCK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
