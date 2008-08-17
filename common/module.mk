MODULE := common

MODULE_OBJS := \
	config-file.o \
	config-manager.o \
	debug.o \
	file.o \
	fs.o \
	hashmap.o \
	matrix3.o \
	matrix4.o \
	memorypool.o \
	mutex.o
	str.o \
	stream.o \
	utio.o

# Include common rules
include $(srcdir)/rules.mk
