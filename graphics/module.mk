MODULE := graphics

MODULE_OBJS := \
	animation.o \
	font.o \
	fontman.o \
	fonts/consolefont.o \
	fonts/newfont_big.o \
	fonts/newfont.o \
	fonts/scummfont.o \
	ilbm.o \
	imagedec.o \
	imageman.o \
	paletteman.o \
	primitives.o \
	scaler.o \
	scaler/thumbnail.o \
	surface.o

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

MODULE_DIRS += \
	graphics \
	graphics/fonts \
	graphics/scaler

# Include common rules 
include $(srcdir)/common.rules
