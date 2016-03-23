MODULE := engines/adl

MODULE_OBJS := \
	adl.o \
	adl_v2.o \
	detection.o \
	disk.o \
	display.o \
	graphics.o \
	graphics_v1.o \
	graphics_v2.o \
	hires1.o \
	hires2.o \
	speaker.o

MODULE_DIRS += \
	engines/adl

# This module can be built as a plugin
ifeq ($(ENABLE_ADL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
