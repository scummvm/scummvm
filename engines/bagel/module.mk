MODULE := engines/bagel

MODULE_OBJS = \
	bagel.o \
	console.o \
	metaengine.o \
	boflib/bit_buf.o \
	boflib/bof_debug.o \
	boflib/bof_error.o \
	boflib/bof_file.o \
	boflib/bof_fixed.o \
	boflib/bof_list.o \
	boflib/bof_log.o \
	boflib/bof_object.o \
	boflib/bof_opt.o \
	boflib/bof_str.o \
	boflib/cache.o \
	boflib/crc.o \
	boflib/deflate.o \
	boflib/llist.o \
	boflib/misc.o \
	boflib/stack.o \
	boflib/vector.o

# This module can be built as a plugin
ifeq ($(ENABLE_BAGEL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
