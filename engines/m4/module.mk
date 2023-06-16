MODULE := engines/m4

MODULE_OBJS = \
	m4.o \
	console.o \
	game.o \
	globals.o \
	kernel.o \
	metaengine.o \
	param.o \
	res.o \
	term.o \
	adv_r/db_env.o \
	adv_r/db_rmlst.o \
	fileio/extensions.o \
	fileio/fileio.o \
	fileio/info.o \
	fileio/sys_file.o \
	mem/memman.o \
	mem/reloc.o \
	burger/burger.o

# This module can be built as a plugin
ifeq ($(ENABLE_M4), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
