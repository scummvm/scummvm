MODULE := engines/dragons
 
MODULE_OBJS := \
	detection.o \
	dragons.o
 
MODULE_DIRS += \
	engines/dragons
 
# This module can be built as a plugin
ifeq ($(ENABLE_QUUX), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
