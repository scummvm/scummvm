MODULE := sky

MODULE_OBJS = \
	sky/sky.o \
	sky/disk.o \
	sky/screen.o \
	sky/intro.o \
	sky/rnc_deco.o \
	sky/grid.o \

# Include common rules 
include common.rules
