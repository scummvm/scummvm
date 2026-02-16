MODULE := engines/bolt

MODULE_OBJS = \
	anim.o \
	bolt.o \
	booth.o \
	console.o \
	gfx.o \
	metaengine.o \
	resource.o \
	xplib\blit.o \
	xplib\cursor.o \
	xplib\display.o \
	xplib\events.o \
	xplib\file.o \
	xplib\mem.o \
	xplib\palette.o \
	xplib\random.o \
	xplib\sound.o \
	xplib\timer.o \
	xplib\xplib.o

# This module can be built as a plugin
ifeq ($(ENABLE_BOLT), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
