MODULE := sky

MODULE_OBJS = \
	sky/sky.o \
	sky/disk.o \
	sky/screen.o \
	sky/timer.o \
	sky/text.o \
	sky/intro.o \
	sky/cd_intro.o \
	sky/rnc_deco.o \
	sky/grid.o \
	sky/sound.o \

# Include common rules 
include common.rules
