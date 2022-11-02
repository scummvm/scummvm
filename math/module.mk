MODULE := math

MODULE_OBJS := \
	aabb.o \
	angle.o \
	frustum.o \
	glmath.o \
	line2d.o \
	line3d.o \
	matrix3.o \
	matrix4.o \
	plane.o \
	quat.o \
	ray.o \
	rect2d.o \
	vector2d.o \
	vector3d.o \
	vector4d.o

# Include common rules
include $(srcdir)/rules.mk
