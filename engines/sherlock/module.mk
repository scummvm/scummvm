MODULE := engines/sherlock

MODULE_OBJS = \
	scalpel/darts.o \
	scalpel/scalpel.o \
	scalpel/drivers/adlib.o \
	scalpel/drivers/mt32.o \
	scalpel/tsage/logo.o \
	scalpel/tsage/resources.o \
	scalpel/scalpel_scene.o \
	scalpel/scalpel_user_interface.o \
	scalpel/settings.o \
	tattoo/tattoo.o \
	tattoo/tattoo_scene.o \
	tattoo/tattoo_user_interface.o \
	animation.o \
	debugger.o \
	detection.o \
	events.o \
	inventory.o \
	journal.o \
	map.o \
	music.o \
	objects.o \
	people.o \
	resources.o \
	saveload.o \
	scene.o \
	screen.o \
	sherlock.o \
	sound.o \
	surface.o \
	talk.o \
	user_interface.o

# This module can be built as a plugin
ifeq ($(ENABLE_SHERLOCK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
