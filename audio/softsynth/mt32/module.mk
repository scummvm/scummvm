MODULE := audio/softsynth/mt32

MODULE_OBJS := \
	Analog.o \
	BReverbModel.o \
	File.o \
	FileStream.o \
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
	c_interface/c_interface.o

#	SampleRateConverter.o \
#	srchelper/InternalResampler.o \
#	srchelper/SamplerateAdapter.o \
#	srchelper/SoxrAdapter.o \
#	srchelper/srctools/src/FIRResampler.o \
#	srchelper/srctools/src/IIR2xResampler.o \
#	srchelper/srctools/src/LinearResampler.o \
#	srchelper/srctools/src/ResamplerModel.o \
#	srchelper/srctools/src/SincResampler.o
# TODO: The Munt SampleRateConverter requires these additional -I options.
# This is not a very nice way of doing that, though, as it adds them globally.
# INCLUDES += -I $(srcdir)/$(MODULE)/srchelper/srctools/include
# INCLUDES += -I $(srcdir)/$(MODULE)/


# Include common rules
include $(srcdir)/rules.mk
