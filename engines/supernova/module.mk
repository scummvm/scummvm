MODULE := engines/supernova

MODULE_OBJS := \
	console.o \
	detection.o \
	graphics.o \
	resman.o \
	rooms.o \
	screen.o \
	sound.o \
	state.o \
	supernova.o

MODULE_DIRS += \
	engines/supernova

# This module can be built as a plugin
ifeq ($(ENABLE_SUPERNOVA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
