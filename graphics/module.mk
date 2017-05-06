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
	korfont.o \
	larryScale.o \
	maccursor.o \
	macgui/datafiles.o \
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
	renderer.o \
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
	yuv_to_rgb.o \
	pixelbuffer.o \
	opengl/context.o \
	opengl/framebuffer.o \
	opengl/texture.o \
	opengl/tiledsurface.o \
	opengl/shader.o \
	opengl/surfacerenderer.o \
	opengl/box_shaders.o \
	opengl/control_shaders.o \
	opengl/compat_shaders.o

ifdef USE_TINYGL
MODULE_OBJS += \
	tinygl/api.o \
	tinygl/arrays.o \
	tinygl/clear.o \
	tinygl/clip.o \
	tinygl/get.o \
	tinygl/image_util.o \
	tinygl/init.o \
	tinygl/light.o \
	tinygl/list.o \
	tinygl/matrix.o \
	tinygl/memory.o \
	tinygl/misc.o \
	tinygl/select.o \
	tinygl/specbuf.o \
	tinygl/texture.o \
	tinygl/texelbuffer.o \
	tinygl/vertex.o \
	tinygl/zbuffer.o \
	tinygl/zline.o \
	tinygl/zmath.o \
	tinygl/ztriangle.o \
	tinygl/zblit.o \
	tinygl/zdirtyrect.o
endif

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
