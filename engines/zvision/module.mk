MODULE := engines/zvision
 
MODULE_OBJS := \
	scripting/actions.o \
	animation_node.o \
	utility/clock.o \
	core/console.o \
	scripting/control.o \
	cursors/cursor.o \
	cursors/cursor_manager.o \
	detection.o \
	core/events.o \
	scripting/controls/input_control.o \
	scripting/controls/lever_control.o \
	utility/lzss_read_stream.o \
	scripting/controls/push_toggle_control.o \
	graphics/render_manager.o \
	graphics/render_table.o \
	animation/rlf_animation.o \
	core/save_manager.o \
	scripting/scr_file_handling.o \
	scripting/script_manager.o \
	utility/single_value_container.o \
	strings/string_manager.o \
	scripting/controls/timer_node.o \
	fonts/truetype_font.o \
	utility/utility.o \
	video/video.o \
	zvision.o \
	archives/zfs_archive.o \
	video/zork_avi_decoder.o \
	sound/zork_raw.o \
	sidefx.o \
	music_node.o \
	inventory_manager.o \
	slot_control.o \
	menu.o \
	meta_animation.o \
	search_manager.o \
	text.o \
	subtitles.o \
	syncsound_node.o \
	ttytext_node.o
 
MODULE_DIRS += \
	engines/zvision
 
# This module can be built as a plugin
ifeq ($(ENABLE_ZVISION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
