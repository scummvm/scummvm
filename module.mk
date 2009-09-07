MODULE := engines/stark

MODULE_OBJS := \
	adpcm.o \
	archive.o \
	detection.o \
	sound.o \
	stark.o \
	xrc.o

# Include common rules
include $(srcdir)/rules.mk
