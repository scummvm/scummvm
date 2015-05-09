MODULE := engines/sherlock

MODULE_OBJS = \
	scalpel/darts.o \
	scalpel/scalpel.o \
	tattoo/tattoo.o \
	animation.o \
	decompress.o \
	debugger.o \
	detection.o \
	events.o \
	graphics.o \
	inventory.o \
	journal.o \
	map.o \
	objects.o \
	people.o \
	resources.o \
	saveload.o \
	scene.o \
	screen.o \
	settings.o \
	sherlock.o \
	sound.o \
	talk.o \
	user_interface.o

# This module can be built as a plugin
ifeq ($(ENABLE_SHERLOCK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
