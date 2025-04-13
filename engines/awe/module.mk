MODULE := engines/awe

MODULE_OBJS = \
	metaengine.o \
	awe.o \
	bank.o \
	engine.o \
	logic.o \
	resource.o \
	sdlstub.o \
	serializer.o \
	staticres.o \
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
