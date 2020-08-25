MODULE := engines/zvision

MODULE_OBJS := \
	core/console.o \
	core/clock.o \
	core/events.o \
	file/lzss_read_stream.o \
	file/save_manager.o \
	file/search_manager.o \
	file/zfs_archive.o \
	graphics/cursors/cursor_manager.o \
	graphics/cursors/cursor.o \
	graphics/effects/fog.o \
	graphics/effects/light.o \
	graphics/effects/wave.o \
	graphics/render_manager.o \
	graphics/render_table.o \
	metaengine.o \
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
	scripting/menu.o \
	scripting/scr_file_handling.o \
	scripting/script_manager.o \
	scripting/effects/animation_effect.o \
	scripting/effects/distort_effect.o \
	scripting/effects/music_effect.o \
	scripting/effects/region_effect.o \
	scripting/effects/syncsound_effect.o \
	scripting/effects/timer_effect.o \
	scripting/effects/ttytext_effect.o \
	sound/midi.o \
	sound/zork_raw.o \
	text/string_manager.o \
	text/subtitles.o \
	text/text.o \
	text/truetype_font.o \
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

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
