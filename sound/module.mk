MODULE := sound

MODULE_OBJS := \
	audiocd.o \
	audiostream.o \
	flac.o \
	fmopl.o \
	mididrv.o \
	midiparser.o \
	mixer.o \
	mp3.o \
	mpu401.o \
	musicplugin.o \
	null.o \
	raw.o \
	timestamp.o \
	vorbis.o \
	softsynth/adlib.o \
	softsynth/opl/dosbox.o \
	softsynth/opl/mame.o \
	softsynth/ym2612.o \
	softsynth/fluidsynth.o \
	softsynth/mt32.o \
	softsynth/pcspk.o \
	softsynth/sid.o \
	softsynth/wave6581.o

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
