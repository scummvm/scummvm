MODULE := engines/stark

MODULE_OBJS := \
	adpcm.o \
	archive.o \
	detection.o \
	gfx_opengl.o \
	sound.o \
	stark.o \
	xmg.o \
	xrc.o

# Include common rules
include $(srcdir)/rules.mk
