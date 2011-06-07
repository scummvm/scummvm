MODULE := engines/cge
 
MODULE_OBJS := \
	cge.o \
	console.o \
	detection.o

MODULE_DIRS += \
	engines/cge
 
# This module can be built as a plugin
ifeq ($(ENABLE_CGE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
