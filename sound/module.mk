MODULE := sound

MODULE_OBJS = \
	sound/fmopl.o \
	sound/mididrv.o \
	sound/midistreamer.o \
	sound/mixer.o

# Include common rules 
include common.rules
