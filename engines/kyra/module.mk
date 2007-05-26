MODULE := engines/kyra

MODULE_OBJS := \
	animator.o \
	debugger.o \
	detection.o \
	gui.o \
	items.o \
	kyra.o \
	kyra_v1.o \
	kyra_v2.o \
	kyra_v3.o \
	resource.o \
	saveload.o \
	scene.o \
	screen.o \
	script_v1.o \
	script.o \
	seqplayer.o \
	sequences_v1.o \
	sequences_v2.o \
	sound_adlib.o \
	sound_digital.o \
	sound_towns.o \
	sound.o \
	sprites.o \
	staticres.o \
	text.o \
	timer.o \
	vqa.o \
	wsamovie.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/rules.mk
