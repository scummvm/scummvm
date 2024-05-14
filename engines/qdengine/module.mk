MODULE := engines/qdengine

MODULE_OBJS = \
	qdengine.o \
	console.o \
	metaengine.o \
	core/qd_precomp.o \
	core/parser/qdscr_parser.o \
	core/parser/xml_parser.o \
	core/parser/xml_tag_buffer.o \
	core/runtime/qd_dialogs.o \
	core/runtime/qd_dialog_control.o \
	core/runtime/comline_parser.o \
	core/system/app_log.o \
	core/system/app_log_file.o \
	core/system/app_error_handler.o \
	core/system/app_core.o \
	core/system/graphics/gr_draw_sprite_rle_z.o \
	core/system/graphics/gr_draw_sprite_rle.o \
	core/system/graphics/gr_font.o \
	core/system/graphics/gr_screen_region.o \
	core/system/graphics/gr_tile_animation.o \
	core/system/graphics/gr_tile_sprite.o \
	core/system/graphics/rle_compress.o \
	core/system/input/mouse_input.o \
	core/system/input/keyboard_input.o \
	core/system/input/input_wndproc.o \
	core/system/input/input_recorder.o \
	core/qdcore/qd_animation_frame.o \
    core/qdcore/qd_animation_info.o \
	core/qdcore/qd_animation_maker.o \
    core/qdcore/qd_animation_set_info.o \
    core/qdcore/qd_animation_set_preview.o \
    core/qdcore/qd_animation_set.o \
	core/qdcore/qd_animation.o \

# This module can be built as a plugin
ifeq ($(ENABLE_QDENGINE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
