MODULE := sound

MODULE_OBJS := \
	adpcm.o \
	aiff.o \
	audiocd.o \
	audiostream.o \
	iff_sound.o \
	flac.o \
	fmopl.o \
	mididrv.o \
	midiparser.o \
	midiparser_smf.o \
	midiparser_xmidi.o \
	mixer.o \
	mp3.o \
	mpu401.o \
	musicplugin.o \
	null.o \
	shorten.o \
	timestamp.o \
	vag.o \
	voc.o \
	vorbis.o \
	wave.o \
	mods/infogrames.o \
	mods/maxtrax.o \
	mods/module.o \
	mods/protracker.o \
	mods/paula.o \
	mods/rjp1.o \
	mods/soundfx.o \
	mods/tfmx.o \
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
