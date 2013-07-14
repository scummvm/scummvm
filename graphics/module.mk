MODULE := graphics

MODULE_OBJS := \
	conversion.o \
	cursorman.o \
	font.o \
	fontman.o \
	fonts/bdf.o \
	fonts/consolefont.o \
	fonts/newfont_big.o \
	fonts/newfont.o \
	fonts/ttf.o \
	primitives.o \
	surface.o \
	thumbnail.o \
	VectorRenderer.o \
	VectorRendererSpec.o \
	yuv_to_rgb.o \
	yuva_to_rgba.o \
	decoders/bmp.o \
	decoders/jpeg.o \
	decoders/tga.o \
	pixelbuffer.o \
	opengles2/shader.o \
	opengles2/framebuffer.o \
	opengles2/box_shaders.o \
	opengles2/control_shaders.o \
	opengles2/compat_shaders.o \
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
