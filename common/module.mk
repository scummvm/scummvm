MODULE := common

MODULE_OBJS := \
	debug.o \
	file.o \
	fs.o \
	hashmap.o \
	matrix3.o \
	matrix4.o \
	memorypool.o \
	str.o \
	stream.o \
	mutex.o

# Include common rules
include $(srcdir)/rules.mk
