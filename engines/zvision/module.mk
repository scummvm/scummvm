MODULE := engines/zvision

MODULE_OBJS := \
	animation/meta_animation.o \
	core/console.o \
	core/events.o \
	core/menu.o \
	core/midi.o \
	core/save_manager.o \
	core/search_manager.o \
	cursors/cursor_manager.o \
	cursors/cursor.o \
	detection.o \
	graphics/effects/fog.o \
	graphics/effects/light.o \
	graphics/effects/wave.o \
	graphics/render_manager.o \
	graphics/render_table.o \
	graphics/subtitles.o \
	graphics/truetype_font.o \
	scripting/actions.o \
	scripting/control.o \
	scripting/controls/fist_control.o \
	scripting/controls/hotmov_control.o \
	scripting/controls/input_control.o \
	scripting/controls/lever_control.o \
	scripting/controls/paint_control.o \
	scripting/controls/push_toggle_control.o \
	scripting/controls/safe_control.o \
	scripting/controls/save_control.o \
	scripting/controls/slot_control.o \
	scripting/controls/titler_control.o \
	scripting/inventory.o \
	scripting/scr_file_handling.o \
	scripting/script_manager.o \
	scripting/sidefx/animation_node.o \
	scripting/sidefx/distort_node.o \
	scripting/sidefx/music_node.o \
	scripting/sidefx/region_node.o \
	scripting/sidefx/syncsound_node.o \
	scripting/sidefx/timer_node.o \
	scripting/sidefx/ttytext_node.o \
	sound/zork_raw.o \
	text/string_manager.o \
	text/text.o \
	utility/clock.o \
	utility/lzss_read_stream.o \
	utility/utility.o \
	utility/zfs_archive.o \
	video/rlf_decoder.o \
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
