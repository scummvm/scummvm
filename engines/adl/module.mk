MODULE := engines/adl

MODULE_OBJS := \
	adl.o \
	adl_v2.o \
	adl_v3.o \
	adl_v4.o \
	console.o \
	detection.o \
	disk.o \
	display.o \
	graphics.o \
	graphics_v1.o \
	graphics_v2.o \
	hires0.o \
	hires1.o \
	hires2.o \
	hires4.o \
	hires5.o \
	hires6.o \
	speaker.o

MODULE_DIRS += \
	engines/adl

# This module can be built as a plugin
ifeq ($(ENABLE_ADL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
