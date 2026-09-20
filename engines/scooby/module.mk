MODULE := engines/scooby

MODULE_OBJS = \
	scooby.o \
	console.o \
	md/vdp.o \
	assets/rom.o \
    compression/byte_rle_decoder.o \
    compression/ring_lz_decoder.o \
    graphics/binary_mask_tile_generator.o \
    graphics/genesis_asset_decoder.o \
    graphics/genesis_sprite_table_decoder.o \
    graphics/sprite_layer.o \
    graphics/tile_scene.o \
    input/controller_input.o \
    interactions/action_menu_controller.o \
    interactions/dialogue_controller.o \
    interactions/interaction_action_menu_executor.o \
    interactions/interaction_controller.o \
    interactions/interaction_resolver.o \
    main_menu/main_menu_controller.o \
    main_menu/main_menu_presentation.o \
    main_menu/object_selection/room_object_selection_catalog.o \
    main_menu/object_selection/room_object_selection_controller.o \
    main_menu/passwords/episode_password_menu_controller.o \
    main_menu/passwords/password_display_builder.o \
    main_menu/passwords/password_entry_decoder.o \
    main_menu/reveal/menu_reveal_compositor.o \
    main_menu/room_selection/room_selection_catalog.o \
    main_menu/room_selection/room_selection_controller.o \
    main_menu/selection/menu_unlock_sequence.o \
    main_menu/sound_test/sound_test_catalog.o \
    main_menu/sound_test/sound_test_controller.o \
    main_menu/vblank/main_menu_vblank_handler.o \
    movement/actor_directional_animation_catalog.o \
    movement/ambient_movement_controller.o \
    movement/scripted_room_movement_executor.o \
    presentation/frame_presenter.o \
    randomness/deterministic_random.o \
    raylib_host.o \
    rendering/frame_clock.o \
    rendering/frame_viewport.o \
    rooms/actor_animation_descriptor_catalog.o \
    rooms/actor_sprite_transformer.o \
    rooms/companion_actor_position_transition_animation_catalog.o \
    rooms/duel_room_script_action_executor.o \
    rooms/lead_actor_position_transition_animation_catalog.o \
    rooms/room_actor_animator.o \
    rooms/room_actor_initializer.o \
    rooms/room_actor_updater.o \
    rooms/room_collision_probe.o \
    rooms/room_interface_transition_executor.o \
    rooms/room_object_tile_patch_renderer.o \
    rooms/room_scene_loader.o \
    rooms/room_script_action_05_or_0c_executor.o \
    rooms/room_script_action_1e_executor.o \
    rooms/room_script_action_27_executor.o \
    rooms/room_script_action_executor.o \
    rooms/room_script_camera_pan_executor.o \
    rooms/room_script_command_01_executor.o \
    rooms/room_script_command_02_executor.o \
    rooms/room_script_command_05_executor.o \
    rooms/room_script_command_08_executor.o \
    rooms/room_script_command_09_executor.o \
    rooms/room_script_command_0b_executor.o \
    rooms/room_script_command_0e_or_1b_executor.o \
    rooms/room_script_command_0f_executor.o \
    rooms/room_script_command_10_executor.o \
    rooms/room_script_command_19_executor.o \
    rooms/room_script_command_1a_executor.o \
    rooms/room_script_command_1c_executor.o \
    rooms/room_script_command_1d_executor.o \
    rooms/room_script_command_1e_executor.o \
    rooms/room_script_command_1f_executor.o \
    rooms/room_script_command_20_executor.o \
    rooms/room_script_command_21_executor.o \
    rooms/room_script_command_executor.o \
    rooms/room_script_condition_evaluator.o \
    rooms/room_script_controller.o \
    rooms/room_script_coordinate_sprite_executor.o \
    rooms/room_script_foreground_override_executor.o \
    rooms/room_script_paired_actor_transition_executor.o \
    rooms/room_sprite_renderer.o \
    rooms/room_tile_streamer.o \
    rooms/vblank/room_vblank_handler.o \
    runtime/episode_initializer.o \
    runtime/episode_opening_sequence.o \
    runtime/runtime_state.o \
    runtime/session_controller.o \
    startup/application_loop.o \
    startup/executable_directory.o \
    startup/intro_sequence.o \
    startup/studio_logo_sequence.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_SCOOBY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
