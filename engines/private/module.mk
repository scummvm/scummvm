MODULE := engines/private
 
MODULE_OBJS := \
	metaengine.o \
	private.o
 
MODULE_DIRS += \
	engines/private
 
# This module can be built as a plugin
ifeq ($(ENABLE_QUUX), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
