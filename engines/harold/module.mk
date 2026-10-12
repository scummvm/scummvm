MODULE := engines/harold

MODULE_OBJS := \
	bcb/bcb.o \
	bcb/resources.o \
	bcb/video.o \
	bcb/captions.o \
	bcb/scene.o \
	bcb/screens.o \
	bcb/input.o \
	bcb/computer.o \
	bcb/saveload.o \
	bcb/map.o \
	bcb/script.o \
	bcb/executable_data.o \
	bcb/static_data.o \
	metaengine.o

MODULE_DIRS += \
	engines/harold \
	engines/harold/bcb

# This module can be built as a plugin
ifeq ($(ENABLE_HAROLD), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
