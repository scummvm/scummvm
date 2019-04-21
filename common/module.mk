MODULE := common

MODULE_OBJS := \
	archive.o \
	config-manager.o \
	coroutines.o \
	dcl.o \
	debug.o \
	error.o \
	EventDispatcher.o \
	EventMapper.o \
	file.o \
	fs.o \
	gui_options.o \
	hashmap.o \
	iff_container.o \
	ini-file.o \
	installshield_cab.o \
	json.o \
	language.o \
	localization.o \
	macresman.o \
	memorypool.o \
	md5.o \
	mutex.o \
	osd_message_queue.o \
	platform.o \
	quicktime.o \
	random.o \
	rational.o \
	rendermode.o \
	str.o \
	stream.o \
	system.o \
	textconsole.o \
	tokenizer.o \
	translation.o \
	unarj.o \
	unzip.o \
	ustr.o \
	util.o \
	winexe.o \
	winexe_ne.o \
	winexe_pe.o \
	xmlparser.o \
	zlib.o

MODULE_OBJS += \
	cosinetables.o \
	dct.o \
	fft.o \
	rdft.o \
	sinetables.o

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
