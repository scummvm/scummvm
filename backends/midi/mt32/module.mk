MODULE := backends/midi/mt32

MODULE_OBJS := \
	backends/midi/mt32/mt32_file.o \
	backends/midi/mt32/i386.o \
	backends/midi/mt32/part.o \
	backends/midi/mt32/partial.o \
	backends/midi/mt32/partialManager.o \
	backends/midi/mt32/synth.o \
	backends/midi/mt32/tables.o \
	backends/midi/mt32/freeverb.o

MODULE_DIRS += \
	backends/midi/mt32

# Include common rules 
include $(srcdir)/common.rules
