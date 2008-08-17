MODULE := common

MODULE_OBJS := \
	config-file.o \
	config-manager.o \
	debug.o \
	file.o \
	fs.o \
	hashmap.o \
	memorypool.o \
	mutex.o \
	str.o \
	stream.o \
	util.o

# Include common rules
include $(srcdir)/rules.mk
