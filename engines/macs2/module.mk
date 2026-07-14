MODULE := engines/macs2

MODULE_OBJS = \
	midiparser_macs2.o \
	music.o \
	dialogs.o \
	events.o \
	gameobjects.o \
	hotspot_names.o \
	macs2.o \
	messages.o \
	metaengine.o \
	saveload.o \
	scriptexecutor.o \
	scummui.o \
	view1.o

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
