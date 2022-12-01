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
	iff_container.o \
	ini-file.o \
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
	system.o \
	textconsole.o \
	text-to-speech.o \
	tokenizer.o \
	translation.o \
	unicode-bidi.o \
	ustr.o \
	util.o \
	winexe.o \
	winexe_ne.o \
	winexe_pe.o \
	xmlparser.o \
	xpfloat.o \
	compression/clickteam.o \
	compression/dcl.o \
	compression/gzio.o \
	compression/installshield_cab.o \
	compression/installshieldv3_archive.o \
	compression/rnc_deco.o \
	compression/stuffit.o \
	compression/unarj.o \
	compression/unzip.o \
	compression/vise.o \
	compression/zlib.o

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
