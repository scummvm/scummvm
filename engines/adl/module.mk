MODULE := engines/adl

MODULE_OBJS := \
	adl.o \
	adl_v2.o \
	adl_v3.o \
	adl_v4.o \
	adl_v5.o \
	console.o \
	disk.o \
	display.o \
	display_a2.o \
	hires1.o \
	hires2.o \
	hires4.o \
	hires5.o \
	hires6.o \
	metaengine.o \
	sound.o

MODULE_DIRS += \
	engines/adl

# This module can be built as a plugin
ifeq ($(ENABLE_ADL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_ADL), STATIC_PLUGIN)
# External dependencies for detection.
DETECT_OBJS += $(MODULE)/disk.o
endif
