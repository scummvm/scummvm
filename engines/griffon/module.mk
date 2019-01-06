MODULE := engines/griffon

MODULE_OBJS := \
	config.o \
	engine.o \
	griffon.o \
	detection.o \
	saveload.o

MODULE_DIRS += \
	engines/griffon

# This module can be built as a plugin
ifeq ($(ENABLE_GRIFFON), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
