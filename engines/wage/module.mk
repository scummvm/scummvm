MODULE := engines/wage

MODULE_OBJS := \
	detection.o \
	wage.o

MODULE_DIRS += \
	engines/wage

# This module can be built as a plugin
ifeq ($(ENABLE_WAGE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/rules.mk