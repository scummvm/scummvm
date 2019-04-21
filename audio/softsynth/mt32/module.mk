MODULE := audio/softsynth/mt32

MODULE_OBJS := \
	Analog.o \
	BReverbModel.o \
	File.o \
	FileStream.o \
	LA32FloatWaveGenerator.o \
	LA32Ramp.o \
	LA32WaveGenerator.o \
	MidiStreamParser.o \
	Part.o \
	Partial.o \
	PartialManager.o \
	Poly.o \
	ROMInfo.o \
	Synth.o \
	Tables.o \
	TVA.o \
	TVF.o \
	TVP.o \
	sha1/sha1.o \
	c_interface/c_interface.o \
	SampleRateConverter.o

# Include common rules
include $(srcdir)/rules.mk
