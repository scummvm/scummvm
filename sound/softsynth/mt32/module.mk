MODULE := sound/softsynth/mt32

MODULE_OBJS := \
	mt32_file.o \
	i386.o \
	part.o \
	partial.o \
	partialManager.o \
	synth.o \
	tables.o \
	freeverb.o

# Include common rules
include $(srcdir)/rules.mk
