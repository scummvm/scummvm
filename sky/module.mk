MODULE := sky

MODULE_OBJS = \
	sky/adlibchannel.o \
	sky/adlibmusic.o \
	sky/autoroute.o \
	sky/cd_intro.o \
	sky/compact.o \
	sky/debug.o \
	sky/disk.o \
	sky/gmchannel.o \
	sky/gmmusic.o \
	sky/grid.o \
	sky/intro.o \
	sky/logic.o \
	sky/mouse.o \
	sky/musicbase.o \
	sky/rnc_deco.o \
	sky/screen.o \
	sky/sky.o \
	sky/sound.o \
	sky/text.o \

# Include common rules 
include common.rules
