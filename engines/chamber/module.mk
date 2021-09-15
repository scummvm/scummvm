MODULE := engines/chamber

MODULE_OBJS := \
	anim.o \
	chamber.o \
	metaengine.o

MODULE_DIRS += \
	engines/chamber

# This module can be built as a plugin
ifeq ($(ENABLE_CHAMBER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
