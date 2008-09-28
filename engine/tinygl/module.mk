MODULE := engine/tinygl

MODULE_OBJS := \
	api.o \
	arrays.o \
	clear.o \
	clip.o \
	get.o \
	image_util.o \
	init.o \
	light.o \
	list.o \
	matrix.o \
	memory.o \
	misc.o \
	select.o \
	specbuf.o \
	texture.o \
	vertex.o \
	zbuffer.o \
	zline.o \
	zmath.o \
	ztriangle.o \
	ztriangle_shadow.o

# Include common rules
include $(srcdir)/rules.mk
