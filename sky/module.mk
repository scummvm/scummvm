MODULE := sky

MODULE_OBJS := \
	sky/autoroute.o \
	sky/compact.o \
	sky/control.o \
	sky/debug.o \
	sky/disk.o \
	sky/grid.o \
	sky/hufftext.o \
	sky/intro.o \
	sky/logic.o \
	sky/mouse.o \
	sky/rnc_deco.o \
	sky/screen.o \
	sky/sky.o \
	sky/sound.o \
	sky/text.o \
	sky/music/adlibchannel.o \
	sky/music/adlibmusic.o \
	sky/music/gmchannel.o \
	sky/music/gmmusic.o \
	sky/music/mt32music.o \
	sky/music/musicbase.o \
	sky/compacts/savedata.o \

MODULE_DIRS += \
	sky \
	sky/music \
	sky/compacts

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include common.rules
