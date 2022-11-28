MODULE := engines/efh

MODULE_OBJS = \
	constants.o \
	efh.o \
	fight.o \
	files.o \
	graphics.o \
	savegames.o \
	script.o \
	utils.o \
	metaengine.o

MODULE_DIRS += \
	engines/efh

# This module can be built as a plugin
ifeq ($(ENABLE_EFH), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
