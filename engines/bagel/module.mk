MODULE := engines/bagel

MODULE_OBJS = \
	bagel.o \
	console.o \
	metaengine.o \
	baglib/area_object.o \
	baglib/parse_object.o \
	boflib/app.o \
	boflib/bit_buf.o \
	boflib/cache.o \
	boflib/crc.o \
	boflib/dat_file.o \
	boflib/date_functions.o \
	boflib/debug.o \
	boflib/deflate.o \
	boflib/error.o \
	boflib/file.o \
	boflib/file_functions.o \
	boflib/fixed.o \
	boflib/list.o \
	boflib/llist.o \
	boflib/log.o \
	boflib/misc.o \
	boflib/object.o \
	boflib/options.o \
	boflib/queue.o \
	boflib/stack.o \
	boflib/string_functions.o \
	boflib/res.o \
	boflib/sound.o \
	boflib/string.o \
	boflib/timer.o \
	boflib/vector.o \
	boflib/gfx/bitmap.o \
	boflib/gfx/cursor.o \
	boflib/gfx/movie.o \
	boflib/gfx/palette.o \
	boflib/gui/display_button.o \
	boflib/gui/display_object.o \
	boflib/gui/display_window.o \
	boflib/gui/window.o

# This module can be built as a plugin
ifeq ($(ENABLE_BAGEL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
