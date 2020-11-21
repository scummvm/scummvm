MODULE := engines/ags

MODULE_OBJS = \
	ags.o \
	metaengine.o \
	stubs/allegro.o \
	stubs/allegro/color.o \
	stubs/allegro/config.o \
	stubs/allegro/digi.o \
	stubs/allegro/error.o \
	stubs/allegro/file.o \
	stubs/allegro/fixed.o \
	stubs/allegro/gfx.o \
	stubs/allegro/keyboard.o \
	stubs/allegro/midi.o \
	stubs/allegro/mouse.o \
	stubs/allegro/sound.o \
	stubs/allegro/system.o \
	stubs/allegro/unicode.o


# This module can be built as a plugin
ifeq ($(ENABLE_AGS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
