MODULE := common

MODULE_OBJS := \
	achievements.o \
	archive.o \
	base-str.o \
	config-manager.o \
	coroutines.o \
	dcl.o \
	debug.o \
	error.o \
	events.o \
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
	mdct.o \
	mutex.o \
	osd_message_queue.o \
	platform.o \
	quicktime.o \
	random.o \
	rational.o \
	rendermode.o \
	sinewindows.o \
	str.o \
	stream.o \
	streamdebug.o \
	str-enc.o \
	encodings/singlebyte.o \
	stuffit.o \
	system.o \
	textconsole.o \
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

ifdef USE_LUA
MODULE_OBJS += \
	lua/double_serialization.o \
	lua/lapi.o \
	lua/lauxlib.o \
	lua/lbaselib.o \
	lua/lcode.o \
	lua/ldblib.o \
	lua/ldebug.o \
	lua/ldo.o \
	lua/lfunc.o \
	lua/lgc.o \
	lua/linit.o \
	lua/liolib.o \
	lua/llex.o \
	lua/lmathlib.o \
	lua/lmem.o \
	lua/loadlib.o \
	lua/lobject.o \
	lua/lopcodes.o \
	lua/loslib.o \
	lua/lparser.o \
	lua/lstate.o \
	lua/lstring.o \
	lua/lstrlib.o \
	lua/ltable.o \
	lua/ltablib.o \
	lua/ltm.o \
	lua/lua_persist.o \
	lua/lua_persistence_util.o \
	lua/lua_unpersist.o \
	lua/lvm.o \
	lua/lzio.o \
	lua/scummvm_file.o
endif

ifdef USE_TTS
MODULE_OBJS += \
	text-to-speech.o
endif

# Include common rules
include $(srcdir)/rules.mk
