MODULE := video

MODULE_OBJS := \
	avi_decoder.o \
	coktel_decoder.o \
	dxa_decoder.o \
	flic_decoder.o \
	qt_decoder.o \
	smk_decoder.o \
	video_decoder.o \
	codecs/cdtoons.o \
	codecs/cinepak.o \
	codecs/indeo3.o \
	codecs/mjpeg.o \
	codecs/msrle.o \
	codecs/msvideo1.o \
	codecs/qtrle.o \
	codecs/rpza.o \
	codecs/smc.o \
	codecs/truemotion1.o

ifdef USE_BINK
MODULE_OBJS += \
	bink_decoder.o
endif

# Include common rules
include $(srcdir)/rules.mk
