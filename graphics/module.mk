MODULE := graphics

MODULE_OBJS := \
	conversion.o \
	cursorman.o \
	dither.o \
	font.o \
	fontman.o \
	fonts/consolefont.o \
	fonts/newfont_big.o \
	fonts/newfont.o \
	fonts/scummfont.o \
	iff.o \
	imagedec.o \
	jpeg.o \
	primitives.o \
	scaler.o \
	scaler/thumbnail_intern.o \
	sjis.o \
	surface.o \
	thumbnail.o \
	VectorRenderer.o \
	VectorRendererSpec.o \
	video/avi_decoder.o \
	video/dxa_decoder.o \
	video/flic_decoder.o \
	video/mpeg_player.o \
	video/smk_decoder.o \
	video/video_player.o \
	video/codecs/msvideo1.o \
	video/coktelvideo/indeo3.o \
	video/coktelvideo/coktelvideo.o

ifndef DISABLE_SCALERS
MODULE_OBJS += \
	scaler/2xsai.o \
	scaler/aspect.o \
	scaler/scale2x.o \
	scaler/scale3x.o \
	scaler/scalebit.o

ifdef USE_ARM_SCALER_ASM
MODULE_OBJS += \
	scaler/scale2xARM.o \
	scaler/Normal2xARM.o
endif

ifndef DISABLE_HQ_SCALERS
MODULE_OBJS += \
	scaler/hq2x.o \
	scaler/hq3x.o

ifdef HAVE_NASM
MODULE_OBJS += \
	scaler/hq2x_i386.o \
	scaler/hq3x_i386.o
endif

endif

endif

# Include common rules
include $(srcdir)/rules.mk
