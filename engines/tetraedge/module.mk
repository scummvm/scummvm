MODULE := engines/tetraedge

MODULE_OBJS := \
	tetraedge.o \
	to_lua.o \
	game/amerzone_game.o \
	game/application.o \
	game/billboard.o \
	game/bonus_menu.o \
	game/cellphone.o \
	game/character.o \
	game/character_settings_xml_parser.o \
	game/characters_shadow.o \
	game/confirm.o \
	game/credits.o \
	game/dialog2.o \
	game/document.o \
	game/documents_browser.o \
	game/documents_browser_xml_parser.o \
	game/gallery_menu.o \
	game/game.o \
	game/game_achievements.o \
	game/game_sound.o \
	game/global_bonus_menu.o \
	game/help_option_menu.o \
	game/how_to.o \
	game/in_game_scene.o \
	game/in_game_scene_xml_parser.o \
	game/inventory.o \
	game/inventory_menu.o \
	game/inventory_object.o \
	game/inventory_objects_xml_parser.o \
	game/loc_file.o \
	game/lua_binds.o \
	game/main_menu.o \
	game/notifier.o \
	game/object3d.o \
	game/object_settings_xml_parser.o \
	game/objectif.o \
	game/options_menu.o \
	game/owner_error_menu.o \
	game/particle_xml_parser.o \
	game/puzzle_cadenas.o \
	game/puzzle_coffre.o \
	game/puzzle_computer_hydra.o \
	game/puzzle_computer_pwd.o \
	game/puzzle_disjoncteur.o \
	game/puzzle_hanjie.o \
	game/puzzle_liquides.o \
	game/puzzle_pentacle.o \
	game/puzzle_transfusion.o \
	game/question2.o \
	game/scene_lights_xml_parser.o \
	game/splash_screens.o \
	game/syberia_game.o \
	game/upsell_screen.o \
	game/youki_manager.o \
	te/micropather.o \
	te/te_3d_object2.o \
	te/te_3d_texture.o \
	te/te_act_zone.o \
	te/te_animation.o \
	te/te_bezier_curve.o \
	te/te_button_layout.o \
	te/te_camera.o \
	te/te_camera_xml_parser.o \
	te/te_checkbox_layout.o \
	te/te_clip_layout.o \
	te/te_color.o \
	te/te_core.o \
	te/te_extended_text_layout.o \
	te/te_font2.o \
	te/te_font3.o \
	te/te_frame_anim.o \
	te/te_free_move_zone.o \
	te/te_frustum.o \
	te/te_i_3d_object2.o \
	te/te_i_font.o \
	te/te_i_layout.o \
	te/te_i_loc.o \
	te/te_i_text_layout.o \
	te/te_image.o \
	te/te_images_sequence.o \
	te/te_input_mgr.o \
	te/te_interpolation.o \
	te/te_jpeg.o \
	te/te_layout.o \
	te/te_light.o \
	te/te_list_layout.o \
	te/te_lua_context.o \
	te/te_lua_gui.o \
	te/te_lua_gui_lua_callbacks.o \
	te/te_lua_script.o \
	te/te_lua_thread.o \
	te/te_marker.o \
	te/te_material.o \
	te/te_matricies_stack.o \
	te/te_matrix4x4.o \
	te/te_mesh.o \
	te/te_model.o \
	te/te_model_animation.o \
	te/te_model_vertex_animation.o \
	te/te_music.o \
	te/te_name_val_xml_parser.o \
	te/te_object.o \
	te/te_obp.o \
	te/te_palette.o \
	te/te_particle.o \
	te/te_pick_mesh.o \
	te/te_pick_mesh2.o \
	te/te_png.o \
	te/te_quaternion.o \
	te/te_ray_intersection.o \
	te/te_real_timer.o \
	te/te_renderer.o \
	te/te_resource.o \
	te/te_resource_manager.o \
	te/te_scene.o \
	te/te_scene_warp.o \
	te/te_scene_warp_xml_parser.o \
	te/te_scrolling_layout.o \
	te/te_scummvm_codec.o \
	te/te_sound_manager.o \
	te/te_sprite_layout.o \
	te/te_text_base2.o \
	te/te_text_layout.o \
	te/te_text_layout_xml_parser.o \
	te/te_tga.o \
	te/te_theora.o \
	te/te_tiled_surface.o \
	te/te_tiled_texture.o \
	te/te_timer.o \
	te/te_trs.o \
	te/te_variant.o \
	te/te_vector2f32.o \
	te/te_vector2s32.o \
	te/te_vector3f32.o \
	te/te_visual_fade.o \
	te/te_warp.o \
	te/te_warp_bloc.o \
	te/te_warp_marker.o \
	te/te_xml_parser.o \
	te/te_zlib_jpeg.o \
	te/te_xml_gui.o \
	metaengine.o \
	obb_archive.o

ifdef USE_TINYGL
MODULE_OBJS += \
	game/characters_shadow_tinygl.o \
	te/te_3d_texture_tinygl.o \
	te/te_light_tinygl.o \
	te/te_mesh_tinygl.o \
	te/te_renderer_tinygl.o
endif

ifdef USE_OPENGL_GAME
MODULE_OBJS += \
	game/characters_shadow_opengl.o \
	te/te_3d_texture_opengl.o \
	te/te_light_opengl.o \
	te/te_mesh_opengl.o \
	te/te_renderer_opengl.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_TETRAEDGE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o $(MODULE)/obb_archive.o
