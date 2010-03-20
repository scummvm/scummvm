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
	macresman.o \
	memorypool.o \
	md5.o \
	mutex.o \
	random.o \
	str.o \
	stream.o \
	system.o \
	textconsole.o \
	tokenizer.o \
	unarj.o \
	unzip.o \
	util.o \
	xmlparser.o \
	zlib.o

# Include common rules
include $(srcdir)/rules.mk
