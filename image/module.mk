MODULE := image

MODULE_OBJS := \
	ani.o \
	bmp.o \
	cel_3do.o \
	icocur.o \
	iff.o \
	jpeg.o \
	neo.o \
	pcx.o \
	pict.o \
	png.o \
	scr.o \
	tga.o \
	xbm.o \
	codecs/bmp_raw.o \
	codecs/cdtoons.o \
	codecs/cinepak.o \
	codecs/codec.o \
	codecs/hlz.o \
	codecs/jyv1.o \
	codecs/mjpeg.o \
	codecs/msrle.o \
	codecs/msrle4.o \
	codecs/msvideo1.o \
	codecs/qtrle.o \
	codecs/rpza.o \
	codecs/smc.o \
	codecs/svq1.o \
	codecs/truemotion1.o \
	codecs/xan.o

ifdef USE_GIF
MODULE_OBJS += \
	gif.o
endif

ifdef USE_MPEG2
MODULE_OBJS += \
	codecs/mpeg.o
endif

ifdef USE_INDEO3
MODULE_OBJS += \
	codecs/indeo3.o
endif

ifdef USE_INDEO45
MODULE_OBJS += \
	codecs/indeo4.o \
	codecs/indeo5.o \
	codecs/indeo/indeo.o \
	codecs/indeo/indeo_dsp.o \
	codecs/indeo/mem.o \
	codecs/indeo/vlc.o
endif

ifdef USE_HNM
MODULE_OBJS += \
	codecs/hnm.o
endif

# Include common rules
include $(srcdir)/rules.mk
