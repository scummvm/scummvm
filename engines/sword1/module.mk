MODULE := engines/sword1

MODULE_OBJS := \
	animation.o \
	control.o \
	credits.o \
	debug.o \
	eventman.o \
	logic.o \
	memman.o \
	menu.o \
	mouse.o \
	music.o \
	objectman.o \
	resman.o \
	router.o \
	screen.o \
	sound.o \
	staticres.o \
	sword1.o \
	text.o

MODULE_DIRS += \
	engines/sword1 

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
