MODULE := engines/simon

MODULE_OBJS := \
	animation.o \
	charset.o \
	cursor.o \
	debug.o \
	debugger.o \
	event.o \
	game.o \
	icons.o \
	items.o \
	midi.o \
	midiparser_s1d.o \
 	oracle.o \
	res.o \
	saveload.o \
	simon.o \
	sound.o \
	string.o \
	subroutine.o \
	verb.o \
	vga.o \
	window.o \

MODULE_DIRS += \
	engines/simon

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
