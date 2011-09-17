MODULE := graphics

MODULE_OBJS := \
	cursorman.o \
	font.o \
	fontman.o \
	fonts/bdf.o \
	fonts/consolefont.o \
	fonts/newfont_big.o \
	fonts/newfont.o \
	imagedec.o \
	primitives.o \
	surface.o \
	thumbnail.o \
	VectorRenderer.o \
	VectorRendererSpec.o \
	yuv_to_rgb.o \
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
	tinygl/vertex.o \
	tinygl/zbuffer.o \
	tinygl/zline.o \
	tinygl/zmath.o \
	tinygl/ztriangle.o \
	tinygl/ztriangle_shadow.o

# Include common rules
include $(srcdir)/rules.mk
