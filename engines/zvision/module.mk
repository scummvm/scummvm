MODULE := engines/zvision

MODULE_OBJS := \
	animation/rlf_animation.o \
	archives/zfs_archive.o \
	core/console.o \
	core/events.o \
	core/save_manager.o \
	cursors/cursor.o \
	cursors/cursor_manager.o \
	detection.o \
	fonts/truetype_font.o \
	graphics/render_manager.o \
	graphics/render_table.o \
	scripting/actions.o \
	scripting/control.o \
	scripting/controls/animation_control.o \
	scripting/controls/input_control.o \
	scripting/controls/lever_control.o \
	scripting/controls/push_toggle_control.o \
	scripting/controls/timer_node.o \
	scripting/scr_file_handling.o \
	scripting/script_manager.o \
	sound/zork_raw.o \
	strings/string_manager.o \
	utility/clock.o \
	utility/lzss_read_stream.o \
	utility/single_value_container.o \
	utility/utility.o \
	video/video.o \
	video/zork_avi_decoder.o \
	zvision.o

MODULE_DIRS += \
	engines/zvision

# This module can be built as a plugin
ifeq ($(ENABLE_ZVISION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
