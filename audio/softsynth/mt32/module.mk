MODULE := audio/softsynth/mt32

MODULE_OBJS := \
	AReverbModel.o \
	BReverbModel.o \
	DelayReverb.o \
	FreeverbModel.o \
	LA32Ramp.o \
	LA32WaveGenerator.o \
	LegacyWaveGenerator.o \
	Part.o \
	Partial.o \
	PartialManager.o \
	Poly.o \
	ROMInfo.o \
	Synth.o \
	TVA.o \
	TVF.o \
	TVP.o \
	Tables.o \
	freeverb.o

# Include common rules
include $(srcdir)/rules.mk
