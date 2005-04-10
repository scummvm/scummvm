MODULE := sound

MODULE_OBJS := \
	sound/audiocd.o \
	sound/audiostream.o \
	sound/flac.o \
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
	sound/vorbis.o \
	sound/wave.o \
	sound/softsynth/adlib.o \
	sound/softsynth/ym2612.o \
	sound/softsynth/fluidsynth.o \
	sound/softsynth/mt32.o \

MODULE_DIRS += \
	sound \
	sound/softsynth

# Include common rules 
include $(srcdir)/common.rules
