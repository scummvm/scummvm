MODULE := sky

MODULE_OBJS = \
	sky/sky.o \
	sky/compact.o \
	sky/logic.o \
	sky/debug.o \
	sky/disk.o \
	sky/screen.o \
	sky/timer.o \
	sky/text.o \
	sky/intro.o \
	sky/cd_intro.o \
	sky/rnc_deco.o \
	sky/grid.o \
	sky/sound.o \
	sky/skychannel.o \
	sky/skymusic.o \

# Include common rules 
include common.rules
