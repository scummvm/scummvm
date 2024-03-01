MODULE := engines/bagel

MODULE_OBJS = \
	bagel.o \
	console.o \
	metaengine.o \
	boflib/bit_buf.o \
	boflib/bof_app.o \
	boflib/bof_debug.o \
	boflib/bof_error.o \
	boflib/bof_file.o \
	boflib/bof_fixed.o \
	boflib/bof_list.o \
	boflib/bof_log.o \
	boflib/bof_object.o \
	boflib/bof_options.o \
	boflib/bof_res.o \
	boflib/bof_sound.o \
	boflib/bof_string.o \
	boflib/bof_timer.o \
	boflib/cache.o \
	boflib/crc.o \
	boflib/date_functions.o \
	boflib/deflate.o \
	boflib/file_functions.o \
	boflib/llist.o \
	boflib/misc.o \
	boflib/queue.o \
	boflib/stack.o \
	boflib/string_functions.o \
	boflib/vector.o \
	boflib/gfx/bof_bitmap.o \
	boflib/gfx/bof_cursor.o \
	boflib/gfx/bof_movie.o \
	boflib/gfx/bof_palette.o \
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
