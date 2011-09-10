MODULE := math

MODULE_OBJS := \
	matrix3.o \
	matrix4.o \
	line3d.o \
	line2d.o \
	rect2d.o \
	vector2d.o \
	vector3d.o

# Include common rules
include $(srcdir)/rules.mk
