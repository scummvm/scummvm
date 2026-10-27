MODULE := engines/harold

MODULE_OBJS := \
	metaengine.o

ifdef ENABLE_BCB
MODULE_OBJS += \
	bcb/bcb.o \
	bcb/captions.o \
	bcb/computer.o \
	bcb/debugger.o \
	bcb/executable_data.o \
	bcb/input.o \
	bcb/map.o \
	bcb/resources.o \
	bcb/saveload.o \
	bcb/scene.o \
	bcb/screens.o \
	bcb/script.o \
	bcb/static_data.o \
	bcb/video.o
endif

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
