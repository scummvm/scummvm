/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/icb/fn_routines.h"

namespace ICB {

mcodeFunctionReturnCodes fn_test(int32 &, int32 *);
mcodeFunctionReturnCodes fn_create_mega(int32 &, int32 *);
mcodeFunctionReturnCodes fn_shut_down_object(int32 &, int32 *);
mcodeFunctionReturnCodes fn_prop_animate(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_voxel_image_path(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_xyz(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_to_floor(int32 &, int32 *);
mcodeFunctionReturnCodes fn_init_from_nico_file(int32 &, int32 *);
mcodeFunctionReturnCodes fn_init_from_marker_file(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_pan(int32 &, int32 *);
mcodeFunctionReturnCodes fn_context_chosen_logic(int32 &, int32 *);
mcodeFunctionReturnCodes fn_tiny_route(int32 &, int32 *);
mcodeFunctionReturnCodes fn_player(int32 &, int32 *);
mcodeFunctionReturnCodes fn_pause(int32 &, int32 *);
mcodeFunctionReturnCodes fn_face_coord(int32 &, int32 *);
mcodeFunctionReturnCodes fn_face_object(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_generic_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_player_can_interact(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_player_cannot_interact(int32 &, int32 *);
mcodeFunctionReturnCodes fn_generic_prop_interact(int32 &, int32 *);
mcodeFunctionReturnCodes fn_custom_prop_interact(int32 &, int32 *);
mcodeFunctionReturnCodes fn_register_for_event(int32 &, int32 *);
mcodeFunctionReturnCodes fn_unregister_for_event(int32 &, int32 *);
mcodeFunctionReturnCodes fn_register_object_for_event(int32 &, int32 *);
mcodeFunctionReturnCodes fn_unregister_object_for_event(int32 &, int32 *);
mcodeFunctionReturnCodes fn_post_event(int32 &, int32 *);
mcodeFunctionReturnCodes fn_post_future_event(int32 &, int32 *);
mcodeFunctionReturnCodes fn_post_repeating_event(int32 &, int32 *);
mcodeFunctionReturnCodes fn_post_named_event_to_object(int32 &, int32 *);
mcodeFunctionReturnCodes fn_register_for_line_of_sight(int32 &, int32 *);
mcodeFunctionReturnCodes fn_unregister_for_line_of_sight(int32 &, int32 *);
mcodeFunctionReturnCodes fn_register_object_for_line_of_sight(int32 &, int32 *);
mcodeFunctionReturnCodes fn_unregister_object_for_line_of_sight(int32 &, int32 *);
mcodeFunctionReturnCodes fn_can_see(int32 &, int32 *);
mcodeFunctionReturnCodes fn_can_object_see(int32 &, int32 *);
mcodeFunctionReturnCodes fn_call_socket(int32 &, int32 *);
mcodeFunctionReturnCodes fn_prop_set_to_last_frame(int32 &, int32 *);
mcodeFunctionReturnCodes fn_prop_set_to_first_frame(int32 &, int32 *);
mcodeFunctionReturnCodes fn_prop_near_a_mega(int32 &, int32 *);
mcodeFunctionReturnCodes socket_force_new_logic(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_weapon(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_mega_field_of_view(int32 &, int32 *);
mcodeFunctionReturnCodes fn_apply_bullet(int32 &, int32 *);
mcodeFunctionReturnCodes fn_add_inventory_item(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remove_inventory_item(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_carrying(int32 &, int32 *);
mcodeFunctionReturnCodes fn_reset_player(int32 &, int32 *);
mcodeFunctionReturnCodes fn_speak(int32 &, int32 *);
mcodeFunctionReturnCodes fn_request_speech(int32 &, int32 *);
mcodeFunctionReturnCodes fn_add_talker(int32 &, int32 *);
mcodeFunctionReturnCodes fn_issue_speech_request(int32 &, int32 *);
mcodeFunctionReturnCodes fn_confirm_requests(int32 &, int32 *);
mcodeFunctionReturnCodes fn_anon_speech_invite(int32 &, int32 *);
mcodeFunctionReturnCodes fn_converse(int32 &, int32 *);
mcodeFunctionReturnCodes speak_object_face_object(int32 &, int32 *);
mcodeFunctionReturnCodes speak_play_generic_anim(int32 &, int32 *);
mcodeFunctionReturnCodes speak_wait_for_everyone(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_weapon(int32 &, int32 *);
mcodeFunctionReturnCodes fn_rig_test(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_holding(int32 &, int32 *);
mcodeFunctionReturnCodes fn_drop(int32 &, int32 *);
mcodeFunctionReturnCodes fn_item_held(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_to_last_frame_generic_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_common_xa_music(int32 &, int32 *);
mcodeFunctionReturnCodes fn_pass_flag_to_engine(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_is_active(int32 &, int32 *);
mcodeFunctionReturnCodes fn_easy_play_generic_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_teleport(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_custom_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_pan_from_nico(int32 &, int32 *);
mcodeFunctionReturnCodes fn_teleport_to_nico(int32 &, int32 *);
mcodeFunctionReturnCodes fn_are_we_on_this_floor(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_object_on_our_floor(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_object_on_screen(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_object_dead(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_custom(int32 &, int32 *);
mcodeFunctionReturnCodes fn_message(int32 &, int32 *);
mcodeFunctionReturnCodes fn_clear_all_events(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_state_flag(int32 &, int32 *);
mcodeFunctionReturnCodes fn_check_event_waiting(int32 &, int32 *);
mcodeFunctionReturnCodes fn_event_check_last_sender(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_last_event_sender_id(int32 &, int32 *);
mcodeFunctionReturnCodes fn_near(int32 &, int32 *);
mcodeFunctionReturnCodes fn_easy_play_custom_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_snap_face_object(int32 &, int32 *);
mcodeFunctionReturnCodes fn_teleport_z(int32 &, int32 *);
mcodeFunctionReturnCodes fn_on_screen(int32 &, int32 *);
mcodeFunctionReturnCodes fn_hold_if_off_screen(int32 &, int32 *);
mcodeFunctionReturnCodes fn_mega_interacts(int32 &, int32 *);
mcodeFunctionReturnCodes fn_restart_object(int32 &, int32 *);
mcodeFunctionReturnCodes fn_object_near_nico(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_get_mode(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_set_mode(int32 &, int32 *);
mcodeFunctionReturnCodes fn_teleport_y_to_id(int32 &, int32 *);
mcodeFunctionReturnCodes fn_call_socket_id(int32 &, int32 *);
mcodeFunctionReturnCodes fn_route_to_nico(int32 &, int32 *);
mcodeFunctionReturnCodes fn_lift_process_list(int32 &, int32 *);
mcodeFunctionReturnCodes fn_add_object_name_to_list(int32 &, int32 *);
mcodeFunctionReturnCodes fn_add_object_id_to_list(int32 &, int32 *);
mcodeFunctionReturnCodes fn_hold_while_list_near_nico(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_watch(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_persons_weapon(int32 &, int32 *);
mcodeFunctionReturnCodes fn_kill_me(int32 &, int32 *);
mcodeFunctionReturnCodes fn_kill_object(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_to_first_frame_custom_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_to_last_frame_custom_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_shut_down_event_processing(int32 &, int32 *);
mcodeFunctionReturnCodes fn_three_sixty_interact(int32 &, int32 *);
mcodeFunctionReturnCodes fn_route_to_marker(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_mission_xa_music(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_common_vag_music(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_mission_vag_music(int32 &, int32 *);
mcodeFunctionReturnCodes fn_near_list(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_list_result(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_common_fx(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_mission_fx(int32 &, int32 *);
mcodeFunctionReturnCodes fn_add_icon_to_icon_list(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remove_icon_from_icon_list(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_is_email_waiting(int32 &, int32 *);
mcodeFunctionReturnCodes fn_destroy_icon_list(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_send_email(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_menu_return(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_picture(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_clear_screen(int32 &, int32 *);
mcodeFunctionReturnCodes fn_new_apply_bullet(int32 &, int32 *);
mcodeFunctionReturnCodes fn_new_script(int32 &, int32 *);
mcodeFunctionReturnCodes fn_random(int32 &, int32 *);
mcodeFunctionReturnCodes fn_change_session(int32 &, int32 *);
mcodeFunctionReturnCodes fn_changed_sessions(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_set_current_zoom(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_there_interact_object(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_interact_object_id(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_object_interact_object(int32 &, int32 *);
mcodeFunctionReturnCodes speak_chosen(int32 &, int32 *);
mcodeFunctionReturnCodes speak_user_chooser(int32 &, int32 *);
mcodeFunctionReturnCodes speak_add_chooser_icon(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_add_icon(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_remove_icon(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_reset_icon_list(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_choose(int32 &, int32 *);
mcodeFunctionReturnCodes fn_simple_chi(int32 &, int32 *);
mcodeFunctionReturnCodes fn_chi(int32 &, int32 *);
mcodeFunctionReturnCodes speak_menu_still_active(int32 &, int32 *);
mcodeFunctionReturnCodes speak_menu_choices_remain(int32 &, int32 *);
mcodeFunctionReturnCodes speak_close_menu(int32 &, int32 *);
mcodeFunctionReturnCodes speak_new_menu(int32 &, int32 *);
mcodeFunctionReturnCodes speak_end_conversation(int32 &, int32 *);
mcodeFunctionReturnCodes speak_end_menu(int32 &, int32 *);
mcodeFunctionReturnCodes speak_add_special_chooser_icon(int32 &, int32 *);
mcodeFunctionReturnCodes fn_floor_and_floor_camera_linked(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_object_adjacent(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_object_on_this_floor(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_objects_lvar_value(int32 &, int32 *);
mcodeFunctionReturnCodes fn_mega_generic_interact(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_objects_lvar_value(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_object_sight_range(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_object_sight_height(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_registered_for_event(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_object_registered_for_event(int32 &, int32 *);
mcodeFunctionReturnCodes fn_switch_on_the_really_neat_and_special_script_debugging_facility(int32 &, int32 *);
mcodeFunctionReturnCodes fn_switch_off_the_really_neat_and_special_script_debugging_facility(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sound_heard_this(int32 &, int32 *);
mcodeFunctionReturnCodes fn_can_see_in_dark(int32 &, int32 *);
mcodeFunctionReturnCodes fn_mega_never_in_shadow(int32 &, int32 *);
mcodeFunctionReturnCodes fn_message_var(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_new_menu_on_icon(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_new_menu(int32 &, int32 *);
mcodeFunctionReturnCodes fn_no_logic(int32 &, int32 *);
mcodeFunctionReturnCodes fn_gosub(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_custom_simple_animator(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sharp_route(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_custom_button_operated_door(int32 &, int32 *);
mcodeFunctionReturnCodes fn_test_prop_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_lift2_process(int32 &, int32 *);
mcodeFunctionReturnCodes fn_post_named_event_to_object_id(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_custom_auto_door(int32 &, int32 *);
mcodeFunctionReturnCodes fn_switch_to_manual_camera(int32 &, int32 *);
mcodeFunctionReturnCodes fn_cancel_manual_camera(int32 &, int32 *);
mcodeFunctionReturnCodes fn_carrying_how_many(int32 &, int32 *);
mcodeFunctionReturnCodes fn_preload_custom_mega_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_wait_on_icon(int32 &, int32 *);
mcodeFunctionReturnCodes fn_reverse_generic_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_register_for_auto_interaction(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sound_set_hearing_sensitivity(int32 &, int32 *);
mcodeFunctionReturnCodes fn_suspend_events(int32 &, int32 *);
mcodeFunctionReturnCodes fn_unsuspend_events(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sound_heard_something(int32 &, int32 *);
mcodeFunctionReturnCodes fn_prime_player_history(int32 &, int32 *);
mcodeFunctionReturnCodes fn_inherit_prop_anim_height(int32 &, int32 *);
mcodeFunctionReturnCodes fn_start_chi_following(int32 &, int32 *);
mcodeFunctionReturnCodes fn_init_mega_from_nico(int32 &, int32 *);
mcodeFunctionReturnCodes fn_record_player_interaction(int32 &, int32 *);
mcodeFunctionReturnCodes fn_mega_use_lift(int32 &, int32 *);
mcodeFunctionReturnCodes fn_snap_to_nico_y(int32 &, int32 *);
mcodeFunctionReturnCodes fn_inherit_prop_anim_height_id(int32 &, int32 *);
mcodeFunctionReturnCodes fn_route_to_custom_prop_interact(int32 &, int32 *);
mcodeFunctionReturnCodes fn_route_to_generic_prop_interact(int32 &, int32 *);
mcodeFunctionReturnCodes fn_fetch_chi_mode(int32 &, int32 *);
mcodeFunctionReturnCodes fn_lib_lift_chord_and_chi(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_interacting(int32 &, int32 *);
mcodeFunctionReturnCodes fn_clear_interacting(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_emp_flash(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_check_email_id(int32 &, int32 *);
mcodeFunctionReturnCodes fx_widescreen(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_mark_email_read(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_set_map_knowledge_level(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_fix_motion_scan_xz(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_update_player(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_script_activate(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_script_deactivate(int32 &, int32 *);
mcodeFunctionReturnCodes fn_line_of_sight_suspend(int32 &, int32 *);
mcodeFunctionReturnCodes fn_line_of_sight_unsuspend(int32 &, int32 *);
mcodeFunctionReturnCodes fn_check_for_nico(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sony_door_interact(int32 &, int32 *);
mcodeFunctionReturnCodes fn_apply_anim_y(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_default_logic(int32 &, int32 *);
mcodeFunctionReturnCodes fn_line_of_sight_now(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_sfx(int32 &, int32 *);
mcodeFunctionReturnCodes fn_stop_sfx(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_to_first_frame_generic_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_easy_play_generic_anim_with_pan(int32 &, int32 *);
mcodeFunctionReturnCodes speak_set_custom(int32 &, int32 *);
mcodeFunctionReturnCodes speak_play_custom_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_check_for_chi(int32 &, int32 *);
mcodeFunctionReturnCodes fn_wait_for_chi(int32 &, int32 *);
mcodeFunctionReturnCodes fn_send_chi_to_this_object(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_ids_lvar_value(int32 &, int32 *);
mcodeFunctionReturnCodes fn_teleport_to_nico_y(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_id_the_player(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_to_exlusive_coords(int32 &, int32 *);
mcodeFunctionReturnCodes fn_chi_wait_for_player_to_move(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_evil(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_menu_return_on_icon(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_mega_says(int32 &, int32 *);
mcodeFunctionReturnCodes fn_changed_via_this_shaft(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_objects_x(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_objects_y(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_objects_z(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_cad_lock_status(int32 &, int32 *);
mcodeFunctionReturnCodes fn_are_we_on_screen(int32 &, int32 *);
mcodeFunctionReturnCodes fn_reset_icon_list(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_speech_status(int32 &, int32 *);
mcodeFunctionReturnCodes fn_interact_choose(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_mega_within_area(int32 &, int32 *);
mcodeFunctionReturnCodes fn_get_cad_state_flag(int32 &, int32 *);
mcodeFunctionReturnCodes fn_end_mission(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_set_max_zoom(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_set_min_zoom(int32 &, int32 *);
mcodeFunctionReturnCodes fn_route_to_near_mega(int32 &, int32 *);
mcodeFunctionReturnCodes fn_stop_chi_following(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_pose(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_crouching(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_armed(int32 &, int32 *);
mcodeFunctionReturnCodes fn_push_player_status(int32 &, int32 *);
mcodeFunctionReturnCodes fn_pop_player_status(int32 &, int32 *);
mcodeFunctionReturnCodes fn_post_third_party_speech(int32 &, int32 *);
mcodeFunctionReturnCodes fn_fast_face_object(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_paragraph_text(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_main_heading(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_sub_heading(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_option_text(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_warning_text(int32 &, int32 *);
mcodeFunctionReturnCodes fn_object_rerun_logic_context(int32 &, int32 *);
mcodeFunctionReturnCodes fn_interact_near_mega(int32 &, int32 *);
mcodeFunctionReturnCodes fn_am_i_player(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_shoot_overide(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_strike_overide(int32 &, int32 *);
mcodeFunctionReturnCodes fn_start_player_interaction(int32 &, int32 *);
mcodeFunctionReturnCodes fn_start_conveyor(int32 &, int32 *);
mcodeFunctionReturnCodes fn_stop_conveyor(int32 &, int32 *);
mcodeFunctionReturnCodes fn_interact_with_id(int32 &, int32 *);
mcodeFunctionReturnCodes fn_face_nicos_pan(int32 &, int32 *);
mcodeFunctionReturnCodes fn_unregister_for_auto_interaction(int32 &, int32 *);
mcodeFunctionReturnCodes fn_register_chi(int32 &, int32 *);
mcodeFunctionReturnCodes fn_add_y(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_sfx_xyz(int32 &, int32 *);
mcodeFunctionReturnCodes fn_register_stairway(int32 &, int32 *);
mcodeFunctionReturnCodes fn_reverse_custom_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_trace(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_player_has_weapon(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sound_new_entry(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sound_remove_entry(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sound_simulate(int32 &, int32 *);
mcodeFunctionReturnCodes fn_PLEASE_REUSE_THIS_SLOT_2(int32 &, int32 *);
mcodeFunctionReturnCodes fn_PLEASE_REUSE_THIS_SLOT_3(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_object_type(int32 &, int32 *);
mcodeFunctionReturnCodes fn_register_ladder(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_movie(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_an_object_crouching(int32 &, int32 *);
mcodeFunctionReturnCodes fn_align_with_floor(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_sfx_offset(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_sfx_time(int32 &, int32 *);
mcodeFunctionReturnCodes fn_add_medipacks(int32 &, int32 *);
mcodeFunctionReturnCodes fn_use_medipacks(int32 &, int32 *);
mcodeFunctionReturnCodes fn_add_ammo_clips(int32 &, int32 *);
mcodeFunctionReturnCodes fn_use_ammo_clips(int32 &, int32 *);
mcodeFunctionReturnCodes fn_load_players_gun(int32 &, int32 *);
mcodeFunctionReturnCodes fn_flash_health(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_player_pose(int32 &, int32 *);
mcodeFunctionReturnCodes fn_kill_conversations(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_player_running(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_player_walking(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_anim_speed(int32 &, int32 *);
mcodeFunctionReturnCodes fn_push_coords(int32 &, int32 *);
mcodeFunctionReturnCodes fn_pop_coords(int32 &, int32 *);
mcodeFunctionReturnCodes fn_fast_face_coord(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sound_fast_face(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_add_floor_range(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_reset_floor_ranges(int32 &, int32 *);
mcodeFunctionReturnCodes fn_send_chi_to_named_object(int32 &, int32 *);
mcodeFunctionReturnCodes fx_brighten_to(int32 &, int32 *);
mcodeFunctionReturnCodes fx_brighten_from(int32 &, int32 *);
mcodeFunctionReturnCodes fx_narrow_screen(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sound_route_to_near(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sound_suspend(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sound_unsuspend(int32 &, int32 *);
mcodeFunctionReturnCodes fn_wandering_custom_prop_interact(int32 &, int32 *);
mcodeFunctionReturnCodes fn_wandering_generic_prop_interact(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_texture(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_palette(int32 &, int32 *);
mcodeFunctionReturnCodes fn_restart_gamescript(int32 &, int32 *);
mcodeFunctionReturnCodes fn_quick_restart(int32 &, int32 *);
mcodeFunctionReturnCodes fx_generic_fade(int32 &, int32 *);
mcodeFunctionReturnCodes fx_darken_to(int32 &, int32 *);
mcodeFunctionReturnCodes fx_darken_from(int32 &, int32 *);
mcodeFunctionReturnCodes fx_fade_to(int32 &, int32 *);
mcodeFunctionReturnCodes fx_fade_from(int32 &, int32 *);
mcodeFunctionReturnCodes fx_blend(int32 &, int32 *);
mcodeFunctionReturnCodes fn_speech_colour(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_current_camera(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_mega_near_mega(int32 &, int32 *);
mcodeFunctionReturnCodes fn_easy_play_custom_anim_with_pan(int32 &, int32 *);
mcodeFunctionReturnCodes fn_make_remora_beep(int32 &, int32 *);
mcodeFunctionReturnCodes fn_shadows_on(int32 &, int32 *);
mcodeFunctionReturnCodes fn_shadows_off(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_neck_bone(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_neck_vector(int32 &, int32 *);
mcodeFunctionReturnCodes speak_set_neck_vector(int32 &, int32 *);
mcodeFunctionReturnCodes fn_simple_look(int32 &, int32 *);
mcodeFunctionReturnCodes speak_simple_look(int32 &, int32 *);
mcodeFunctionReturnCodes fn_panless_teleport_to_nico(int32 &, int32 *);
mcodeFunctionReturnCodes fn_can_mega_see_dead_megas(int32 &, int32 *);
mcodeFunctionReturnCodes fn_breath(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_dynamic_light(int32 &, int32 *);
mcodeFunctionReturnCodes speak_set_dynamic_light(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sharp_route_to_near_mega(int32 &, int32 *);
mcodeFunctionReturnCodes fn_spectre_route_to_mega(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_shade_percentage(int32 &, int32 *);
mcodeFunctionReturnCodes fn_can_hear_players_feet(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_screen_offset(int32 &, int32 *);
mcodeFunctionReturnCodes fn_shake_screen(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_player_striking(int32 &, int32 *);
mcodeFunctionReturnCodes fn_do_not_disturb(int32 &, int32 *);
mcodeFunctionReturnCodes fn_has_mega_our_height(int32 &, int32 *);
mcodeFunctionReturnCodes fn_chi_heard_gunshot(int32 &, int32 *);
mcodeFunctionReturnCodes fn_prime_custom_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_preload_basics(int32 &, int32 *);
mcodeFunctionReturnCodes fn_activate_timer(int32 &, int32 *);
mcodeFunctionReturnCodes fn_stop_timer(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_sfx_special(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_default_footstep_sfx(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_floor_footstep_sfx(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_footstep_weight(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_special_footstep(int32 &, int32 *);
mcodeFunctionReturnCodes speak_reverse_custom_anim(int32 &, int32 *);
mcodeFunctionReturnCodes speak_preload_custom_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_register_platform_coords(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_progress_bar(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_sfx_offset_time(int32 &, int32 *);
mcodeFunctionReturnCodes fn_activate_stair_or_ladder(int32 &, int32 *);
mcodeFunctionReturnCodes fn_deactivate_stair_or_ladder(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_half_character_width(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_interact_look_height(int32 &, int32 *);
mcodeFunctionReturnCodes fn_swordfight(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_visible(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_object_visible(int32 &, int32 *);
mcodeFunctionReturnCodes fn_calibrate_chi(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_to_dead(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sound_link_floors(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_voice_over_colour(int32 &, int32 *);
mcodeFunctionReturnCodes fn_default_voice_over_colour(int32 &, int32 *);
mcodeFunctionReturnCodes fn_remora_blank_line(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_camera_hold(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_mega_wait_for_player(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_mega_off_camera_hold(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_mega_slice_hold(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_mesh(int32 &, int32 *);
mcodeFunctionReturnCodes fn_sync_with_mega(int32 &, int32 *);
mcodeFunctionReturnCodes fn_laser_route(int32 &, int32 *);
mcodeFunctionReturnCodes fn_prop_crouch_interact(int32 &, int32 *);
mcodeFunctionReturnCodes fn_preload_sting(int32 &, int32 *);
mcodeFunctionReturnCodes fn_play_sting(int32 &, int32 *);
mcodeFunctionReturnCodes fn_stop_sting(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_sleep(int32 &, int32 *);
mcodeFunctionReturnCodes speak_allocate_music(int32 &, int32 *);
mcodeFunctionReturnCodes speak_preload_music(int32 &, int32 *);
mcodeFunctionReturnCodes speak_play_music(int32 &, int32 *);
mcodeFunctionReturnCodes speak_stop_music(int32 &, int32 *);
mcodeFunctionReturnCodes speak_end_music(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_sfx(int32 &, int32 *);
mcodeFunctionReturnCodes fn_wait_for_button(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_as_player(int32 &, int32 *);
mcodeFunctionReturnCodes fn_lock_y(int32 &, int32 *);
mcodeFunctionReturnCodes fn_unlock_y(int32 &, int32 *);
mcodeFunctionReturnCodes fn_is_current_location(int32 &, int32 *);
mcodeFunctionReturnCodes fn_flip_pan(int32 &, int32 *);
mcodeFunctionReturnCodes fn_snap_to_ladder_top(int32 &, int32 *);
mcodeFunctionReturnCodes fn_snap_to_ladder_bottom(int32 &, int32 *);
mcodeFunctionReturnCodes fn_set_feet_to_pan(int32 &, int32 *);
mcodeFunctionReturnCodes fn_room_route(int32 &, int32 *);
mcodeFunctionReturnCodes fn_hard_load_generic_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_hard_load_custom_anim(int32 &, int32 *);
mcodeFunctionReturnCodes fn_activate_sparkle(int32 &, int32 *);
mcodeFunctionReturnCodes fn_deactivate_sparkle(int32 &, int32 *);
mcodeFunctionReturnCodes fn_missing_routine(int32 &, int32 *);

mcodeFunctionReturnCodes (*McodeTable[NO_API_ROUTINES])(int32 &, int32 *) = {
    fn_test,
    fn_create_mega,
    fn_shut_down_object,
    fn_prop_animate,
    fn_set_voxel_image_path,
    fn_set_xyz,
    fn_set_to_floor,
    fn_init_from_nico_file,
    fn_init_from_marker_file,
    fn_set_pan,
    fn_context_chosen_logic,
    fn_tiny_route,
    fn_player,
    fn_pause,
    fn_face_coord,
    fn_face_object,
    fn_play_generic_anim,
    fn_set_player_can_interact,
    fn_set_player_cannot_interact,
    fn_generic_prop_interact,
    fn_custom_prop_interact,
    fn_register_for_event,
    fn_unregister_for_event,
    fn_register_object_for_event,
    fn_unregister_object_for_event,
    fn_post_event,
    fn_post_future_event,
    fn_post_repeating_event,
    fn_post_named_event_to_object,
    fn_register_for_line_of_sight,
    fn_unregister_for_line_of_sight,
    fn_register_object_for_line_of_sight,
    fn_unregister_object_for_line_of_sight,
    fn_can_see,
    fn_can_object_see,
    fn_call_socket,
    fn_prop_set_to_last_frame,
    fn_prop_set_to_first_frame,
    fn_prop_near_a_mega,
    socket_force_new_logic,
    fn_set_weapon,
    fn_set_mega_field_of_view,
    fn_apply_bullet,
    fn_add_inventory_item,
    fn_remove_inventory_item,
    fn_is_carrying,
    fn_reset_player,
    fn_speak,
    fn_request_speech,
    fn_add_talker,
    fn_issue_speech_request,
    fn_confirm_requests,
    fn_anon_speech_invite,
    fn_converse,
    speak_object_face_object,
    speak_play_generic_anim,
    speak_wait_for_everyone,
    fn_get_weapon,
    fn_rig_test,
    fn_is_holding,
    fn_drop,
    fn_item_held,
    fn_set_to_last_frame_generic_anim,
    fn_play_common_xa_music,
    fn_pass_flag_to_engine,
    fn_remora_is_active,
    fn_easy_play_generic_anim,
    fn_teleport,
    fn_play_custom_anim,
    fn_get_pan_from_nico,
    fn_teleport_to_nico,
    fn_are_we_on_this_floor,
    fn_is_object_on_our_floor,
    fn_is_object_on_screen,
    fn_is_object_dead,
    fn_set_custom,
    fn_message,
    fn_clear_all_events,
    fn_get_state_flag,
    fn_check_event_waiting,
    fn_event_check_last_sender,
    fn_get_last_event_sender_id,
    fn_near,
    fn_easy_play_custom_anim,
    fn_snap_face_object,
    fn_teleport_z,
    fn_on_screen,
    fn_hold_if_off_screen,
    fn_mega_interacts,
    fn_restart_object,
    fn_object_near_nico,
    fn_remora_get_mode,
    fn_remora_set_mode,
    fn_teleport_y_to_id,
    fn_call_socket_id,
    fn_route_to_nico,
    fn_lift_process_list,
    fn_add_object_name_to_list,
    fn_add_object_id_to_list,
    fn_hold_while_list_near_nico,
    fn_set_watch,
    fn_get_persons_weapon,
    fn_kill_me,
    fn_kill_object,
    fn_set_to_first_frame_custom_anim,
    fn_set_to_last_frame_custom_anim,
    fn_shut_down_event_processing,
    fn_three_sixty_interact,
    fn_route_to_marker,
    fn_play_mission_xa_music,
    fn_play_common_vag_music,
    fn_play_mission_vag_music,
    fn_near_list,
    fn_get_list_result,
    fn_play_common_fx,
    fn_play_mission_fx,
    fn_add_icon_to_icon_list,
    fn_remove_icon_from_icon_list,
    fn_remora_is_email_waiting,
    fn_destroy_icon_list,
    fn_remora_send_email,
    fn_remora_menu_return,
    fn_remora_picture,
    fn_remora_clear_screen,
    fn_new_apply_bullet,
    fn_new_script,
    fn_random,
    fn_change_session,
    fn_changed_sessions,
    fn_remora_set_current_zoom,
    fn_is_there_interact_object,
    fn_get_interact_object_id,
    fn_is_object_interact_object,
    speak_chosen,
    speak_user_chooser,
    speak_add_chooser_icon,
    fn_remora_add_icon,
    fn_remora_remove_icon,
    fn_remora_reset_icon_list,
    fn_remora_choose,
    fn_simple_chi,
    fn_chi,
    speak_menu_still_active,
    speak_menu_choices_remain,
    speak_close_menu,
    speak_new_menu,
    speak_end_conversation,
    speak_end_menu,
    speak_add_special_chooser_icon,
    fn_floor_and_floor_camera_linked,
    fn_is_object_adjacent,
    fn_is_object_on_this_floor,
    fn_get_objects_lvar_value,
    fn_mega_generic_interact,
    fn_set_objects_lvar_value,
    fn_set_object_sight_range,
    fn_set_object_sight_height,
    fn_is_registered_for_event,
    fn_is_object_registered_for_event,
    fn_switch_on_the_really_neat_and_special_script_debugging_facility,
    fn_switch_off_the_really_neat_and_special_script_debugging_facility,
    fn_sound_heard_this,
    fn_can_see_in_dark,
    fn_mega_never_in_shadow,
    fn_message_var,
    fn_remora_new_menu_on_icon,
    fn_remora_new_menu,
    fn_no_logic,
    fn_gosub,
    fn_set_custom_simple_animator,
    fn_sharp_route,
    fn_set_custom_button_operated_door,
    fn_test_prop_anim,
    fn_lift2_process,
    fn_post_named_event_to_object_id,
    fn_set_custom_auto_door,
    fn_switch_to_manual_camera,
    fn_cancel_manual_camera,
    fn_carrying_how_many,
    fn_preload_custom_mega_anim,
    fn_remora_wait_on_icon,
    fn_reverse_generic_anim,
    fn_register_for_auto_interaction,
    fn_sound_set_hearing_sensitivity,
    fn_suspend_events,
    fn_unsuspend_events,
    fn_sound_heard_something,
    fn_prime_player_history,
    fn_inherit_prop_anim_height,
    fn_start_chi_following,
    fn_init_mega_from_nico,
    fn_record_player_interaction,
    fn_mega_use_lift,
    fn_snap_to_nico_y,
    fn_inherit_prop_anim_height_id,
    fn_route_to_custom_prop_interact,
    fn_route_to_generic_prop_interact,
    fn_fetch_chi_mode,
    fn_lib_lift_chord_and_chi,
    fn_set_interacting,
    fn_clear_interacting,
    fn_remora_emp_flash,
    fn_remora_check_email_id,
    fx_widescreen,
    fn_remora_mark_email_read,
    fn_remora_set_map_knowledge_level,
    fn_remora_fix_motion_scan_xz,
    fn_remora_update_player,
    fn_remora_script_activate,
    fn_remora_script_deactivate,
    fn_line_of_sight_suspend,
    fn_line_of_sight_unsuspend,
    fn_check_for_nico,
    fn_sony_door_interact,
    fn_apply_anim_y,
    fn_remora_default_logic,
    fn_line_of_sight_now,
    fn_play_sfx,
    fn_stop_sfx,
    fn_set_to_first_frame_generic_anim,
    fn_easy_play_generic_anim_with_pan,
    speak_set_custom,
    speak_play_custom_anim,
    fn_check_for_chi,
    fn_wait_for_chi,
    fn_send_chi_to_this_object,
    fn_set_ids_lvar_value,
    fn_teleport_to_nico_y,
    fn_is_id_the_player,
    fn_set_to_exlusive_coords,
    fn_chi_wait_for_player_to_move,
    fn_set_evil,
    fn_remora_menu_return_on_icon,
    fn_remora_mega_says,
    fn_changed_via_this_shaft,
    fn_get_objects_x,
    fn_get_objects_y,
    fn_get_objects_z,
    fn_set_cad_lock_status,
    fn_are_we_on_screen,
    fn_reset_icon_list,
    fn_get_speech_status,
    fn_interact_choose,
    fn_is_mega_within_area,
    fn_get_cad_state_flag,
    fn_end_mission,
    fn_remora_set_max_zoom,
    fn_remora_set_min_zoom,
    fn_route_to_near_mega,
    fn_stop_chi_following,
    fn_set_pose,
    fn_is_crouching,
    fn_is_armed,
    fn_push_player_status,
    fn_pop_player_status,
    fn_post_third_party_speech,
    fn_fast_face_object,
    fn_remora_paragraph_text,
    fn_remora_main_heading,
    fn_remora_sub_heading,
    fn_remora_option_text,
    fn_remora_warning_text,
    fn_object_rerun_logic_context,
    fn_interact_near_mega,
    fn_am_i_player,
    fn_set_shoot_overide,
    fn_set_strike_overide,
    fn_start_player_interaction,
    fn_start_conveyor,
    fn_stop_conveyor,
    fn_interact_with_id,
    fn_face_nicos_pan,
    fn_unregister_for_auto_interaction,
    fn_register_chi,
    fn_add_y,
    fn_play_sfx_xyz,
    fn_register_stairway,
    fn_reverse_custom_anim,
    fn_trace,
    fn_set_player_has_weapon,
    fn_sound_new_entry,
    fn_sound_remove_entry,
    fn_sound_simulate,
    fn_PLEASE_REUSE_THIS_SLOT_2,
    fn_PLEASE_REUSE_THIS_SLOT_3,
    fn_set_object_type,
    fn_register_ladder,
    fn_play_movie,
    fn_is_an_object_crouching,
    fn_align_with_floor,
    fn_play_sfx_offset,
    fn_play_sfx_time,
    fn_add_medipacks,
    fn_use_medipacks,
    fn_add_ammo_clips,
    fn_use_ammo_clips,
    fn_load_players_gun,
    fn_flash_health,
    fn_set_player_pose,
    fn_kill_conversations,
    fn_is_player_running,
    fn_is_player_walking,
    fn_set_anim_speed,
    fn_push_coords,
    fn_pop_coords,
    fn_fast_face_coord,
    fn_sound_fast_face,
    fn_remora_add_floor_range,
    fn_remora_reset_floor_ranges,
    fn_send_chi_to_named_object,
    fx_brighten_to,
    fx_brighten_from,
    fx_narrow_screen,
    fn_sound_route_to_near,
    fn_sound_suspend,
    fn_sound_unsuspend,
    fn_wandering_custom_prop_interact,
    fn_wandering_generic_prop_interact,
    fn_set_texture,
    fn_set_palette,
    fn_restart_gamescript,
    fn_quick_restart,
    fx_generic_fade,
    fx_darken_to,
    fx_darken_from,
    fx_fade_to,
    fx_fade_from,
    fx_blend,
    fn_speech_colour,
    fn_is_current_camera,
    fn_is_mega_near_mega,
    fn_easy_play_custom_anim_with_pan,
    fn_make_remora_beep,
    fn_shadows_on,
    fn_shadows_off,
    fn_set_neck_bone,
    fn_set_neck_vector,
    speak_set_neck_vector,
    fn_simple_look,
    speak_simple_look,
    fn_panless_teleport_to_nico,
    fn_can_mega_see_dead_megas,
    fn_breath,
    fn_set_dynamic_light,
    speak_set_dynamic_light,
    fn_sharp_route_to_near_mega,
    fn_spectre_route_to_mega,
    fn_set_shade_percentage,
    fn_can_hear_players_feet,
    fn_set_screen_offset,
    fn_shake_screen,
    fn_is_player_striking,
    fn_do_not_disturb,
    fn_has_mega_our_height,
    fn_chi_heard_gunshot,
    fn_prime_custom_anim,
    fn_preload_basics,
    fn_activate_timer,
    fn_stop_timer,
    fn_play_sfx_special,
    fn_set_default_footstep_sfx,
    fn_set_floor_footstep_sfx,
    fn_set_footstep_weight,
    fn_set_special_footstep,
    speak_reverse_custom_anim,
    speak_preload_custom_anim,
    fn_register_platform_coords,
    fn_remora_progress_bar,
    fn_play_sfx_offset_time,
    fn_activate_stair_or_ladder,
    fn_deactivate_stair_or_ladder,
    fn_set_half_character_width,
    fn_set_interact_look_height,
    fn_swordfight,
    fn_set_visible,
    fn_set_object_visible,
    fn_calibrate_chi,
    fn_set_to_dead,
    fn_sound_link_floors,
    fn_set_voice_over_colour,
    fn_default_voice_over_colour,
    fn_remora_blank_line,
    fn_set_camera_hold,
    fn_set_mega_wait_for_player,
    fn_set_mega_off_camera_hold,
    fn_set_mega_slice_hold,
    fn_set_mesh,
    fn_sync_with_mega,
    fn_laser_route,
    fn_prop_crouch_interact,
    fn_preload_sting,
    fn_play_sting,
    fn_stop_sting,
    fn_set_sleep,
    speak_allocate_music,
    speak_preload_music,
    speak_play_music,
    speak_stop_music,
    speak_end_music,
    fn_set_sfx,
    fn_wait_for_button,
    fn_set_as_player,
    fn_lock_y,
    fn_unlock_y,
    fn_is_current_location,
    fn_flip_pan,
    fn_snap_to_ladder_top,
    fn_snap_to_ladder_bottom,
    fn_set_feet_to_pan,
    fn_room_route,
    fn_hard_load_generic_anim,
    fn_hard_load_custom_anim,
    fn_activate_sparkle,
    fn_deactivate_sparkle,
    fn_missing_routine,
    fn_missing_routine,
    fn_missing_routine,
    fn_missing_routine,
    fn_missing_routine,
};

} // End of namespace ICB
