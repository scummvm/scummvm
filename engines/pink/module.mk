MODULE := engines/pink

MODULE_OBJS = \
	pink.o \
	console.o \
	detection.o \
	director.o \
	sound.o \
	file.o \


# This module can be built as a plugin
ifeq ($(ENABLE_PLUMBERS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk