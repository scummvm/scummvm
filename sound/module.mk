MODULE := sound

MODULE_OBJS = \
	sound/audiostream.o \
	sound/fmopl.o \
	sound/midiparser.o \
	sound/midiparser_smf.o \
	sound/midiparser_xmidi.o \
	sound/mixer.o \
	sound/mpu401.o \
	sound/rate.o
#	sound/resample.o

# Include common rules 
include common.rules
