MODULE := math

MODULE_OBJS := \
	angle.o \
	matrix3.o \
	matrix4.o \
	line3d.o \
	line2d.o \
	quat.o \
	rect2d.o \
	vector2d.o \
	vector3d.o \
	vector4d.o \
	aabb.o \
	frustum.o \
	plane.o

# Include common rules
include $(srcdir)/rules.mk
