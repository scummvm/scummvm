MODULE := engines/harold

MODULE_OBJS := \
	bcb/bcb.o \
	bcb/resources.o \
	bcb/scene.o \
	bcb/input.o \
	bcb/script.o \
	bcb/static_data.o \
	metaengine.o

MODULE_DIRS += \
	engines/harold \
	engines/harold/bcb

# This module can be built as a plugin
ifeq ($(ENABLE_HAROLD), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
