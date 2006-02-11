MODULE := engines/simon

MODULE_OBJS := \
	engines/simon/charset.o \
	engines/simon/cursor.o \
	engines/simon/debug.o \
	engines/simon/debugger.o \
	engines/simon/game.o \
	engines/simon/icons.o \
	engines/simon/items.o \
	engines/simon/midi.o \
	engines/simon/midiparser_s1d.o \
	engines/simon/res.o \
	engines/simon/saveload.o \
	engines/simon/sound.o \
	engines/simon/simon.o \
	engines/simon/verb.o \
	engines/simon/vga.o \

MODULE_DIRS += \
	engines/simon

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
