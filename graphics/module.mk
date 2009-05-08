MODULE := graphics

MODULE_OBJS := \
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
