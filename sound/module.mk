MODULE := sound

MODULE_OBJS = \
	sound/fmopl.o \
	sound/mididrv.o \
	sound/midistreamer.o \
	sound/mixer.o \
	sound/mpu401.o

# Include common rules 
include common.rules
