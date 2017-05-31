MODULE := engines/supernova

MODULE_OBJS := \
	supernova.o \
	console.o \
	detection.o

MODULE_DIRS += \
	engines/supernova

# This module can be built as a plugin
ifeq ($(ENABLE_SUPERNOVA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
