MODULE := engines/bolt

MODULE_OBJS = \
	anim.o \
	av.o \
	barker.o \
	bolt.o \
	booth.o \
	booths/fred.o \
	booths/george.o \
	booths/huck.o \
	booths/scooby.o \
	booths/topcat.o \
	booths/yogi.o \
	metaengine.o \
	resource.o \
	rtf.o \
	ssprite.o \
	state.o \
	swap.o \
	utils.o \
	xplib/blit.o \
	xplib/cursor.o \
	xplib/display.o \
	xplib/events.o \
	xplib/file.o \
	xplib/palette.o \
	xplib/random.o \
	xplib/sound.o \
	xplib/timer.o \
	xplib/xplib.o

# This module can be built as a plugin
ifeq ($(ENABLE_BOLT), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
