MODULE := common

MODULE_OBJS := \
	debug.o \
	hashmap.o \
	matrix3.o \
	matrix4.o \
	memorypool.o \
	str.o \
	mutex.o \
	zlib.o

# Include common rules
include $(srcdir)/rules.mk
