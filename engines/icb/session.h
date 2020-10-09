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

#ifndef ICB_SESSION_H
#define ICB_SESSION_H

#include "engines/icb/p4_generic.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/common/px_game_object.h"
#include "engines/icb/common/px_walkarea_integer.h"
#include "engines/icb/barriers.h"
#include "engines/icb/player.h"
#include "engines/icb/map_marker.h"
#include "engines/icb/prim_route_builder.h"
#include "engines/icb/speech.h"
#include "engines/icb/text_sprites.h"
#include "engines/icb/string_vest.h"
#include "engines/icb/set.h"

namespace ICB {

enum __chi_think_mode { __FOLLOWING = 1, __LOST = 2, __NOTHING = 3 };

enum __chi_do_mode {
	__ROUTING,
	__INTERACT_FOLLOW,
	__PAUSING,
	__ANIMATE_TO_THINK,
	__ANIMATE_TO_FIGHT_HELP,
	__TURN_RND,
	__GET_WEAPON_OUT,
	__THINKING,
	__BUMBLING,
	__CHASING,
	__GO_CORD_GO,
	__FIGHT_HELP,
	__DISARM_TO_THINK,
	__TURN_TO_FACE_OBJECT
};

enum __barrier_result {
	__BLOCKED, // ran into a wall
	__OK, // moved forward
	__NUDGED,
	__DODGED, // player hit mega
	__CORRECTED // ran into wall but was at a shallow enough angle to be corrected
};

enum __alert { __ASTOOD, __ARUNNING, __AWALKING, __APUNCHING };

enum __rtype {
	__FULL, // normal route - start and end boxes
	__ENDB, // just end box
	__STARTB, // just our box
	__LASER // straight there - no barriers
};

#define MAX_extra_floors 12

class _floor_cam_list {
public:
	uint32 num_extra_floors;
	uint32 extra_floors[MAX_extra_floors];
};

#define MAX_player_history 10

typedef struct {
	uint32 id; // of floor or object
	PXreal first_x; // coords recorded as player walks onto floor - chi will route here first
	PXreal first_z;
	bool8 interaction; // is this item an interaction id or a floor number
	uint8 padding1;
	uint8 padding2;
	uint8 padding3;
} _history;

#define HISTORY_RUBBER (30 * REAL_ONE)
#define MAX_local_history 3
#define TIME_to_next_local_history (1 * 12)
typedef struct {
	PXreal x; // coords recorded as player walks around the floor
	PXreal z;
} _local_history;

#define NO_CAMERA_CHOSEN 0xfffffffe

#define MAX_session_objects MAX_props // number of game objects allowed per session (and their structs)
// THIS MUST BE A MULTIPLE OF 4
// SEE BARRIERS.H FOR MASTER DEFINITION

// test lower figure
#define MAX_voxel_list 24

// how many individual walkareas
#define MAX_was 32

#define MAX_missing_objects 16
#define MAX_missing_object_name_length 32

#define MAX_auto_interact 20

// fn-new-apply-bullet
#define ALWAYS_HIT_DIST (300 * REAL_ONE)

#define MAX_conveyors 10

#define MAX_footstepFloors 32

typedef struct {
	bool8 moving;
	PXreal x, y, z;
	PXreal x1, z1;
	PXreal xm, zm;
	uint8 pad1;
	uint8 pad2;

} _conveyor;

#define MAX_lift_platforms 10
typedef struct {
	uint32 id;
	PXreal x, y, z;
	PXreal x1, z1;

} _lift_platform;

#define MAX_stairs 32

typedef struct {
	_route_barrier bar;
	PXfloat pan, pan_ref, x, z;

	uint8 units;
	bool8 up;
	uint8 stair_id;
	bool8 is_stair;

	bool8 live;
	uint8 opposite_number; // as in stair id not object id
	uint8 pad2;
	uint8 pad3;

} _stair;

class _floor_world;
class _game_session {
public:
	_game_session(){};

	void ___init(const char *mission, const char *new_session_name);
	void ___destruct();

	void Pre_initialise_objects();
	void Init_objects();

	void One_logic_cycle();
	void Script_cycle();
	void Pre_logic_event_check();
	const char *Fetch_session_name();
	const char *Fetch_h_session_name(); // file name of #mission//#session
	const char *Fetch_session_h_name(); // file name of hash name of this session
	const char *Fetch_session_cluster();
	uint32 Fetch_session_cluster_hash();
	void Initialise_set(const char *name, const char *cluster_name);

	// Compute if mega's are on or off current camera and send events to them to say "ON_CAMERA" / "OFF_CAMERA"
	void UpdateOnOffCamera();

	inline PXcamera &GetCamera();
	inline int SetOK();
	inline void SetReset();

	void Stage_draw();
	void Stage_draw_poly();
	void Display_mega_times();

	void UpdateMegaFX();

	void Rebuild_working_buffers();
	void Render_3d_nicos();
	void Show_lit_unlit_diagnostics();

	int32 GetSelectedPropId();
	void GetSelectedPropRGB(u_char &r, u_char &g, u_char &b);
	bool8 IsPropSelected(const char *propName);

	int32 GetSelectedMegaId();
	void GetSelectedMegaRGB(u_char &r, u_char &g, u_char &b);

	uint32 Fetch_number_of_props();
	uint32 Fetch_number_of_objects();
	_logic *Fetch_object_struct(uint32 id);
	uint32 Fetch_cur_id();
	uint32 Fetch_script_var();
	_object_status Fetch_object_status(uint32 id);
	void Set_object_status(uint32 id, _object_status val);
	const char *Fetch_object_name(uint32 id);
	void Force_context_check(uint32 id);
	uint32 Fetch_named_objects_id(const char *name) const;
	inline uint32 Fetch_object_integer_variable(const char *pcName, const char *pcVar) const;

	uint32 Validate_prop_anim(const char *anim_name);

	uint32 Fetch_prop_state(char *prop_name);
	void Set_prop_state(char *name, uint32 value);

	void Script_version_check();

	bool8 Rig_test_mode();

	void Custom_simple_animator();
	void Custom_button_operated_door();
	void Custom_auto_door();

	void Reset_all_objects();

	void Process_player_floor_status();

	void Restart_player();

	bool8 Engine_start_interaction(const char *script, uint32 id);

	void Setup_prop_sleep_states();
	void Awaken_doors();

	// speech on psx

	bool8 can_save, prev_save_state;
	void Set_can_save(bool8 stat) {
		can_save = stat;
	}
	bool8 Can_save() {
		return can_save;
	}

	mcodeFunctionReturnCodes fn_test(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_create_mega(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_prop_animate(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_voxel_image_path(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_xyz(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_pan(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_to_floor(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_init_from_nico_file(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_init_from_marker_file(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_shut_down_object(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_context_chosen_logic(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_pause(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_face_coord(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_face_object(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_play_generic_anim(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_play_custom_anim(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_start_player_interaction(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_custom_prop_interact(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_generic_prop_interact(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_player_can_interact(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_player_cannot_interact(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_register_for_event(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_unregister_for_event(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_post_event(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_call_socket(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_register_object_for_event(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_unregister_object_for_event(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_register_for_line_of_sight(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_unregister_for_line_of_sight(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_register_object_for_line_of_sight(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_unregister_object_for_line_of_sight(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_post_future_event(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_post_repeating_event(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_prop_set_to_first_frame(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_prop_set_to_last_frame(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_can_see(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_can_object_see(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_prop_near_a_mega(int32 &, int32 *);
	mcodeFunctionReturnCodes socket_force_new_logic(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_weapon(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_request_speech(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_add_talker(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_issue_speech_request(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_confirm_requests(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_converse(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_anon_speech_invite(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_mega_field_of_view(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_apply_bullet(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_add_inventory_item(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_remove_inventory_item(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_initialise_inventory(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_is_carrying(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_reset_player(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_speak(int32 &, int32 *);
	mcodeFunctionReturnCodes speak_object_face_object(int32 &, int32 *);
	mcodeFunctionReturnCodes speak_play_generic_anim(int32 &, int32 *);
	mcodeFunctionReturnCodes speak_wait_for_everyone(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_get_weapon(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_rig_test(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_item_held(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_is_holding(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_drop(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_to_last_frame_generic_anim(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_play_common_xa_music(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_pass_flag_to_engine(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_init_mega_from_nico(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_remora_is_active(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_easy_play_generic_anim(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_teleport(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_get_pan_from_nico(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_teleport_to_nico(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_are_we_on_this_floor(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_is_object_on_our_floor(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_is_object_on_screen(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_is_object_dead(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_custom(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_message(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_message_var(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_get_state_flag(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_clear_all_events(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_check_event_waiting(int32 &, int32 *);
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
	mcodeFunctionReturnCodes fn_lift_process_list(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_add_object_name_to_list(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_add_object_id_to_list(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_post_named_event_to_object(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_route_to_nico(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_hold_while_list_near_nico(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_watch(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_get_persons_weapon(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_kill_me(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_kill_object(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_to_first_frame_custom_anim(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_to_last_frame_custom_anim(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_shut_down_event_processing(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_player(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_three_sixty_interact(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_route_to_marker(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_play_mission_xa_music(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_play_common_vag_music(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_play_mission_vag_music(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_near_list(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_get_list_result(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_event_check_last_sender(int32 &, int32 *);
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
	mcodeFunctionReturnCodes speak_add_chooser_icon(int32 &, int32 *);
	mcodeFunctionReturnCodes speak_user_chooser(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_is_there_interact_object(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_get_interact_object_id(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_is_object_interact_object(int32 &, int32 *);
	mcodeFunctionReturnCodes speak_chosen(int32 &, int32 *);
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
	mcodeFunctionReturnCodes fn_is_registered_for_event(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_is_object_registered_for_event(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_object_sight_range(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_object_sight_height(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_switch_on_the_really_neat_and_special_script_debugging_facility(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_switch_off_the_really_neat_and_special_script_debugging_facility(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_sound_heard_this(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_can_see_in_dark(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_mega_never_in_shadow(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_remora_new_menu_on_icon(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_remora_new_menu(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_no_logic(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_gosub(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_custom_simple_animator(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_tiny_route(int32 &, int32 *);
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
	mcodeFunctionReturnCodes fn_start_chi_following(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_inherit_prop_anim_height(int32 &, int32 *);
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
	mcodeFunctionReturnCodes fn_remora_mark_email_read(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_do_not_disturb(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_remora_check_email_id(int32 &, int32 *);
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
	mcodeFunctionReturnCodes speak_play_custom_anim(int32 &, int32 *);
	mcodeFunctionReturnCodes speak_set_custom(int32 &, int32 *);
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
	mcodeFunctionReturnCodes fn_start_conveyor(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_stop_conveyor(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_interact_with_id(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_face_nicos_pan(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_unregister_for_auto_interaction(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_register_chi(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_add_y(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_register_stairway(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_play_sfx_xyz(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_reverse_custom_anim(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_trace(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_PLEASE_REUSE_THIS_SLOT_2(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_PLEASE_REUSE_THIS_SLOT_3(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_player_has_weapon(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_align_with_floor(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_sound_new_entry(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_sound_remove_entry(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_sound_simulate(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_object_type(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_register_ladder(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_play_movie(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_is_an_object_crouching(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_play_sfx_offset(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_play_sfx_time(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_add_medipacks(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_use_medipacks(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_add_ammo_clips(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_use_ammo_clips(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_load_players_gun(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_flash_health(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_sound_get_x(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_sound_get_z(int32 &, int32 *);
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
	mcodeFunctionReturnCodes fn_is_player_striking(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_has_mega_our_height(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_chi_heard_gunshot(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_prime_custom_anim(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_preload_basics(int32 &, int32 *);
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
	mcodeFunctionReturnCodes fn_remora_blank_line(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_set_voice_over_colour(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_default_voice_over_colour(int32 &, int32 *);
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
	mcodeFunctionReturnCodes fn_snap_to_ladder_bottom(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_snap_to_ladder_top(int32 &, int32 *);

	mcodeFunctionReturnCodes fn_set_feet_to_pan(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_room_route(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_hard_load_generic_anim(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_hard_load_custom_anim(int32 &, int32 *);

	mcodeFunctionReturnCodes fn_face_camera(int32 &, int32 *);

	mcodeFunctionReturnCodes fn_activate_sparkle(int32 &, int32 *);
	mcodeFunctionReturnCodes fn_deactivate_sparkle(int32 &, int32 *);

	void Set_script(const char *script_name);
	void Context_check(uint32 script_name);
	void Shut_down_object();
	void Shut_down_object(const char *ascii);
	bool8 Console_shut_down_object(const char *name);
	void Console_shut_down_all_objects();
	void Console_shut_down_all_mega_objects();
	bool8 Free_object(const char *name);
	void Shut_down_id(uint32 id);
	void Set_objects_conversation_uid(uint32 id, uint32 uid);
	uint32 Return_objects_conversation_uid(uint32 id);
	void List_session_mega_objects();

	bool8 Call_socket(uint32 id, const char *script, int32 *retval);
	uint32 socket_id;
	c_game_object *socket_object;

	void Create_initial_route(__rtype type);

	bool8 Process_route();
	void Reset_route_manager();
	uint32 Animate_points(_route_description *route); // the mega character route animator
	void Calc_dist_and_pan(PXreal x, PXreal z, _route_description *route);

	void Calc_dist_and_target_pan(PXreal x, PXreal z, _route_description *route);
	bool8 Calc_target_pan(PXreal x, PXreal z, PXreal x2, PXreal z2);
	bool8 Calc_target_pan_no_bones(PXreal x, PXreal z, PXreal x2, PXreal z2);

	void Animate_turn_to_pan(__mega_set_names anim_type, uint32 speedup);

	void Advance_auto_pan();

	bool8 Is_route_required(PXreal startx, PXreal startz, PXreal destx, PXreal destz);

	bool8 Setup_route(int32 &result, int32 corex, int32 corez, int32 runs, __rtype type, int32 end_on_stand);
	_route_description *Fetch_route_desc(uint32 id);

	// primitive line router service object
	_prim_route_builder troute;

	mcodeFunctionReturnCodes Route_to_near_mega_core(const char *name, int32 run, int32 initial_turn, uint32 dist, int32 walk_to_run, int32 &result);

	void Start_new_router_game_cycle();
	void Set_router_busy();
	bool8 Is_router_busy();
	bool8 router_busy;

	char speech_conv_name[ENGINE_STRING_LEN];

	__conversation speech_info[1];

	uint32 total_convs;

	uint32 conv_focus; // which conversation has the view/listen focus

	uint32 conv; // number of currently processed conversation

	text_sprite *text_bloc; // render bloc for remora speech
	text_sprite *text_speech_bloc;

#define S speech_info[0]
#define CONV_ID 0

	uint32 menu_number;
	uint32 no_click_zone; // cant click past text until this reaches a certain number

	// The object ID of the currently highlighted prop
	int32 selected_prop_id;
	// The object ID of the currently highlighted mega
	int32 selected_mega_id;

	void Service_speech();
	void End_conversation(uint32 uid);
	void Exit_speech(uint32 id);
	void Render_speech(text_sprite *bloc);

	void Format_remora_text(const char *pcText, int32 nLineSpacing, int32 nCharSpacing, uint32 nMaxWidth);
	void Create_remora_text(uint32 x, uint32 y, const char *ascii,
	                        int32 margin, _pin_position pin_pos, int32 lineSpacing, int32 charSpacing,
	                        uint32 maxWidth,
	                        bool8 analysisAlreadyDone = FALSE8,
	                        int32 stopAtLine = -1);
	void Kill_remora_text();

	uint32 Fetch_no_barriers();

	uint32 Fetch_no_megas_barriers();
	uint32 Fetch_no_megas_nudge_barriers();

	uint32 Fetch_megas_barrier_number(uint32 barrier);
	uint32 Fetch_megas_nudge_barrier_number(uint32 barrier);

	uint32 External_fetch_no_megas_barriers(uint32 id);
	uint32 External_fetch_no_megas_nudge_barriers(uint32 id);
	uint32 External_fetch_no_megas_anim_barriers(uint32 id);
	uint32 External_fetch_megas_barrier(uint32 id, uint32 barrier);
	uint32 External_fetch_megas_nudge_barrier(uint32 id, uint32 barrier);

	PXfloat adjusted_pan;
	bool8 made_adjust;
	PXfloat normalAngle;
	__barrier_result Check_barrier_bump_and_bounce(PXreal newx, PXreal newy, PXreal newz, PXreal oldx, PXreal oldy, PXreal oldz, bool8 player);

	__barrier_result Check_this_barrier(_route_barrier *bar, PXreal newx, PXreal newz, PXreal oldx, PXreal oldz, PXreal bar_close, int *ignore);

	void Prepare_megas_route_barriers(bool8 player);
	void Prepare_megas_abarriers(uint32 slice_number, uint32 parent_number);
	void Fetch_mega_barriers_for_player();

	// animation
	bool8 Advance_frame_and_motion(__mega_set_names anim_type, bool8 player, uint8 nFrames);
	__barrier_result Core_advance(__mega_set_names anim_type, bool8 player, uint8 nFrames);
	bool8 Easy_frame_and_motion(__mega_set_names anim_type, bool8 player, uint8 nFrames);
	bool8 Easy_frame_motion_and_pan(__mega_set_names anim_type, bool8 player);
	bool8 Reverse_frame_and_motion(__mega_set_names anim_type, bool8 player, uint8 nFrames);
	__barrier_result Core_reverse(__mega_set_names anim_type, bool8 player, uint8 nFrames);

	void Normalise_anim_pc();

	void Soft_start_with_single_link(__mega_set_names link_anim, __mega_set_names next_anim);
	void Soft_start_with_double_link(__mega_set_names link_one, __mega_set_names link_two, __mega_set_names next_anim);
	void Soft_start_single_anim(__mega_set_names next_anim);
	int32 Soften_up_anim_file(__mega_set_names link, int32 diff);

	void Hard_start_single_anim(__mega_set_names next_anim);
	bool8 Play_anim();
	bool8 Play_reverse_anim();
	bool8 Play_anim_with_no_movement();
	bool8 fast_face_object(uint32 id, uint32 speed);
	bool8 Need_to_turn_to_face_object(uint32 id);
	bool8 fast_face_rnd(uint32 speed);

	bool8 Find_interact_marker_in_anim(__mega_set_names anim, PXreal *xoff, PXreal *zoff);
	bool8 Compute_target_interaction_coordinate(__mega_set_names anim, PXreal *destx, PXreal *destz);

	void Set_pose(__weapon weapon);
	void Set_motion(__motion motion);
	__motion Get_motion();
	void Change_pose_in_current_anim_set();

	bool8 Start_generic_ascii_anim(const char *ascii_name);
	__mega_set_names Fetch_generic_anim_from_ascii(const char *ascii_name);

	bool8 Fetch_cur_megas_armed_status();
	__weapon Fetch_cur_megas_pose();
	bool8 Fetch_custom();
	void Reset_cur_megas_custom_type();
	const char *Fetch_cur_megas_custom_text();
	uint32 Fetch_last_frame(__mega_set_names anim);

	bool8 speech_face_object(uint32 tar_id);

	mcodeFunctionReturnCodes Core_prop_interact(int32 &result, int32 *params, bool8 custom, bool8 coord_correction);

	uint32 Fetch_number_of_megas() const {
		return (number_of_voxel_ids);
	}
	uint32 Fetch_voxel_object_id(uint32 i) const {
		return (voxel_id_list[i]);
	}

	void Snap_to_ladder(_stair *lad, uint32 dist);
	void Set_init_voxel_floors();
	void Idle_manager();

	uint32 cur_camera_number;
	uint32 anchor_floor; // the floor number that the object was on when the camera changed - used for rubber-banding when moving off

	uint32 wa_number; // number of current wa camera
	PXreal wa_pin_x;
	PXreal wa_pin_y;
	PXreal wa_pin_z;

	char manual_camera_name[ENGINE_STRING_LEN]; // saved for save game

	uint32 num_cameras;
	char *camera_name_list[64];
	char *camera_cluster_list[64];

	uint32 floor_to_camera_index[MAX_floors];
	_floor_cam_list cam_floor_list[MAX_floors];

	void Build_camera_table();
	void Camera_director();
	void Reset_camera_director();
	bool8 Object_visible_to_camera(uint32 id);
	bool8 Process_wa_list();
	void Prepare_camera_floors();

	bool8 chi_interacts(int32 id, const char *script_name);
	__chi_think_mode fn_fetch_chi_mode();
	bool8 Make_floor_coordinate(PXreal *x, PXreal *z);
	bool8 Find_a_chi_target();
	void Chi_leaves_fight_mode();
	void Set_chi_permission();
	bool8 Process_chi();
	PXreal Cord_dist();

	_history history[MAX_player_history];
	uint32 cur_history; // players current position
	uint32 chi_history; // chi's current position
	uint32 pre_interact_floor; // floor we were on before going up stair or ladder -
	__chi_think_mode chi_think_mode;
	__chi_do_mode chi_do_mode;
	uint32 chi_id; // id of chi object
	uint32 chi_next_move; // counts down to chi's next move
	_local_history local_history[MAX_local_history];
	uint32 local_history_count; // how many moves this floor
	uint32 next_local;
	uint32 local_count_down;
	PXreal hist_pin_x;
	PXreal hist_pin_y;
	PXreal hist_pin_z;

	uint32 chi_target_id; // id of target
	bool8 chi_has_target; // has a valid target yes or no
	uint8 fight_pause; // time to next move
	bool8 permission_to_fire; // only after player has shot
	PXreal chi_catch_up_dist;
	PXreal chi_lost_dist;

	_local_history spectre_hist[MAX_floors];

	void Process_conveyors();
	_conveyor conveyors[MAX_conveyors];

	_lift_platform lifts[MAX_lift_platforms];
	uint8 num_lifts;

	void Draw_health_bar();
	uint32 health_time;

	bool8 alert_list[MAX_voxel_list];
	void Process_guard_alert(__alert alert_type);
	void Reset_guard_alert();
	void Signal_to_guards();
	void Signal_to_other_guards();

	_player_stat player_stat_was; // asyncer remembers previous player state - ready for change
	_player_stat player_stat_use; // use this which has skipped the linking status
	void Service_generic_async();
	bool8 Route_async_ready(bool8 run, bool8 turn);

	bool8 init_asyncs;
	uint32 cur_sync_set;
	bool8 caching;
	_vox_image sync_set[4];

	void Async_helper();
	int async_counter;
	int async_off;

	uint32 num_stairs;
	_stair stairs[MAX_stairs];
	uint32 Register_stair_or_ladder(const char *target, bool8 top, uint32 length, bool8 stair, uint32 stepcms);

	void Set_state_of_stair_or_ladder(uint32 nIndex, bool8 bState);

	uint32 prop_state_table[MAX_session_objects]; // pointer to pc's of props for current session

	_set set; // the set object

	// floor definitions
	_floor_world *floor_def; // pointer to a _floor_world object
	// floor viewer and room router
	// route barriers
	_barrier_handler *session_barriers; // pointer to _barrier_handler object - loads file so must be pointer

	// game object stuff - objects.linked
	_linked_data_file *objects;
	uint32 total_objects; // number of objects in the objects.object file - pulled out at session start for convenience
	_logic *logic_structs[MAX_session_objects]; // pointers to current sessions logic structs
	uint32 num_megas; // keeps a running total of megas initialised - used when assigning megas structures
	uint32 num_vox_images; // as above but for vox_images - in theory these 2 counters are the same thing but that would be an assumption too far tbh

	// and the scripts.linked file
	_linked_data_file *scripts;

	// handles player object and user interface
	_player player;
	PXreal prop_interact_dist; // distance to prop player is going to interact with - needed by sony door functions

	// map markers
	_marker markers;

	// initial map tag positions of props, people, etc.
	_linked_data_file *features;

	// ascii speech text
	_linked_data_file *text;

	// list of auto interact objects
	uint8 auto_interact_list[MAX_auto_interact];

	// speech text font
	char speech_font_one[ENGINE_STRING_LEN];
	char remora_font[ENGINE_STRING_LEN];
	uint32 speech_font_one_hash; // the hash value of the speech_font_one string
	uint32 remora_font_hash;     // the hash value of the speech_font_one string

	// prop animations
	_linked_data_file *prop_anims; // prop anims are loaded into the special private_session_resman

	// make los_timing be global so we can print it at a global level
	uint32 los_time;

	uint32 total_was; // how many individual walk-areas
	_linked_data_file *walk_areas;
	const __aWalkArea *wa_list[MAX_was]; // walk areas

	bool8 manual_camera; // overiden by a script command
	bool8 camera_lock; // lock camera director - used to fix ladder problems (easier to fix like this than at animation end)
	bool8 wa_camera; // currently using a wa camera
	bool8 wa_tied_to_pin;
	bool8 wa_tied_to_exit_pin;
	bool8 is_there_a_chi; // yes or no
	bool8 first_session_cycle;

private:
	char session_name[ENGINE_STRING_LEN];
	char h_session_name[ENGINE_STRING_LEN];
	char session_cluster[ENGINE_STRING_LEN];

	uint32 session_cluster_hash; // the hash value of the session_cluster string
	char session_h_name[8];      // filename equivalent of the hash'ed version of session

	uint32 total_props; // holds number of props in current session

	// speech

	// logic cycle stuff
	// all these are of course available to fn_functions and associated engine modules
	uint32 cur_id; // number of current object
	_logic *L; // current objects logic structure
	_vox_image *I; // pointer to current objects _voxel_image structure - megas only
	_mega *M; // pointer to current objects _mega struct - megas only
	c_game_object *object; // represents the current game object at logic run time
	uint32 script_var_value; // holds script variables passed back via fn_pass_flag_to_engine

	// list of ids that are voxel characters - built per game cycle
	uint8 voxel_id_list[MAX_voxel_list];
	uint32 number_of_voxel_ids; // kept for safety

	// prop state dummy object vars
	uint32 number_of_missing_objects;
	char missing_obs[MAX_missing_objects][MAX_missing_object_name_length];
	uint8 missing_ob_prop_states[MAX_missing_objects];

	// FOOTSTEPS for mega:
	void UpdateFootstep();

	// cartridge case for mega
	void UpdateCartridgeCase();

	uint32 defaultFootSfx;
	uint32 specialFootSfx;
	uint32 ladderFootSfx;
	uint32 floorFootSfx[MAX_footstepFloors][2]; // gives number of floor (0) and hash of sfx to use (1)...
	int32 numFloorFootSfx;

	// Block use of the copy constructor and the default '='.
	_game_session(const _game_session &);
	void operator=(const _game_session &) { ; }
};

inline uint32 _game_session::Fetch_script_var() {
	return (script_var_value);
}

inline uint32 _game_session::Fetch_number_of_props() {
	return (total_props);
}

inline uint32 _game_session::Fetch_number_of_objects() {
	return (total_objects);
}

inline _logic *_game_session::Fetch_object_struct(uint32 id) {
	_ASSERT(id < total_objects);

	return (logic_structs[id]);
}

inline void _game_session::Force_context_check(uint32 id) {
	_ASSERT(id < total_objects);

	logic_structs[id]->context_request = TRUE8;
}

inline _object_status _game_session::Fetch_object_status(uint32 id) {
	_ASSERT(id < total_objects);

	return (logic_structs[id]->ob_status);
}

inline void _game_session::Set_object_status(uint32 id, _object_status val) {
	_ASSERT(id < total_objects);

	logic_structs[id]->ob_status = val;
}

inline const char *_game_session::Fetch_object_name(uint32 id) {
	_ASSERT(id < total_objects);

	return ((const char *)(logic_structs[id]->GetName()));
}

inline uint32 _game_session::Fetch_cur_id() {
	return (cur_id);
}

inline uint32 _game_session::Fetch_no_barriers() {
	// return number of current player barriers
	return (L->mega->number_of_barriers);
}

inline uint32 _game_session::Fetch_no_megas_barriers() {
	// return number of current player barriers
	return (M->number_of_barriers);
}

inline uint32 _game_session::Fetch_no_megas_nudge_barriers() {
	// return number of current player barriers
	return (M->number_of_nudge);
}

inline uint32 _game_session::External_fetch_no_megas_barriers(uint32 id) {
	// return number of current barriers
	// we are not in logic loop so cant rely on M, I, L etc.

	return (logic_structs[id]->mega->number_of_barriers);
}

inline uint32 _game_session::External_fetch_no_megas_anim_barriers(uint32 id) {
	// return number of current barriers
	// we are not in logic loop so cant rely on M, I, L etc.

	return (logic_structs[id]->mega->number_of_animating);
}

inline uint32 _game_session::External_fetch_no_megas_nudge_barriers(uint32 id) {
	// return number of current special player nudgebarriers
	// we are not in logic loop so cant rely on M, I, L etc.

	return (logic_structs[id]->mega->number_of_nudge);
}

inline uint32 _game_session::Fetch_megas_barrier_number(uint32 barrier) {
	// can only be called within logic loop

	_ASSERT(barrier < M->number_of_barriers + M->number_of_animating);

	return M->barrier_list[barrier];
}

inline uint32 _game_session::Fetch_megas_nudge_barrier_number(uint32 barrier) {
	// can only be called within logic loop

	_ASSERT(barrier < M->number_of_nudge);

	return M->nudge_list[barrier];
}

inline uint32 _game_session::External_fetch_megas_barrier(uint32 id, uint32 barrier) {
	// can only be called within logic loop
	_ASSERT(barrier < (logic_structs[id]->mega->number_of_barriers) + (logic_structs[id]->mega->number_of_animating));

	return logic_structs[id]->mega->barrier_list[barrier];
}

inline uint32 _game_session::External_fetch_megas_nudge_barrier(uint32 id, uint32 barrier) {
	// can only be called within logic loop
	_ASSERT(barrier < logic_structs[id]->mega->number_of_nudge);

	return logic_structs[id]->mega->nudge_list[barrier];
}

inline void _game_session::Set_objects_conversation_uid(uint32 id, uint32 uid) {
	if (id >= total_objects)
		Fatal_error("Object id %d out-of-range (total_objects=%d)", id, total_objects);

	logic_structs[id]->conversation_uid = uid;
}

inline uint32 _game_session::Return_objects_conversation_uid(uint32 id) {
	_ASSERT(id < total_objects);

	return (logic_structs[id]->conversation_uid);
}

inline const char *_game_session::Fetch_session_name() {
	return ((const char *)session_name);
}

inline const char *_game_session::Fetch_session_cluster() {
	return ((const char *)session_cluster);
}

inline const char *_game_session::Fetch_session_h_name() {
	return ((const char *)session_h_name);
}

inline const char *_game_session::Fetch_h_session_name() {
	return ((const char *)h_session_name);
}

inline uint32 _game_session::Fetch_session_cluster_hash() {
	return (session_cluster_hash);
}

inline uint32 _game_session::Fetch_object_integer_variable(const char *pcName, const char *pcVar) const {
	int32 nVariableNumber;
	c_game_object *pGameObject;

	// Get the object itself.
	pGameObject = (c_game_object *)objects->Fetch_item_by_name(pcName);

	if (!pGameObject)
		Fatal_error("_game_session::Fetch_object_integer_variable( %s, %s ) couldn't find object", pcName, pcVar);

	// Find the position of the requested variable.
	nVariableNumber = pGameObject->GetVariable(pcVar);

	if (nVariableNumber == -1)
		Fatal_error("_game_session::Fetch_object_integer_variable( %s, %s ) couldn't find variable", pcName, pcVar);

	// Get the lvar.
	return (pGameObject->GetIntegerVariable(nVariableNumber));
}

inline PXcamera &_game_session::GetCamera() { return set.GetCamera(); }

inline int _game_session::SetOK() { return set.OK(); }

inline void _game_session::SetReset() { set.Reset(); }

} // End of namespace ICB

#endif
