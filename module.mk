MODULE := engines/stark

MODULE_OBJS := \
	adpcm.o \
	archive.o \
	detection.o \
	gfx/driver.o \
	gfx/opengl.o \
	gfx/tinygl.o \
	sound.o \
	stark.o \
	xmg.o \
	xrc.o

# Include common rules
include $(srcdir)/rules.mk
