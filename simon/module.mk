MODULE := simon

MODULE_OBJS := \
	simon/charset.o \
	simon/debug.o \
	simon/items.o \
	simon/midi.o \
	simon/midiparser_s1d.o \
	simon/res.o \
	simon/sound.o \
	simon/simon.o \
	simon/verb.o \
	simon/vga.o \

MODULE_DIRS += \
	simon

# Include common rules 
include common.rules
