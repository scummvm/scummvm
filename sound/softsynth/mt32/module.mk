MODULE := sound/softsynth/mt32

MODULE_OBJS := \
	sound/softsynth/mt32/mt32_file.o \
	sound/softsynth/mt32/i386.o \
	sound/softsynth/mt32/part.o \
	sound/softsynth/mt32/partial.o \
	sound/softsynth/mt32/partialManager.o \
	sound/softsynth/mt32/synth.o \
	sound/softsynth/mt32/tables.o \
	sound/softsynth/mt32/freeverb.o

MODULE_DIRS += \
	sound/softsynth/mt32

# Include common rules 
include $(srcdir)/common.rules
