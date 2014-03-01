MODULE := image

MODULE_OBJS := \
	bmp.o \
	iff.o \
	jpeg.o \
	pcx.o \
	pict.o \
	png.o \
	tga.o \
	codecs/bmp_raw.o \
	codecs/cdtoons.o \
	codecs/cinepak.o \
	codecs/codec.o \
	codecs/indeo3.o \
	codecs/mjpeg.o \
	codecs/msrle.o \
	codecs/msvideo1.o \
	codecs/qtrle.o \
	codecs/rpza.o \
	codecs/smc.o \
	codecs/svq1.o \
	codecs/truemotion1.o

ifdef USE_MPEG2
MODULE_OBJS += \
	codecs/mpeg.o
endif

# Include common rules
include $(srcdir)/rules.mk
