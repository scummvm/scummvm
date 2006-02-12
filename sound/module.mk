MODULE := sound

MODULE_OBJS := \
	adpcm.o \
	audiocd.o \
	audiostream.o \
	flac.o \
	fmopl.o \
	mididrv.o \
	midiparser.o \
	midiparser_smf.o \
	midiparser_xmidi.o \
	mixer.o \
	mp3.o \
	mpu401.o \
	rate.o \
	voc.o \
	vorbis.o \
	wave.o \
	softsynth/adlib.o \
	softsynth/ym2612.o \
	softsynth/fluidsynth.o \
	softsynth/mt32.o \

MODULE_DIRS += \
	sound \
	sound/softsynth

# Include common rules 
include $(srcdir)/common.rules
