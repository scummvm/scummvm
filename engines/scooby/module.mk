MODULE := engines/scooby

MODULE_OBJS = \
	scooby.o \
	console.o \
	assets/rom.o \
    compression/ring_lz_decoder.o \
    graphics/genesis_asset_decoder.o \
    graphics/indexed_frame.o \
    graphics/sprite_layer.o \
    graphics/tile_scene.o \
    input/controller_input.o \
    input/mouse.o \
    input/room_mouse_controller.o \
    interactions/action_menu_controller.o \
    interactions/action_menu_executor.o \
    interactions/controller.o \
    interactions/dialogue_controller.o \
    interactions/resolver.o \
    main_menu/controller.o \
    main_menu/object_selection/catalog.o \
    main_menu/object_selection/controller.o \
    main_menu/passwords/controller.o \
    main_menu/passwords/display_state.o \
    main_menu/presentation.o \
    main_menu/reveal/compositor.o \
    main_menu/room_selection/catalog.o \
    main_menu/room_selection/controller.o \
    main_menu/selection/debug_unlock_sequence.o \
    main_menu/sound_test/catalog.o \
    main_menu/sound_test/controller.o \
    main_menu/vblank/handler.o \
    movement/ambient_controller.o \
    movement/scripted_room_movement_executor.o \
    presentation/frame_presenter.o \
    randomness/deterministic_random.o \
    rooms/actor_animation_descriptor_catalog.o \
    rooms/actor_animator.o \
    rooms/actor_facing_animation.o \
    rooms/actor_initializer.o \
    rooms/actor_sprite_transformer.o \
    rooms/actor_updater.o \
    rooms/collision_probe.o \
    rooms/interface_transition_executor.o \
    rooms/object_tile_patch_renderer.o \
    rooms/scene_loader.o \
    rooms/script/action_05_or_0c_executor.o \
    rooms/script/action_1e_executor.o \
    rooms/script/action_27_executor.o \
    rooms/script/action_executor.o \
    rooms/script/camera_pan_executor.o \
    rooms/script/command_01.o \
    rooms/script/command_02.o \
    rooms/script/command_05.o \
    rooms/script/command_08.o \
    rooms/script/command_09.o \
    rooms/script/command_0b.o \
    rooms/script/command_0e_or_1b.o \
    rooms/script/command_0f.o \
    rooms/script/command_10.o \
    rooms/script/command_19.o \
    rooms/script/command_1a.o \
    rooms/script/command_1c.o \
    rooms/script/command_1d.o \
    rooms/script/command_1e.o \
    rooms/script/command_1f.o \
    rooms/script/command_20.o \
    rooms/script/command_21.o \
    rooms/script/command_executor.o \
    rooms/script/condition_evaluator.o \
    rooms/script/controller.o \
    rooms/script/coordinate_sprite_executor.o \
    rooms/script/duel_action_executor.o \
    rooms/script/foreground_override_executor.o \
    rooms/script/paired_actor_transition_executor.o \
    rooms/sprite_renderer.o \
    rooms/tile_streamer.o \
    rooms/vblank/handler.o \
    runtime/episode_initializer.o \
    runtime/episode_opening_sequence.o \
    runtime/runtime_state.o \
    runtime/save_game_data.o \
    runtime/session_controller.o \
    sound/gems_audio.o \
    sound/gems_driver.o \
    sound/genesis_chip.o \
    startup/application_loop.o \
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
