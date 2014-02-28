MODULE := image

MODULE_OBJS := \
	bmp.o \
	iff.o \
	jpeg.o \
	pcx.o \
	pict.o \
	png.o \
	tga.o

# Include common rules
include $(srcdir)/rules.mk
