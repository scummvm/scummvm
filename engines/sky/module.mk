MODULE := engines/sky

MODULE_OBJS := \
	engines/sky/autoroute.o \
	engines/sky/compact.o \
	engines/sky/control.o \
	engines/sky/debug.o \
	engines/sky/disk.o \
	engines/sky/grid.o \
	engines/sky/hufftext.o \
	engines/sky/intro.o \
	engines/sky/logic.o \
	engines/sky/mouse.o \
	engines/sky/rnc_deco.o \
	engines/sky/screen.o \
	engines/sky/sky.o \
	engines/sky/sound.o \
	engines/sky/text.o \
	engines/sky/music/adlibchannel.o \
	engines/sky/music/adlibmusic.o \
	engines/sky/music/gmchannel.o \
	engines/sky/music/gmmusic.o \
	engines/sky/music/mt32music.o \
	engines/sky/music/musicbase.o

MODULE_DIRS += \
	engines/sky \
	engines/sky/music \
	engines/sky/compacts

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
