MODULE := lure

MODULE_OBJS := \
	lure/animseq.o \
	lure/debug-input.o \
	lure/debug-methods.o \
	lure/decode.o \
	lure/disk.o \
	lure/events.o \
	lure/game.o \
	lure/hotspots.o \
	lure/intro.o \
	lure/lure.o \
	lure/memory.o \
	lure/menu.o \
	lure/palette.o \
	lure/res.o \
	lure/res_struct.o \
	lure/room.o \
	lure/screen.o \
	lure/scripts.o \
	lure/strings.o \
	lure/surface.o \
	lure/system.o

MODULE_DIRS += \
	lure

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules

