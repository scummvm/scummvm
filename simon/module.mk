MODULE := simon

MODULE_OBJS = \
	simon/debug.o \
	simon/items.o \
	simon/midi.o \
	simon/res.o \
	simon/sound.o \
	simon/simon.o \
	simon/verb.o \
	simon/vga.o \

# Include common rules 
include common.rules
