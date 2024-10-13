MODULE := engines/wintermute

MODULE_OBJS := \
	ad/ad_actor.o \
	ad/ad_entity.o \
	ad/ad_game.o \
	ad/ad_inventory.o \
	ad/ad_inventory_box.o \
	ad/ad_item.o \
	ad/ad_layer.o \
	ad/ad_node_state.o \
	ad/ad_object.o \
	ad/ad_path.o \
	ad/ad_path_point.o \
	ad/ad_region.o \
	ad/ad_response.o \
	ad/ad_response_box.o \
	ad/ad_response_context.o \
	ad/ad_rot_level.o \
	ad/ad_scale_level.o \
	ad/ad_scene.o \
	ad/ad_scene_node.o \
	ad/ad_scene_state.o \
	ad/ad_sentence.o \
	ad/ad_sprite_set.o \
	ad/ad_talk_def.o \
	ad/ad_talk_holder.o \
	ad/ad_talk_node.o \
	ad/ad_waypoint_group.o \
	base/scriptables/debuggable/debuggable_script.o \
	base/scriptables/debuggable/debuggable_script_engine.o \
	base/scriptables/script.o \
	base/scriptables/script_engine.o \
	base/scriptables/script_stack.o \
	base/scriptables/script_value.o \
	base/scriptables/script_ext_array.o \
	base/scriptables/script_ext_date.o \
	base/scriptables/script_ext_directory.o \
	base/scriptables/script_ext_file.o \
	base/scriptables/script_ext_math.o \
	base/scriptables/script_ext_object.o \
	base/scriptables/script_ext_mem_buffer.o \
	base/scriptables/script_ext_string.o \
	base/file/base_disk_file.o \
	base/file/base_file.o \
	base/file/base_file_entry.o \
	base/file/base_package.o \
	base/file/base_save_thumb_file.o \
	base/file/base_savefile_manager_file.o \
	base/font/base_font_bitmap.o \
	base/font/base_font_truetype.o \
	base/font/base_font.o \
	base/font/base_font_storage.o \
	base/gfx/base_image.o \
	base/gfx/base_renderer.o \
	base/gfx/base_surface.o \
	base/gfx/osystem/base_surface_osystem.o \
	base/gfx/osystem/base_render_osystem.o \
	base/gfx/osystem/render_ticket.o \
	base/particles/part_particle.o \
	base/particles/part_emitter.o \
	base/particles/part_force.o \
	base/sound/base_sound.o \
	base/sound/base_sound_buffer.o \
	base/sound/base_sound_manager.o \
	base/base_active_rect.o \
	base/base.o \
	base/base_dynamic_buffer.o \
	base/base_engine.o \
	base/base_fader.o \
	base/base_file_manager.o \
	base/base_frame.o \
	base/base_game.o \
	base/base_game_music.o \
	base/base_game_settings.o \
	base/base_keyboard_state.o \
	base/base_named_object.o \
	base/base_object.o \
	base/base_parser.o \
	base/base_persistence_manager.o \
	base/base_point.o \
	base/base_quick_msg.o \
	base/base_region.o \
	base/base_scriptable.o \
	base/base_script_holder.o \
	base/base_sprite.o \
	base/base_string_table.o \
	base/base_sub_frame.o \
	base/base_surface_storage.o \
	base/base_transition_manager.o \
	base/base_viewport.o \
	base/saveload.o \
	base/save_thumb_helper.o \
	base/timer.o \
	ext/dll_dlltest.o \
	ext/dll_geturl.o \
	ext/dll_httpconnect.o \
	ext/dll_img.o \
	ext/dll_installutil.o \
	ext/dll_kernel32.o \
	ext/dll_shell32.o \
	ext/dll_tools.o \
	ext/scene_achievements.o \
	ext/wme_3fstatistics.o \
	ext/wme_commandlinehelper.o \
	ext/wme_galaxy.o \
	ext/wme_steam.o \
	ext/wme_windowmode.o \
	debugger/breakpoint.o \
	debugger/debugger_controller.o \
	debugger/error.o \
	debugger/listing_providers/blank_listing.o \
	debugger/listing_providers/blank_listing_provider.o \
	debugger/listing_providers/basic_source_listing_provider.o \
	debugger/listing_providers/cached_source_listing_provider.o \
	debugger/listing_providers/source_listing.o \
	debugger/listing.o \
	debugger/script_monitor.o \
	debugger/watch.o \
	debugger/watch_instance.o \
	math/math_util.o \
	math/matrix4.o \
	math/vector2.o \
	metaengine.o \
	platform_osystem.o \
	system/sys_class.o \
	system/sys_class_registry.o \
	system/sys_instance.o \
	ui/ui_button.o \
	ui/ui_edit.o \
	ui/ui_entity.o \
	ui/ui_object.o \
	ui/ui_text.o \
	ui/ui_tiled_image.o \
	ui/ui_window.o \
	utils/crc.o \
	utils/path_util.o \
	utils/string_util.o \
	utils/utils.o \
	video/subtitle_card.o \
	video/video_player.o \
	video/video_subtitler.o \
	video/video_theora_player.o \
	debugger.o \
	wintermute.o \
	persistent.o

ifdef ENABLE_WME3D
MODULE_OBJS += \
	ad/ad_actor_3dx.o \
	ad/ad_attach_3dx.o \
	ad/ad_block.o \
	ad/ad_generic.o \
	ad/ad_geom_ext.o \
	ad/ad_geom_ext_node.o \
	ad/ad_object_3d.o \
	ad/ad_path3d.o \
	ad/ad_path_point3d.o \
	ad/ad_scene_geometry.o \
	ad/ad_walkplane.o \
	ad/ad_waypoint_group3d.o \
	base/gfx/3dcamera.o \
	base/gfx/3dlight.o \
	base/gfx/3dloader_3ds.o \
	base/gfx/3dmesh.o \
	base/gfx/3dshadow_volume.o \
	base/gfx/base_renderer3d.o \
	base/gfx/skin_mesh_helper.o \
	base/gfx/xactive_animation.o \
	base/gfx/xanimation.o \
	base/gfx/xanimation_channel.o \
	base/gfx/xanimation_set.o \
	base/gfx/xfile.o \
	base/gfx/xfile_loader.o \
	base/gfx/xframe_node.o \
	base/gfx/xmaterial.o \
	base/gfx/xmath.o \
	base/gfx/xmesh.o \
	base/gfx/xmodel.o \
	base/gfx/xskinmesh.o \
	base/gfx/opengl/base_surface_opengl3d.o \
	base/gfx/opengl/base_render_opengl3d.o \
	base/gfx/opengl/base_render_opengl3d_shader.o \
	base/gfx/opengl/meshx_opengl.o \
	base/gfx/opengl/meshx_opengl_shader.o \
	base/gfx/opengl/mesh3ds_opengl.o \
	base/gfx/opengl/mesh3ds_opengl_shader.o \
	base/gfx/opengl/shadow_volume_opengl.o \
	base/gfx/opengl/shadow_volume_opengl_shader.o \
	base/base_animation_transition_time.o
endif

MODULE_DIRS += \
	engines/wintermute

# This module can be built as a plugin
ifeq ($(ENABLE_WINTERMUTE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
