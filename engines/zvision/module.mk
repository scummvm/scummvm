MODULE := engines/zvision
 
MODULE_OBJS := \
	animation/meta_animation.o \
	animation/rlf_animation.o \
	archives/zfs_archive.o \
	core/console.o \
	core/events.o \
	core/menu.o \
	core/save_manager.o \
	core/search_manager.o \
	cursors/cursor_manager.o \
	cursors/cursor.o \
	detection.o \
	fonts/truetype_font.o \
	graphics/render_manager.o \
	graphics/render_table.o \
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
	scripting/inventory.o \
	scripting/scr_file_handling.o \
	scripting/script_manager.o \
	scripting/sidefx.o \
	scripting/sidefx/animation_node.o \
	scripting/sidefx/music_node.o \
	scripting/sidefx/syncsound_node.o \
	scripting/sidefx/timer_node.o \
	scripting/sidefx/ttytext_node.o \
	sound/zork_raw.o \
	subtitles/subtitles.o \
	text/string_manager.o \
	text/text.o \
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
