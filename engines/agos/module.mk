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
	gfx.o \
	icons.o \
	input.o \
	items.o \
	menus.o \
	midi.o \
	midiparser_s1d.o \
	oracle.o \
	res.o \
	res_ami.o \
	rooms.o \
	saveload.o \
	script.o \
	script_e1.o \
	script_e2.o \
	script_ww.o \
	script_s1.o \
	script_s2.o \
	script_ff.o \
	script_pp.o \
	sound.o \
	string.o \
	subroutine.o \
	verb.o \
	vga.o \
	vga_s1.o \
	vga_s2.o \
	vga_ff.o \
	window.o \
	zones.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/rules.mk
