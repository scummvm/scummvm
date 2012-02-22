MODULE := common

MODULE_OBJS := \
	archive.o \
	config-file.o \
	config-manager.o \
	debug.o \
	streamdebug.o \
	error.o \
	EventDispatcher.o \
	EventMapper.o \
	EventRecorder.o \
	file.o \
	fs.o \
	hashmap.o \
	macresman.o \
	memorypool.o \
	md5.o \
	mutex.o \
	random.o \
	rational.o \
	str.o \
	stream.o \
	system.o \
	textconsole.o \
	tokenizer.o \
	translation.o \
	unzip.o \
	util.o \
	xmlparser.o \
	zlib.o

ifdef USE_BINK
MODULE_OBJS += \
	cosinetables.o \
	dct.o \
	fft.o \
	huffman.o \
	rdft.o \
	sinetables.o
endif

# Include common rules
include $(srcdir)/rules.mk
