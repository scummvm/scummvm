MODULE := engines/awe

MODULE_OBJS = \
	metaengine.o \
	awe.o \
	bank.o \
	logic.o \
	resource.o \
	serializer.o \
	staticres.o \
	system_stub.o \
	util.o \
	video.o

# This module can be built as a plugin
ifeq ($(ENABLE_AWE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
