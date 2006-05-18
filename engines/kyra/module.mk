MODULE := engines/kyra

MODULE_OBJS := \
	animator.o \
	debugger.o \
	gui.o \
	items.o \
	kyra.o \
	kyra3.o \
	plugin.o \
	resource.o \
	saveload.o \
	scene.o \
	screen.o \
	script_v1.o \
	script.o \
	seqplayer.o \
	sequences_v1.o \
	sound_adlib.o \
	sound_digital.o \
	sound.o \
	sprites.o \
	staticres.o \
	text.o \
	timer.o \
	vqa.o \
	wsamovie.o

MODULE_DIRS += \
	engines/kyra

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
