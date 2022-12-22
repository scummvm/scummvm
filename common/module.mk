MODULE := common

MODULE_OBJS := \
	archive.o \
	base-str.o \
	concatstream.o \
	config-manager.o \
	coroutines.o \
	debug.o \
	error.o \
	events.o \
	file.o \
	fs.o \
	gui_options.o \
	hashmap.o \
	language.o \
	localization.o \
	macresman.o \
	memorypool.o \
	md5.o \
	mutex.o \
	osd_message_queue.o \
	path.o \
	platform.o \
	punycode.o \
	random.o \
	rational.o \
	rendermode.o \
	str.o \
	stream.o \
	streamdebug.o \
	str-enc.o \
	encodings/singlebyte.o \
	system.o \
	textconsole.o \
	text-to-speech.o \
	tokenizer.o \
	translation.o \
	unicode-bidi.o \
	ustr.o \
	util.o \
	xpfloat.o

ifdef ENABLE_EVENTRECORDER
MODULE_OBJS += \
	recorderfile.o
endif

ifdef USE_UPDATES
MODULE_OBJS += \
	updates.o
endif

# Include common rules
include $(srcdir)/rules.mk
