MODULE := engines/bolt

MODULE_OBJS = \
	bolt.o \
	booth.o \
	console.o \
	gfx.o \
	metaengine.o \
	resource.o \
	video.o \
	xplib\xp_cursor.o \
	xplib\xp_events.o \
	xplib\xp_file.o \
	xplib\xp_gfx.o \
	xplib\xp_mem.o \
	xplib\xp_sound.o \
	xplib\xplib.o

# This module can be built as a plugin
ifeq ($(ENABLE_BOLT), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
