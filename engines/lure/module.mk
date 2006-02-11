MODULE := engines/lure

MODULE_OBJS := \
	engines/lure/animseq.o \
	engines/lure/debug-input.o \
	engines/lure/debug-methods.o \
	engines/lure/decode.o \
	engines/lure/disk.o \
	engines/lure/events.o \
	engines/lure/game.o \
	engines/lure/hotspots.o \
	engines/lure/intro.o \
	engines/lure/lure.o \
	engines/lure/memory.o \
	engines/lure/menu.o \
	engines/lure/palette.o \
	engines/lure/res.o \
	engines/lure/res_struct.o \
	engines/lure/room.o \
	engines/lure/screen.o \
	engines/lure/scripts.o \
	engines/lure/strings.o \
	engines/lure/surface.o \
	engines/lure/system.o

MODULE_DIRS += \
	engines/lure

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules

