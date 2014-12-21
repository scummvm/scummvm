MODULE := audio/softsynth/mt32

MODULE_OBJS := \
	Analog.o \
	BReverbModel.o \
	LA32Ramp.o \
	LA32WaveGenerator.o \
	Part.o \
	Partial.o \
	PartialManager.o \
	Poly.o \
	ROMInfo.o \
	Synth.o \
	Tables.o \
	TVA.o \
	TVF.o \
	TVP.o

# Include common rules
include $(srcdir)/rules.mk
