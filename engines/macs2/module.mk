MODULE := engines/macs2

MODULE_OBJS = \
	macs2.o \
	saveload.o \
	console.o \
	events.o \
	messages.o \
	metaengine.o \
	view1.o \
	gameobjects.o \
	adlib.o \
	script/scriptexecutor.o

ifdef USE_IMGUI
MODULE_OBJS += \
	debugtools.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_MACS2), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
