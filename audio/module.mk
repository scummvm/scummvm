MODULE := audio

MODULE_OBJS := \
	audiostream.o \
	mididrv.o \
	midiparser_qt.o \
	midiparser.o \
	mixer.o \
	mpu401.o \
	musicplugin.o \
	null.o \
	timestamp.o \
	decoders/3do.o \
	decoders/aac.o \
	decoders/adpcm.o \
	decoders/aiff.o \
	decoders/asf.o \
	decoders/flac.o \
	decoders/iff_sound.o \
	decoders/mac_snd.o \
	decoders/mp3.o \
	decoders/qdm2.o \
	decoders/quicktime.o \
	decoders/raw.o \
	decoders/voc.o \
	decoders/vorbis.o \
	decoders/wave.o \
	decoders/wma.o \
	decoders/xa.o \
	softsynth/cms.o \
	softsynth/fluidsynth.o

ifdef USE_ALSA
MODULE_OBJS += \
	alsa_opl.o
endif

ifdef USE_A52
MODULE_OBJS += \
	decoders/ac3.o
endif

ifndef USE_ARM_SOUND_ASM
MODULE_OBJS += \
	rate.o
else
MODULE_OBJS += \
	rate_arm.o \
	rate_arm_asm.o
endif

# Include common rules
include $(srcdir)/rules.mk
