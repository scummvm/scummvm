MODULE := engines/simon

MODULE_OBJS := \
	charset.o \
	cursor.o \
	debug.o \
	debugger.o \
	game.o \
	icons.o \
	items.o \
	midi.o \
	midiparser_s1d.o \
	res.o \
	saveload.o \
	sound.o \
	simon.o \
	verb.o \
	vga.o \

MODULE_DIRS += \
	engines/simon

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
