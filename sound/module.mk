MODULE := sound

MODULE_OBJS = \
	sound/fmopl.o \
	sound/midiparser.o \
	sound/midiparser_smf.o \
	sound/midiparser_xmidi.o \
	sound/mixer.o \
	sound/mpu401.o

# Include common rules 
include common.rules
