MODULE := video

MODULE_OBJS := \
	3do_decoder.o \
	avi_decoder.o \
	coktel_decoder.o \
	dxa_decoder.o \
	flic_decoder.o \
	mpegps_decoder.o \
	mve_decoder.o \
	paco_decoder.o \
	psx_decoder.o \
	qt_decoder.o \
	smk_decoder.o \
	subtitles.o \
	video_decoder.o

ifdef USE_BINK
MODULE_OBJS += \
	bink_decoder.o
endif

ifdef USE_HNM
MODULE_OBJS += \
	hnm_decoder.o
endif

ifdef USE_THEORADEC
MODULE_OBJS += \
	theora_decoder.o
endif

ifdef USE_VPX
MODULE_OBJS += \
	mkv_decoder.o \
	mkv/mkvparser.o
endif

# Include common rules
include $(srcdir)/rules.mk
