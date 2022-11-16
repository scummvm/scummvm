MODULE := math

MODULE_OBJS := \
	aabb.o \
	angle.o \
	cosinetables.o \
	dct.o \
	fft.o \
	frustum.o \
	glmath.o \
	line2d.o \
	line3d.o \
	matrix3.o \
	matrix4.o \
	mdct.o \
	plane.o \
	quat.o \
	ray.o \
	rdft.o \
	rect2d.o \
	sinetables.o \
	sinewindows.o \
	vector2d.o \
	vector3d.o \
	vector4d.o

# Include common rules
include $(srcdir)/rules.mk
