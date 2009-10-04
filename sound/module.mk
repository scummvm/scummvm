MODULE := sound

MODULE_OBJS := \
	audiostream.o \
	audiocd.o \
	flac.o \
	mixer.o \
	mp3.o \
	rate.o \
	vorbis.o

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
