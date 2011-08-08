MODULE := audio/softsynth/mt32

MODULE_OBJS := \
	ANSIFile.o \
	AReverbModel.o \
	DelayReverb.o \
	File.o \
	FreeverbModel.o \
	LA32Ramp.o \
	Part.o \
	Partial.o \
	PartialManager.o \
	Poly.o \
	Synth.o \
	Tables.o \
	TVA.o \
	TVF.o \
	TVP.o \
	freeverb\allpass.o \
	freeverb\comb.o \
	freeverb\revmodel.o

# Include common rules
include $(srcdir)/rules.mk
