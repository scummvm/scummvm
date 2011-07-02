MODULE := common

MODULE_OBJS := \
	archive.o \
	bitstream.o \
	config-file.o \
	config-manager.o \
	dcl.o \
	debug.o \
	error.o \
	EventDispatcher.o \
	EventRecorder.o \
	fft.o \
	file.o \
	fs.o \
	hashmap.o \
	huffman.o \
	iff_container.o \
	macresman.o \
	math.o \
	memorypool.o \
	md5.o \
	mutex.o \
	quicktime.o \
	random.o \
	rational.o \
	str.o \
	stream.o \
	system.o \
	textconsole.o \
	tokenizer.o \
	translation.o \
	unarj.o \
	unzip.o \
	util.o \
	winexe.o \
	winexe_ne.o \
	winexe_pe.o \
	xmlparser.o \
	zlib.o

# Include common rules
include $(srcdir)/rules.mk
