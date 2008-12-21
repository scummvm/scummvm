MODULE := graphics

MODULE_OBJS := \
	cursorman.o \
	font.o \
	fontman.o \
	fonts/consolefont.o \
	fonts/newfont_big.o \
	fonts/newfont.o \
	fonts/scummfont.o \
	iff.o \
	imagedec.o \
	imageman.o \
	primitives.o \
	scaler.o \
	scaler/thumbnail_intern.o \
	surface.o \
	thumbnail.o \
	VectorRenderer.o \
	VectorRendererSpec.o \
	video/dxa_player.o \
	video/flic_player.o \
	video/mpeg_player.o \
	video/smk_player.o

ifndef DISABLE_SCALERS
MODULE_OBJS += \
	scaler/2xsai.o \
	scaler/aspect.o \
	scaler/scale2x.o \
	scaler/scale3x.o \
	scaler/scalebit.o

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
