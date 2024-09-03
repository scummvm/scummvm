/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/icb.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/floors.h"
#include "engines/icb/fn_routines.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/sound.h" // to get PauseSounds()
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/common/px_floor_map.h"
#include "engines/icb/common/px_features.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/actor.h"
#include "engines/icb/remora.h"
#include "engines/icb/res_man.h"

#include "common/keyboard.h"

namespace ICB {

mcodeFunctionReturnCodes fn_set_voxel_image_path(int32 &result, int32 *params) { return (g_mission->session->fn_set_voxel_image_path(result, params)); }

mcodeFunctionReturnCodes fn_test(int32 &result, int32 *params) { return (g_mission->session->fn_test(result, params)); }

mcodeFunctionReturnCodes fn_create_mega(int32 &result, int32 *params) { return (g_mission->session->fn_create_mega(result, params)); }

mcodeFunctionReturnCodes fn_set_xyz(int32 &result, int32 *params) { return (g_mission->session->fn_set_xyz(result, params)); }

mcodeFunctionReturnCodes fn_set_to_floor(int32 &result, int32 *params) { return (g_mission->session->fn_set_to_floor(result, params)); }

mcodeFunctionReturnCodes fn_init_from_nico_file(int32 &result, int32 *params) { return (g_mission->session->fn_init_from_nico_file(result, params)); }

mcodeFunctionReturnCodes fn_init_from_marker_file(int32 &result, int32 *params) { return (MS->fn_init_from_marker_file(result, params)); }

mcodeFunctionReturnCodes fn_set_player_can_interact(int32 &result, int32 *params) { return (MS->fn_set_player_can_interact(result, params)); }

mcodeFunctionReturnCodes fn_set_player_cannot_interact(int32 &result, int32 *params) { return (MS->fn_set_player_cannot_interact(result, params)); }

mcodeFunctionReturnCodes fn_call_socket(int32 &result, int32 *params) { return (MS->fn_call_socket(result, params)); }

mcodeFunctionReturnCodes fn_prop_near_a_mega(int32 &result, int32 *params) { return (MS->fn_prop_near_a_mega(result, params)); }

mcodeFunctionReturnCodes socket_force_new_logic(int32 &result, int32 *params) { return (MS->socket_force_new_logic(result, params)); }

mcodeFunctionReturnCodes fn_reset_player(int32 &result, int32 *params) { return (MS->fn_reset_player(result, params)); }

mcodeFunctionReturnCodes fn_rig_test(int32 &result, int32 *params) { return (MS->fn_rig_test(result, params)); }

mcodeFunctionReturnCodes fn_teleport(int32 &result, int32 *params) { return (MS->fn_teleport(result, params)); }

mcodeFunctionReturnCodes fn_teleport_to_nico(int32 &result, int32 *params) { return (MS->fn_teleport_to_nico(result, params)); }

mcodeFunctionReturnCodes fn_get_pan_from_nico(int32 &result, int32 *params) { return (MS->fn_get_pan_from_nico(result, params)); }

mcodeFunctionReturnCodes fn_are_we_on_this_floor(int32 &result, int32 *params) { return (MS->fn_are_we_on_this_floor(result, params)); }

mcodeFunctionReturnCodes fn_is_object_on_our_floor(int32 &result, int32 *params) { return (MS->fn_is_object_on_our_floor(result, params)); }

mcodeFunctionReturnCodes fn_is_object_on_screen(int32 &result, int32 *params) { return (MS->fn_is_object_on_screen(result, params)); }

mcodeFunctionReturnCodes fn_is_object_dead(int32 &result, int32 *params) { return (MS->fn_is_object_dead(result, params)); }

mcodeFunctionReturnCodes fn_set_weapon(int32 &result, int32 *params) { return (g_mission->session->fn_set_weapon(result, params)); }

mcodeFunctionReturnCodes fn_set_custom(int32 &result, int32 *params) { return (g_mission->session->fn_set_custom(result, params)); }

mcodeFunctionReturnCodes fn_message(int32 &result, int32 *params) { return (g_mission->session->fn_message(result, params)); }

mcodeFunctionReturnCodes fn_message_var(int32 &result, int32 *params) { return (g_mission->session->fn_message_var(result, params)); }

mcodeFunctionReturnCodes fn_get_state_flag(int32 &result, int32 *params) { return (g_mission->session->fn_get_state_flag(result, params)); }

mcodeFunctionReturnCodes fn_near(int32 &result, int32 *params) { return (g_mission->session->fn_near(result, params)); }

mcodeFunctionReturnCodes fn_teleport_z(int32 &result, int32 *params) { return (MS->fn_teleport_z(result, params)); }

mcodeFunctionReturnCodes fn_on_screen(int32 &result, int32 *params) { return (MS->fn_on_screen(result, params)); }

mcodeFunctionReturnCodes fn_hold_if_off_screen(int32 &result, int32 *params) { return (MS->fn_hold_if_off_screen(result, params)); }

mcodeFunctionReturnCodes fn_object_near_nico(int32 &result, int32 *params) { return (MS->fn_object_near_nico(result, params)); }

mcodeFunctionReturnCodes fn_teleport_y_to_id(int32 &result, int32 *params) { return (MS->fn_teleport_y_to_id(result, params)); }

mcodeFunctionReturnCodes fn_call_socket_id(int32 &result, int32 *params) { return (MS->fn_call_socket_id(result, params)); }

mcodeFunctionReturnCodes fn_lift_process_list(int32 &result, int32 *params) { return (MS->fn_lift_process_list(result, params)); }

mcodeFunctionReturnCodes fn_add_object_id_to_list(int32 &result, int32 *params) { return (MS->fn_add_object_id_to_list(result, params)); }

mcodeFunctionReturnCodes fn_add_object_name_to_list(int32 &result, int32 *params) { return (MS->fn_add_object_name_to_list(result, params)); }

mcodeFunctionReturnCodes fn_hold_while_list_near_nico(int32 &result, int32 *params) { return (MS->fn_hold_while_list_near_nico(result, params)); }

mcodeFunctionReturnCodes fn_set_watch(int32 &result, int32 *params) { return (MS->fn_set_watch(result, params)); }

mcodeFunctionReturnCodes fn_three_sixty_interact(int32 &result, int32 *params) { return (MS->fn_three_sixty_interact(result, params)); }

mcodeFunctionReturnCodes fn_radial_interact(int32 &result, int32 *params) { return (MS->fn_radial_interact(result, params)); }

mcodeFunctionReturnCodes fn_near_list(int32 &result, int32 *params) { return (MS->fn_near_list(result, params)); }

mcodeFunctionReturnCodes fn_get_list_result(int32 &result, int32 *params) { return (MS->fn_get_list_result(result, params)); }

mcodeFunctionReturnCodes fn_random(int32 &result, int32 *params) { return (MS->fn_random(result, params)); }

mcodeFunctionReturnCodes fn_change_session(int32 &result, int32 *params) { return (MS->fn_change_session(result, params)); }

mcodeFunctionReturnCodes fn_changed_sessions(int32 &result, int32 *params) { return (MS->fn_changed_sessions(result, params)); }

mcodeFunctionReturnCodes fn_is_object_adjacent(int32 &result, int32 *params) { return (MS->fn_is_object_adjacent(result, params)); }

mcodeFunctionReturnCodes fn_is_object_on_this_floor(int32 &result, int32 *params) { return (MS->fn_is_object_on_this_floor(result, params)); }

mcodeFunctionReturnCodes fn_get_objects_lvar_value(int32 &result, int32 *params) { return (MS->fn_get_objects_lvar_value(result, params)); }

mcodeFunctionReturnCodes fn_set_objects_lvar_value(int32 &result, int32 *params) { return (MS->fn_set_objects_lvar_value(result, params)); }

mcodeFunctionReturnCodes fn_switch_on_the_really_neat_and_special_script_debugging_facility(int32 &result, int32 *params) {
	return (MS->fn_switch_on_the_really_neat_and_special_script_debugging_facility(result, params));
}

mcodeFunctionReturnCodes fn_switch_off_the_really_neat_and_special_script_debugging_facility(int32 &result, int32 *params) {
	return (MS->fn_switch_off_the_really_neat_and_special_script_debugging_facility(result, params));
}

mcodeFunctionReturnCodes fn_no_logic(int32 &result, int32 *params) { return (MS->fn_no_logic(result, params)); }

mcodeFunctionReturnCodes fn_lift2_process(int32 &result, int32 *params) { return (MS->fn_lift2_process(result, params)); }

mcodeFunctionReturnCodes fn_preload_custom_mega_anim(int32 &result, int32 *params) { return (MS->fn_preload_custom_mega_anim(result, params)); }

mcodeFunctionReturnCodes fn_init_mega_from_nico(int32 &result, int32 *params) { return (MS->fn_init_mega_from_nico(result, params)); }

mcodeFunctionReturnCodes fn_mega_use_lift(int32 &result, int32 *params) { return (MS->fn_mega_use_lift(result, params)); }

mcodeFunctionReturnCodes fn_snap_to_nico_y(int32 &result, int32 *params) { return (MS->fn_snap_to_nico_y(result, params)); }

mcodeFunctionReturnCodes fn_lib_lift_chord_and_chi(int32 &result, int32 *params) { return (MS->fn_lib_lift_chord_and_chi(result, params)); }

mcodeFunctionReturnCodes fn_set_interacting(int32 &result, int32 *params) { return (MS->fn_set_interacting(result, params)); }

mcodeFunctionReturnCodes fn_clear_interacting(int32 &result, int32 *params) { return (MS->fn_clear_interacting(result, params)); }

mcodeFunctionReturnCodes fn_check_for_nico(int32 &result, int32 *params) { return (MS->fn_check_for_nico(result, params)); }

mcodeFunctionReturnCodes fn_set_ids_lvar_value(int32 &result, int32 *params) { return (MS->fn_set_ids_lvar_value(result, params)); }

mcodeFunctionReturnCodes fn_teleport_to_nico_y(int32 &result, int32 *params) { return (MS->fn_teleport_to_nico_y(result, params)); }

mcodeFunctionReturnCodes fn_set_evil(int32 &result, int32 *params) { return (MS->fn_set_evil(result, params)); }

mcodeFunctionReturnCodes fn_changed_via_this_shaft(int32 &result, int32 *params) { return (MS->fn_changed_via_this_shaft(result, params)); }

mcodeFunctionReturnCodes fn_get_objects_x(int32 &result, int32 *params) { return (MS->fn_get_objects_x(result, params)); }

mcodeFunctionReturnCodes fn_get_objects_y(int32 &result, int32 *params) { return (MS->fn_get_objects_y(result, params)); }

mcodeFunctionReturnCodes fn_get_objects_z(int32 &result, int32 *params) { return (MS->fn_get_objects_z(result, params)); }

mcodeFunctionReturnCodes fn_are_we_on_screen(int32 &result, int32 *params) { return (MS->fn_are_we_on_screen(result, params)); }

mcodeFunctionReturnCodes fn_is_mega_within_area(int32 &result, int32 *params) { return (MS->fn_is_mega_within_area(result, params)); }

mcodeFunctionReturnCodes fn_end_mission(int32 &result, int32 *params) { return (MS->fn_end_mission(result, params)); }

mcodeFunctionReturnCodes fn_set_pose(int32 &result, int32 *params) { return (MS->fn_set_pose(result, params)); }

mcodeFunctionReturnCodes fn_is_crouching(int32 &result, int32 *params) { return (MS->fn_is_crouching(result, params)); }

mcodeFunctionReturnCodes fn_is_armed(int32 &result, int32 *params) { return (MS->fn_is_armed(result, params)); }

mcodeFunctionReturnCodes fn_am_i_player(int32 &result, int32 *params) { return (MS->fn_am_i_player(result, params)); }

mcodeFunctionReturnCodes fn_start_conveyor(int32 &result, int32 *params) { return (MS->fn_start_conveyor(result, params)); }

mcodeFunctionReturnCodes fn_stop_conveyor(int32 &result, int32 *params) { return (MS->fn_stop_conveyor(result, params)); }

mcodeFunctionReturnCodes fn_register_stairway(int32 &result, int32 *params) { return (MS->fn_register_stairway(result, params)); }

mcodeFunctionReturnCodes fn_set_object_type(int32 &result, int32 *params) { return (MS->fn_set_object_type(result, params)); }

mcodeFunctionReturnCodes fn_register_ladder(int32 &result, int32 *params) { return (MS->fn_register_ladder(result, params)); }

mcodeFunctionReturnCodes fn_is_an_object_crouching(int32 &result, int32 *params) { return (MS->fn_is_an_object_crouching(result, params)); }

mcodeFunctionReturnCodes fn_align_with_floor(int32 &result, int32 *params) { return (MS->fn_align_with_floor(result, params)); }

mcodeFunctionReturnCodes fn_load_players_gun(int32 &result, int32 *params) { return (MS->fn_load_players_gun(result, params)); }

mcodeFunctionReturnCodes fn_flash_health(int32 &result, int32 *params) { return (MS->fn_flash_health(result, params)); }

mcodeFunctionReturnCodes fn_set_player_pose(int32 &result, int32 *params) { return (MS->fn_set_player_pose(result, params)); }

mcodeFunctionReturnCodes fn_set_anim_speed(int32 &result, int32 *params) { return (MS->fn_set_anim_speed(result, params)); }

mcodeFunctionReturnCodes fn_push_coords(int32 &result, int32 *params) { return (MS->fn_push_coords(result, params)); }

mcodeFunctionReturnCodes fn_pop_coords(int32 &result, int32 *params) { return (MS->fn_pop_coords(result, params)); }

mcodeFunctionReturnCodes fn_set_texture(int32 &result, int32 *params) { return (MS->fn_set_texture(result, params)); }

mcodeFunctionReturnCodes fn_set_palette(int32 &result, int32 *params) { return (MS->fn_set_palette(result, params)); }

mcodeFunctionReturnCodes fn_restart_gamescript(int32 &result, int32 *params) { return (MS->fn_restart_gamescript(result, params)); }

mcodeFunctionReturnCodes fn_quick_restart(int32 &result, int32 *params) { return (MS->fn_quick_restart(result, params)); }

mcodeFunctionReturnCodes fn_is_mega_near_mega(int32 &result, int32 *params) { return (MS->fn_is_mega_near_mega(result, params)); }

mcodeFunctionReturnCodes fn_make_remora_beep(int32 &result, int32 *params) { return (MS->fn_make_remora_beep(result, params)); }

mcodeFunctionReturnCodes fn_shadows_on(int32 &result, int32 *params) { return (MS->fn_shadows_on(result, params)); }

mcodeFunctionReturnCodes fn_shadows_off(int32 &result, int32 *params) { return (MS->fn_shadows_off(result, params)); }

mcodeFunctionReturnCodes fn_panless_teleport_to_nico(int32 &result, int32 *params) { return (MS->fn_panless_teleport_to_nico(result, params)); }

mcodeFunctionReturnCodes fn_can_mega_see_dead_megas(int32 &result, int32 *params) { return (MS->fn_can_mega_see_dead_megas(result, params)); }

mcodeFunctionReturnCodes fn_set_shade_percentage(int32 &result, int32 *params) { return (MS->fn_set_shade_percentage(result, params)); }

mcodeFunctionReturnCodes fn_do_not_disturb(int32 &result, int32 *params) { return (MS->fn_do_not_disturb(result, params)); }

mcodeFunctionReturnCodes fn_has_mega_our_height(int32 &result, int32 *params) { return (MS->fn_has_mega_our_height(result, params)); }

mcodeFunctionReturnCodes fn_register_platform_coords(int32 &result, int32 *params) { return (MS->fn_register_platform_coords(result, params)); }

mcodeFunctionReturnCodes fn_activate_stair_or_ladder(int32 &result, int32 *params) { return (MS->fn_activate_stair_or_ladder(result, params)); }

mcodeFunctionReturnCodes fn_deactivate_stair_or_ladder(int32 &result, int32 *params) { return (MS->fn_deactivate_stair_or_ladder(result, params)); }

mcodeFunctionReturnCodes fn_set_half_character_width(int32 &result, int32 *params) { return (MS->fn_set_half_character_width(result, params)); }

mcodeFunctionReturnCodes fn_set_interact_look_height(int32 &result, int32 *params) { return (MS->fn_set_interact_look_height(result, params)); }

mcodeFunctionReturnCodes fn_set_visible(int32 &result, int32 *params) { return (MS->fn_set_visible(result, params)); }

mcodeFunctionReturnCodes fn_set_object_visible(int32 &result, int32 *params) { return (MS->fn_set_object_visible(result, params)); }

mcodeFunctionReturnCodes fn_set_to_dead(int32 &result, int32 *params) { return (MS->fn_set_to_dead(result, params)); }

mcodeFunctionReturnCodes fn_set_camera_hold(int32 &result, int32 *params) { return (MS->fn_set_camera_hold(result, params)); }

mcodeFunctionReturnCodes fn_set_mega_wait_for_player(int32 &result, int32 *params) { return (MS->fn_set_mega_wait_for_player(result, params)); }

mcodeFunctionReturnCodes fn_set_mega_off_camera_hold(int32 &result, int32 *params) { return (MS->fn_set_mega_off_camera_hold(result, params)); }

mcodeFunctionReturnCodes fn_set_mega_slice_hold(int32 &result, int32 *params) { return (MS->fn_set_mega_slice_hold(result, params)); }

mcodeFunctionReturnCodes fn_set_mesh(int32 &result, int32 *params) { return (MS->fn_set_mesh(result, params)); }

mcodeFunctionReturnCodes fn_prop_crouch_interact(int32 &result, int32 *params) { return (MS->fn_prop_crouch_interact(result, params)); }

mcodeFunctionReturnCodes fn_set_sleep(int32 &result, int32 *params) { return (MS->fn_set_sleep(result, params)); }

mcodeFunctionReturnCodes fn_wait_for_button(int32 &result, int32 *params) { return (MS->fn_wait_for_button(result, params)); }

mcodeFunctionReturnCodes fn_trace(int32 &result, int32 *params) { return (MS->fn_trace(result, params)); }

mcodeFunctionReturnCodes fn_lock_y(int32 &result, int32 *params) { return (MS->fn_lock_y(result, params)); }

mcodeFunctionReturnCodes fn_unlock_y(int32 &result, int32 *params) { return (MS->fn_unlock_y(result, params)); }

mcodeFunctionReturnCodes fn_flip_pan(int32 &result, int32 *params) { return (MS->fn_flip_pan(result, params)); }

mcodeFunctionReturnCodes fn_snap_to_ladder_bottom(int32 &result, int32 *params) { return (MS->fn_snap_to_ladder_bottom(result, params)); }

mcodeFunctionReturnCodes fn_snap_to_ladder_top(int32 &result, int32 *params) { return (MS->fn_snap_to_ladder_top(result, params)); }

mcodeFunctionReturnCodes fn_set_manual_interact_object(int32 &result, int32 *params) { return (MS->fn_set_manual_interact_object(result, params)); }

mcodeFunctionReturnCodes fn_cancel_manual_interact_object(int32 &result, int32 *params) { return (MS->fn_cancel_manual_interact_object(result, params)); }

mcodeFunctionReturnCodes fn_display_objects_lvar(int32 &result, int32 *params) { return (MS->fn_display_objects_lvar(result, params)); }

mcodeFunctionReturnCodes fn_set_override_pose(int32 &result, int32 *params) { return (MS->fn_set_override_pose(result, params)); }

mcodeFunctionReturnCodes fn_cancel_override_pose(int32 &result, int32 *params) { return (MS->fn_cancel_override_pose(result, params)); }

mcodeFunctionReturnCodes fn_preload_actor_file(int32 &result, int32 *params) { return (MS->fn_preload_actor_file(result, params)); }

mcodeFunctionReturnCodes fn_preload_mesh(int32 &result, int32 *params) { return (MS->fn_preload_mesh(result, params)); }

mcodeFunctionReturnCodes fn_hard_load_mesh(int32 &result, int32 *params) { return (MS->fn_hard_load_mesh(result, params)); }

mcodeFunctionReturnCodes fn_preload_texture(int32 &result, int32 *params) { return (MS->fn_preload_texture(result, params)); }

mcodeFunctionReturnCodes fn_preload_palette(int32 &result, int32 *params) { return (MS->fn_preload_palette(result, params)); }

mcodeFunctionReturnCodes fn_preload_animation(int32 &result, int32 *params) { return (MS->fn_preload_animation(result, params)); }

mcodeFunctionReturnCodes fn_PLEASE_REUSE_THIS_SLOT_2(int32 &, int32 *) { return IR_CONT; }

mcodeFunctionReturnCodes fn_PLEASE_REUSE_THIS_SLOT_3(int32 &, int32 *) { return IR_CONT; }

mcodeFunctionReturnCodes fn_swordfight(int32 &result, int32 *params) { return (MS->fn_swordfight(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_swordfight(int32 &, int32 *) { return (IR_CONT); }

mcodeFunctionReturnCodes fn_set_as_player(int32 &result, int32 *params) { return (MS->fn_set_as_player(result, params)); }

mcodeFunctionReturnCodes fn_inventory_active(int32 &result, int32 *params) { return (MS->fn_inventory_active(result, params)); }

mcodeFunctionReturnCodes fn_can_save(int32 &result, int32 *params) { return (MS->fn_can_save(result, params)); }

mcodeFunctionReturnCodes fn_shadow(int32 &result, int32 *params) { return (MS->fn_shadow(result, params)); }

mcodeFunctionReturnCodes fn_is_actor_relative(int32 &result, int32 *params) { return (MS->fn_is_actor_relative(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_rig_test(int32 &, int32 *) {
	//	no params

	if (!MS->prev_save_state)
		return IR_REPEAT;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_test(int32 & /*result*/, int32 * /*params*/) {
#if 0
	char buf[256];

	Common::sprintf_s(buf, "z_%s.txt", object->GetName());

	if (params[0] < 256)
		Tdebug(buf, "%d", params[0]);
	else
		Tdebug(buf, "%s", params[0]);

	result = TRUE8;
#endif
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_create_mega(int32 &, int32 *) {
	Zdebug("FN_create_mega");

	// assign _mega object
	logic_structs[cur_id]->mega = g_megas[num_megas];

	logic_structs[cur_id]->mega->___init();

	// set the view state to off camera for this cycle and last cycle
	logic_structs[cur_id]->mega->viewState = OFF_OFF_CAMERA;

	num_megas++;

	if (num_megas == MAX_voxel_list)
		Fatal_error("fn_create_mega - too many megas!");

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::socket_force_new_logic(int32 &, int32 *params) {
	// force in a new logic script mega who owns the socket script
	// IGNORES if already running

	// params    [0] extension part of script name

	char *ad;
	uint32 script_hash;
	const char *script_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	script_hash = HashString(script_name);

	if (g_px->socket_watch)
		Message_box("socket_force_new_logic - obj %s, script %s", CGameObject::GetName(socket_object), script_name);

	// now try and find a script with the passed extension i.e. ???::looping
	for (uint32 k = 0; k < CGameObject::GetNoScripts(socket_object); k++) {
		// now check for actual script name
		if (script_hash == CGameObject::GetScriptNamePartHash(socket_object, k)) {
			Zdebug("script %d matches", k);
			// script k is the one to run
			// get the address of the script we want to run
			ad = (char *)LinkedDataObject::Try_fetch_item_by_hash(scripts, CGameObject::GetScriptNameFullHash(socket_object, k));

			if (g_px->socket_watch)
				Message_box("replacing logic");

			// write actual offset
			logic_structs[socket_id]->logic[1] = ad;

			// write reference for change script checks later - i.e. FN_context_chosen_script
			logic_structs[socket_id]->logic_ref[1] = ad;

			logic_structs[socket_id]->logic_level = 1; // reset to level 1

			logic_structs[socket_id]->looping = 0; // reset logic

			// if a mega then cancel interacting
			if (logic_structs[socket_id]->image_type == VOXEL)
				logic_structs[socket_id]->mega->interacting = 0;

			logic_structs[socket_id]->do_not_disturb = 3; // object will not be caught by events this cycle - events pending will be cleared
			return (IR_CONT);
		}
	}

	// didn't find the script in the target
	// shut that object down as its full of holes!

	Fatal_error("socket_force_new_logic can't find script - obj %s, script %s", CGameObject::GetName(socket_object), script_name);

	return (IR_TERMINATE);
}

const char *default_palette = "default";
const char *default_texture = "material";
const char *default_mesh = "mesh";

mcodeFunctionReturnCodes _game_session::fn_set_voxel_image_path(int32 &, int32 *params) {
	// set image_type to VOXEL and create the _vox_image object which inits all the animation set filenames and caps
	// we are passed the character name AND the graphic set
	// for example, cord, wetsuit

	//	**this routine inits an object as a voxel object**
	// **more precisely, a mega character\actor with either voxel or polygon image to be sent to stage_draw**

	// enforce this convention
	if (!logic_structs[cur_id]->mega)
		Fatal_error("fn_set_voxel_image_path %s is not a mega", CGameObject::GetName(object));

	const char *param0Str = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *param1Str = (const char *)MemoryUtil::resolvePtr(params[1]);

	// set 'cord'
	Set_string(const_cast<char *>(param0Str), logic_structs[cur_id]->mega->chr_name, MAX_CHAR_NAME_LENGTH);

	// set 'casual_wear'
	Set_string(const_cast<char *>(param1Str), logic_structs[cur_id]->mega->anim_set, MAX_OUTFIT_NAME_LENGTH);

	// create _vox_image object
	if (!logic_structs[cur_id]->voxel_info) {
		// assign a struct
		logic_structs[cur_id]->voxel_info = g_vox_images[num_vox_images];
		num_vox_images++;
	}

	logic_structs[cur_id]->voxel_info->___init(param0Str, param1Str, logic_structs[cur_id]->mega->Fetch_pose());

	// Set the default texture & palette
	logic_structs[cur_id]->voxel_info->Set_palette(default_palette);
	logic_structs[cur_id]->voxel_info->Set_texture(default_texture);
	logic_structs[cur_id]->voxel_info->Set_mesh(default_mesh);

	// set type while here
	logic_structs[cur_id]->image_type = VOXEL;

	// script continues
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_xyz(int32 &, int32 *) {
	// set a game objects x,y,z worldspace coords

	Fatal_error("[%s] fn_set_xyz - not supported", CGameObject::GetName(object));

	return IR_CONT;
}

mcodeFunctionReturnCodes fn_set_pan(int32 &result, int32 *params) {
	// set a game objects pan value
	return (g_mission->session->fn_set_pan(result, params));
}

mcodeFunctionReturnCodes _game_session::fn_set_pan(int32 &, int32 *) {
	//	params[0]    pan value

	Fatal_error("fn_set_pan - not supported");
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_init_from_nico_file(int32 &, int32 * /*params*/) {
	// **this is for props - non mega actors**
	// set the object position and pan
	// if the object does not have a corresponding entry in the positions file then we may ULTIMATELY shut the game object down
	// ** we may need to make this a status option via linc **

	//	no params

	_feature_info *start_pos;

	Zdebug("fn_init_from_nico_file - %s (cur_id %d)", CGameObject::GetName(object), cur_id);

	if (L->image_type == VOXEL)
		Fatal_error("fn_init_from_nico_file called by a mega! [%s] - use fn_init_mega_from_nico", CGameObject::GetName(object));

	// fetch tag file for this item
	start_pos = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, CGameObject::GetName(object));

	if (!start_pos) {
		Message_box("fn_init_from_nico_file - missing nico for item %s", CGameObject::GetName(object));
		Shut_down_object("fn_init_from_nico_file - missing nico for item");
		return IR_STOP;
	}

	// set coordinates
	logic_structs[cur_id]->prop_xyz.x = FROM_LE_FLOAT32(start_pos->x);

	if (FROM_LE_FLOAT32(start_pos->y) < FROM_LE_FLOAT32(start_pos->floor_y)) // nico is under the floor!
		logic_structs[cur_id]->prop_xyz.y = FROM_LE_FLOAT32(start_pos->floor_y);

	else
		logic_structs[cur_id]->prop_xyz.y = FROM_LE_FLOAT32(start_pos->y);

	logic_structs[cur_id]->prop_xyz.z = FROM_LE_FLOAT32(start_pos->z);

	// set pan
	logic_structs[cur_id]->pan = FROM_LE_FLOAT32(start_pos->direction); // is this right?

	logic_structs[cur_id]->prop_interact_pan = FROM_LE_FLOAT32(start_pos->direction); // this might be more sensible

	// set owner floor for things lile fn_on_screen
	logic_structs[cur_id]->owner_floor_rect = floor_def->Return_floor_rect(FROM_LE_FLOAT32(start_pos->x), FROM_LE_FLOAT32(start_pos->z), FROM_LE_FLOAT32(start_pos->floor_y), 0);

	if (logic_structs[cur_id]->owner_floor_rect == PXNULL) {
		Message_box("fn_init_from_nico_file - %s nico not on a legal floor position - object has been shutdown", CGameObject::GetName(object));
		Shut_down_object("fn_init_from_nico_file");
		return IR_STOP;
	}

	// prop has coords
	logic_structs[cur_id]->prop_coords_set = TRUE8;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_check_for_nico(int32 &result, int32 *) {
	// check to see if there is a nico in this objects name

	_feature_info *start_pos;

	start_pos = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, CGameObject::GetName(object));

	if (!start_pos)
		result = FALSE8;
	else
		result = TRUE8;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_init_from_marker_file(int32 &, int32 *) {
	// set the object position and pan from an engine created map-marker file

	// if the object does not have a corresponding entry in the positions file then we may ULTIMATELY shut the game object down
	// ** we may need to make this a status option via linc **

	//	no params

	_map_marker *start_pos;

	Zdebug("fn_init_from_marker_file - %s (cur_id %d)", CGameObject::GetName(object), cur_id);

	// if this object does not have a voxel_info struct then we're done here!
	if (!logic_structs[cur_id]->voxel_info)
		Fatal_error("fn_init_from_marker_file fails because object is not registered as a mega.");

	// fetch tag file for this item
	start_pos = (_map_marker *)markers.Fetch_marker_by_object_name(const_cast<char *>(CGameObject::GetName(object)));

	if (!start_pos) {
		Message_box("fn_init_from_marker_file missing map marker file entry for item %s.  You must edit the markers - don't play the game.", CGameObject::GetName(object));
		Shut_down_object("fn_init_from_marker_file");
		return IR_STOP;
	}

	logic_structs[cur_id]->mega->actor_xyz.x = FROM_LE_FLOAT32(start_pos->x);
	logic_structs[cur_id]->mega->actor_xyz.y = FROM_LE_FLOAT32(start_pos->y);
	logic_structs[cur_id]->mega->actor_xyz.z = FROM_LE_FLOAT32(start_pos->z);

	// set pan
	logic_structs[cur_id]->pan = FROM_LE_FLOAT32(start_pos->pan);

	// has coords
	logic_structs[cur_id]->prop_coords_set = TRUE8;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_init_mega_from_nico(int32 &, int32 *) {
	// set the object position and pan from a MAX NICO marker

	//	no params

	_feature_info *start_pos;

	Zdebug("fn_init_mega_from_nico - %s (cur_id %d)", CGameObject::GetName(object), cur_id);

	// if this object does not have a voxel_info struct then we're done here!
	if (!logic_structs[cur_id]->voxel_info)
		Fatal_error("fn_init_mega_from_nico fails because object is not registered as a mega");

	// fetch tag file for this item
	start_pos = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, CGameObject::GetName(object));

	if (!start_pos) {
		Message_box("fn_init_mega_from_nico missing nico for item %s", CGameObject::GetName(object));
		Shut_down_object("fn_init_mega_from_nico missing nico for item");
		return IR_STOP;
	}

	// set coordinates
	logic_structs[cur_id]->mega->actor_xyz.x = FROM_LE_FLOAT32(start_pos->x);
	logic_structs[cur_id]->mega->actor_xyz.y = FROM_LE_FLOAT32(start_pos->floor_y);
	logic_structs[cur_id]->mega->actor_xyz.z = FROM_LE_FLOAT32(start_pos->z);

	// set pan
	logic_structs[cur_id]->pan = FROM_LE_FLOAT32(start_pos->direction);

	// has coords
	logic_structs[cur_id]->prop_coords_set = TRUE8;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_teleport_to_nico(int32 &, int32 *params) {
	// set the object position and pan from a MAX NICO marker

	//	params 0     name of nico

	_feature_info *start_pos;

	const char *nico_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("fn_teleport_to_nico - %s (to %s)", CGameObject::GetName(object), nico_name);

	// if this object does not have a voxel_info struct then we're done here!
	if (!logic_structs[cur_id]->voxel_info)
		Fatal_error("fn_teleport_to_nico fails because object is not registered as a mega");

	// fetch tag file for this item
	start_pos = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, nico_name);
	if (!start_pos)
		Fatal_error("no NICO marker (fn_teleport_to_nico) ob %s, nico %s", CGameObject::GetName(object), nico_name);

	// set coordinates
	logic_structs[cur_id]->mega->actor_xyz.x = FROM_LE_FLOAT32(start_pos->x);
	logic_structs[cur_id]->mega->actor_xyz.y = FROM_LE_FLOAT32(start_pos->floor_y); // Gravitise_y(start_pos->y);
	logic_structs[cur_id]->mega->actor_xyz.z = FROM_LE_FLOAT32(start_pos->z);

	// set pan
	logic_structs[cur_id]->pan = FROM_LE_FLOAT32(start_pos->direction);

	// for safety
	logic_structs[cur_id]->cur_anim_type = __STAND;
	logic_structs[cur_id]->anim_pc = 0;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_panless_teleport_to_nico(int32 &, int32 *params) {
	// set the object position from a MAX NICO marker

	//	params 0     name of nico

	_feature_info *start_pos;

	const char *nico_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("fn_panless_teleport_to_nico");

	// if this object does not have a voxel_info struct then we're done here!
	if (!logic_structs[cur_id]->voxel_info)
		Fatal_error("fn_panless_teleport_to_nico_ fails because object is not registered as a mega");

	// fetch tag file for this item
	start_pos = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, nico_name);
	if (!start_pos)
		Fatal_error("no NICO marker (fn_panless_teleport_to_nico_) ob %s, nico %s", CGameObject::GetName(object), nico_name);

	// set coordinates
	logic_structs[cur_id]->mega->actor_xyz.x = FROM_LE_FLOAT32(start_pos->x);
	logic_structs[cur_id]->mega->actor_xyz.y = FROM_LE_FLOAT32(start_pos->floor_y); // Gravitise_y(start_pos->y);
	logic_structs[cur_id]->mega->actor_xyz.z = FROM_LE_FLOAT32(start_pos->z);

	// for safety
	logic_structs[cur_id]->cur_anim_type = __STAND;
	logic_structs[cur_id]->anim_pc = 0;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_teleport_to_nico_y(int32 &, int32 *params) {
	// get y coord from a nico

	//	params 0     name of nico

	_feature_info *start_pos;

	const char *nico_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("fn_teleport_to_nico_y - %s (to %s)", CGameObject::GetName(object), nico_name);

	// if this object does not have a voxel_info struct then we're done here!
	if (!logic_structs[cur_id]->voxel_info)
		Fatal_error("fn_teleport_to_nico_y fails because object is not registered as a mega");

	// fetch tag file for this item
	start_pos = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, nico_name);
	if (!start_pos)
		Fatal_error("no NICO marker (fn_teleport_to_nico_y) ob %s, nico %s", CGameObject::GetName(object), nico_name);

	// set coordinates
	logic_structs[cur_id]->mega->actor_xyz.y = FROM_LE_FLOAT32(start_pos->floor_y); // Gravitise_y(start_pos->y);

	// for safety
	logic_structs[cur_id]->cur_anim_type = __STAND;
	logic_structs[cur_id]->anim_pc = 0;

	if (cur_id == player.Fetch_player_id())
		Prepare_megas_route_barriers(TRUE8); // update barriers

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_snap_to_nico_y(int32 &, int32 *params) {
	// get y from nico

	//	params 0     name of nico

	_feature_info *start_pos;

	const char *nico_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("fn_snap_to_nico_y - %s (to %s)", CGameObject::GetName(object), nico_name);

	// if this object does not have a voxel_info struct then we're done here!
	if (!logic_structs[cur_id]->voxel_info)
		Fatal_error("fn_snap_to_nico_y fails because object is not registered as a mega");

	// fetch tag file for this item
	start_pos = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, nico_name);
	if (!start_pos)
		Fatal_error("no NICO marker (fn_snap_to_nico_y) ob %s, nico %s", CGameObject::GetName(object), nico_name);

	// set coordinates
	logic_structs[cur_id]->mega->actor_xyz.y = FROM_LE_FLOAT32(start_pos->floor_y);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_get_pan_from_nico(int32 &, int32 *params) {
	// set the object pan from a MAX NICO marker
	// prop or mega

	//	params 0     name of nico

	_feature_info *start_pos;

	const char *nico_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("fn_get_pan_from_nico - %s (nico %s)", CGameObject::GetName(object), nico_name);

	// fetch tag file for this item
	start_pos = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, nico_name);

	if (!start_pos) {
		// item does not have an entry but clearly expects one
		// for now, just carry on and log a warning
		Zdebug("WARNING missing feature file entry for item %s", CGameObject::GetName(object));
		Fatal_error("no NICO marker (fn_get_pan_from_nico) %s", CGameObject::GetName(object));
	}

	// set pan
	logic_structs[cur_id]->pan = FROM_LE_FLOAT32(start_pos->direction);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_player_can_interact(int32 &, int32 *) {
	Zdebug("set interact");

	L->player_can_interact = TRUE8;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_player_cannot_interact(int32 &, int32 *) {
	Zdebug("stop interact");

	L->player_can_interact = FALSE8;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_call_socket(int32 &result, int32 *params) {
	// call a script of another object
	// the script must be of a run-once and terminate nature

	//	params   0       ascii name of target object
	//				1     ascii name of socket script

	int32 retval;
	uint32 script_hash;

	const char *target_object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *socket_script_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	Zdebug("fn_call_socket - obj %s, script %s", target_object_name, socket_script_name);

	if (g_px->socket_watch)
		Message_box("%s fn_call_socket - obj %s, script %s", CGameObject::GetName(object), target_object_name, socket_script_name);

	script_hash = HashString(socket_script_name);

	// get target object
	socket_object = (CGame *)LinkedDataObject::Try_fetch_item_by_name(MS->objects, target_object_name);
	if (!socket_object)
		Fatal_error("%s call to fn_call_socket - object %s doesn't exist", CGameObject::GetName(object), target_object_name);

	// set socket_id ready for any special socket functions
	socket_id = LinkedDataObject::Fetch_item_number_by_name(MS->objects, target_object_name);
	if (socket_id == 0xffffffff)
		Fatal_error("fn_call_socket couldnt find object [%s]", target_object_name);

	// now try and find a script with the passed extension i.e. ???::looping
	for (uint32 k = 0; k < CGameObject::GetNoScripts(socket_object); k++) {
		// now check for actual script name
		if (script_hash == CGameObject::GetScriptNamePartHash(socket_object, k)) {
			Zdebug("calling socket %d", k);
			// script k is the one to run
			// get the address of the script we want to run

			const char *pc = (const char *)LinkedDataObject::Try_fetch_item_by_hash(scripts, CGameObject::GetScriptNameFullHash(socket_object, k));

			// run the script - pass its object so vars can be accessed
			RunScript(pc, socket_object, &retval);

			Zdebug("return val = %d", retval);

			result = retval; // pass return value of socket call into result flag

			return (IR_CONT);
		}
	}

	Fatal_error("fn_call_socket couldnt find script %s", socket_script_name);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_call_socket_id(int32 &result, int32 *params) {
	// call a script of another object
	// the script must be of a run-once and terminate nature

	//	params   0       ID of target object
	//				1     ascii name of socket script

	int32 ret;

	Zdebug("fn_call_socket_id id=%d", params[0]);

	const char *socket_script_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	Call_socket(params[0], socket_script_name, &ret);

	result = ret;

	return (IR_CONT);
}

bool8 _game_session::Call_socket(uint32 id, const char *script, int32 *retval) {
	// call a script of an object
	// the script must be of a run-once and terminate nature
	// this routine kept in here as its closely related to fn-call-socket
	// we pass id and script name

	// engine uses this

	uint32 script_hash;

	script_hash = HashString(script);

	// get target object
	socket_object = (CGame *)LinkedDataObject::Fetch_item_by_number(MS->objects, id);
	if (!socket_object)
		Fatal_error("internal Call_socket - named object don't exist");

	Zdebug("\nCall_socket - obj %s, script %s", CGameObject::GetName(socket_object), script);

	// set this for socket fn_ functions
	socket_id = id;

	// now try and find a script with the passed extension i.e. ???::looping
	for (uint32 k = 0; k < CGameObject::GetNoScripts(socket_object); k++) {
		// skip past the object:: aspect

		// now check for actual script name
		if (script_hash == CGameObject::GetScriptNamePartHash(socket_object, k)) {
			Zdebug("calling socket %d", k);
			// script k is the one to run
			// get the address of the script we want to run
			const char *pc = (const char *)LinkedDataObject::Try_fetch_item_by_hash(scripts, CGameObject::GetScriptNameFullHash(socket_object, k));

			int32 result = static_cast<int>(*retval);

			// run the script - pass its object so vars can be accessed
			RunScript(pc, socket_object, &result);

			*retval = result;

			return (TRUE8);
		}
	}

	Tdebug("Call_socket_fails.txt", "[%s] couldnt find script [%s] in [%s]", CGameObject::GetName(object), script, CGameObject::GetName(socket_object));

	return (FALSE8);
}

mcodeFunctionReturnCodes _game_session::fn_prop_near_a_mega(int32 &result, int32 *params) {
	// check all live megas against the coordinate of this prop for rough distance of param passed

	//	params   0 address of result variable
	//				1 distance away value

	uint32 j;

	// run through all the objects calling their logic
	for (j = 0; j < total_objects; j++) { // object 0 is used
		// object must be alive and interactable and a mega
		if ((logic_structs[j]->image_type == VOXEL) && (logic_structs[j]->ob_status != OB_STATUS_HELD)) { // not if the object has been manually switched out
			if (PXfabs(L->prop_xyz.y - logic_structs[j]->mega->actor_xyz.y) < (200 * REAL_ONE)) {     // slack for height calc
				if ((PXfabs(L->prop_xyz.x - logic_structs[j]->mega->actor_xyz.x) < (PXreal)params[0]) &&
				    (PXfabs(L->prop_xyz.z - logic_structs[j]->mega->actor_xyz.z) < (PXreal)params[0])) {
					// yes
					result = TRUE8;
					return (IR_CONT);
				}
			}
		}
	}

	// no
	result = FALSE8;
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_reset_player(int32 &, int32 *) {
	// reset the player to standing - useful for after he's been interupted - perhaps after being shot...

	player.Reset_player();

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_teleport(int32 &, int32 *params) {
	// move the player to another objects coordinate

	// params        0 name of target object
	//				1 xoff
	//				2 zoff

	const char *target_object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("\nfn_teleport to %s x%d z%d", target_object_name, params[1], params[2]);
	Zdebug("cur_id %d [%s]", cur_id, CGameObject::GetName(object));

	// Made this so it takes a special name "from_origin" to indicate that the offset is to be applied
	// from 0,0.
	if (strcmp(target_object_name, "from_origin") != 0) {
		uint32 tar = LinkedDataObject::Fetch_item_number_by_name(MS->objects, target_object_name);

		if (tar == 0xffffffff)
			Fatal_error("'destination' teleport object [%s] does not exist", target_object_name);

		if (!logic_structs[tar]->prop_coords_set)
			Fatal_error("fn_teleport by [%s] finds object [%s] is not yet initialised :O - i.e. its not run its init script yet", CGameObject::GetName(object),
			            target_object_name);

		if (logic_structs[tar]->image_type == PROP) {
			Tdebug("teleport.txt", "target prop y=%3.1f - our y=%3.1f", logic_structs[tar]->prop_xyz.y, logic_structs[cur_id]->mega->actor_xyz.y);
			Zdebug("y=%3.1f , grav y =%3.1f", logic_structs[tar]->prop_xyz.y, floor_def->Gravitise_y(logic_structs[tar]->prop_xyz.y));
			logic_structs[cur_id]->mega->actor_xyz.x = logic_structs[tar]->prop_xyz.x;
			logic_structs[cur_id]->mega->actor_xyz.y = floor_def->Gravitise_y(logic_structs[tar]->prop_xyz.y); // logic_structs[tar]->prop_xyz.y;
			logic_structs[cur_id]->mega->actor_xyz.z = logic_structs[tar]->prop_xyz.z;
		} else { // mega
			logic_structs[cur_id]->mega->actor_xyz.x = logic_structs[tar]->mega->actor_xyz.x;
			logic_structs[cur_id]->mega->actor_xyz.y = logic_structs[tar]->mega->actor_xyz.y;
			logic_structs[cur_id]->mega->actor_xyz.z = logic_structs[tar]->mega->actor_xyz.z;
		}

		// add offset
		logic_structs[cur_id]->mega->actor_xyz.x += (PXfloat)params[1];
		logic_structs[cur_id]->mega->actor_xyz.z += (PXfloat)params[2];
	} else {
		// Absolute jump has been requested, so just set the new x,z for the object to the given values.
		logic_structs[cur_id]->mega->actor_xyz.x = (PXfloat)params[1];
		logic_structs[cur_id]->mega->actor_xyz.z = (PXfloat)params[2];
	}

	// set floor rect value - used by stage draw to find indexed camera name
	// gotta keep this bang up to date for player history system
	floor_def->Set_floor_rect_flag(L);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_teleport_z(int32 &, int32 *params) {
	// move the player to another objects Y coordinate

	// params        0 name of target object

	const char *target_object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("fn_teleport_z to %s", target_object_name);

	uint32 tar = LinkedDataObject::Fetch_item_number_by_name(MS->objects, target_object_name);

	if (tar == 0xffffffff)
		Fatal_error("'destination' teleport object [%s] does not exist", target_object_name);
	if (!logic_structs[tar]->prop_coords_set)
		Fatal_error("fn_teleport by [%s] finds object [%s] is not yet initialised :O - i.e. its not run its init script yet", CGameObject::GetName(object), target_object_name);

	if (logic_structs[tar]->image_type == PROP) {
		logic_structs[cur_id]->mega->actor_xyz.y = floor_def->Gravitise_y(logic_structs[tar]->prop_xyz.y); // logic_structs[tar]->prop_xyz.y;
	} else {                                                                                                   // mega
		logic_structs[cur_id]->mega->actor_xyz.y = logic_structs[tar]->mega->actor_xyz.y;
	}

	// set floor rect value - used by stage draw to find indexed camera name
	// gotta keep this bang up to date for player history system
	floor_def->Set_floor_rect_flag(L);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_teleport_y_to_id(int32 &, int32 *params) {
	// move the player to another objects Y coordinate

	// params        0 ID of target object

	Zdebug("fn_teleport_y_to_id to %d", params[0]);

	assert((uint32)params[0] < total_objects);

	if (logic_structs[params[0]]->image_type == PROP) {
		logic_structs[cur_id]->mega->actor_xyz.y = floor_def->Gravitise_y(logic_structs[params[0]]->prop_xyz.y); // logic_structs[tar]->prop_xyz.y;
	} else {                                                                                                         // mega
		logic_structs[cur_id]->mega->actor_xyz.y = logic_structs[params[0]]->mega->actor_xyz.y;
	}

	// set floor rect value - used by stage draw to find indexed camera name
	// gotta keep this bang up to date for player history system
	floor_def->Set_floor_rect_flag(L);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_are_we_on_this_floor(int32 &result, int32 *params) {
	// check to see if object is on the floor passed

	// params        0 name of floor

	uint32 floor_id;

	result = FALSE8;

	const char *floor_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (first_session_cycle)
		return IR_CONT;

	uint32 hash = HashString(floor_name);
	floor_id = LinkedDataObject::Fetch_item_number_by_hash(floor_def->floors, hash);

	if (floor_id == 0xffffffff)
		Fatal_error("fn_are_we_on_this_floor can't locate floor [%s]", floor_name);

	if (floor_id == L->owner_floor_rect)
		result = TRUE8;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_is_object_on_our_floor(int32 &result, int32 *params) {
	// check to see if object is on the floor passed

	// params        0 name of object

	uint32 id, num_extra, j, cam;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	result = FALSE8;

	if (first_session_cycle)
		return IR_CONT;

	id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_is_object_on_our_floor - illegal object [%s]", object_name);

	// same camera means same floor
	if (floor_to_camera_index[L->owner_floor_rect] == floor_to_camera_index[logic_structs[id]->owner_floor_rect]) {
		result = TRUE8;
	} else {
		// ok, but is our floor linked to theirs?
		cam = floor_to_camera_index[L->owner_floor_rect]; // the camera associated with calling objects (my) floor
		num_extra = cam_floor_list[cam].num_extra_floors;

		for (j = 0; j < num_extra; j++) {
			if (cam_floor_list[cam].extra_floors[j] == logic_structs[id]->owner_floor_rect) {
				result = TRUE8; // yes - the floors are linked
				return IR_CONT;
			}
		}
	}

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_can_mega_see_dead_megas(int32 &result, int32 *) {
	// are there dead megas on this megas floor?

	uint32 j, cam, num_extra;

	for (j = 0; j < number_of_voxel_ids; j++) {
		if (cur_id != voxel_id_list[j]) {
			if ((logic_structs[voxel_id_list[j]]->mega->dead) &&                  // dead
			    (logic_structs[voxel_id_list[j]]->ob_status != OB_STATUS_HELD)) { // not held
				// found a dead mega who isn't us - is it on our floor?

				// same camera means same floor
				if (floor_to_camera_index[L->owner_floor_rect] == floor_to_camera_index[logic_structs[voxel_id_list[j]]->owner_floor_rect]) {
					result = TRUE8; //
					return IR_CONT; // cor, found one
				}

				// otherwise do a check to see if our camera is linked to others floor
				cam = floor_to_camera_index[L->owner_floor_rect];
				num_extra = cam_floor_list[cam].num_extra_floors;

				for (uint32 k = 0; k < num_extra; k++) {
					if (cam_floor_list[cam].extra_floors[k] == logic_structs[voxel_id_list[j]]->owner_floor_rect) {
						result = TRUE8; // yes - the floors are linked
						return IR_CONT;
					}
				}
			}
		}
	}

	result = FALSE8;
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_is_object_on_screen(int32 &result, int32 *params) {
	// check to see if object is in current camera space - i.e. will be drawn by actor-draw

	// params        0 name of object

	uint32 id;
	PXvector pos;
	bool8 resu = FALSE8;
	PXvector filmpos;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("fn_is_object_on_screen [%s]", object_name);

	if (!SetOK()) {
		result = FALSE8; // no camera as yet
		return (IR_CONT);
	}

	// get object to check
	id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_is_object_on_screen - illegal object [%s]", object_name);

	Zdebug(" id = %d", id);

	// mega or prop
	if (logic_structs[id]->image_type == PROP) {
		Zdebug(" prop");
		pos.x = logic_structs[id]->prop_xyz.x;
		pos.y = logic_structs[id]->prop_xyz.y; // talks over head rather than from the feet
		pos.z = logic_structs[id]->prop_xyz.z;
	} else {
		Zdebug(" mega");
		pos.x = logic_structs[id]->mega->actor_xyz.x;
		pos.y = logic_structs[id]->mega->actor_xyz.y; // talks over head rather than from the feet
		pos.z = logic_structs[id]->mega->actor_xyz.z;
	}

	// setup camera
	PXcamera &camera = GetCamera();

	// compute screen coord
	Zdebug(" PXWorldToFilm");
	PXWorldToFilm(pos, camera, resu, filmpos);
	result = (int32)resu;
	Zdebug(" ~PXWorldToFilm");

	// Ignore actors who are on the wrong side of the hither plane
	if (filmpos.z > -g_actor_hither_plane)
		result = 0;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_are_we_on_screen(int32 &result, int32 *) {
	// Similar to the above function but does the check just for the object calling the function, which
	// must be a mega character.

	// No params.

	// Write the call in the debug file.
	Zdebug("fn_are_we_on_screen() - object id = %d", cur_id);

	// Check if we have a camera set up.
	if (!SetOK()) {
		result = FALSE8;
		return (IR_CONT);
	}

	// Caller must be a mega.  (We can change this if we find we need to.)
	if (logic_structs[cur_id]->image_type != VOXEL)
		Fatal_error("Non mega object (id=%d) called fn_are_we_on_screen()", cur_id);

	result = 0;

	if (Object_visible_to_camera(cur_id)) {
		bool8 aresult = TRUE8;
		PXvector filmPosition;
		PXWorldToFilm(M->actor_xyz, set.GetCamera(), aresult, filmPosition);

		if (filmPosition.z < -g_actor_hither_plane) {
			VECTOR v;
			v.vx = (int32)M->actor_xyz.x;
			v.vy = (int32)M->actor_xyz.y;
			v.vz = (int32)M->actor_xyz.z;

			SVECTOR orient;
			orient.vx = 0;
			orient.vy = 0;
			orient.vz = 0;

			// finally if this is true then we are okay so on_screen is true
			if (QuickActorCull((psxCamera *)&(set.GetCamera()), &v, &orient) != 1)
				result = 1;
		}
	}

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_get_objects_lvar_value(int32 &result, int32 *params) {
	// params        0 name of object
	//				1 name of lvar

	int32 ret;
	CGame *ob;
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *lvar_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	Zdebug("fn_get_objects_lvar_value - [%s] [%s]", object_name, lvar_name);

	uint32 hash = HashString(object_name);
	ob = (CGame *)LinkedDataObject::Try_fetch_item_by_hash(objects, hash);
	if (!ob)
		Fatal_error("fn_get_objects_lvar_value - illegal object [%s]", object_name);

	ret = CGameObject::GetVariable(ob, lvar_name);
	if (ret == -1)
		Fatal_error("%s finds fn_get_objects_lvar_value - target object [%s] doesn't have [%s] lvar", CGameObject::GetName(object), object_name, lvar_name);

	result = CGameObject::GetIntegerVariable(ob, ret);

	Zdebug(" var==%d", result);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_objects_lvar_value(int32 &, int32 *params) {
	// params        0 name of object
	//				1 name of lvar
	//				2 new value of lvar

	int32 var_num;
	CGame *ob;
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *lvar_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	Zdebug("[%s] calls fn_set_objects_lvar_value - [%s] [%s, %d]", CGameObject::GetName(object), object_name, lvar_name, params[2]);

	ob = (CGame *)LinkedDataObject::Fetch_item_by_name(objects, object_name);
	if (!ob)
		Fatal_error("fn_set_objects_lvar_value - illegal object [%s]", object_name);

	var_num = CGameObject::GetVariable(ob, lvar_name);
	if (var_num == -1)
		Fatal_error("[%s] fn_set_objects_lvar_value - object [%s] doesn't have [%s] lvar", CGameObject::GetName(object), object_name, lvar_name);

	CGameObject::SetIntegerVariable(ob, var_num, params[2]);

	Zdebug(" var==%d", params[2]);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_ids_lvar_value(int32 &, int32 *params) {
	// params        0 id of object
	//				1 name of lvar
	//				2 new value of lvar

	int32 var_num;
	CGame *ob;
	const char *lvar_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	Zdebug("fn_set_ids_lvar_value - [%s] [%s]", LinkedDataObject::Fetch_items_name_by_number(objects, params[0]), lvar_name);

	ob = (CGame *)LinkedDataObject::Fetch_item_by_number(objects, params[0]);
	if (!ob)
		Fatal_error("fn_set_ids_lvar_value - illegal object [%d]", params[0]);

	var_num = CGameObject::GetVariable(ob, lvar_name);
	if (var_num == -1)
		Fatal_error("fn_set_ids_lvar_value - object [%d] doesn't have [%s] lvar", params[0], lvar_name);

	CGameObject::SetIntegerVariable(ob, var_num, params[2]);

	Zdebug(" var==%d", params[2]);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_get_state_flag(int32 &result, int32 *params) {
	// params        0 name of object

	int32 ret;
	CGame *ob;
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	ob = (CGame *)LinkedDataObject::Fetch_item_by_name(objects, object_name);
	if (!ob)
		Fatal_error("fn_get_state_flag - illegal object [%s]", object_name);
	ret = CGameObject::GetVariable(ob, "state");
	if (ret == -1)
		Fatal_error("fn_get_state_flag - object [%s] doesn't have 'state' lvar", object_name);
	result = CGameObject::GetIntegerVariable(ob, ret);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_is_object_dead(int32 &result, int32 *params) {
	// params        0 name of object

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	uint32 id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);

	if (!logic_structs[id]->mega)
		Fatal_error("fn_get_state_flag - object [%s] not mega", object_name);

	result = logic_structs[id]->mega->dead;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_weapon(int32 &, int32 *params) {
	// change/set the weapon type
	// if the new one is not the current then an anim must be played...

	//	params   0 ascii name of weapon - should be compatible with weapon_text

	const char *weapon_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// search the weapon set table

	uint32 j;

	Zdebug("fn-set-weapon [%s]", weapon_name);

	for (j = 0; j < __TOTAL_WEAPONS; j++) {
		Zdebug("test [%s]", weapon_text[j]);
		if (!strcmp(weapon_name, weapon_text[j])) {
			Zdebug("found %d", j);
			L->mega->weapon = (__weapon)j;
			return (IR_CONT);
		}
	}

	Fatal_error("WARNING -  %s specified weapon does not exist [%s]", CGameObject::GetName(object), weapon_name);

	return (IR_STOP);
}

mcodeFunctionReturnCodes _game_session::fn_is_crouching(int32 &result, int32 *) {
	// are we crouching yes or no

	result = M->Is_crouched();

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_is_an_object_crouching(int32 &result, int32 *params) {
	// is an object crouching yes or no

	// params    0   name

	uint32 id;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_is_an_object_crouching - illegal object [%s]", object_name);

	// Make sure it is a mega.
	if (!logic_structs[id]->mega)
		Fatal_error("fn_is_an_object_crouching - object [%s] not a mega", object_name);

	result = logic_structs[id]->mega->Is_crouched();

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_is_armed(int32 &result, int32 *) {
	// are we armed yes or no

	result = Fetch_cur_megas_armed_status();

	return IR_CONT;
}

extern _player_stat player_stat_table[__TOTAL_WEAPONS];
mcodeFunctionReturnCodes _game_session::fn_set_pose(int32 &, int32 *params) {
	// change/set the weapon type
	// set instantly - i.e. init the _vox_image

	//	params   0 ascii name of pose - should be compatible with weapon_text

	// search the weapon set table

	uint32 j;
	const char *pose_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("fn-set-weapon [%s]", pose_name);

	for (j = 0; j < __TOTAL_WEAPONS; j++) {
		Zdebug("test [%s]", weapon_text[j]);
		if (!strcmp(pose_name, weapon_text[j])) {
			Zdebug("found %d", j);
			L->mega->weapon = (__weapon)j;

			// do this here as well as in fn-set-player-pose as its safer in-case of misuse
			if (player.Player_exists())
				if (cur_id == player.Fetch_player_id())
					player.Set_player_status(player_stat_table[j]);

			I->___init(M->chr_name, M->anim_set, (__weapon)j); // we pass the person, set names through

			return (IR_CONT);
		}
	}

	Fatal_error("WARNING -  %s specified weapon does not exist [%s]", CGameObject::GetName(object), pose_name);

	return (IR_STOP);
}

mcodeFunctionReturnCodes _game_session::fn_set_texture(int32 &, int32 *params) {
	// fn_set_texture(text)

	const char *texture_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	L->voxel_info->Set_texture(texture_name);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_palette(int32 &, int32 *params) {
	const char *palette = (const char *)MemoryUtil::resolvePtr(params[0]);

	L->voxel_info->Set_palette(palette);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_mesh(int32 &, int32 *params) {
	// fn_set_mesh(text)

	const char *mesh = (const char *)MemoryUtil::resolvePtr(params[0]);

	L->voxel_info->Set_mesh(mesh);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_override_pose(int32 &, int32 *params) {
	// fn_set_override_pose(text)

	const char *pose = (const char *)MemoryUtil::resolvePtr(params[0]);

	L->voxel_info->Set_override_pose(pose);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_cancel_override_pose(int32 &, int32 *) {
	// fn_cancel_override_pose(text)

	L->voxel_info->Cancel_override_pose();

	return IR_CONT;
}

extern _player_stat player_stat_table[__TOTAL_WEAPONS];

mcodeFunctionReturnCodes _game_session::fn_set_player_pose(int32 &, int32 *params) {
	// change/set the weapon type
	// set instantly - i.e. init the _vox_image
	// sets players status to equivalent mode

	//	params   0 ascii name of pose - should be compatible with weapon_text

	// search the weapon set table
	uint32 j;
	const char *pose_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("fn_set_player_pose [%s]", pose_name);

	for (j = 0; j < __TOTAL_WEAPONS; j++) {
		Zdebug("test [%s]", weapon_text[j]);
		if (!strcmp(pose_name, weapon_text[j])) {
			Zdebug("found %d", j);
			L->mega->weapon = (__weapon)j;
			player.Set_player_status(player_stat_table[j]);

			player.Push_player_stat(); // because its popped on return from a conversation - hmmm

			I->___init(M->chr_name, M->anim_set, (__weapon)j); // we pass the person, set names through

			return (IR_CONT);
		}
	}

	Fatal_error("WARNING - fn_set_player_pose %s specified weapon does not exist [%s]", CGameObject::GetName(object), pose_name);

	return (IR_STOP);
}

mcodeFunctionReturnCodes _game_session::fn_set_custom(int32 &, int32 *params) {
	// change/set the custom anim type
	// if the new one is not the current then an anim must be played...

	//	params   0 ascii name of custom

	const char *custom_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("fn_set_custom [%s]", custom_name);

	if (!M)
		Fatal_error("fn_set_custom finds [%s] is not a mega", CGameObject::GetName(object));

	Set_string(custom_name, M->custom_set, MAX_CUSTOM_NAME_LENGTH);
	M->custom = TRUE8;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_message(int32 &, int32 *params) {
	const char *message = (const char *)MemoryUtil::resolvePtr(params[0]);

	// pc has to muck around to clear sticky ctrl key
	// hold until ctrl key released
	if ((Read_DI_keys(Common::KEYCODE_LCTRL)) && (!first_session_cycle))
		return (IR_REPEAT);

	if (params[0] < 256) {
		Message_box("%d", params[0]);
	} else {
		Message_box("%s - %s", CGameObject::GetName(object), message);
	}
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_message_var(int32 &, int32 *params) {
	const char *var_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	uint32 var = CGameObject::GetVariable(object, var_name);
	if (var == 0xffffffff)
		Fatal_error("fn_message_var - object %s has no var %s", CGameObject::GetName(object), var_name);

	// pc has to muck around to clear sticky ctrl key
	// hold until ctrl key released
	if (Read_DI_keys(Common::KEYCODE_LCTRL))
		return (IR_REPEAT);

	char txt[100];

	if (CGameObject::IsVariableString(object, var))
		Common::sprintf_s(txt, "%s=\"%s\"", var_name, CGameObject::GetStringVariable(object, var));
	else
		Common::sprintf_s(txt, "%s=%d", var_name, CGameObject::GetIntegerVariable(object, var));

	Message_box(txt);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_trace(int32 &, int32 *) { return (IR_CONT); }

mcodeFunctionReturnCodes _game_session::fn_get_objects_x(int32 &result, int32 *params) {
	// return objects x coord

	// params    0   name of object

	uint32 id;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	id = (uint32)LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_get_objects_x - illegal object [%s]", object_name);

	if (logic_structs[id]->image_type == PROP) {
		result = (uint32)logic_structs[id]->prop_xyz.x;
	} else {
		result = (uint32)logic_structs[id]->mega->actor_xyz.x;
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_get_objects_y(int32 &result, int32 *params) {
	// return objects y coord

	// params    0   name of object

	uint32 id;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	id = (uint32)LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_get_objects_y - illegal object [%s]", object_name);

	if (logic_structs[id]->image_type == PROP) {
		result = (uint32)logic_structs[id]->prop_xyz.y;
	} else {
		result = (uint32)logic_structs[id]->mega->actor_xyz.y;
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_get_objects_z(int32 &result, int32 *params) {
	// return objects z coord

	// params    0   name of object

	uint32 id;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	id = (uint32)LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_get_objects_z - illegal object [%s]", object_name);

	if (logic_structs[id]->image_type == PROP) {
		result = (uint32)logic_structs[id]->prop_xyz.z;
	} else {
		result = (uint32)logic_structs[id]->mega->actor_xyz.z;
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_has_mega_our_height(int32 &result, int32 *params) {
	// params    0   name of object

	uint32 id;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// get target
	id = (uint32)LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_has_mega_our_height - illegal object [%s]", object_name);

	// check its a mega
	if (logic_structs[id]->image_type == PROP)
		Fatal_error("fn_has_mega_our_height - [%s] not a mega", object_name);

	// same y?
	if (logic_structs[id]->mega->actor_xyz.y == M->actor_xyz.y)
		result = 1; // yes!
	else
		result = 0; // no

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_near(int32 &result, int32 *params) {
	// params    0   name of object
	//			1  dist

	uint32 id;
	PXreal sub1, sub2, len;
	PXreal ourx, oury, ourz;
	PXreal itsx, itsy, itsz;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	id = (uint32)LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_is_object_dead - illegal object [%s]", object_name);

	if (L->image_type == PROP) {
		ourx = L->prop_xyz.x;
		oury = L->prop_xyz.y;
		ourz = L->prop_xyz.z;
	} else {
		ourx = M->actor_xyz.x;
		oury = M->actor_xyz.y;
		ourz = M->actor_xyz.z;
	}

	if (logic_structs[id]->image_type == PROP) {
		itsx = logic_structs[id]->prop_xyz.x;
		itsy = logic_structs[id]->prop_xyz.y;
		itsz = logic_structs[id]->prop_xyz.z;
	} else {
		itsx = logic_structs[id]->mega->actor_xyz.x;
		itsy = logic_structs[id]->mega->actor_xyz.y;
		itsz = logic_structs[id]->mega->actor_xyz.z;
	}

	if (PXfabs(itsy - oury) < (200 * REAL_ONE)) { // slack for height calc
		sub1 = itsx - ourx;
		sub2 = itsz - ourz;

		// dist
		len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

		if (len < (PXreal)(params[1] * params[1])) {
			result = TRUE8;
		} else {
			result = FALSE8;
		}
		return (IR_CONT);
	} else {
		result = FALSE8;
		return (IR_CONT);
	}
}

mcodeFunctionReturnCodes _game_session::fn_is_mega_near_mega(int32 &result, int32 *params) {
	// params    0   name of object
	//			1  name of other
	//			2  dist

	uint32 id, id2;
	PXreal sub1, sub2, len;
	PXreal ourx, oury, ourz;
	PXreal itsx, itsy, itsz;
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *other_object_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	id = (uint32)LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_is_mega_near_mega - illegal object [%s]", object_name);
	id2 = (uint32)LinkedDataObject::Fetch_item_number_by_name(objects, other_object_name);
	if (id2 == 0xffffffff)
		Fatal_error("fn_is_mega_near_mega - illegal object [%s]", other_object_name);

	if (logic_structs[id]->image_type == PROP)
		Fatal_error("fn_is_mega_near_mega %s not a mega", object_name);
	if (logic_structs[id2]->image_type == PROP)
		Fatal_error("fn_is_mega_near_mega %s not a mega", other_object_name);

	itsx = logic_structs[id]->mega->actor_xyz.x;
	itsy = logic_structs[id]->mega->actor_xyz.y;
	itsz = logic_structs[id]->mega->actor_xyz.z;

	ourx = logic_structs[id2]->mega->actor_xyz.x;
	oury = logic_structs[id2]->mega->actor_xyz.y;
	ourz = logic_structs[id2]->mega->actor_xyz.z;

	if (PXfabs(itsy - oury) < (200 * REAL_ONE)) { // slack for height calc
		sub1 = itsx - ourx;
		sub2 = itsz - ourz;

		// dist
		len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

		if (len < (PXreal)(params[2] * params[2]))
			result = TRUE8;
		else
			result = FALSE8;

		return IR_CONT;
	} else { // failed on height
		result = FALSE8;
		return IR_CONT;
	}
}

mcodeFunctionReturnCodes _game_session::fn_on_screen(int32 &result, int32 * /* params */) {
	//	no params

	result = Object_visible_to_camera(cur_id);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_hold_if_off_screen(int32 &, int32 *) {
	//	no params

	if (L->image_type != VOXEL)
		Fatal_error("fn_hold_if_off_screen only works with megas [%s]", CGameObject::GetName(object));

	if (!Object_visible_to_camera(cur_id)) {
		return (IR_REPEAT);
	}

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_object_near_nico(int32 &result, int32 *params) {
	// params        0 object name
	//				1 nico name
	//				2 dist

	uint32 id;
	_feature_info *nico;
	PXreal sub1, sub2, len;
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *nico_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	id = (uint32)LinkedDataObject::Fetch_item_number_by_name(objects, object_name);

	if (id == 0xffffffff)
		Fatal_error("fn_object_near_nico - illegal object [%s]", object_name);

	if (logic_structs[id]->image_type == PROP)
		Fatal_error("fn_object_near_nico object [%s] is not a mega!", object_name);

	// fetch tag file for this item
	nico = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, nico_name);

	if (!nico)
		Fatal_error("fn_object_near_nico can't find nico [%s]", nico_name);

	if (PXfabs(logic_structs[id]->mega->actor_xyz.y - nico->y) < (200 * REAL_ONE)) { // slack for height calc
		sub1 = logic_structs[id]->mega->actor_xyz.x - nico->x;
		sub2 = logic_structs[id]->mega->actor_xyz.z - nico->z;

		// dist
		len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

		if (len < (PXreal)(params[2] * params[2])) {
			// near
			result = TRUE8;
		} else {
			result = FALSE8;
		}
	} else
		result = FALSE8; // failed on y

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_add_object_name_to_list(int32 &, int32 *params) {
	// params        0 object name

	int32 id;
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// check for list overflow
	if (L->total_list == MAX_list)
		Fatal_error("fn_add_object_name_to_list [%s] has exceeded list size of %d", CGameObject::GetName(object), MAX_list);

	id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);

	if (id == -1)
		Fatal_error("[%s] calling fn_add_object_name_to_list finds [%s] is not a legal object", CGameObject::GetName(object), object_name);

	L->list[L->total_list++] = (uint32)id;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_add_object_id_to_list(int32 &, int32 *params) {
	// params        0 object id

	// check for list overflow
	if (L->total_list == MAX_list)
		Fatal_error("fn_add_object_id_to_list [%s] has exceeded list size of %d", CGameObject::GetName(object), MAX_list);

	assert((uint32)params[0] < total_objects);

	L->list[L->total_list++] = params[0];

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_lift_process_list(int32 &result, int32 *params) {
	// special lift logic function - checks all megas in the lift against named nico

	// params        0 name of nico
	//				1 distance
	//				2 0 top, 1 bottom

	// returns   FALSE8  did nothing
	//				TRUE8     someone used lift - ascend or descend

	uint32 j;
	_feature_info *monica; // or nico to you and i
	PXreal sub1, sub2, len;

	const char *nico_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// check for no people in list! Could be redefined as an error in-fact
	if (!L->total_list) {
		result = FALSE8; // did nothing
		return (IR_CONT);
	}

	monica = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, nico_name);
	if (!monica)
		Fatal_error("fn_lift_process_list can't find nico [%s]", nico_name);

	for (j = 0; j < L->total_list; j++) {
		if (logic_structs[L->list[j]]->image_type != VOXEL)
			Fatal_error("fn_lift_process_list finds [%s] is not a mega", (const char *)logic_structs[L->list[j]]->GetName());

		if (PXfabs(logic_structs[L->list[j]]->mega->actor_xyz.y - monica->y) < (200 * REAL_ONE)) { // slack for height calc
			sub1 = logic_structs[L->list[j]]->mega->actor_xyz.x - monica->x;
			sub2 = logic_structs[L->list[j]]->mega->actor_xyz.z - monica->z;

			// dist
			len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

			if (len < (PXreal)(params[1] * params[1])) {
				// near

				result = TRUE8; // did something

				if (params[2]) { // bottom

					// Call the function that does the work in the event manager.
					g_oEventManager->PostNamedEventToObject(EVENT_LIFT_ASCEND, L->list[j], cur_id);
					return (IR_CONT);
				} else { // top
					g_oEventManager->PostNamedEventToObject(EVENT_LIFT_DESCEND, L->list[j], cur_id);
					return (IR_CONT);
				}
			}
		}
	}

	result = FALSE8; // no action
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_lib_lift_chord_and_chi(int32 &result, int32 *params) {
	// special lift platform handler for cord and chi

	// look for player being on the lift

	// params        0 name of nico (if not using platform coords)
	//				1 distance
	//				2 0 top, 1 bottom

	// returns   FALSE8  did nothing
	//				TRUE8     someone used lift - ascend or descend

	_feature_info *monica; // or nico to you and i
	PXreal sub1, sub2, len;
	PXreal lifty = REAL_ZERO;
	bool8 has_platform = FALSE8;
	uint32 lift = 0; // lift number in platform list
	bool8 hit = FALSE8;
	uint32 j = 0;
	static int32 issued_warning = FALSE8;
	const char *nico_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!prev_save_state) { // could not save last go - then can't operate lift either. Player is in a private script
		result = FALSE8;
		return IR_CONT;
	}

	monica = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, nico_name);
	if (!monica)
		Fatal_error("fn_lib_lift_chord_and_chi can't find nico [%s]", nico_name);
	lifty = monica->y;

	// see if the lift has registered a coordinate platform - if so, we use that
	for (j = 0; j < num_lifts; j++) {
		if (cur_id == lifts[j].id) {
			has_platform = TRUE8;
			lift = j;
			break; // yup - found a platform
		}
	}

	// issue one warning at a time
	if ((!has_platform) && (!issued_warning)) {
		issued_warning = TRUE8;
		Message_box("lift [%s] says please can i have proper platform coords?", CGameObject::GetName(object));
	}

	if (PXfabs(logic_structs[player.Fetch_player_id()]->mega->actor_xyz.y - lifty) < (200 * REAL_ONE)) { // slack for height calc
		if (!has_platform) {
			sub1 = logic_structs[player.Fetch_player_id()]->mega->actor_xyz.x - monica->x;
			sub2 = logic_structs[player.Fetch_player_id()]->mega->actor_xyz.z - monica->z;
			// dist
			len = (PXreal)((sub1 * sub1) + (sub2 * sub2));
			if (len < (PXreal)(params[1] * params[1]))
				hit = TRUE8;
		} else {                                                                                     // has a registered platform
			if ((logic_structs[player.Fetch_player_id()]->mega->actor_xyz.x >= lifts[lift].x) && // area box method
			    (logic_structs[player.Fetch_player_id()]->mega->actor_xyz.x <= lifts[lift].x1) &&
			    (logic_structs[player.Fetch_player_id()]->mega->actor_xyz.z >= lifts[lift].z) &&
			    (logic_structs[player.Fetch_player_id()]->mega->actor_xyz.z <= lifts[lift].z1))
				hit = TRUE8;
		}

		if (hit) {
			// tell player logic - stop ability to interact with anything else
			player.stood_on_lift = TRUE8;

			if ((player.cur_state.IsButtonSet(__INTERACT)) && (!player.interact_lock) && (player.player_status == STOOD)) {
				player.interact_lock = TRUE8;

				result = TRUE8; // did something

				if (params[2]) { // bottom
					// Call the function that does the work in the event manager.
					g_oEventManager->PostNamedEventToObject(EVENT_LIFT_ASCEND, player.Fetch_player_id(), cur_id);
					return (IR_CONT);
				} else { // top
					g_oEventManager->PostNamedEventToObject(EVENT_LIFT_DESCEND, player.Fetch_player_id(), cur_id);
					return (IR_CONT);
				}
			}
		}
	}

	result = FALSE8; // no action
	return IR_CONT;
}

typedef struct {
	uint32 init;
	int32 params[4];
} _lift_verify;

_lift_verify lift2s[MAX_session_objects];

mcodeFunctionReturnCodes _game_session::fn_lift2_process(int32 &result, int32 *params) {
	// special lift logic function - checks all megas in the lift against named nico
	// if no one is here we IR_CONT
	// of someone hits the spot we teleport them

	// params        0 name of nico
	//				1 catch distance
	//				2 0 top, 1 bottom
	//				3 release distance

	// returns   FALSE8  did nothing
	//				TRUE8     someone used lift - ascend or descend

	uint32 j = 0;
	_feature_info *monica; // or nico to you and i
	PXreal sub1, sub2, len;
	bool8 hit = FALSE8;
	bool8 has_platform = FALSE8;
	PXreal lifty = REAL_ZERO;
	uint32 lift = 0; // lift number in platform list
	static int32 issued_warning = FALSE8;
	const char *nico_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	static int32 inited = FALSE8;
	if (!inited) {
		for (j = 0; j < MAX_session_objects; j++)
			lift2s[j].init = 0;
		inited = TRUE8;
	}

	if (!lift2s[cur_id].init) {
		lift2s[cur_id].init = TRUE8;
		lift2s[cur_id].params[0] = params[0];
		lift2s[cur_id].params[1] = params[1];
		lift2s[cur_id].params[2] = params[2];
		lift2s[cur_id].params[3] = params[3];
	}

	if (lift2s[cur_id].params[0] != params[0])
		Message_box("%s param 0 changed from %d to %d", CGameObject::GetName(object), lift2s[cur_id].params[0], params[0]);
	if (lift2s[cur_id].params[1] != params[1])
		Message_box("%s param 1 changed from %d to %d", CGameObject::GetName(object), lift2s[cur_id].params[1], params[1]);
	if (lift2s[cur_id].params[2] != params[2])
		Message_box("%s param 2 changed from %d to %d", CGameObject::GetName(object), lift2s[cur_id].params[2], params[2]);
	if (lift2s[cur_id].params[3] != params[3])
		Message_box("%s param 3 changed from %d to %d", CGameObject::GetName(object), lift2s[cur_id].params[3], params[3]);

	// check for no people in list! Could be redefined as an error in-fact
	if (!L->total_list) {
		if (!issued_warning) {
			Message_box("lift [%s] says no items in list", CGameObject::GetName(object));
			issued_warning = TRUE8;
		}

		result = FALSE8; // did nothing
		return (IR_CONT);
	}

	// get nico
	monica = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, nico_name);
	if (!monica)
		Fatal_error("fn_lift2_process can't find nico [%s]", nico_name);
	lifty = monica->y;

	// see if the lift has registered a coordinate platform - if so, we use that
	for (j = 0; j < num_lifts; j++)
		if (cur_id == lifts[j].id) {
			has_platform = TRUE8;
			lift = j;
			break; // yup - found a platform
		}

	// issue one warning at a time
	if ((!has_platform) && (!issued_warning)) {
		issued_warning = TRUE8;
		Message_box("lift [%s] says please can i have proper platform coords?", CGameObject::GetName(object));
	}

	for (j = 0; j < L->total_list; j++) {
		if (logic_structs[L->list[j]]->image_type != VOXEL)
			Fatal_error("fn_lift2_process finds [%s] is not a mega", (const char *)logic_structs[L->list[j]]->GetName());

		if (logic_structs[L->list[j]]->mega->dead)
			continue;

		if (PXfabs(logic_structs[L->list[j]]->mega->actor_xyz.y - lifty) < (200 * REAL_ONE)) { // slack for height calc

			sub1 = logic_structs[L->list[j]]->mega->actor_xyz.x - monica->x;
			sub2 = logic_structs[L->list[j]]->mega->actor_xyz.z - monica->z;

			// dist
			len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

			// are we inside release distance
			if (len < (PXreal)(params[3] * params[3])) {
				hit = TRUE8; // hurray, we found someone inside the release distance
			}

			if (((!has_platform) && (len < (PXreal)(params[1] * params[1]))) ||     // crude inner nico method, or
			    ((logic_structs[L->list[j]]->mega->actor_xyz.x >= lifts[lift].x) && // area box method
			     (logic_structs[L->list[j]]->mega->actor_xyz.x <= lifts[lift].x1) && (logic_structs[L->list[j]]->mega->actor_xyz.z >= lifts[lift].z) &&
			     (logic_structs[L->list[j]]->mega->actor_xyz.z <= lifts[lift].z1))) {
				if (L->list[j] == player.Fetch_player_id()) {
					// tell player logic - stop ability to interact with anything else
					player.stood_on_lift = TRUE8;

					if ((player.cur_state.IsButtonSet(__INTERACT)) && (!player.interact_lock) && (player.player_status == STOOD)) {
						player.interact_lock = TRUE8;

						L->list_result = L->list[j]; // save id for later retrieval

						// near
						Zdebug("%s hits lift", (const char *)logic_structs[L->list[j]]->GetName());

						result = TRUE8;

						return (IR_CONT);
					}
				} else {                             // other megas
					L->list_result = L->list[j]; // save id for later retrieval

					// near
					Zdebug("mega %s hits lift", (const char *)logic_structs[L->list[j]]->GetName());

					result = TRUE8;

					return (IR_CONT);
				}
			}
		}
	}

	// someone was within release range so hold
	if (hit) {
		Zdebug("repeating");
		return (IR_REPEAT);
	}

	L->list_result = 999; // means no one - release

	// detected no one so continue in script - which means doors will close
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_wait_for_button(int32 &, int32 *params) {
	// wait for specified button
	//	params   0   button number
	//					0 interact
	//					1 punch
	//					2 forward
	//					3 backward
	//					4 right
	//					5 left

	player.Update_input_state();

	if ((params[0] == 0) && (player.cur_state.IsButtonSet(__INTERACT)))
		return IR_CONT;
	if ((params[0] == 1) && (player.cur_state.IsButtonSet(__ATTACK)))
		return IR_CONT;
	if ((params[0] == 2) && (player.cur_state.momentum == __FORWARD_1))
		return IR_CONT;
	if ((params[0] == 3) && (player.cur_state.momentum == __BACKWARD_1))
		return IR_CONT;
	if ((params[0] == 4) && ((player.cur_state.turn == __RIGHT) || (player.cur_state.turn == __HARD_RIGHT)))
		return IR_CONT;
	if ((params[0] == 5) && ((player.cur_state.turn == __LEFT) || (player.cur_state.turn == __HARD_LEFT)))
		return IR_CONT;

	return IR_REPEAT;
}

mcodeFunctionReturnCodes _game_session::fn_register_platform_coords(int32 &, int32 *params) {
	// register coords of a lift platform

	// params    0   x
	//			1  z
	//			2  x1
	//			3  z1

	//			we'll still take the y from the nico

	// safety
	if (num_lifts == MAX_lift_platforms)
		Fatal_error("too many lifts - max = %d", MAX_lift_platforms);

	lifts[num_lifts].id = cur_id;            // our id
	lifts[num_lifts].x = (PXreal)params[0];  // x
	lifts[num_lifts].z = (PXreal)params[1];  // z
	lifts[num_lifts].x1 = (PXreal)params[2]; // x1
	lifts[num_lifts].z1 = (PXreal)params[3]; // z1

	num_lifts++;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_near_list(int32 &result, int32 *params) {
	// works from our coordiate
	// we can be prop or mega
	// targets much be mega

	// params    0   dist

	uint32 j;
	PXreal sub1, sub2, len;
	PXreal ourx, oury, ourz;

	if (L->image_type == PROP) {
		ourx = L->prop_xyz.x;
		oury = L->prop_xyz.y;
		ourz = L->prop_xyz.z;
	} else {
		ourx = M->actor_xyz.x;
		oury = M->actor_xyz.y;
		ourz = M->actor_xyz.z;
	}

	for (j = 0; j < L->total_list; j++) {
		if (logic_structs[L->list[j]]->image_type != VOXEL)
			Fatal_error("fn_near_list finds [%s] is not a mega", (const char *)logic_structs[L->list[j]]->GetName());

		if (!logic_structs[L->list[j]]->mega->dead) { // alive

			Zdebug("%3.2f %3.2f", logic_structs[L->list[j]]->mega->actor_xyz.y, oury);

			if (PXfabs(logic_structs[L->list[j]]->mega->actor_xyz.y - oury) < (200 * REAL_ONE)) { // slack for height calc
				sub1 = logic_structs[L->list[j]]->mega->actor_xyz.x - ourx;
				sub2 = logic_structs[L->list[j]]->mega->actor_xyz.z - ourz;

				// dist
				len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

				if (len < (PXreal)(params[0] * params[0])) {
					// near

					L->list_result = L->list[j]; // save id for later retrieval

					result = TRUE8; // did something

					return (IR_CONT);
				}
			}
		}
	}

	result = FALSE8; // no action
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_get_list_result(int32 &result, int32 *) {
	// returns the result of a previous list process based function such as fn_near_list

	// no params

	result = L->list_result;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_display_objects_lvar(int32 &, int32 *params) {
	//draw specified objects specified lvar on the screen - for script debugging

	warning("fn_display_objects_lvar() not implemented");
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_hold_while_list_near_nico(int32 &result, int32 *params) {
	// function holds the script position while a mega from the list of megas registers as being near the named nico

	// params        0       nico
	//				1     distance

	uint32 j;
	_feature_info *monica; // or nico to you and i
	PXreal sub1, sub2, len;
	int32 ret, res;
	const char *nico_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// check for no people in list! Could be redefined as an error in-fact
	if (!L->total_list) {
		result = FALSE8; // did nothing
		return (IR_CONT);
	}

	monica = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, nico_name);
	if (!monica)
		Fatal_error("fn_hold_while_list_near_nico can't find nico [%s]", nico_name);

	for (j = 0; j < L->total_list; j++) {
		if (logic_structs[L->list[j]]->image_type != VOXEL)
			Fatal_error("fn_hold_while_list_near_nico finds [%s] is not a mega", (const char *)logic_structs[L->list[j]]->GetName());

		// ignore people who are dead
		res = Call_socket(L->list[j], "give_state", &ret);
		if (!res)
			Fatal_error("fn_hold_while_list_near_nico - object doesn't have 'give_state' script. Perhaps it's not a mega");

		if ((!ret) && (PXfabs(logic_structs[L->list[j]]->mega->actor_xyz.y - monica->y) < (200 * REAL_ONE))) { // slack for height calc
			sub1 = logic_structs[L->list[j]]->mega->actor_xyz.x - monica->x;
			sub2 = logic_structs[L->list[j]]->mega->actor_xyz.z - monica->z;

			// dist
			len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

			if (len < (PXreal)(params[1] * params[1])) {
				// near
				return (IR_REPEAT);
			}
		}
	}

	// no one was near the nico
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_watch(int32 &, int32 *params) {
	if (g_icb->getGameType() == GType_ELDORADO) {
		return IR_CONT;
	}

	_input *psInputState;
	_input sInputState;

	// set the camera to follow the named mega

	// params        0       name of object

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	uint32 id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);

	if (id == 0xffffffff)
		Fatal_error("fn_set_watch - object [%s] does not exist", object_name);

	if (logic_structs[id]->image_type != VOXEL)
		Fatal_error("fn_set_watch - object [%s] is not a person!", object_name);

	// if setting back to player then cancel the camera overide
	if (id == player.Fetch_player_id())
		g_mission->camera_follow_id_overide = 0;
	else
		g_mission->camera_follow_id_overide = id;

	if (g_icb->getGameType() == GType_ICB) {
		// If we are switching back to the player then we need to put the Remora back up if
		// it was up when we switched to a manual watch (but only in 3D).
		if (g_px->display_mode == THREED) {
			// Check if switching to player.
			if (id == player.Fetch_player_id()) {
				// If the Remora was active, need to bring it back up.
				if (g_mission->remora_save_mode != -1) {
					MS->player.Push_control_mode(ACTOR_RELATIVE);
					g_oRemora->SetModeOverride((_remora::RemoraMode)g_mission->remora_save_mode);
					g_oRemora->ActivateRemora((_remora::RemoraMode)g_mission->remora_save_mode);
					MS->player.Set_player_status(REMORA);
					MS->player.Update_input_state();
					psInputState = MS->player.Fetch_input_state();
					g_oRemora->CycleRemoraLogic(*psInputState);
					g_mission->remora_save_mode = -1;
				}
			} else {
				// Not switching to player so flag whether or not the Remora is active.
				if (g_oRemora->IsActive()) {
					// Deactivate it and remember its mode.
					g_mission->remora_save_mode = (int32)g_oRemora->GetMode();
					g_oRemora->SetMode(_remora::MOTION_SCAN);
					g_oRemora->DeactivateRemora(TRUE8);
					sInputState.UnSetButton(__UNUSEDBUTTON);
					g_oRemora->CycleRemoraLogic(sInputState);
					MS->player.Pop_control_mode();
					MS->player.Set_player_status(STOOD);
				} else {
					g_mission->remora_save_mode = -1;
				}
			}
		}
		// Deactivate the Remora.
	}

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_three_sixty_interact(int32 &, int32 *) {
	// set object to use the prop 360deg interaction type

	if (L->image_type != PROP)
		Fatal_error("fn_three_sixty_interact - object [%s] is not a prop!", CGameObject::GetName(object));

	if (g_icb->getGameType() == GType_ELDORADO)
		L->prop_interact_method = __THREE_SIXTY;
	else
		L->three_sixty_interact |= THREE_SIXTY_INTERACT;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_radial_interact(int32 &, int32 *params) {
	//set object to use the simple distance based prop interaction type

	if (L->image_type != PROP)
		Fatal_error("fn_radial_interact - object [%s] is not a prop!", CGameObject::GetName(object));

	L->prop_interact_method = __RADIAL;

	L->radial_interact_distance = params[0];

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_prop_crouch_interact(int32 &, int32 *) {
	// set object to use the prop 360deg interaction type

	L->three_sixty_interact |= PROP_CROUCH_INTERACT;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_random(int32 &result, int32 *params) {
	// return a random number to script

	// params    0   largest number possible from choice

	result = g_icb->getRandomSource()->getRandomNumber(params[0] - 1);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_change_session(int32 &, int32 *params) {
	// change session within the current mission

	// params    0   name of session
	// params    1   name of new nico

	uint32 ret;
	const char *ses_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *nico_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	Tdebug("session_log.txt", "fn_change_session changing to [%s]", ses_name);

	g_mission->Set_new_session_name(ses_name);
	g_mission->Set_init_nico_name(nico_name);

	// save the players 'hits' variable
	CGame *ob;
	ob = (CGame *)LinkedDataObject::Fetch_item_by_number(objects, player.Fetch_player_id());
	ret = CGameObject::GetVariable(ob, "hits");
	g_mission->old_hits_value = CGameObject::GetIntegerVariable(ob, ret);

	return (IR_STOP); // do no more
}

mcodeFunctionReturnCodes _game_session::fn_changed_sessions(int32 &result, int32 *) {
	// look for nico name logged by fn_change_session
	// if found init there
	// return yes or no

	_feature_info *nico;
	uint32 ret;

	result = g_mission->Is_there_init_nico();

	if (result) { // nico is waiting Removed explicit test against TRUE8 to get rid of VC5 warning
		nico = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, g_mission->Return_init_nico_name());
		if (!nico)
			Fatal_error("fn_changed_sessions can't find nico [%s]", g_mission->Return_init_nico_name());

#define XX logic_structs[cur_id]->mega->actor_xyz.x
#define ZZ logic_structs[cur_id]->mega->actor_xyz.z

		logic_structs[cur_id]->mega->actor_xyz.x = nico->x;
		logic_structs[cur_id]->mega->actor_xyz.y = nico->floor_y;
		logic_structs[cur_id]->mega->actor_xyz.z = nico->z;

		// set pan
		logic_structs[cur_id]->pan = nico->direction;

		// reset current hits from previous session
		ret = CGameObject::GetVariable(object, "hits");
		CGameObject::SetIntegerVariable(object, ret, g_mission->old_hits_value);

		// has coords
		logic_structs[cur_id]->prop_coords_set = TRUE8;

		// move player forwards a little
		if (cur_id == LinkedDataObject::Fetch_item_number_by_name(objects, "chi")) {
			// we are the player then jump player in-front of chi

			PXfloat ang = nico->direction * TWO_PI;
			PXfloat cang = (PXfloat)PXcos(ang);
			PXfloat sang = (PXfloat)PXsin(ang);

			XX += PXfloat2PXreal((75 * REAL_ONE) * sang);
			ZZ += PXfloat2PXreal((75 * REAL_ONE) * cang);
		}
	}

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_changed_via_this_shaft(int32 &result, int32 *params) {
	// a lib_session_changer_lift checks to see if its marker was the one that the player started the session on
	// if so then the lift knows that it was the lift that has just arrived and therefore it is here and open

	// params    0   name of our marker

	const char *marker_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (g_mission->Is_there_init_nico()) {
		// there is a nico
		// is it ours

		if (!strcmp(marker_name, g_mission->Return_init_nico_name())) {
			// yes!
			result = 1;
			return IR_CONT;
		}
	}

	result = 0; // no
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_is_object_adjacent(int32 &result, int32 *params) {
	// is the name object on a floor adjacent to this objects floor

	// params        0   name of object

	_floor *our_floor;
	uint32 their_floor;
	uint32 id;
	uint32 j;
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	our_floor = floor_def->Fetch_floor_number(L->owner_floor_rect);

	if (!our_floor->num_neighbours) {
		result = FALSE8; // can't be adjacent as there are non - which is impossible of course but hey
		return (IR_CONT);
	}

	id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_is_object_on_our_floor - illegal object [%s]", object_name);

	their_floor = logic_structs[id]->owner_floor_rect;

	// are they on our floor - haha
	if (their_floor == L->owner_floor_rect) { //
		result = FALSE8;                  //
		return (IR_CONT);
	}

	// check each neighbour
	for (j = 0; j < our_floor->num_neighbours; j++) {
		if (our_floor->neighbour_map[j].neighbour == their_floor) {
			result = TRUE8;
			return (IR_CONT);
		}
	}

	result = FALSE8;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_is_object_on_this_floor(int32 &result, int32 *params) {
	// check to see if object is on the floor passed

	// params        0 name of object
	//				1 name of floor

	uint32 floor_id;
	uint32 id;
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *floor_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	Zdebug("fn_is_object_on_this_floor [%s], [%s]", object_name, floor_name);

	id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_is_object_on_our_floor - illegal object [%s]", object_name);

	floor_id = LinkedDataObject::Fetch_item_number_by_name(floor_def->floors, floor_name);
	if (floor_id == 0xffffffff)
		Fatal_error("fn_are_we_on_this_floor can't locate floor [%s]", floor_name);

	if (floor_id == logic_structs[id]->owner_floor_rect)
		result = TRUE8;
	else
		result = FALSE8;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_switch_on_the_really_neat_and_special_script_debugging_facility(int32 &result, int32 *params) {
	// switch on daves debugging
	bool8 flag = (params[0] == 0) ? FALSE8 : TRUE8;

	Zdebug("\n\n****** switching ON script debugging *******\n\n");

	SetScriptDebugging(flag);

	result = 0;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_switch_off_the_really_neat_and_special_script_debugging_facility(int32 &result, int32 *) {
	// switch off daves debugging

	Zdebug("\n\n------ switching OFF script debugging ------\n\n");

	SetScriptDebugging(FALSE8);

	result = 0;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_preload_custom_mega_anim(int32 &result, int32 *params) {
	// set a resource loading
	// we have divided fn_functions into types (i.e. mega animations) in-case there are cluster complications

	// params    0   name of final anim

	return (fn_prime_custom_anim(result, params));
}

mcodeFunctionReturnCodes _game_session::fn_no_logic(int32 &, int32 *) {
	// set wait_status to no-logic which stops the script being invoked

	Tdebug("logic_modes.txt", "fn_no_logic freezing [%s]", CGameObject::GetName(object));

	L->big_mode = __NO_LOGIC;
	L->cycle_time = 0; // for mt display

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_sleep(int32 &, int32 *params) {
	// add or remove no-logic

	// params        0       name
	//				1     0 off, 1 on

	uint32 id;
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_set_sleep - illegal object [%s]", object_name);

	if (params[1]) {
		logic_structs[id]->big_mode = __NO_LOGIC;
		logic_structs[id]->cycle_time = 0; // for mt display
	} else {
		logic_structs[id]->big_mode = __SCRIPT;
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_mega_use_lift(int32 &, int32 *params) {
	// trigger a lift - manually
	// no safety checking here - scripts must assure all is in right position, etc.

	// just set a request flag - putting in function gives us scope for seamless future changes

	// params        0   name of lift

	CGame *ob;
	uint32 var_num;
	const char *lift_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	ob = (CGame *)LinkedDataObject::Fetch_item_by_name(objects, lift_name);
	if (!ob)
		Fatal_error("fn_use_lift - illegal object [%s]", lift_name);

	var_num = CGameObject::GetVariable(ob, "request");
	if (var_num == (uint32)-1)
		Fatal_error("fn_use_lift - object [%s] doesn't have REQUEST variable", lift_name);

	CGameObject::SetIntegerVariable(ob, var_num, 2); // two is trigger value

	return (IR_CONT);
}

// fn_make_remora_beep(1) makes the remora beep notice us, 0 makes it not do...
mcodeFunctionReturnCodes _game_session::fn_make_remora_beep(int32 &, int32 *params) {
	if (logic_structs[cur_id]->image_type != VOXEL)
		Fatal_error("fn_make_remora_beep - object %s is not a person! what is this?", CGameObject::GetName(object));

	if (params[0] == 0) {
		L->mega->make_remora_beep = FALSE8;
		Zdebug("%s->fn_make_remora_beep(no beep)\n", CGameObject::GetName(object));
	} else if (params[0] == 1) {
		L->mega->make_remora_beep = TRUE8;
		Zdebug("%s->fn_make_remora_beep(beep)\n", CGameObject::GetName(object));
	} else
		Fatal_error("fn_make_remora_beep - object [%s] called with value %d", CGameObject::GetName(object), params[0]);

	return (IR_CONT);
}

// fn_set_evil(1) sets us to evil mode for beeping and possibly scanner, fn_set_evil(0) sets us to harmless
mcodeFunctionReturnCodes _game_session::fn_set_evil(int32 &, int32 *params) {
	if (logic_structs[cur_id]->image_type != VOXEL)
		Fatal_error("fn_set_evil - object %s is not a person! what is this?", CGameObject::GetName(object));

	if (params[0] == 0) {
		L->mega->is_evil = FALSE8;
		Zdebug("%s->fn_set_evil(NOT_EVIL!)\n", CGameObject::GetName(object));
	} else if (params[0] == 1) {
		L->mega->is_evil = TRUE8;
		Zdebug("%s->fn_set_evil(EVIL!)\n", CGameObject::GetName(object));
	} else
		Fatal_error("fn_set_evil - object [%s] called with value %d", CGameObject::GetName(object), params[0]);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_object_type(int32 &, int32 *params) {
	// This allows an object to set its type variable.  At the moment, this is picked up only by the
	// Remora, for the purposes of displaying a suitable symbol, but we may be able to add other
	// functionality on it if we require.
	logic_structs[cur_id]->object_type = (__object_type)params[0];
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_do_not_disturb(int32 &, int32 *params) {
	bool8 bDoNotDisturb;

	Zdebug("fn_do_not_disturb( %d ) called by object %d", (uint32)params[0], cur_id);

	// Typesafe convert the parameter.
	bDoNotDisturb = ((uint32)params[0] == 0) ? FALSE8 : TRUE8;

	logic_structs[cur_id]->do_not_disturb = bDoNotDisturb;

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_is_mega_within_area(int32 &result, int32 *params) {
	// is the named object within the specified rect

	// params    0           name of object
	//			1,2,3,4    x,z, x1,z1

	// returns   true/false

	uint32 id;
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	id = (uint32)LinkedDataObject::Fetch_item_number_by_name(objects, object_name);

	if (id == 0xffffffff)
		Fatal_error("fn_is_mega_within_area - illegal object [%s]", object_name);

	if (logic_structs[id]->image_type == PROP)
		Fatal_error("fn_is_mega_within_area - object [%s] not a mega", object_name);

	if ((logic_structs[id]->mega->actor_xyz.x > (PXreal)params[1]) && (logic_structs[id]->mega->actor_xyz.x < (PXreal)params[3]) &&
	    (logic_structs[id]->mega->actor_xyz.z > (PXreal)params[2]) && (logic_structs[id]->mega->actor_xyz.z < (PXreal)params[4])) {
		result = TRUE8;
	} else {
		result = FALSE8;
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_end_mission(int32 &, int32 *) {
	// end the mission and drop back to console, or, the main script

	// shhhhhh - make everything quiet
	PauseSounds();

	g_mission->End_mission();

	return IR_STOP;
}

mcodeFunctionReturnCodes _game_session::fn_restart_gamescript(int32 &, int32 *) {
	// reset gamescript so that game begins afresh
	// presumably first thing in GS will be Start new game/Restore previous screen

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_am_i_player(int32 &result, int32 *) {
	// This function returns true if the player calls it, false otherwise.  Allows an interact
	// script to tell if the player is using it or another mega.

	if (cur_id == player.Fetch_player_id())
		result = 1;
	else
		result = 0;

	// Calling script can continue.
	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_start_conveyor(int32 &, int32 *params) {
	// start a conveyor belt

	// params    0   x
	//			1  y
	//			2  z
	//			3  x1
	//			4  z1
	//			5  xm
	//			6  zm

	uint32 j;

	for (j = 0; j < MAX_conveyors; j++) {
		if (conveyors[j].moving == FALSE8) {
			// found a slot
			conveyors[j].x = (PXreal)params[0];
			conveyors[j].y = (PXreal)params[1];
			conveyors[j].z = (PXreal)params[2];
			conveyors[j].x1 = (PXreal)params[3];
			conveyors[j].z1 = (PXreal)params[4];
			conveyors[j].xm = (PXreal)params[5];
			conveyors[j].zm = (PXreal)params[6];

			conveyors[j].moving = TRUE8;

			Tdebug("conveyor.txt", "conveyor %d = %d,%d,%d %d,%d %d,%d", j, params[0], params[1], params[2], params[3], params[4], params[5], params[6]);

			return IR_CONT;
		}
	}

	Fatal_error("too many conveyors!");
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_stop_conveyor(int32 &, int32 *params) {
	// stop a conveyor

	uint32 j;

	for (j = 0; j < MAX_conveyors; j++) {
		if (conveyors[j].moving == TRUE8) {
			if ((conveyors[j].x == (PXreal)params[0]) && (conveyors[j].z == (PXreal)params[1])) {
				Tdebug("conveyor.txt", "conveyor %d stopped", j);
				conveyors[j].moving = FALSE8;
				return IR_CONT;
			}
		}
	}

	Fatal_error("no conveyor @ %d,%d", params[0], params[1]);

	return IR_CONT;
}

void _game_session::Process_conveyors() {
	uint32 j;

	for (j = 0; j < MAX_conveyors; j++) {
		if (conveyors[j].moving == TRUE8) {
			if ((logic_structs[player.Fetch_player_id()]->mega->actor_xyz.x > conveyors[j].x) &&
			    (logic_structs[player.Fetch_player_id()]->mega->actor_xyz.y == conveyors[j].y) &&
			    (logic_structs[player.Fetch_player_id()]->mega->actor_xyz.x < conveyors[j].x1) &&
			    (logic_structs[player.Fetch_player_id()]->mega->actor_xyz.z > conveyors[j].z) &&
			    (logic_structs[player.Fetch_player_id()]->mega->actor_xyz.z < conveyors[j].z1)) {
				logic_structs[player.Fetch_player_id()]->mega->actor_xyz.x += conveyors[j].xm;
				logic_structs[player.Fetch_player_id()]->mega->actor_xyz.z += conveyors[j].zm;
			}
		}
	}
}

mcodeFunctionReturnCodes _game_session::fn_register_ladder(int32 &result, int32 *params) {
	// register a ladder

	// params    0   target marker name
	//			1  1==top (else bottom)
	//			2  length

	const char *marker_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	result = Register_stair_or_ladder(marker_name, (bool8)params[1], params[2], FALSE8, 24);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_register_stairway(int32 &result, int32 *params) {
	// register a stairway

	// params    0   target marker name
	//			1  1==top (else bottom)
	//			2  length

	const char *marker_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	result = Register_stair_or_ladder(marker_name, (bool8)params[1], params[2], TRUE8, 18);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_activate_stair_or_ladder(int32 &, int32 *params) {
	Set_state_of_stair_or_ladder((uint32)params[0], TRUE8);

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_deactivate_stair_or_ladder(int32 &, int32 *params) {
	Set_state_of_stair_or_ladder((uint32)params[0], FALSE8);

	return (IR_CONT);
}

void _game_session::Set_state_of_stair_or_ladder(uint32 nIndex, bool8 bState) {
	if (nIndex > num_stairs)
		Fatal_error("Index %d out or range 0-%d in _game_session::Set_state_of_stair_or_ladder()", nIndex, num_stairs - 1);

	stairs[nIndex].live = bState;
	// do the other end
	stairs[stairs[nIndex].opposite_number].live = bState;
}

uint32 _game_session::Register_stair_or_ladder(const char *target, bool8 top, uint32 length, bool8 isstair, uint32 stepcms) {
	_feature_info *stair;
	_feature_info *dest_stair;
	PXreal x1, z1, x2, z2, x3, z3;
	uint32 dest_stair_id;
	uint8 j = 0;

	if (!length)
		Fatal_error("%s has 0 length", CGameObject::GetName(object));
	if (length > 1000)
		Fatal_error("%s has illegal length %d", CGameObject::GetName(object), length);

	// get our nico
	stair = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, const_cast<char *>(CGameObject::GetName(object)));
	// get other end
	dest_stair = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, target);
	dest_stair_id = LinkedDataObject::Fetch_item_number_by_name(objects, target);

	if (!stair)
		Fatal_error("fn_register_stairway - can't find nico %s", CGameObject::GetName(object));
	if (!dest_stair)
		Fatal_error("fn_register_stairway - can't find nico %s", target);
	if (num_stairs == MAX_stairs)
		Fatal_error("too many stairs - max = %d", MAX_stairs);

	stairs[num_stairs].pan = stair->direction;
	stairs[num_stairs].pan_ref = stair->direction;
	stairs[num_stairs].x = stair->x;
	stairs[num_stairs].z = stair->z;

	// link up with other end
	for (j = 0; j < num_stairs; j++) {
		if (stairs[j].stair_id == dest_stair_id) {             // found the target - its already set itself up
			stairs[j].opposite_number = (uint8)num_stairs; // its opposite is us
			stairs[num_stairs].opposite_number = j;        // its our opposite number
			break;
		}
	}

	PXfloat ang = stair->direction * TWO_PI;
	PXfloat cang = (PXfloat)PXcos(ang);
	PXfloat sang = (PXfloat)PXsin(ang);
	PXreal dx = PXfloat2PXreal((PXfloat)length * cang);
	PXreal dz = PXfloat2PXreal((PXfloat)length * sang);

#define STAIR_BARRIER_OFFSET 0

	x1 = stair->x + PXfloat2PXreal(STAIR_BARRIER_OFFSET * sang);
	z1 = stair->z + PXfloat2PXreal(STAIR_BARRIER_OFFSET * cang);

	x2 = x1 - dx;
	z2 = z1 + dz;

	x3 = x1 + dx;
	z3 = z1 - dz;

	// new constructor for barrier...
	stairs[num_stairs].bar.m_x1 = x2;
	stairs[num_stairs].bar.m_z1 = z2;
	stairs[num_stairs].bar.m_x2 = x3;
	stairs[num_stairs].bar.m_z2 = z3;
	stairs[num_stairs].bar.m_bottom = stair->floor_y;
	stairs[num_stairs].bar.m_top = (PXreal)0;
	stairs[num_stairs].bar.m_material = BRICK;

	// create the BCM maths----------------------------------------------
	// The barrier has coordinates x2, z2 to x3, z3.
	// Fill in the structure that holds the extra collision maths.

	BarrierCollisionMathsObject::Generate(&stairs[num_stairs].bar.m_bcm, x2, z2, x3, z3);
	routeBarrierCreatePan(&stairs[num_stairs].bar);

	Tdebug("stairs.txt", "try this: %.4f, %g,%g,%g %g,%g %g,%g %g,%g\n", stairs[num_stairs].bar.m_pan, stairs[num_stairs].bar.m_bcm.m_linedist,
	       stairs[num_stairs].bar.m_bcm.m_alinedist, stairs[num_stairs].bar.m_bcm.m_blinedist, stairs[num_stairs].bar.m_bcm.m_lpx, stairs[num_stairs].bar.m_bcm.m_lpz,
		   BarrierCollisionMathsObject::alpx(&stairs[num_stairs].bar.m_bcm), BarrierCollisionMathsObject::alpz(&stairs[num_stairs].bar.m_bcm), BarrierCollisionMathsObject::blpx( &stairs[num_stairs].bar.m_bcm), BarrierCollisionMathsObject::blpz(&stairs[num_stairs].bar.m_bcm));

	// create the BCM maths----------------------------------------------

	// work out unit height
	stairs[num_stairs].units = (uint8)((PXfabs(dest_stair->floor_y - stair->floor_y) + (stepcms / 2)) / stepcms);

	// dir
	if (!top)                              // if not the top
		stairs[num_stairs].up = TRUE8; // then its the bottom which means we're going up
	else {
		stairs[num_stairs].up = FALSE8; // else down

		// top of ladders have their pan adjusted
		if (!isstair)
			stairs[num_stairs].pan_ref = stair->direction + HALF_TURN;
	}

	// set id
	stairs[num_stairs].stair_id = (uint8)cur_id;

	// type
	stairs[num_stairs].is_stair = isstair;

	// make stair active
	stairs[num_stairs].live = TRUE8;

	// diagnostics
	if (isstair)
		Tdebug("stairs.txt", "\nInit stair %d", num_stairs);
	else
		Tdebug("stairs.txt", "\nInit ladder %d", num_stairs);
	Tdebug("stairs.txt", "marker %3.2f,%3.2f, %3.2f", stair->x, stair->floor_y, stair->z);
	Tdebug("stairs.txt", "point 1 = %3.2f, %3.2f", x2, z2);
	Tdebug("stairs.txt", "point 2 = %3.2f, %3.2f", x3, z3);
	Tdebug("stairs.txt", "units = %d", stairs[num_stairs].units);
	if (stairs[num_stairs].up)
		Tdebug("stairs.txt", "going up");
	else
		Tdebug("stairs.txt", "going down");

	L->list_result = num_stairs; // script needs to get this

	return stairs[num_stairs++].units; // set to uploop in object - instead of implementor!
}

mcodeFunctionReturnCodes _game_session::fn_align_with_floor(int32 &, int32 *) {
	// mega climbing stairs and ladder calls this afterward to realign with artist misaligned floors

	floor_def->Align_with_floor(M);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_load_players_gun(int32 &, int32 *params) {
	// put bullets in the players gun

	player.SetBullets(params[0]);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_flash_health(int32 &, int32 *) {
	// flash the health when you get shot

	health_time = 12;

	return IR_CONT;
}
mcodeFunctionReturnCodes _game_session::fn_set_anim_speed(int32 &, int32 *params) {
	// set a megas animation playback rate

	// params    0   rate

	if (logic_structs[cur_id]->image_type != VOXEL)
		Fatal_error("fn_set_anim_speed says people only!");

	M->anim_speed = (uint8)params[0];

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_push_coords(int32 &, int32 *) {
	// push coordinates - used before megas attempt to climb stairs or ladders - for save games

	M->pushed = TRUE8;

	memcpy(&M->pushed_actor_xyz, &M->actor_xyz, sizeof(PXvector));

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_pop_coords(int32 &, int32 *) {
	// remove previous pop

	M->pushed = FALSE8;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_quick_restart(int32 &result, int32 *) {
	// reset game now or not
	// if not then mission ends
	// if so then simply reset the player

	// if ( MessageBox(windowHandle, "reset player?", "would you like to...", MB_YESNO | MB_DEFBUTTON2 ) == IDYES )
	{
		result = 1; // do
		MS->Restart_player();
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_shadows_on(int32 &, int32 *) {
	// shadows back on

	if (logic_structs[cur_id]->image_type != VOXEL)
		Fatal_error("fn_shadows_on says people only!");

	M->drawShadow = TRUE8; // shadows back on

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_shadows_off(int32 &, int32 *) {
	// shadows off

	if (logic_structs[cur_id]->image_type != VOXEL)
		Fatal_error("fn_shadows_off says people only!");

	M->drawShadow = FALSE8; // shadows off

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_shadow(int32 &, int32 *params) {
	if (logic_structs[cur_id]->image_type != VOXEL)
		Fatal_error("fn_shadow says people only!");

	M->drawShadow = (uint8)params[0];

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_shade_percentage(int32 &, int32 *params) {
	// set shade percentage figure for mega

	if (L->image_type != VOXEL)
		Fatal_error("fn_set_shade_percentage [%s] says people only!", CGameObject::GetName(object));
	if (params[0] > 99)
		Fatal_error("fn_set_shade_percentage [%s] percentages usually go 0-99", CGameObject::GetName(object));

	M->inShadePercentage = (uint8)params[0];

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_half_character_width(int32 &, int32 *params) {
	// adjust the default barrier extrapolation for route barriers - for people like spectre who is very big

	if (!M)
		Fatal_error("fn_set_half_character_width only for megas [%s]", CGameObject::GetName(object));

	M->extrap_size = (uint8)params[0];

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_visible(int32 &, int32 *params) {
	// set high level on off clip
	// params        0   0 off 1 on

	if (!M)
		Fatal_error("%s fn_set_visible is for megas only", CGameObject::GetName(object));

	M->display_me = (bool8)params[0];

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_inventory_active(int32 &, int32 *params) {
	if ((params[0] != 0) && (params[0] != 1))
		Message_box("Must pass either 0 or 1 into fn_inventory_active");

	M->inventoryActive = (bool8)params[0];

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_object_visible(int32 &, int32 *params) {
	uint32 nObjectID;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Find the target object's ID.
	nObjectID = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);

	// Make sure object is a mega character.
	if (!(logic_structs[nObjectID]->mega))
		Fatal_error("fn_set_object_visible is for megas only");

	// Right it is a mega, so set its flag.
	logic_structs[nObjectID]->mega->display_me = ((int32)params[1] == 0) ? FALSE8 : TRUE8;

	// Calling script can continue.
	return (IR_CONT);
}

// fn_set_interact_look_height(h)
// sets the height of an interact object for the looking around code
// the units are cm above the floor...
mcodeFunctionReturnCodes _game_session::fn_set_interact_look_height(int32 &, int32 *params) {
	int32 h = params[0];

	L->look_height = h;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_to_dead(int32 &, int32 *) {
	// set the dead flag - we have a flag for speed of access

	if (!L->mega)
		Fatal_error("fn_set_to_dead called for [%s] but not a mega", CGameObject::GetName(object));

	L->mega->dead = TRUE8;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_camera_hold(int32 &, int32 *params) {
	// switch camera hold mode on or off

	if (M)
		Fatal_error("fn_set_camera_hold called for [%s] but this is for props only", CGameObject::GetName(object));

	// find entry for this object via its name, which we find via its number :(
	if (!LinkedDataObject::Try_fetch_item_by_name(prop_anims, CGameObject::GetName(object)))
		return IR_CONT; // item has no prop entry - so keep it live

	if (params[0])
		L->hold_mode = prop_camera_hold;
	else
		L->hold_mode = none;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_mega_wait_for_player(int32 &, int32 *) {
	// mega will pause until the player arrives

	if (!M)
		Fatal_error("fn_set_mega_wait_for_player called for [%s] but not a mega", CGameObject::GetName(object));

	L->big_mode = __MEGA_INITIAL_FLOOR_HELD;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_mega_off_camera_hold(int32 &, int32 *) {
	// mega will pause when not on_camera

	if (!M)
		Fatal_error("fn_set_mega_off_camera_hold called for [%s] but not a mega", CGameObject::GetName(object));

	L->big_mode = __MEGA_PLAYER_FLOOR_HELD;
	L->hold_mode = mega_player_floor_hold;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_mega_slice_hold(int32 &, int32 *params) {
	// mega will pause until the player arrives

	// params        0   y distance to stray before character holds again

	if (!M)
		Fatal_error("fn_set_mega_slice_hold called for [%s] but not a mega", CGameObject::GetName(object));

	L->big_mode = __MEGA_SLICE_HELD;
	L->hold_mode = mega_slice_hold;
	M->slice_hold_tolerance = (uint32)params[0];

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_lock_y(int32 &, int32 *params) {
	// lock a y coord of a named nico marker - the floor chooser will use this value. Good for lifts that cross multiple slices, etc

	_feature_info *nico;
	const char *marker_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	nico = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, marker_name);
	if (!nico)
		Fatal_error("fn_lock_y by [%s] for nico [%s] finds no such nico", CGameObject::GetName(object), marker_name);

	M->y_locked = TRUE8;
	M->y_lock = nico->y;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_unlock_y(int32 &, int32 *) {
	// cancel locked y

	M->y_locked = FALSE8;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_flip_pan(int32 &, int32 *) {
	// flip the pan 180deg - used, for example, when megas get off the bottom of ladders
	// this is why we are going to engine code this type of logic next time

	L->pan += HALF_TURN;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_snap_to_ladder_bottom(int32 &, int32 *) {
// used to position non player megas going up ladders

#define MEGA_SNAP_UP 40

	for (uint8 j = 0; j < num_stairs; j++) {
		if (stairs[j].stair_id == M->target_id) { // found the target - its already set itself up
			Snap_to_ladder(&stairs[j], MEGA_SNAP_UP);
			return IR_CONT;
		}
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_snap_to_ladder_top(int32 &, int32 *) {
// used to position non player megas going down ladders

#define MEGA_SNAP_DOWN 85

	for (uint8 j = 0; j < num_stairs; j++) {
		if (stairs[j].stair_id == M->target_id) { // found the target - its already set itself up
			Snap_to_ladder(&stairs[j], MEGA_SNAP_DOWN);
			return IR_CONT;
		}
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_manual_interact_object(int32 &, int32 *params) {
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// Find the id for the named object.
	uint32 nObjectID = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);

	// Check the object is valid.
	if (nObjectID == PX_LINKED_DATA_FILE_ERROR)
		Fatal_error("fn_set_manual_interact_object( %s ) - object does not exist", object_name);

	// Clear the script-forced object interact id.
	player.cur_interact_id = nObjectID;

	// Calling script can continue.
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_cancel_manual_interact_object(int32 &, int32 *) {
	// Clear the script-forced object interact id.
	player.cur_interact_id = (uint32)-1;

	// Calling script can continue.
	return IR_CONT;

}

mcodeFunctionReturnCodes _game_session::fn_preload_actor_file(int32 &, int32 *params ) {
	const char *filename = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (L->voxel_info->Preload_file(filename) == 0)
		return IR_REPEAT;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_hard_load_mesh(int32 &, int32 *params) {
	const char *base_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	char mesh[32];
	Common::strcpy_s(mesh, base_name);
	Common::strcat_s(mesh, ".rap");

	uint32 fileHash = NULL_HASH;
	rs_anims->Res_open(mesh, fileHash, L->voxel_info->base_path, L->voxel_info->base_path_hash);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_preload_mesh(int32 &, int32 *params) {
	const char *base_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	char filename[32];
	Common::strcpy_s(filename, base_name);
	Common::strcat_s(filename, ".rap");

	if (L->voxel_info->Preload_file(filename) == 0)
		return IR_REPEAT;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_preload_texture(int32 &, int32 *params) {
	const char *base_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	char filename[32];
	Common::strcpy_s(filename, base_name);
	Common::strcat_s(filename, ".revtex");

	if (L->voxel_info->Preload_file(filename) == 0)
		return IR_REPEAT;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_preload_palette(int32 &, int32 *params) {
	const char *base_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	char filename[32];
	Common::strcpy_s(filename, base_name);
	Common::strcat_s(filename, ".revtex");

	if (L->voxel_info->Preload_file(filename) == 0)
		return IR_REPEAT;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_preload_animation(int32 &param1, int32 *params) {
	const char *set_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	char stub[128];
	char file[128];

	// The set name
	Common::strcpy_s(stub, set_name);

	// The actual animation name
	Common::strcat_s(stub, "\\");
	Common::strcat_s(stub, anim_name);

	// Preload the marker file into memory
	Common::strcpy_s(file, stub);
	Common::strcat_s(file, ".raj");

	if (L->voxel_info->Preload_file(file) == 0) {
		return IR_REPEAT;
	}

	// Preload the animation file into memory
	Common::strcpy_s(file, stub);
	Common::strcat_s(file, ".rab");

	 if (L->voxel_info->Preload_file(file) == 0) {
		return IR_REPEAT;
	 }

	 return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_to_floor(int32 &, int32 *params) {
	// locate a character onto a specified floor
	// crudely sticks the character in the middle of the first floor LRECT

	//	params[0]    ascii name of floor

	//const char *floor_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// get the floor
	//_floor *floor = (_floor *)floor_def->Fetch_named_floor(floor_name);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_is_actor_relative(int32 &result, int32 *) {
	if (player.Get_control_mode() == ACTOR_RELATIVE)
		result = TRUE8;
	else
		result = FALSE8;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_can_save(int32 &result, int32 *) {
	result = (int32)(MS->prev_save_state);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_as_player(int32 &, int32 *params) {
	if (g_icb->getGameType() != GType_ELDORADO)
		return IR_CONT;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	uint32 id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);

	if (id == PX_LINKED_DATA_FILE_ERROR)
		Fatal_error("fn_set_as_player object [%s] does not exist", object_name);

	// first, if there is a current player then he needs to be told to re-run context as he ain't anymore
	if (player.Player_exists()) {
		logic_structs[player.Fetch_player_id()]->context_request = TRUE8;
		logic_structs[player.Fetch_player_id()]->mega->async_list_pos = 0; // reset list position
		logic_structs[player.Fetch_player_id()]->mega->asyncing = 0;
		ResetPlayerLook(); // old player straightens out - i.e. reset look at interact object
	}

	player.Set_player_id(id);

	player.Reset_player();

	player.interact_lock = TRUE8;

	logic_structs[id]->context_request = TRUE8;
	logic_structs[id]->mega->async_list_pos = 0; // reset list position
	logic_structs[id]->mega->asyncing = 0;

	return IR_CONT;
}

} // End of namespace ICB
