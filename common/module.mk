MODULE := common

MODULE_OBJS := \
	achievements.o \
	archive.o \
	base-str.o \
	clickteam.o \
	config-manager.o \
	coroutines.o \
	dcl.o \
	debug.o \
	error.o \
	events.o \
	file.o \
	fs.o \
	gui_options.o \
	gzio.o \
	hashmap.o \
	iff_container.o \
	ini-file.o \
	installshield_cab.o \
	installshieldv3_archive.o \
	json.o \
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
	quicktime.o \
	random.o \
	rational.o \
	rendermode.o \
	str.o \
	stream.o \
	streamdebug.o \
	str-enc.o \
	encodings/singlebyte.o \
	stuffit.o \
	system.o \
	textconsole.o \
	text-to-speech.o \
	tokenizer.o \
	translation.o \
	unarj.o \
	unicode-bidi.o \
	unzip.o \
	ustr.o \
	util.o \
	winexe.o \
	winexe_ne.o \
	winexe_pe.o \
	xmlparser.o \
	xpfloat.o \
	zlib.o

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
