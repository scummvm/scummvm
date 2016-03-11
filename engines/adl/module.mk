MODULE := engines/adl

MODULE_OBJS := \
	adl.o \
	detection.o \
	display.o \
	hires1.o \
	hires2.o \
	picture.o

MODULE_DIRS += \
	engines/adl

# This module can be built as a plugin
ifeq ($(ENABLE_ADL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
