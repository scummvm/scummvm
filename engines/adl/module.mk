MODULE := engines/adl

MODULE_OBJS := \
	adl.o \
	adl_v2.o \
	adl_v3.o \
	adl_v4.o \
	adl_v5.o \
	console.o \
	detection.o \
	disk.o \
	display.o \
	display_a2.o \
	hires1.o \
	hires2.o \
	hires4.o \
	hires5.o \
	hires6.o \
	sound.o

MODULE_DIRS += \
	engines/adl

# This module can be built as a plugin
ifeq ($(ENABLE_ADL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
