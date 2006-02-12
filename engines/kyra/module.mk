MODULE := engines/kyra

MODULE_OBJS := \
	kyra.o \
	resource.o \
	screen.o \
	script_v1.o \
	script.o \
	seqplayer.o \
	sound.o \
	staticres.o \
	sprites.o \
	wsamovie.o \
	debugger.o \
	animator.o \
	gui.o \
	sequences_v1.o \
	items.o \
	scene.o \
	text.o \
	timer.o \
	saveload.o

MODULE_DIRS += \
	engines/kyra

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
