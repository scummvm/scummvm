MODULE := engines/hopkins

MODULE_OBJS := \
	anim.o \
	detection.o \
	dialogs.o \
	events.o \
	files.o \
	font.o \
	graphics.o \
	globals.o \
	hopkins.o \
	menu.o \
	objects.o \
	sound.o \
	talk.o

# This module can be built as a plugin
ifeq ($(ENABLE_HOPKINS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
