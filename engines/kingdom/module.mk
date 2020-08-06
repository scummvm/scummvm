MODULE := engines/kingdom

MODULE_OBJS = \
	kingdom.o \
	constants.o \
	console.o \
	logic1.o \
	logic2.o \
	logic3.o \
	logic4.o \
	logic.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_KINGDOM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
