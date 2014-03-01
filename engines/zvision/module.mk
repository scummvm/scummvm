MODULE := engines/zvision
 
MODULE_OBJS := \
	actions.o \
	animation_node.o \
	clock.o \
	console.o \
	control.o \
	cursor.o \
	cursor_manager.o \
	detection.o \
	events.o \
	input_control.o \
	lever_control.o \
	lzss_read_stream.o \
	push_toggle_control.o \
	render_manager.o \
	render_table.o \
	rlf_animation.o \
	save_manager.o \
	scr_file_handling.o \
	script_manager.o \
	single_value_container.o \
	string_manager.o \
	timer_node.o \
	truetype_font.o \
	utility.o \
	video.o \
	zvision.o \
	zfs_archive.o \
	zork_avi_decoder.o \
	zork_raw.o \
	sidefx.o \
	music_node.o \
	inventory_manager.o \
	slot_control.o \
	menu.o \
	meta_animation.o \
	search_manager.o \
	text.o \
	subtitles.o \
	syncsound_node.o
 
MODULE_DIRS += \
	engines/zvision
 
# This module can be built as a plugin
ifeq ($(ENABLE_ZVISION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
