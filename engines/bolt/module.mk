MODULE := engines/bolt

MODULE_OBJS = \
	anim.o \
	av.o \
	bolt.o \
	carnival/barker.o \
	carnival/booth.o \
	carnival/booths/fred.o \
	carnival/booths/george.o \
	carnival/booths/huck.o \
	carnival/booths/scooby.o \
	carnival/booths/topcat.o \
	carnival/booths/yogi.o \
	carnival/carnival.o \
	carnival/resource.o \
	crete/crete.o \
	merlin/merlin.o \
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
