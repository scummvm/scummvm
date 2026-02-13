MODULE := engines/colony

MODULE_OBJS := \
	colony.o \
	gfx.o \
	metaengine.o \
	render.o \
	ui.o

MODULE_DIRS += \
	engines/colony

# This module can be built as a plugin
ifeq ($(ENABLE_COLONY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
