MODULE := engines/qdengine

MODULE_OBJS = \
	qdengine.o \
	console.o \
	metaengine.o \
	parser/qdscr_parser.o \
	parser/xml_parser.o \
	parser/xml_tag_buffer.o \
	system/sound/snd_dispatcher.o \
	system/sound/snd_sound.o \
	system/sound/wav_sound.o \
	system/graphics/gr_dispatcher.o \
	system/graphics/gr_draw_sprite_rle_z.o \
	system/graphics/gr_draw_sprite_rle.o \
	system/graphics/gr_draw_sprite_z.o \
	system/graphics/gr_draw_sprite.o \
	system/graphics/gr_font.o \
	system/graphics/gr_tile_animation.o \
	system/graphics/gr_tile_sprite.o \
	system/graphics/rle_compress.o \
	system/graphics/UI_TextParser.o \
	system/input/input_wndproc.o \
	system/input/keyboard_input.o \
	system/input/mouse_input.o \
	qdcore/util/AIAStar_API.o \
	qdcore/util/fps_counter.o \
	qdcore/util/LZ77.o \
	qdcore/util/plaympp_api.o \
	qdcore/util/ResourceDispatcher.o \
	qdcore/util/splash_screen.o \
	qdcore/util/WinVideo.o \
	qdcore/qd_animation_frame.o \
	qdcore/qd_animation_info.o \
	qdcore/qd_animation_set_info.o \
	qdcore/qd_animation_set_preview.o \
	qdcore/qd_animation_set.o \
	qdcore/qd_animation.o \
	qdcore/qd_camera.o \
	qdcore/qd_camera_mode.o \
	qdcore/qd_condition.o \
	qdcore/qd_condition_data.o \
	qdcore/qd_condition_group.o \
	qdcore/qd_condition_object_reference.o \
	qdcore/qd_conditional_object.o \
	qdcore/qd_contour.o \
	qdcore/qd_coords_animation_point.o \
	qdcore/qd_coords_animation.o \
	qdcore/qd_counter.o \
	qdcore/qd_d3dutils.o \
	qdcore/qd_file_manager.o \
	qdcore/qd_font_info.o \
	qdcore/qd_game_dispatcher.o \
	qdcore/qd_game_dispatcher_base.o \
	qdcore/qd_game_end.o \
	qdcore/qd_game_object.o \
	qdcore/qd_game_object_animated.o \
	qdcore/qd_game_object_mouse.o \
	qdcore/qd_game_object_moving.o \
	qdcore/qd_game_object_state.o \
	qdcore/qd_game_object_static.o \
	qdcore/qd_game_scene.o \
	qdcore/qd_grid_zone.o \
	qdcore/qd_grid_zone_state.o \
	qdcore/qd_interface_background.o \
	qdcore/qd_interface_button.o \
	qdcore/qd_interface_counter.o \
	qdcore/qd_interface_dispatcher.o \
	qdcore/qd_interface_element.o \
	qdcore/qd_interface_element_state_mode.o \
	qdcore/qd_interface_element_state.o \
	qdcore/qd_interface_object_base.o \
	qdcore/qd_interface_save.o \
	qdcore/qd_interface_screen.o \
	qdcore/qd_interface_slider.o \
	qdcore/qd_interface_text_window.o \
	qdcore/qd_inventory.o \
	qdcore/qd_inventory_cell.o \
	qdcore/qd_minigame.o \
	qdcore/qd_minigame_config.o \
	qdcore/qd_minigame_interface.o \
	qdcore/qd_music_track.o \
	qdcore/qd_named_object.o \
	qdcore/qd_named_object_base.o \
	qdcore/qd_named_object_indexer.o \
	qdcore/qd_named_object_reference.o \
	qdcore/qd_resource.o \
	qdcore/qd_scale_info.o \
	qdcore/qd_screen_text.o \
	qdcore/qd_screen_text_dispatcher.o \
	qdcore/qd_screen_text_set.o \
	qdcore/qd_setup.o \
	qdcore/qd_sound.o \
	qdcore/qd_sound_info.o \
	qdcore/qd_sprite.o \
	qdcore/qd_textdb.o \
	qdcore/qd_trigger_chain.o \
	qdcore/qd_trigger_element.o \
	qdcore/qd_video.o

ifdef USE_IMGUI
MODULE_OBJS += \
	debugger/debugtools.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_QDENGINE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
