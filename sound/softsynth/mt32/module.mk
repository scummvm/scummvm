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

MODULE_DIRS += \
	sound/softsynth/mt32

# Include common rules 
include $(srcdir)/common.rules
