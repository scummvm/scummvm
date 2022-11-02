MODULE := engines/sherlock

MODULE_OBJS = \
	scalpel/scalpel.o \
	scalpel/3do/scalpel_3do_screen.o \
	scalpel/drivers/adlib.o \
	scalpel/drivers/mt32.o \
	scalpel/tsage/logo.o \
	scalpel/tsage/resources.o \
	scalpel/scalpel_darts.o \
	scalpel/scalpel_debugger.o \
	scalpel/scalpel_fixed_text.o \
	scalpel/scalpel_inventory.o \
	scalpel/scalpel_journal.o \
	scalpel/scalpel_map.o \
	scalpel/scalpel_people.o \
	scalpel/scalpel_saveload.o \
	scalpel/scalpel_scene.o \
	scalpel/scalpel_screen.o \
	scalpel/scalpel_talk.o \
	scalpel/scalpel_user_interface.o \
	scalpel/settings.o \
	tattoo/tattoo.o \
	tattoo/tattoo_darts.o \
	tattoo/tattoo_debugger.o \
	tattoo/tattoo_fixed_text.o \
	tattoo/tattoo_inventory.o \
	tattoo/tattoo_journal.o \
	tattoo/tattoo_map.o \
	tattoo/tattoo_people.o \
	tattoo/tattoo_resources.o \
	tattoo/tattoo_scene.o \
	tattoo/tattoo_screen.o \
	tattoo/tattoo_talk.o \
	tattoo/tattoo_user_interface.o \
	tattoo/widget_base.o \
	tattoo/widget_credits.o \
	tattoo/widget_files.o \
	tattoo/widget_foolscap.o \
	tattoo/widget_inventory.o \
	tattoo/widget_lab.o \
	tattoo/widget_options.o \
	tattoo/widget_password.o \
	tattoo/widget_quit.o \
	tattoo/widget_talk.o \
	tattoo/widget_text.o \
	tattoo/widget_tooltip.o \
	tattoo/widget_verbs.o \
	animation.o \
	debugger.o \
	events.o \
	fixed_text.o \
	fonts.o \
	image_file.o \
	inventory.o \
	journal.o \
	map.o \
	metaengine.o \
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

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
