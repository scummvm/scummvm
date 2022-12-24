MODULE := common/lua

ifdef USE_LUA
MODULE_OBJS := \
	double_serialization.o \
	lapi.o \
	lauxlib.o \
	lbaselib.o \
	lcode.o \
	ldblib.o \
	ldebug.o \
	ldo.o \
	lfunc.o \
	lgc.o \
	linit.o \
	liolib.o \
	llex.o \
	lmathlib.o \
	lmem.o \
	loadlib.o \
	lobject.o \
	lopcodes.o \
	loslib.o \
	lparser.o \
	lstate.o \
	lstring.o \
	lstrlib.o \
	ltable.o \
	ltablib.o \
	ltm.o \
	lua_persist.o \
	lua_persistence_util.o \
	lua_unpersist.o \
	lvm.o \
	lzio.o \
	scummvm_file.o
endif

# Include common rules
include $(srcdir)/rules.mk
