MODULE := graphics

MODULE_OBJS := \
	conversion.o \
	cursorman.o \
	font.o \
	fontman.o \
	fonts/bdf.o \
	fonts/consolefont.o \
	fonts/macfont.o \
	fonts/newfont_big.o \
	fonts/newfont.o \
	fonts/ttf.o \
	fonts/winfont.o \
	larryScale.o \
	maccursor.o \
	macgui/macbutton.o \
	macgui/macfontmanager.o \
	macgui/macmenu.o \
	macgui/mactext.o \
	macgui/mactextwindow.o \
	macgui/macwidget.o \
	macgui/macwindow.o \
	macgui/macwindowborder.o \
	macgui/macwindowmanager.o \
	managed_surface.o \
	nine_patch.o \
	pixelformat.o \
	primitives.o \
	scaler.o \
	scaler/thumbnail_intern.o \
	screen.o \
	sjis.o \
	surface.o \
	transform_struct.o \
	transform_tools.o \
	transparent_surface.o \
	thumbnail.o \
	VectorRenderer.o \
	VectorRendererSpec.o \
	wincursor.o \
	yuv_to_rgb.o

ifdef USE_SCALERS
MODULE_OBJS += \
	scaler/2xsai.o \
	scaler/aspect.o \
	scaler/downscaler.o \
	scaler/scale2x.o \
	scaler/scale3x.o \
	scaler/scalebit.o

ifdef USE_ARM_SCALER_ASM
MODULE_OBJS += \
	scaler/downscalerARM.o \
	scaler/scale2xARM.o \
	scaler/Normal2xARM.o
endif

ifdef USE_HQ_SCALERS
MODULE_OBJS += \
	scaler/hq2x.o \
	scaler/hq3x.o

ifdef USE_NASM
MODULE_OBJS += \
	scaler/hq2x_i386.o \
	scaler/hq3x_i386.o
endif

endif

endif

# Include common rules
include $(srcdir)/rules.mk
