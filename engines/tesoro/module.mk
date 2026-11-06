MODULE := engines/tesoro

MODULE_OBJS = \
	tesoro.o \
	console.o \
	metaengine.o \
	core/archive.o \
	core/archiveindex.o \
	core/namekey.o

# This module can be built as a plugin
ifeq ($(ENABLE_TESORO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
