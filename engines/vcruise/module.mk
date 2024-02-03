MODULE := engines/vcruise

MODULE_OBJS = \
	ad2044_items.o \
	ad2044_ui.o \
	audio_player.o \
	circuitpuzzle.o \
	midi_player.o \
	metaengine.o \
	menu.o \
	runtime.o \
	runtime_scriptexec.o \
	sampleloop.o \
	script.o \
	textparser.o \
	vcruise.o


# This module can be built as a plugin
ifeq ($(ENABLE_VCRUISE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
