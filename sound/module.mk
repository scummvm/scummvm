MODULE := sound

MODULE_OBJS := \
	sound/audiocd.o \
	sound/audiostream.o \
	sound/fmopl.o \
	sound/mididrv.o \
	sound/midiparser.o \
	sound/midiparser_smf.o \
	sound/midiparser_xmidi.o \
	sound/mixer.o \
	sound/mp3.o \
	sound/mpu401.o \
	sound/rate.o \
	sound/voc.o \
	sound/vorbis.o
#	sound/resample.o \

MODULE_DIRS += \
	sound

# Include common rules 
include $(srcdir)/common.rules
