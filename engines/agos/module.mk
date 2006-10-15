MODULE := engines/agos

MODULE_OBJS := \
	agos.o \
	animation.o \
	charset.o \
	contain.o \
	cursor.o \
	debug.o \
	debugger.o \
	draw.o \
	event.o \
	game.o \
	icons.o \
	menus.o \
	midi.o \
	midiparser_s1d.o \
	oracle.o \
	res.o \
	res_ami.o \
	rooms.o \
	saveload.o \
	script.o \
	sound.o \
	string.o \
	subroutine.o \
	verb.o \
	vga.o \
	window.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/rules.mk
