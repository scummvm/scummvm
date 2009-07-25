MODULE := common

MODULE_OBJS := \
	archive.o \
	config-file.o \
	config-manager.o \
	debug.o \
	events.o \
	EventRecorder.o \
	file.o \
	fs.o \
	hashmap.o \
	memorypool.o \
	md5.o \
	mutex.o \
	str.o \
	stream.o \
	util.o \
	system.o \
	unarj.o \
	unzip.o \
	xmlparser.o \
	zlib.o

# Include common rules
include $(srcdir)/rules.mk
