MODULE := common

MODULE_OBJS := \
	archive.o \
	config-file.o \
	config-manager.o \
	debug.o \
	EventDispatcher.o \
	EventRecorder.o \
	file.o \
	fs.o \
	hashmap.o \
	libz.o \
	memorypool.o \
	md5.o \
	mutex.o \
	str.o \
	stream.o \
	util.o \
	system.o \
	unzip.o \
	xmlparser.o

# Include common rules
include $(srcdir)/rules.mk
