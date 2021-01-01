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

#include "engines/icb/p4.h" //for machine version
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/mission.h"
#include "engines/icb/session.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/debug.h"
#include "engines/icb/player.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/barriers.h"
#include "engines/icb/common/px_route_barriers.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/animation_mega_set.h"
#include "engines/icb/mission.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/icb.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/res_man.h"
#include "engines/icb/sound.h"
#include "engines/icb/floors.h"
#include "engines/icb/remora.h"

#include "common/textconsole.h"

namespace ICB {

mcodeFunctionReturnCodes fn_player(int32 &result, int32 *params) { return (MS->fn_player(result, params)); }
mcodeFunctionReturnCodes fn_prime_player_history(int32 &result, int32 *params) { return (MS->fn_prime_player_history(result, params)); }
mcodeFunctionReturnCodes fn_is_id_the_player(int32 &result, int32 *params) { return (MS->fn_is_id_the_player(result, params)); }
mcodeFunctionReturnCodes fn_push_player_status(int32 &result, int32 *params) { return (MS->fn_push_player_status(result, params)); }
mcodeFunctionReturnCodes fn_pop_player_status(int32 &result, int32 *params) { return (MS->fn_pop_player_status(result, params)); }
mcodeFunctionReturnCodes fn_post_third_party_speech(int32 &result, int32 *params) { return (MS->fn_post_third_party_speech(result, params)); }
mcodeFunctionReturnCodes fn_set_player_has_weapon(int32 &result, int32 *params) { return (MS->fn_set_player_has_weapon(result, params)); }
mcodeFunctionReturnCodes fn_can_hear_players_feet(int32 &result, int32 *params) { return (MS->fn_can_hear_players_feet(result, params)); }
mcodeFunctionReturnCodes fn_is_player_striking(int32 &result, int32 *params) { return (MS->fn_is_player_striking(result, params)); }

// pc device switch
extern __pc_input device;

int armedChangesMode = 0;
int animToggles = 0;
int CAMERA_SMOOTH_CYCLES = 12;

#define CORRECT_PAN                                                                                                                                                                \
	if (log->pan >= HALF_TURN)                                                                                                                                                 \
		log->pan -= FULL_TURN;                                                                                                                                             \
	else if (log->pan <= -HALF_TURN)                                                                                                                                           \
		log->pan += FULL_TURN;

void _player::___init() {
	Zdebug("constructing _player");

	player_exists = FALSE8;
	haveCamera = 0;
	armedChangesMode = 0;
	hunting = 0;
	lastCameraPan = 3 * FULL_TURN; // a placeholder to say never set
	deltaCameraPan = ZERO_TURN;
	;
	interact_selected = FALSE8;
	cur_interact_id = (uint)-1;

	look_at_selected = 0;

	aim_turn_amount = (FULL_TURN * 2) / 100;        // 2% of a turn per game cycle
	crouch_turn_amount = (FULL_TURN * 1) / 100;     // 1% of a turn per game cycle
	stood_turn_amount = (FULL_TURN * 1) / 100;      // 1% of a turn per game cycle
	stood_fast_turn_amount = (FULL_TURN * 7) / 100; // 7% of a turn per game cycle
	walk_turn_amount = (FULL_TURN * 2 / 100);       // 2% of a turn per game cycle
	run_turn_amount = (FULL_TURN * 4 / 100);        // 2% of a turn per game cycle

	// push it twice to fill up the stack : just in case someone does a pop before a push
	// Push_control_mode( ACTOR_RELATIVE );
	// Push_control_mode( ACTOR_RELATIVE );

	being_shot = (int8)0;

	look_at_id = 0;
	MS->init_asyncs = FALSE8; // not inited the 4 async vox image structs
}

__mode_return _player::Player_press_inv_button() {
	// check if the player has pressed the INVENTORY button

	// return
	//				__FINISHED_THIS_CYCLE, or
	//				__MORE_THIS_CYCLE

	if ((cur_state.IsButtonSet(__INVENTORY)) && (!inv_lock)) {

		// Don't bring up the inventory if there is nothing in it.
		{
			// This brings up the inventory.
			if ((player_status == NEW_AIM) || (player_status == CROUCH_AIM)) {
			} else {
				g_oIconListManager->ActivateIconMenu(ICON_LIST_INVENTORY, TRUE8, TRUE8);
			}

			g_oIconListManager->CycleInventoryLogic(cur_state);

			inv_lock = TRUE8;

			Push_control_mode(ACTOR_RELATIVE);
			Push_player_stat();
			Set_player_status(INVENTORY);
		}

		return (__FINISHED_THIS_CYCLE);
	} else {
		// Stopped inventory dropping in repeatedly when you hold down the key.
		if (!(cur_state.IsButtonSet(__INVENTORY))) {
			inv_lock = FALSE8; // released
		}
	}

	return (__MORE_THIS_CYCLE);
}

__mode_return _player::Player_press_remora_button() {
	// check if the player has pressed the FIRE button
	// if so see if there's a current interact object and if so setup the interaction

	// return
	//				__FINISHED_THIS_CYCLE, or
	//				__MORE_THIS_CYCLE

	if ((cur_state.IsButtonSet(__REMORA)) && (!remora_lock)) {
		Push_player_stat();
		Set_player_status(REMORA);
		remora_lock = TRUE8;
		interact_lock = TRUE8; // avoid problems coming out
		Push_control_mode(ACTOR_RELATIVE);
		MS->Awaken_doors(); // sleeping doors come alive while in remora display!

		// This sets a flag which the Remora will pick up next cycle.
		g_oRemora->ActivateRemora(_remora::MOTION_SCAN);
		g_oRemora->CycleRemoraLogic(cur_state);
		return (__FINISHED_THIS_CYCLE);
	} else if (!(cur_state.IsButtonSet(__REMORA)))
		remora_lock = FALSE8;

	return (__MORE_THIS_CYCLE);
}

__mode_return _player::Player_press_fire_button() {
	// check if the player has pressed the FIRE button
	// if so see if there's a current interact object and if so setup the interaction

	// return
	//				__FINISHED_THIS_CYCLE, or
	//				__MORE_THIS_CYCLE

	bool8 res;
	int32 retval;

	// check for interact button
	if ((being_shot == 0) && (cur_state.IsButtonSet(__ATTACK)) && (!fire_lock) && (GetNoBullets())) {
		// cant shoot at non evils
		if ((interact_selected) && (!MS->logic_structs[cur_interact_id]->mega->is_evil)) {
			if (!MS->Engine_start_interaction("non_evil_interact", cur_interact_id))
				return __MORE_THIS_CYCLE;

			fire_lock = TRUE8; // switch the lock on

			// do this as exit conversation will pop the player status
			if (MS->logic_structs[Fetch_player_id()]->mega->Fetch_armed_status())
				Set_player_status(NEW_AIM);
			else
				Set_player_status(STOOD);
			Push_player_stat();

			return (__FINISHED_THIS_CYCLE);
		}

		fire_lock = TRUE8; // switch the lock on

		// check for special script content overide
		if ((interact_selected) && (MS->logic_structs[cur_interact_id]->mega))
			if (MS->logic_structs[cur_interact_id]->mega->use_fire_script) {
				res = MS->Call_socket(cur_interact_id, "shoot", &retval);
				if (!res)
					Fatal_error("shoot script missing");

				// do this as exit conversation will pop the player status
				if (MS->logic_structs[Fetch_player_id()]->mega->Fetch_armed_status())
					Set_player_status(NEW_AIM);
				else
					Set_player_status(STOOD);
				Push_player_stat();

				return (__FINISHED_THIS_CYCLE);
			}

		// play gun sound
		if (MS->logic_structs[player_id]->sfxVars[GUNSHOT_SFX_VAR] != 0)
			RegisterSound(player_id, NULL, MS->logic_structs[player_id]->sfxVars[GUNSHOT_SFX_VAR], gunDesc,
			              (int8)127); // have to use full version so we can give hash instead of string
		else
			RegisterSound(player_id, defaultGunSfx, gunDesc); // use small version as we have string not hash

		// dynamic light
		log->mega->SetDynamicLight(1, 255, 255, 255, 0, 150, 100, 200); // 2m rgb=WHITE

		// Hey we are shooting
		log->mega->is_shooting = TRUE8;

		UseBullets(1); // used another bullet

		// if target and target alive then stick a bullet in them
		if ((interact_selected) && (MS->Call_socket(cur_interact_id, "give_state", &retval))) {
			if (!retval) {
				//      try to fetch the object
				MS->socket_object = (c_game_object *)MS->objects->Fetch_item_by_number(cur_interact_id);

				res = MS->Call_socket(cur_interact_id, "gun_shot", &retval);

				MS->Set_chi_permission(); // if chi's around she gets permission to start shooting

				if (!res)
					Tdebug("gun_shot_errors.txt", "no [%s] for object [%s]", "gun_shot", MS->socket_object->GetName());
			}
		} else {
			// no hit play ricochet sound
			if (MS->logic_structs[player_id]->sfxVars[RICOCHET_SFX_VAR] != 0)
				RegisterSound(player_id, NULL, MS->logic_structs[player_id]->sfxVars[RICOCHET_SFX_VAR], ricochetDesc,
				              (int8)127); // have to use full version so we can give hash instead of string
			else
				RegisterSound(player_id, defaultRicochetSfx, ricochetDesc); // use small version as we have string not hash

			// now, we hit nothing, but if chi cant see us then set her permission
			if (!g_oLineOfSight->LineOfSight(MS->chi_id, Fetch_player_id()))
				MS->Set_chi_permission(); // if chi's around she gets permission to start shooting
		}

		// back to stand aim or crouch aim
		if (player_status == NEW_AIM)
			Hard_start_new_mode(NEW_AIM, __STAND_AND_SHOOT);
		else
			Hard_start_new_mode(CROUCH_AIM, __STAND_AND_SHOOT);

		return (__FINISHED_THIS_CYCLE);
	} else if ((cur_state.IsButtonSet(__ATTACK)) && (!fire_lock) && (!GetNoBullets())) {
		// would have fired but we have no bullets
		// if we have clips then reload

		if (GetNoAmmoClips()) {
			// reload
			UseAmmoClips(1); // use a clip

			int32 bull_per_clip = GetBulletsPerClip();

			SetBullets(bull_per_clip); // reload

			if (player_status == NEW_AIM)
				Hard_start_new_mode(NEW_AIM, __LOAD_GUN);
			else
				Hard_start_new_mode(CROUCH_AIM, __LOAD_GUN);

			return (__FINISHED_THIS_CYCLE);
		}
	}

	if (!cur_state.IsButtonSet(__ATTACK)) // release the lock
		fire_lock = FALSE8; // let go

	return (__MORE_THIS_CYCLE);
}

__mode_return _player::Player_press_strike_button() {
	// check if the player has pressed the FIRE button when stood unarmed - to punch

	// return
	//				__FINISHED_THIS_CYCLE, or
	//				__MORE_THIS_CYCLE

	bool8 ret;
	int32 retval;

	// check for interact button
	if ((cur_state.IsButtonSet(__ATTACK)) && (!fire_lock)) {

		fire_lock = TRUE8; // switch the lock on

		// physically cant punch chi or no evils
		if ((interact_selected) && (MS->logic_structs[cur_interact_id]->image_type == VOXEL) && (!MS->logic_structs[cur_interact_id]->mega->is_evil)) {
			PXreal sub1, sub2, dist;

			sub1 = MS->logic_structs[cur_interact_id]->mega->actor_xyz.x - log->mega->actor_xyz.x;
			sub2 = MS->logic_structs[cur_interact_id]->mega->actor_xyz.z - log->mega->actor_xyz.z;

			dist = ((sub1 * sub1) + (sub2 * sub2));

			if (dist < (PXreal(200 * 200)))
				return (__FINISHED_THIS_CYCLE);
		}

		// check for special script content overide
		if ((interact_selected) && (MS->logic_structs[cur_interact_id]->mega))
			if (MS->logic_structs[cur_interact_id]->mega->use_strike_script) {
				if (MS->logic_structs[cur_interact_id]->mega->Is_crouched())
					return __MORE_THIS_CYCLE;

				ret = MS->Call_socket(cur_interact_id, "strike", &retval);
				if (!ret)
					Fatal_error("strike script missing");

				return (__FINISHED_THIS_CYCLE);
			}

		// can be stood or crouched, unarmed
		if (player_status == CROUCHING) {
			Hard_start_new_mode(CROUCH_TO_PUNCH, __STAND_CROUCHED_TO_STAND);
		} else {
			//      link anim has been requested - the link exists
			log->anim_pc = 0; // start at hard 0

			//      set to linking mode
			player_status = STRIKING;

			//      set anim type to link
			log->cur_anim_type = __STRIKE;

			//			dales tweeky low strike thing
			if ((interact_selected) && (MS->logic_structs[cur_interact_id]->image_type == VOXEL)) {
				if (!(g_icb->getRandomSource()->getRandomNumber(10 - 1)))
					log->cur_anim_type = __LOW_STRIKE;
			}
		}
		return (__FINISHED_THIS_CYCLE);
	}

	if (!cur_state.IsButtonSet(__ATTACK)) // release the lock
		fire_lock = FALSE8; // let go

	return (__MORE_THIS_CYCLE);
}

mcodeFunctionReturnCodes _game_session::fn_prime_player_history(int32 &, int32 *) {
	// put the first entry in the history list

	_floor *floor;

	history[cur_history].interaction = FALSE8;
	history[cur_history].id =
	    floor_def->Locate_floor_rect(logic_structs[cur_id]->mega->actor_xyz.x, logic_structs[cur_id]->mega->actor_xyz.z, logic_structs[cur_id]->mega->actor_xyz.y, &floor);

	logic_structs[cur_id]->owner_floor_rect = history[cur_history].id; // gotta set this for first fn_player
	pre_interact_floor = history[cur_history].id;

	if (history[cur_history].id == PXNULL)
		Message_box("fn_prime_player_history hasnt got a legal coordinate from player?");

	Tdebug("history.txt", ">> %d", history[cur_history].id);

	// prime local position stuff
	hist_pin_x = logic_structs[cur_id]->mega->actor_xyz.x;
	hist_pin_z = logic_structs[cur_id]->mega->actor_xyz.z;

	local_history_count = 0; // total points recorded in this room
	next_local = 0; // place in array for next
	local_count_down = TIME_to_next_local_history;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_is_id_the_player(int32 &result, int32 *params) {
	// returns true or false if the passed id is the player

	// params        0   id of object

	if (params[0] == (int32)player.Fetch_player_id()) {
		result = TRUE8;
		return IR_CONT;
	}

	result = FALSE8;
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_push_player_status(int32 &, int32 *) {
	// reset player to either stood or stood armed
	if (logic_structs[player.Fetch_player_id()]->mega->Is_crouched())
		player.Set_player_status(CROUCHING);
	else if (logic_structs[player.Fetch_player_id()]->mega->Fetch_armed_status())
		player.Set_player_status(NEW_AIM);
	else
		player.Set_player_status(STOOD);

	player.Push_player_stat();

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_pop_player_status(int32 &, int32 *) {
	player.Pop_player_stat();

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_post_third_party_speech(int32 &, int32 *) {
	// called after player has been grabbed by a non player started conversation

	player.Pop_player_stat();

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_player_has_weapon(int32 &, int32 *params) {
	// set the has gun flag

	// params    0   yes/no

	player.has_weapon = (bool8)(params[0]);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_player(int32 &, int32 *) {
	mcodeFunctionReturnCodes ret;
	PXreal sub1, sub2, len;

	// if there is a set watch to someone other than player do no player processing
	if ((g_mission->camera_follow_id_overide) && (player.Fetch_player_id() != g_mission->camera_follow_id_overide))
		return IR_REPEAT;

	if (!L->looping) {
		L->anim_pc = 0; // reset for when we are coming straight back in after a context re-run has changed the logic unexpectedly
		L->looping = TRUE8;
		M->cur_parent = 0; // force a reset - if players routes first cycle then the player barriers are never brought in
	}

	// run player user, control and animation logic
	ret = player.Gateway();

	// find current intereact object
	player.Find_current_player_interact_object();

	//	set floor rect value - used by stage draw to find indexed camera name
	floor_def->Set_floor_rect_flag(L);

	// build the players movement list
	// add new entry if cur entry is an interaction, or, floor has changed
	if ((L->owner_floor_rect != pre_interact_floor) && (floor_def->On_a_floor(M)) && (prev_save_state)) {

		//		changed floor
		cur_history++;
		if (cur_history == MAX_player_history)
			cur_history = 0; // wrap

		history[cur_history].interaction = FALSE8;
		history[cur_history].id = L->owner_floor_rect; // record new floor id
		history[cur_history].first_x = M->actor_xyz.x;
		history[cur_history].first_z = M->actor_xyz.z;

		// spectre history
		spectre_hist[pre_interact_floor].x = hist_pin_x;
		spectre_hist[pre_interact_floor].z = hist_pin_z;

		hist_pin_x = M->actor_xyz.x;
		hist_pin_y = M->actor_xyz.y;
		hist_pin_z = M->actor_xyz.z;

		pre_interact_floor = L->owner_floor_rect; // last floor
		local_history_count = 0; // set to 0 points in this room - but, record the coordinate for pin purposes
		next_local = 0; // place in array for next
		local_count_down = TIME_to_next_local_history;

		Tdebug("history.txt", "> %d", L->owner_floor_rect);
	}

	if ((!local_count_down) && (prev_save_state)) {
		//		ok, we're pinned to last spot
		//		unless we've moved significantly then we dont record another position

		sub1 = (PXreal)M->actor_xyz.x - hist_pin_x;
		sub2 = (PXreal)M->actor_xyz.z - hist_pin_z;

		//		dist
		len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

		if ((floor_def->On_a_floor(M)) && (len > (PXreal)(HISTORY_RUBBER * HISTORY_RUBBER))) {

			//			Message_box("writing %3.1f, %3.1f, next_local=%d, local_history_count %d", M->actor_xyz.x, M->actor_xyz.z, next_local,
			//local_history_count);
			//			record coordinate
			local_history[next_local].x = M->actor_xyz.x;
			local_history[next_local].z = M->actor_xyz.z;

			hist_pin_x = M->actor_xyz.x;
			hist_pin_y = M->actor_xyz.y;
			hist_pin_z = M->actor_xyz.z;

			//			loop forever overwriting earlier entries as we move around
			next_local++;
			if (next_local == MAX_local_history)
				next_local = 0;

			if (local_history_count < MAX_local_history)
				local_history_count++;
		}

		local_count_down = TIME_to_next_local_history;
	} else {

		if (player.player_status == WALKING)
			local_count_down--;

		if ((player.player_status == RUNNING) && (local_count_down))
			local_count_down--;
	}

	// reset for a cycle
	player.stood_on_lift = 0; // clear stood on lift platform flag

	return (ret);
}

mcodeFunctionReturnCodes _player::Gateway() {
	// top level call for player interface

	// basically,

	// fetch player object

	// check for events that affect the player object

	// see what we're doing
	//	see what player is doing

	// Jake 15/2/98 : set the default
	__mode_return ret = __FINISHED_THIS_CYCLE;
	int32 bull_per_clip;
	c_game_object *ob;

	// Set the player control mode correctly
	switch (px.display_mode) {
	case NETHACK:
	case TEMP_NETHACK: {
		// Set the player into ACTOR_RELATIVE mode for nethack modes
		Set_control_mode(ACTOR_RELATIVE);
	}
	case THREED: {}
	}

	// when not in menu put into user selected control mode
	// the idea is that this is a lot more secure than the previous push/pop scheme
	if ((player_status != INVENTORY) && (player_status != REMORA))
		focus_mode = master_mode;

	// check keys/pads/etc. to see what the user is trying to do
	Update_input_state();

	do {
		switch (player_status) {
		//      not all of these will be user controllable - like being blown up, etc.

		case STOOD:
			ret = Player_interact();
			if (ret == __FINISHED_THIS_CYCLE)
				return (IR_TERMINATE); // set to go around again and do a cycle of the interact script

			ret = Player_press_strike_button();
			if (ret == __FINISHED_THIS_CYCLE)
				return (IR_TERMINATE); // set to go around again and do a cycle of the interact script

			// no interact so keep going as normal
			ret = Player_stood();
			break;

		case WALKING:
			ret = Player_interact();
			// check if interact has set up a new script to run on level 2
			if (ret == __FINISHED_THIS_CYCLE)
				return (IR_TERMINATE); // set to go around again and do a cycle of the interact script
			// no interact so keep going as normal

			ret = Player_press_strike_button();
			if (ret == __FINISHED_THIS_CYCLE)
				return (IR_TERMINATE); // set to go around again and do a cycle of the interact script

			ret = Player_walking();

			break;

		case CROUCH_WALK:
			ret = Player_crouch_walk();
			break;

		case NEW_AIM:
			ret = Player_interact(); // check if interact has set up a new script to run on level 2
			if (ret == __FINISHED_THIS_CYCLE)
				return (IR_TERMINATE); // set to go around again and do a cycle of the interact script

			ret = Player_press_fire_button();
			if (ret == __FINISHED_THIS_CYCLE)
				return (IR_REPEAT); //

			ret = Player_new_aim();
			break;

		case CROUCH_AIM:
			ret = Player_press_fire_button();
			if (ret == __FINISHED_THIS_CYCLE)
				return (IR_REPEAT); //

			ret = Player_crouch_aim();
			break;

		case PUTTING_AWAY_GUN: // going from armed with gun to unarmed
			MS->Set_pose(__NOT_ARMED);
			MS->Change_pose_in_current_anim_set();
			if (armedChangesMode == 1)
				Pop_control_mode();
			Hard_start_new_mode(STOOD, __STAND);
			return (IR_STOP);

		case PUTTING_AWAY_CROUCH_GUN: // going from armed with gun to unarmed
			MS->Set_pose(__CROUCH_NOT_ARMED);
			MS->Change_pose_in_current_anim_set();
			if (armedChangesMode == 1)
				Push_control_mode(ACTOR_RELATIVE);
			Hard_start_new_mode(CROUCHING, __STAND);
			return (IR_REPEAT);

		case RUNNING:
			ret = Player_press_strike_button();
			if (ret == __FINISHED_THIS_CYCLE)
				return (IR_TERMINATE); // set to go around again and do a cycle of the interact script

			ret = Player_running();
			break;

		case CROUCH_TO_STAND_UNARMED: // crouch to stood unarmed
			MS->Set_pose(__NOT_ARMED);
			MS->Change_pose_in_current_anim_set();
			Hard_start_new_mode(STOOD, __STAND);
			return (IR_STOP);

		case CROUCH_TO_PUNCH:
			MS->Set_pose(__NOT_ARMED);
			MS->Change_pose_in_current_anim_set();
			//      link anim has been requested - the link exists
			log->anim_pc = 0; // start at hard 0

			//      set to linking mode
			player_status = STRIKING;

			//      set anim type to link
			log->cur_anim_type = __STRIKE;

			return (IR_STOP);

		case CROUCH_TO_STAND_ARMED: // crouch to stood armed
			MS->Set_pose(__GUN);
			MS->Change_pose_in_current_anim_set();
			Hard_start_new_mode(NEW_AIM, __STAND);

			return (IR_REPEAT);

		case CROUCHING:
			ret = Player_interact(); // check if interact has set up a new script to run on level 2
			if (ret == __FINISHED_THIS_CYCLE)
				return (IR_TERMINATE); // set to go around again and do a cycle of the interact script

			ret = Player_press_strike_button();
			if (ret == __FINISHED_THIS_CYCLE)
				return (IR_TERMINATE); // set to go around again and do a cycle of the interact script

			ret = Player_crouching();
			if (ret == __FINISHED_THIS_CYCLE)
				return (IR_REPEAT); //

			break;

		case INVENTORY:

			// Cycle the inventory's logic.
			g_oIconListManager->CycleInventoryLogic(cur_state);

			// If the Remora is now shut down, we need to return to player-standing mode.
			if (!g_oIconListManager->IsActive()) {

				if (g_oIconListManager->IsHolding()) {

					// reload gun
					if ((g_oIconListManager->Holding(ARMS_AMMO_NAME)) && (player_status == STOOD)) {
						interact_lock = TRUE8;
						g_oIconListManager->Drop();

						// Make sure number of clips is > 0
						bull_per_clip = GetBulletsPerClip();

						if ((has_weapon) && GetNoAmmoClips() && (GetNoBullets() < bull_per_clip)) {
							if (previous_stat == CROUCHING) {
								MS->Set_pose(__CROUCH_GUN);
								MS->Change_pose_in_current_anim_set();
								Hard_start_new_mode(FIN_NORMAL_CROUCH_RELOAD, __LOAD_GUN_CROUCH_2);
							} else {
								MS->Set_pose(__GUN);
								MS->Change_pose_in_current_anim_set();
								Hard_start_new_mode(FIN_NORMAL_RELOAD, __LOAD_GUN_2);
							}
							return IR_REPEAT;
						}
					}

					// use a medi kit
					if (g_oIconListManager->Holding(ARMS_HEALTH_NAME)) {
						int32 hit_var;
						uint32 hits;
						uint32 new_energy;

						interact_lock = TRUE8;

						ob = (c_game_object *)MS->objects->Fetch_item_by_number(player_id);

						// Make sure number of medi-packs is > 0
						if (GetNoMediPacks() > 0) {
							hit_var = ob->GetVariable((const char *)"hits");
							hits = ob->GetIntegerVariable(hit_var);
							if (hits != MAX_HITS) {
								new_energy = hits + MAX_HITS / 2;
								if (new_energy > MAX_HITS)
									new_energy = MAX_HITS;

								ob->SetIntegerVariable(hit_var, new_energy); // full health again
								UseMediPacks(1); // use the pack
								// Play the medi-pack sound !
								RegisterSoundSpecial(defaultUsingMediSfx, addingMediDesc, 127, 0);
								MS->health_time = 12;
							}
						}
						g_oIconListManager->Drop();

						Pop_control_mode();
						Pop_player_stat();
						return IR_REPEAT;
					}

					// gun
					if (g_oIconListManager->Holding(ARMS_GUN_NAME)) {
						interact_lock = TRUE8;
						g_oIconListManager->Drop();
						Pop_control_mode();
						Pop_player_stat();
						return IR_REPEAT;
					}

					ret = Player_interact(); // check if interact has set up a new script to run on level 2
					if (ret == __FINISHED_THIS_CYCLE)
						return (IR_TERMINATE); // set to go around again and do a cycle of the interact script
				}

				Pop_control_mode();
				Pop_player_stat();
			}

			break;

		case FIN_NORMAL_RELOAD:
			UseAmmoClips(1); // use a a clip
			bull_per_clip = GetBulletsPerClip();
			SetBullets(bull_per_clip); // reload
			MS->Set_pose(__NOT_ARMED);
			MS->Change_pose_in_current_anim_set();
			Hard_start_new_mode(STOOD, __STAND);
			return (IR_STOP);
			break;
		case FIN_NORMAL_CROUCH_RELOAD:
			UseAmmoClips(1); // use a a clip
			bull_per_clip = GetBulletsPerClip();
			SetBullets(bull_per_clip); // reload
			MS->Set_pose(__CROUCH_NOT_ARMED);
			MS->Change_pose_in_current_anim_set();
			Hard_start_new_mode(CROUCHING, __STAND);
			return (IR_STOP);
			break;

		case FINISHED_RELOADING:

			UseAmmoClips(1); // use a a clip

			bull_per_clip = GetBulletsPerClip();

			SetBullets(bull_per_clip); // reload

			Pop_control_mode();
			Pop_player_stat();
			break;

		case REMORA:
			// The Remora is currently up over the game screen.  Most important check is to see if player
			// wants to quit it.
			if (cur_state.IsButtonSet(__REMORA)) {
				if (!remora_lock) {
					g_oRemora->DeactivateRemora(FALSE8);
					remora_lock = TRUE8;
				}
			} else {
				remora_lock = FALSE8;
			}

			// Now just run its logic.  It will process the 'deactivate' message if one was posted in previous line.
			g_oRemora->CycleRemoraLogic(cur_state);

			// Cycle the inventory logic if it is active.  Note though that the fact that the Remora is active does
			// not mean the inventory is, because there could be a gap between menus being displayed.
			if (g_oIconListManager->IsActive())
				g_oIconListManager->CycleInventoryLogic(cur_state);

			// If the Remora is now shut down, we need to return to player-standing mode.
			if (!g_oRemora->IsActive()) {
				Pop_control_mode();

				Pop_player_stat();

				MS->Setup_prop_sleep_states(); // recompute prop sleep states once we leave remora
			}

			return (IR_REPEAT);

		case LINKING:
			ret = Process_link();
			break;
		case EASY_LINKING:
			ret = Process_easy_link();
			break;
		case STILL_LINKING:
			ret = Process_still_link();
			break;
		case STILL_REVERSE_LINKING:
			ret = Process_reverse_still_link();
			break;

		case FAST_LINKING:
			ret = Process_fast_link();
			break;
		case REVERSE_LINKING:
			ret = Process_reverse_link();
			break;
		case GUN_LINKING:
			ret = Player_press_fire_button();
			if (ret == __FINISHED_THIS_CYCLE)
				return (IR_REPEAT); //

			ret = Process_link();
			break;

		case STRIKING:
			ret = Process_strike();
			break;

		// stair
		case ON_STAIRS:
			ret = Player_stairs();
			break;
		case RUNNING_ON_STAIRS:
			ret = Player_running_on_stairs();
			break;
		case STOOD_ON_STAIRS:
			ret = Player_stood_on_stairs();
			break;

		case REVERSE_ON_STAIRS:
			if (stair_dir != MS->stairs[stair_num].up)
				log->pan = MS->stairs[stair_num].pan + HALF_TURN; // if we are not traveling in the stairs original direction then we reverse the pan by 180deg

			else
				log->pan = MS->stairs[stair_num].pan; // otherwise we are turning to the stairs true pan and simple use that value

			player_status = ON_STAIRS;
			ret = __MORE_THIS_CYCLE;
			break;

		// ladder
		case ON_LADDER:
			ret = Player_ladder();
			break;
		case SLIP_SLIDIN_AWAY:
			ret = Player_slide_on_ladder();
			break;
		case LEAVE_LADDER: // top
			log->mega->actor_xyz.y += (REAL_ONE * (24 * 4));
			Leave_stair(); // history and stuff
			MS->camera_lock = FALSE8; // stop rough room cut through effect
			log->mega->drawShadow = TRUE8; // shadows back on
			Start_new_mode(STOOD);
			ret = __MORE_THIS_CYCLE;
			break;
		case LEAVE_LADDER_BOTTOM:
			log->pan += HALF_TURN;
			MS->floor_def->Allign_with_floor(log->mega);
			Start_new_mode(STOOD);
			ret = __MORE_THIS_CYCLE;
			break;

		case BEGIN_DOWN_LADDER:
			log->mega->actor_xyz.y -= (REAL_ONE * (24 * 4));
			// log->pan = MS->stairs[stair_num].pan + HALF_TURN; //if we are not traveling in the stairs original direction then we reverse the pan by 180deg
			stair_unit += 5;
			Set_player_status(ON_LADDER);
			ret = __MORE_THIS_CYCLE;
			break;

		default:
			Fatal_error("_player::Gateway player in illegal mode %d", player_status);
			break;
		}
	} while (ret == __MORE_THIS_CYCLE);

	return (IR_REPEAT);
}

void _player::Set_player_status(_player_stat new_mode) {
	// simply force the mode
	// for example, called after an interaction to put us back standing

	player_status = new_mode;
}

void _player::Start_new_mode(_player_stat new_mode) {
	// start new mode - no linking anim

	log->anim_pc = 0;
	player_status = new_mode;
}

void _player::Push_player_stat() {
	previous_stat = player_status;
}

void _player::Pop_player_stat() {
	player_status = previous_stat;
}

void _player::Soft_start_new_mode_no_link(_player_stat new_mode, __mega_set_names type) {
	// change anim mode
	// where type is the anim that will be played in the target mode

	// eg walk to run (which has no link)

	int32 diff = 1000000; // a big number
	uint32 old_leg_pos;
	int j;

	// link anim not in CAPS
	if ((!log->voxel_info->IsAnimTable(type)))
		Fatal_error("Soft_start_new_mode_no_link missing anim caps %s", master_anim_name_table[type].name);

	// JAKE : just in case defrag has moved something about
	PXanim *pCurAnim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(log->cur_anim_type), log->voxel_info->info_name_hash[log->cur_anim_type],
	                                                log->voxel_info->base_path, log->voxel_info->base_path_hash); //

	// find out leg position for current frame
	old_leg_pos = PXFrameEnOfAnim(log->anim_pc, pCurAnim)->left_foot_distance;

	// JAKE : just in case defrag has moved something about
	PXanim *pLnkAnim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(type), log->voxel_info->info_name_hash[type], log->voxel_info->base_path,
	                                                log->voxel_info->base_path_hash); //
	// see which has the closest leg position
	for (j = 0; j < (pLnkAnim->frame_qty - 1); j++) {
		int32 foot = PXFrameEnOfAnim(j, pLnkAnim)->left_foot_distance;
		int32 d = twabs(foot - old_leg_pos);
		if (d < diff) {
			diff = d;
			log->anim_pc = j; // this frame is best so far
		}
	}

	// set to linking mode
	player_status = new_mode;

	// set anim type to link
	log->cur_anim_type = type;
}

void _player::Soft_start_new_mode(_player_stat new_mode, __mega_set_names opt_link) {
	// change anim mode via an optional link anim
	// it is ok for the link anim to not exist despite being specified

	// NOTE this sets the first frame of the link anim so the caller should return to Gate_way() with a __FINISHED_THIS_CYCLE

	int32 diff = 1000000; // a big number
	uint32 old_leg_pos;
	int j;

	// link anim not in CAPS
	if ((!log->voxel_info->IsAnimTable(opt_link))) {
		Zdebug(1, "start_new_mode missing anim caps %s", master_anim_name_table[opt_link].name);

		log->anim_pc = 0;
		player_status = new_mode;

		return;
	}

	//	Zdebug("\ncalc start link frame");

	// JAKE : just in case defrag has moved something about
	PXanim *pCurAnim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(log->cur_anim_type), log->voxel_info->info_name_hash[log->cur_anim_type],
	                                                log->voxel_info->base_path, log->voxel_info->base_path_hash); //

	// find out leg position for current frame
	old_leg_pos = PXFrameEnOfAnim(log->anim_pc, pCurAnim)->left_foot_distance;

	// JAKE : just in case defrag has moved something about
	PXanim *pLnkAnim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(opt_link), log->voxel_info->info_name_hash[opt_link], log->voxel_info->base_path,
	                                                log->voxel_info->base_path_hash); //

	// see which has the closest leg position
	for (j = 0; j < (pLnkAnim->frame_qty - 1); j++) {
		int32 foot = PXFrameEnOfAnim(j, pLnkAnim)->left_foot_distance;
		int32 d = twabs(foot - old_leg_pos);
		if (d < diff) {
			diff = d;
			log->anim_pc = j; // this frame is best so far
		}
	}

	// set to linking mode
	player_status = FAST_LINKING;

	// remember new mode to switch in after link anim
	stat_after_link = new_mode;

	// set anim type to link
	log->cur_anim_type = opt_link;
}

void _player::Fast_hard_start_new_mode(_player_stat new_mode, __mega_set_names opt_link) {
	Hard_start_new_mode(new_mode, opt_link);
	player_status = FAST_LINKING;
}

void _player::Hard_start_new_mode(_player_stat new_mode, __mega_set_names opt_link) {
	// change anim mode via an optional link anim
	// it is ok for the link anim to not exist despite being specified

	// NOTE this sets the first frame of the link anim so the caller should return to Gate_way() with a __FINISHED_THIS_CYCLE

	// link anim not in CAPS
	if ((!log->voxel_info->IsAnimTable(opt_link))) {
		warning("start_new_mode missing anim caps %s", master_anim_name_table[opt_link].name);

		log->anim_pc = 0;
		player_status = new_mode;

		return;
	}

	// link anim has been requested - the link exists

	log->anim_pc = 0; // start at hard 0

	// set to linking mode
	player_status = LINKING;

	// remember new mode to switch in after link anim
	stat_after_link = new_mode;

	// set anim type to link
	log->cur_anim_type = opt_link;
}

void _player::Easy_start_new_mode(_player_stat new_mode, __mega_set_names opt_link) {
	// change anim mode via an optional link anim
	// the link anim is hard started but played easy - i.e. without barrier chacking

	// NOTE this sets the first frame of the link anim so the caller should return to Gate_way() with a __FINISHED_THIS_CYCLE

	// link anim not in CAPS
	if ((!log->voxel_info->IsAnimTable(opt_link))) {
		Zdebug(1, "start_new_mode missing anim caps %s", master_anim_name_table[opt_link].name);

		log->anim_pc = 0;
		player_status = new_mode;

		return;
	}

	// set anim type to link
	log->cur_anim_type = opt_link;

	// get animation
	PXanim *anim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(log->cur_anim_type), log->voxel_info->info_name_hash[log->cur_anim_type],
	                                            log->voxel_info->base_path, log->voxel_info->base_path_hash); //

	log->anim_pc = anim->frame_qty - 2;
	MS->Easy_frame_and_motion(log->cur_anim_type, TRUE8, 1); // push to frame 0
	log->anim_pc = 0;

	// set to linking mode
	player_status = EASY_LINKING;

	// remember new mode to switch in after link anim
	stat_after_link = new_mode;
}

void _player::Set_to_first_frame(__mega_set_names opt_link) {
	// link anim not in CAPS
	if ((!log->voxel_info->IsAnimTable(opt_link)))
		Fatal_error("Set_to_first_frame missing anim caps %s", master_anim_name_table[opt_link].name);

	// set anim type to link
	log->cur_anim_type = opt_link;
	log->anim_pc = 0;
}

void _player::Set_to_last_frame(__mega_set_names opt_link) {
	// link anim not in CAPS
	if ((!log->voxel_info->IsAnimTable(opt_link)))
		Fatal_error("Set_to_last_frame missing anim caps %s", master_anim_name_table[opt_link].name);

	// set anim type to link
	log->cur_anim_type = opt_link;

	// get animation
	PXanim *anim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(log->cur_anim_type), log->voxel_info->info_name_hash[log->cur_anim_type],
	                                            log->voxel_info->base_path, log->voxel_info->base_path_hash); //

	log->anim_pc = anim->frame_qty - 2;
}

void _player::Still_start_new_mode(_player_stat new_mode, __mega_set_names link) {
	// change anim mode via a STILL link anim

	// link anim not in CAPS
	if ((!log->voxel_info->IsAnimTable(link))) {
		log->anim_pc = 0;
		player_status = new_mode;

		return;
	}

	// link anim has been requested - the link exists

	// start at first frame
	log->anim_pc = 0;

	// set to linking mode
	player_status = STILL_LINKING;

	// remember new mode to switch in after link anim
	stat_after_link = new_mode;

	// set anim type to link
	log->cur_anim_type = link;
}

void _player::Still_reverse_start_new_mode(_player_stat new_mode, __mega_set_names link) {
	// change anim mode via a STILL link anim

	Set_to_last_frame(link);

	// set to linking mode
	player_status = STILL_REVERSE_LINKING;

	// remember new mode to switch in after link anim
	stat_after_link = new_mode;

	// set anim type to link
	log->cur_anim_type = link;
}

void _player::Hard_start_reverse_new_mode(_player_stat new_mode, __mega_set_names opt_link) {
	// change anim mode via an optional link anim
	// it is ok for the link anim to not exist despite being specified
	// the link anim is played end to beginning

	// NOTE this sets the first frame of the link anim so the caller should return to Gate_way() with a __FINISHED_THIS_CYCLE

	// link anim not in CAPS
	if ((!log->voxel_info->IsAnimTable(opt_link))) {
		Zdebug(1, "Hard_start_reverse_new_mode missing anim caps %s", master_anim_name_table[opt_link].name);

		log->anim_pc = 0;
		player_status = new_mode;

		return;
	}

	// link anim has been requested - the link exists

	PXanim *anim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(opt_link), log->voxel_info->info_name_hash[opt_link], log->voxel_info->base_path,
	                                            log->voxel_info->base_path_hash); //
	log->anim_pc = anim->frame_qty - 2;

	// set to linking mode
	player_status = REVERSE_LINKING;

	// remember new mode to switch in after link anim
	stat_after_link = new_mode;

	// set anim type to link
	log->cur_anim_type = opt_link;
}

void _player::Soft_start_new_mode(_player_stat new_mode, __mega_set_names opt_link, __mega_set_names opt_link2) {
	// this is the 2 link anim version of Start_new_mode
	// we pick the best frame match from the two link anims specified

	// change anim mode via an optional link anim
	// it is ok for the link anim to not exist despite being specified

	// NOTE this sets the first frame of the link anim so the caller should return to Gate_way() with a __FINISHED_THIS_CYCLE

	int32 diff = 1000000; // a big number

	uint32 old_leg_pos;
	int j;

	// no link anim specified OR
	// either link anim not in CAPS then just forget all about it - hey, too bad, ok?
	if ((!log->voxel_info->IsAnimTable(opt_link)) || (!log->voxel_info->IsAnimTable(opt_link2))) {
		Zdebug(1, "start_new_mode missing anim caps %s %s", master_anim_name_table[opt_link].name, master_anim_name_table[opt_link2].name);

		log->anim_pc = 0;
		player_status = new_mode;

		return;
	}

	// link anim has been requested - the link exists

	// find frame to skip to

	// Load the current anim
	PXanim *pCurAnim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(log->cur_anim_type), log->voxel_info->info_name_hash[log->cur_anim_type],
	                                                log->voxel_info->base_path, log->voxel_info->base_path_hash); //

	// find out leg position for current frame
	old_leg_pos = PXFrameEnOfAnim(log->anim_pc, pCurAnim)->left_foot_distance;

	// Load the first link candidate anim
	PXanim *pLnkAnim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(opt_link), log->voxel_info->info_name_hash[opt_link], log->voxel_info->base_path,
	                                                log->voxel_info->base_path_hash); //

	// see which has the closest leg position
	for (j = 0; j < (pLnkAnim->frame_qty - 1); j++) {
		int32 foot = PXFrameEnOfAnim(j, pLnkAnim)->left_foot_distance;
		int32 d = twabs(foot - old_leg_pos);
		if (d < diff) {
			diff = d;
			log->anim_pc = j; // this frame is best so far
			log->cur_anim_type = opt_link;
		}
	}

	// Load the second link candidate file
	pLnkAnim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(opt_link2), log->voxel_info->info_name_hash[opt_link2], log->voxel_info->base_path,
	                                        log->voxel_info->base_path_hash); //

	// see which has the closest leg position
	for (j = 0; j < (pLnkAnim->frame_qty - 1); j++) {
		int32 foot = PXFrameEnOfAnim(j, pLnkAnim)->left_foot_distance;
		int32 d = twabs(foot - old_leg_pos);
		if (d < diff) {
			diff = d;
			log->anim_pc = j; // this frame is best so far
			log->cur_anim_type = opt_link2;
		}
	}

	// set to linking mode
	player_status = FAST_LINKING;

	// remember new mode to switch in after link anim
	stat_after_link = new_mode;
}

__mode_return _player::Process_strike() {
	// run the strike anim
	bool8 ret;
	int32 retval;
	PXreal int_x = REAL_ZERO;
	PXreal int_z = REAL_ZERO; // for the marker
	PXreal hit_x, hit_z; //

	if ((has_weapon) && (cur_state.IsButtonSet(__ARMUNARM))) { // pull gun

		MS->Set_pose(__GUN);
		MS->Change_pose_in_current_anim_set();

		if (armedChangesMode == 1)
			Push_control_mode(ACTOR_RELATIVE);

		Hard_start_new_mode(NEW_AIM, __PULL_OUT_WEAPON);

		return (__FINISHED_THIS_CYCLE);
	}

	// get animation
	PXanim *pAnim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(log->cur_anim_type), log->voxel_info->info_name_hash[log->cur_anim_type],
	                                             log->voxel_info->base_path, log->voxel_info->base_path_hash); //

	// last frame?
	if ((int)(log->anim_pc + 1) == (pAnim->frame_qty - 1)) {
		// we displayed the last frame last cycle - so display first of new mode this cycle

		player_status = STOOD;

		// reset pc
		log->anim_pc = 0;

		// kick in now
		return (__MORE_THIS_CYCLE);
	}

	// shift character and frame forward by the amount appropriate
	ret = MS->Advance_frame_and_motion(log->cur_anim_type, TRUE8, 1);

	if (!ret) { // could not move? Abandon the link and hit the mode (or perhaps stand?)
		// set target after-link-anim-mode
		player_status = STOOD;

		// reset pc
		log->anim_pc = 0;

		// kick in now
		return (__MORE_THIS_CYCLE);
	}

	// check for the hit frame
	// The animation will have made the INT marker visible on the frame designated to be the "hit" frame
	// Get the current frame from the anim
	PXframe *currentFrame = NULL;
	// Note the weird = NULL & then setting it removes GCC warnings
	currentFrame = PXFrameEnOfAnim(log->anim_pc, pAnim);

	if (currentFrame->marker_qty > INT_POS) {
		if (INT_TYPE == currentFrame->markers[INT_POS].GetType()) {
			// punching a prop
			if ((interact_selected) && (MS->logic_structs[cur_interact_id]->image_type == PROP)) {
				MS->Call_socket(cur_interact_id, "ko", &retval); // call a ko script if there is one
			}
			// punching an evil live mega character
			else if ((interact_selected) && (MS->logic_structs[cur_interact_id]->image_type == VOXEL) &&
			         (MS->logic_structs[cur_interact_id]->ob_status != OB_STATUS_HELD) && (MS->logic_structs[cur_interact_id]->mega->is_evil)) {

				// ok, final check to see if mega is dead
				if (MS->logic_structs[cur_interact_id]->mega->dead)
					return __FINISHED_THIS_CYCLE; // dead

				if (MS->logic_structs[cur_interact_id]->mega->Is_crouched())
					return __FINISHED_THIS_CYCLE; // crouching mega

				// get interact marker offset
				PXreal x_org, z_org, unused;
				PXFrameEnOfAnim(0, pAnim)->markers[ORG_POS].GetXYZ(&x_org, &unused, &z_org);

				// The interact marker exists
				PXreal x_int, z_int;

				currentFrame->markers[INT_POS].GetXYZ(&x_int, &unused, &z_int);

				int_x = x_int - x_org;
				int_z = z_int - z_org;

				// now project the interaction offset forward from our org marker pan
				// this will give us our interact (fist) coordinate
				PXfloat ang = log->pan * TWO_PI;
				PXfloat cang = (PXfloat)PXcos(ang);
				PXfloat sang = (PXfloat)PXsin(ang);

				hit_x = log->mega->actor_xyz.x + PXfloat2PXreal(int_x * cang + int_z * sang);
				hit_z = log->mega->actor_xyz.z + PXfloat2PXreal(int_z * cang - int_x * sang);

				if ((PXfabs(hit_x - MS->logic_structs[cur_interact_id]->mega->actor_xyz.x) < (PXreal)150) && /*original EU PSX release is 100*/
				    (PXfabs(hit_z - MS->logic_structs[cur_interact_id]->mega->actor_xyz.z) < (PXreal)150)) {

					if (!g_oLineOfSight->LineOfSight(cur_interact_id, Fetch_player_id())) {
						//								behind
						ret = MS->Call_socket(cur_interact_id, "ko", &retval);
						if (!ret)
							Fatal_error("no ko script for object [%s]", MS->socket_object->GetName());
					} else { // infront
						ret = MS->Call_socket(cur_interact_id, "see_ko", &retval);
						if (!ret)
							Fatal_error("no see_ko script for object [%s]", MS->socket_object->GetName());
					}
					MS->Signal_to_other_guards(); // make other guards see this!
					return __FINISHED_THIS_CYCLE;
				}
			}
		}
		MS->Signal_to_guards(); // make all guards see this!
	}

	return __FINISHED_THIS_CYCLE;
}

__mode_return _player::Process_fast_link() {
	// DOUBLE SPEED LINK
	// play link anim until finished then switch in next mode WHEN LAST FRAME is reached
	// link plays without user input

	// last frame? Then move mode on

	bool8 ret;

	// get animation
	PXanim *anim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(log->cur_anim_type), log->voxel_info->info_name_hash[log->cur_anim_type],
	                                            log->voxel_info->base_path, log->voxel_info->base_path_hash); //

	// skip a frame
	//	log->anim_pc+=1;

	// last frame?
	if ((int)(log->anim_pc + 2) >= (anim->frame_qty - 1)) {
		// we displayed the last frame last cycle - so display first of new mode this cycle

		// set target after-link-anim-mode
		player_status = stat_after_link;

		// reset pc
		log->anim_pc = 0;

		// kick in now
		return (__MORE_THIS_CYCLE);
	}

	// shift character and frame forward by the amount appropriate
	ret = MS->Advance_frame_and_motion(log->cur_anim_type, TRUE8, 2);

	if (!ret) { // could not move? Abandon the link and hit the mode (or perhaps stand?)
		player_status = stat_after_link;

		// reset pc
		log->anim_pc = 0;

		// kick in now
		return (__MORE_THIS_CYCLE);
	}

	return (__FINISHED_THIS_CYCLE);
}

__mode_return _player::Process_link() {
	// play link anim until finished then switch in next mode WHEN LAST FRAME is reached
	// link plays without user input

	// last frame? Then move mode on

	bool8 ret;

	// get animation
	PXanim *anim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(log->cur_anim_type), log->voxel_info->info_name_hash[log->cur_anim_type],
	                                            log->voxel_info->base_path, log->voxel_info->base_path_hash); //

	// last frame?
	if ((int)(log->anim_pc + 1) == (anim->frame_qty - 1)) {
		//		we displayed the last frame last cycle - so display first of new mode this cycle
		player_status = stat_after_link;

		//		reset pc
		log->anim_pc = 0;

		//		kick in now
		return (__MORE_THIS_CYCLE);
	}

	// shift character and frame forward by the amount appropriate
	ret = MS->Advance_frame_and_motion(log->cur_anim_type, TRUE8, 1);

	// Very nasty hard wiring but hey it is the last day of the project
	if (((log->cur_anim_type == __LOAD_GUN) || (log->cur_anim_type == __LOAD_GUN_2) || (log->cur_anim_type == __LOAD_GUN_CROUCH_2)) && (log->anim_pc == 6)) {
		// Play the clip sound !
		RegisterSoundSpecial(defaultAddingClipSfx, addingClipDesc, 127, 0);
	}

	if (!ret) { // could not move? Abandon the link and hit the mode (or perhaps stand?)

		//		set target after-link-anim-mode
		player_status = stat_after_link;

		//		reset pc
		log->anim_pc = 0;

		//		kick in now
		return (__MORE_THIS_CYCLE);
	}

	return (__FINISHED_THIS_CYCLE);
}

__mode_return _player::Process_easy_link() {
	// play link anim until finished then switch in next mode WHEN LAST FRAME is reached
	// link plays without user input

	// last frame? Then move mode on

	// get animation
	PXanim *anim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(log->cur_anim_type), log->voxel_info->info_name_hash[log->cur_anim_type],
	                                            log->voxel_info->base_path, log->voxel_info->base_path_hash); //

	// last frame?
	if ((int)(log->anim_pc + 1) == (anim->frame_qty - 1)) {
		// we displayed the last frame last cycle - so display first of new mode this cycle
		// set target after-link-anim-mode
		player_status = stat_after_link;

		// reset pc
		log->anim_pc = 0;

		// kick in now
		return (__MORE_THIS_CYCLE);
	}

	// shift character and frame forward by the amount appropriate
	MS->Easy_frame_and_motion(log->cur_anim_type, TRUE8, 1);

	return (__FINISHED_THIS_CYCLE);
}

__mode_return _player::Process_still_link() {
	// play anim with no movement - v easy!

	PXanim *anim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(log->cur_anim_type), log->voxel_info->info_name_hash[log->cur_anim_type],
	                                            log->voxel_info->base_path, log->voxel_info->base_path_hash); //

	// last frame currently displayed?
	if ((int)(log->anim_pc + 1) == (anim->frame_qty - 1)) {
		// we displayed the last frame last cycle

		player_status = stat_after_link;

		// kick in now
		return (__MORE_THIS_CYCLE);
	}

	//	ok, advance frame but not position
	log->anim_pc = (log->anim_pc + 1) % (anim->frame_qty - 1);

	return __FINISHED_THIS_CYCLE;
}

__mode_return _player::Process_reverse_still_link() {
	// play anim with no movement - v easy!

	// last frame currently displayed?
	if (!log->anim_pc) {
		// we displayed the last frame last cycle

		player_status = stat_after_link;

		// kick in now
		return (__MORE_THIS_CYCLE);
	}

	//	ok, advance frame but not position
	log->anim_pc -= 1;

	return __FINISHED_THIS_CYCLE;
}

__mode_return _player::Process_reverse_link() {
	// play link anim until finished then switch in next mode WHEN FIRST FRAME is reached
	// link plays without user input

	// last frame? Then move mode on

	bool8 ret;

	// last frame?
	if (!log->anim_pc) {
		// we displayed the last frame last cycle - so display first of new mode this cycle

		// set target after-link-anim-mode
		player_status = stat_after_link;

		// reset pc
		log->anim_pc = 0;

		// kick in now
		return (__MORE_THIS_CYCLE);
	}

	// shift character and frame forward by the amount appropriate
	ret = MS->Reverse_frame_and_motion(log->cur_anim_type, TRUE8, 1);

	if (!ret) { // could not move forward? Abandon the link and hit the mode (or perhaps stand?)
		// set target after-link-anim-mode
		player_status = stat_after_link;

		// reset pc
		log->anim_pc = 0;

		// kick in now
		return (__MORE_THIS_CYCLE);
	}

	return (__FINISHED_THIS_CYCLE);
}

__mode_return _player::Player_running_on_stairs() {
	if (stair_dir) { // up
		//		not our first step up
		if (was_climbing)
			log->mega->actor_xyz.y += (REAL_ONE * 36);

		if ((MS->stairs[stair_num].units - stair_unit) < 2) { // 1 or 0
			// check for 1 step left
			if ((MS->stairs[stair_num].units - stair_unit)) { // 1
				Still_start_new_mode(STOOD_ON_STAIRS, __WALK_UPSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_UP);
				return (__MORE_THIS_CYCLE);
			}

			if (begun_at_bottom) // begun at bottom so write the history
				Add_to_interact_history();

			Leave_stair();
			return (__FINISHED_THIS_CYCLE);
		} else {
			// still going, turning, stopping

			step_sample_num += 2; // up 2
			if (step_sample_num >= MAX_stair_length)
				Fatal_error("stair correction system ran out of space");

			if (left_right) { // left leg
				if (((stair_unit > 2)) && ((cur_state.momentum == __STILL) || (cur_state.momentum == __BACKWARD_1))) {
					Still_start_new_mode(STOOD_ON_STAIRS, __WALK_UPSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_UP);
					return (__MORE_THIS_CYCLE);
				}
				Easy_start_new_mode(RUNNING_ON_STAIRS, __RUN_UPSTAIRS_LEFT); // still going
			} else { // right leg
				if (((stair_unit > 2)) && ((cur_state.momentum == __STILL) || (cur_state.momentum == __BACKWARD_1))) {
					Still_start_new_mode(STOOD_ON_STAIRS, __WALK_UPSTAIRS_LEFT_TO_STOOD_ON_STAIRS_FACING_UP);
					return (__MORE_THIS_CYCLE);
				}
				Easy_start_new_mode(RUNNING_ON_STAIRS, __RUN_UPSTAIRS_RIGHT); // still going
			}

			left_right = (uint8)(1 - left_right); // toggle
		}
	} else { // down
		// not our first step up
		if (was_climbing)
			log->mega->actor_xyz.y -= (REAL_ONE * 36);

		if ((MS->stairs[stair_num].units - stair_unit) < 2) { // 1 or 0

			// check for 1 step left
			if ((MS->stairs[stair_num].units - stair_unit)) { // 1
				Still_start_new_mode(STOOD_ON_STAIRS, __WALK_DOWNSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_DOWN);
				return (__MORE_THIS_CYCLE);
			}

			if (!begun_at_bottom) // didnt begin at bottom so write the history
				Add_to_interact_history();

			Leave_stair();
			return (__FINISHED_THIS_CYCLE);
		} else {

			if (step_sample_num == 0)
				Fatal_error("stair correction system ran out of space");
			step_sample_num -= 2; // down

			if (left_right) { // left

				if ((cur_state.momentum == __STILL) && (stair_unit > 2)) {
					Still_start_new_mode(STOOD_ON_STAIRS, __WALK_DOWNSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_DOWN);
					return (__MORE_THIS_CYCLE);
				}
				Easy_start_new_mode(RUNNING_ON_STAIRS, __RUN_DOWNSTAIRS_LEFT);
			} else { // right
				if ((cur_state.momentum == __STILL) && (stair_unit > 2)) {
					Still_start_new_mode(STOOD_ON_STAIRS, __WALK_DOWNSTAIRS_LEFT_TO_STOOD_ON_STAIRS_FACING_DOWN);
					return (__MORE_THIS_CYCLE);
				}
				Easy_start_new_mode(RUNNING_ON_STAIRS, __RUN_DOWNSTAIRS_RIGHT);
			}

			left_right = (uint8)(1 - left_right); // toggle
		}
	}

	was_climbing = TRUE8;
	stair_unit += 2; // doing another 2 steps
	return (__MORE_THIS_CYCLE);
}

__mode_return _player::Player_stairs() {
	if (stair_dir) { // up
		//		not our first step up
		if (was_climbing)
			log->mega->actor_xyz.y += (REAL_ONE * 18);

		if (stair_unit == MS->stairs[stair_num].units) {
			if (begun_at_bottom) // begun at bottom so write the history
				Add_to_interact_history();

			Leave_stair();
			return (__FINISHED_THIS_CYCLE);
		} else {
			step_sample_num++; // up
			if (step_sample_num >= MAX_stair_length) {
				Message_box("stair correction system ran out of space");

				log->mega->actor_xyz.x = MS->hist_pin_x;
				log->mega->actor_xyz.y = MS->hist_pin_y;
				log->mega->actor_xyz.z = MS->hist_pin_z;
				log->mega->on_stairs = FALSE8;
				Set_player_status(STOOD);
				return (__FINISHED_THIS_CYCLE);
			}

			if (step_samples[step_sample_num].stepped_on_step) {
				log->mega->actor_xyz.x = step_samples[step_sample_num].x;
				log->mega->actor_xyz.z = step_samples[step_sample_num].z;
			} else {
				step_samples[step_sample_num].stepped_on_step = TRUE8;
				step_samples[step_sample_num].x = log->mega->actor_xyz.x;
				step_samples[step_sample_num].z = log->mega->actor_xyz.z;
			}

			if ((stair_unit > 1) && (stair_unit < MS->stairs[stair_num].units - 1)) {
				//          still going, turning, stopping
				if ((cur_state.momentum == __STILL) || (cur_state.momentum == __BACKWARD_1)) {
					if (left_right)
						Still_start_new_mode(STOOD_ON_STAIRS, __WALK_UPSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_UP);
					else
						Still_start_new_mode(STOOD_ON_STAIRS, __WALK_UPSTAIRS_LEFT_TO_STOOD_ON_STAIRS_FACING_UP);

					return (__MORE_THIS_CYCLE);
				} else if ((cur_state.turn != __NO_TURN)) { // turn around
					if (!left_right)
						Hard_start_new_mode(REVERSE_ON_STAIRS, __WALK_UPSTAIRS_LEFT_TO_WALK_DOWNSTAIRS_RIGHT);
					else
						Hard_start_new_mode(REVERSE_ON_STAIRS, __WALK_UPSTAIRS_RIGHT_TO_WALK_DOWNSTAIRS_LEFT);
					player_status = EASY_LINKING; // no first frame push, and no barrier checking
					stair_dir = 0;
					stair_unit = (uint8)((MS->stairs[stair_num].units - stair_unit) + 1);
					return (__MORE_THIS_CYCLE);
				}
			}

			if (left_right)
				Easy_start_new_mode(ON_STAIRS, __WALK_UPSTAIRS_LEFT); // still going
			else
				Easy_start_new_mode(ON_STAIRS, __WALK_UPSTAIRS_RIGHT); // still going

			left_right = (uint8)(1 - left_right); // toggle
		}

	} else { // down
		//		not our first step up
		if (was_climbing)
			log->mega->actor_xyz.y -= (REAL_ONE * 18);

		if (stair_unit == MS->stairs[stair_num].units) {

			if (!begun_at_bottom) // didnt begin at bottom so write the history
				Add_to_interact_history();

			Leave_stair();
			return (__FINISHED_THIS_CYCLE);
		} else {
			if (!step_sample_num) {
				Message_box("stair correction system ran out of space (went negative)");

				log->mega->actor_xyz.x = MS->hist_pin_x;
				log->mega->actor_xyz.y = MS->hist_pin_y;
				log->mega->actor_xyz.z = MS->hist_pin_z;
				log->mega->on_stairs = FALSE8;
				Set_player_status(STOOD);
				return (__FINISHED_THIS_CYCLE);
			}

			step_sample_num--; // down

			if ((stair_unit > 1) && (stair_unit < MS->stairs[stair_num].units - 1)) {
				if ((cur_state.momentum == __STILL) || (cur_state.momentum == __BACKWARD_1)) {
					if (left_right)
						Still_start_new_mode(STOOD_ON_STAIRS, __WALK_DOWNSTAIRS_RIGHT_TO_STOOD_ON_STAIRS_FACING_DOWN);
					else
						Still_start_new_mode(STOOD_ON_STAIRS, __WALK_DOWNSTAIRS_LEFT_TO_STOOD_ON_STAIRS_FACING_DOWN);

					return (__MORE_THIS_CYCLE);
				} else if ((cur_state.turn != __NO_TURN) && ((stair_unit + 1) != MS->stairs[stair_num].units)) { // turn around
					if (!left_right)
						Hard_start_new_mode(REVERSE_ON_STAIRS, __WALK_DOWNSTAIRS_LEFT_TO_WALK_UPSTAIRS_RIGHT);
					else
						Hard_start_new_mode(REVERSE_ON_STAIRS, __WALK_DOWNSTAIRS_RIGHT_TO_WALK_UPSTAIRS_LEFT);

					player_status = EASY_LINKING; // no first frame push, and no barrier checking

					stair_dir = 1;
					stair_unit = (uint8)((MS->stairs[stair_num].units - stair_unit) + 1);
					return (__MORE_THIS_CYCLE);
				}
			}

			if (left_right)
				Easy_start_new_mode(ON_STAIRS, __WALK_DOWNSTAIRS_LEFT);
			else
				Easy_start_new_mode(ON_STAIRS, __WALK_DOWNSTAIRS_RIGHT);

			left_right = (uint8)(1 - left_right); // toggle
		}
	}

	was_climbing = TRUE8;
	stair_unit++; // doing another
	return (__MORE_THIS_CYCLE);
}

__mode_return _player::Player_stood_on_stairs() {
	if (stair_dir) { // up
		if ((cur_state.momentum != __STILL) && (cur_state.momentum != __BACKWARD_1)) {
			stair_unit++; // doing another

			if (left_right)
				Easy_start_new_mode(ON_STAIRS, __WALK_UPSTAIRS_LEFT);
			else
				Easy_start_new_mode(ON_STAIRS, __WALK_UPSTAIRS_RIGHT);

			left_right = (uint8)(1 - left_right); // toggle
			return (__MORE_THIS_CYCLE);
		} else if (cur_state.turn != __NO_TURN) { // turn around
			if (!left_right)
				Hard_start_new_mode(REVERSE_ON_STAIRS, __WALK_UPSTAIRS_LEFT_TO_WALK_DOWNSTAIRS_RIGHT);
			else
				Hard_start_new_mode(REVERSE_ON_STAIRS, __WALK_UPSTAIRS_RIGHT_TO_WALK_DOWNSTAIRS_LEFT);
			player_status = EASY_LINKING; // no first frame push, and no barrier checking
			stair_dir = 0;
			stair_unit = (uint8)((MS->stairs[stair_num].units - stair_unit) + 1);
			return (__MORE_THIS_CYCLE);
		}

	} else { // down
		if ((cur_state.momentum != __STILL) && (cur_state.momentum != __BACKWARD_1)) {
			stair_unit++; // doing another

			if (left_right)
				Easy_start_new_mode(ON_STAIRS, __WALK_DOWNSTAIRS_LEFT);
			else
				Easy_start_new_mode(ON_STAIRS, __WALK_DOWNSTAIRS_RIGHT);

			left_right = (uint8)(1 - left_right); // toggle
			return (__MORE_THIS_CYCLE);
		} else if ((cur_state.turn != __NO_TURN) && ((stair_unit + 1) != MS->stairs[stair_num].units)) { // turn around
			if (!left_right)
				Hard_start_new_mode(REVERSE_ON_STAIRS, __WALK_DOWNSTAIRS_LEFT_TO_WALK_UPSTAIRS_RIGHT);
			else
				Hard_start_new_mode(REVERSE_ON_STAIRS, __WALK_DOWNSTAIRS_RIGHT_TO_WALK_UPSTAIRS_LEFT);
			player_status = EASY_LINKING; // no first frame push, and no barrier checking
			stair_dir = 1;
			stair_unit = (uint8)((MS->stairs[stair_num].units - stair_unit) + 1);
			return (__MORE_THIS_CYCLE);
		}
	}

	return (__FINISHED_THIS_CYCLE);
}

void _player::Leave_stair() {
	// player is stepping off a ladder or stair

	// align with floor
	// set next mode according to momentum

	MS->floor_def->Allign_with_floor(log->mega);

	// coming off the stair
	if (MS->stairs[stair_num].is_stair) {
		if ((cur_state.momentum == __STILL) || (cur_state.momentum == __BACKWARD_1))
			Soft_start_new_mode(WALKING, __WALK); // Soft_start_new_mode(STOOD, __WALK_TO_STAND);
		else if (cur_state.momentum == __FORWARD_1)
			Soft_start_new_mode(WALKING, __WALK);
		else
			Soft_start_new_mode(RUNNING, __RUN);
	} else { // ladder
		Set_player_status(STOOD);
	}
}

void _player::Add_to_interact_history() {
	// add current stair or ladder

	// advance history
	MS->cur_history++;
	if (MS->cur_history == MAX_player_history)
		MS->cur_history = 0; // wrap
	// record it
	MS->history[MS->cur_history].interaction = TRUE8;
	MS->history[MS->cur_history].id = MS->stairs[stair_num].stair_id;
	Tdebug("history.txt", "Stair [%s]", MS->objects->Fetch_items_name_by_number(MS->stairs[stair_num].stair_id));

	MS->floor_def->Set_floor_rect_flag(log);
	Tdebug("history.txt", "...%d", log->owner_floor_rect);
}

__mode_return _player::Player_ladder() {
	if (stair_dir) { // up

		log->pan = MS->stairs[stair_num].pan_ref;

		//		not our first step up
		if (was_climbing)
			log->mega->actor_xyz.y += (REAL_ONE * 24);

		if (stair_unit == (MS->stairs[stair_num].units) - 5) {
			if (begun_at_bottom) // began at bottom so write the history
				Add_to_interact_history();

			MS->camera_lock = TRUE8; // stop rough room cut through effect
			Easy_start_new_mode(LEAVE_LADDER, __CLIMB_UP_LADDER_RIGHT_TO_STAND); // get off
			return (__MORE_THIS_CYCLE);
		} else {
			//			still going, turning, stopping
			if (cur_state.momentum == __STILL) { //||(cur_state.momentum==__BACKWARD_1))
				was_climbing = 0;

				if (left_right)
					Set_to_first_frame(__CLIMB_UP_LADDER_LEFT); // still going
				else
					Set_to_first_frame(__CLIMB_UP_LADDER_RIGHT); // still going

				return __FINISHED_THIS_CYCLE;
			}
			if (cur_state.momentum == __BACKWARD_1) { // pulling down
				was_climbing = 0;
				stair_dir = 0;
				stair_unit = (uint8)((MS->stairs[stair_num].units - stair_unit));
				left_right = (uint8)(1 - left_right); // toggle
				return __MORE_THIS_CYCLE;
			}
			if (cur_state.momentum == __FORWARD_2) {
				stair_unit = (uint8)((MS->stairs[stair_num].units - stair_unit));

				if (!begun_at_bottom) // didnt begin at bottom so write the history
					Add_to_interact_history();

				Set_to_first_frame(__SLIDE_DOWN_LADDER); //
				Set_player_status(SLIP_SLIDIN_AWAY);
				return __MORE_THIS_CYCLE;
			}

			if (left_right)
				Easy_start_new_mode(ON_LADDER, __CLIMB_UP_LADDER_LEFT); // still going
			else
				Easy_start_new_mode(ON_LADDER, __CLIMB_UP_LADDER_RIGHT); // still going

			left_right = (uint8)(1 - left_right); // toggle
		}

	} else { // down

		log->pan = MS->stairs[stair_num].pan_ref + HALF_TURN;
		//		not our first step up
		if (was_climbing)
			log->mega->actor_xyz.y -= (REAL_ONE * 24);

		if (stair_unit == (MS->stairs[stair_num].units + 0)) {
			if (!begun_at_bottom) // didnt begin at bottom so write the history
				Add_to_interact_history();

			log->mega->actor_xyz.y -= (REAL_ONE * 24);
			log->mega->drawShadow = TRUE8; // shadows om
			Easy_start_new_mode(LEAVE_LADDER_BOTTOM, __CLIMB_DOWN_LADDER_RIGHT_TO_STAND); // get off
			return (__MORE_THIS_CYCLE);
		} else {
			if (cur_state.momentum == __STILL) { //||(cur_state.momentum==__BACKWARD_1))
				was_climbing = 0;

				if (left_right)
					Set_to_first_frame(__CLIMB_DOWN_LADDER_LEFT); // still going
				else
					Set_to_first_frame(__CLIMB_DOWN_LADDER_RIGHT); // still going

				return __FINISHED_THIS_CYCLE;
			}
			if (cur_state.momentum == __FORWARD_1) { // pushing up
				was_climbing = 0;
				stair_dir = 1;
				stair_unit = (uint8)((MS->stairs[stair_num].units - stair_unit));
				left_right = (uint8)(1 - left_right); // toggle
				return __MORE_THIS_CYCLE;
			}
			if (cur_state.IsButtonSet(__JOG)) {
				if (!begun_at_bottom) // didnt begin at bottom so write the history
					Add_to_interact_history();

				log->pan = MS->stairs[stair_num].pan_ref; // if we are not traveling in the stairs original direction then we reverse the pan by 180deg
				Set_to_first_frame(__SLIDE_DOWN_LADDER); //
				Set_player_status(SLIP_SLIDIN_AWAY);
				return __MORE_THIS_CYCLE;
			}

			if (left_right)
				Easy_start_new_mode(ON_LADDER, __CLIMB_DOWN_LADDER_LEFT);
			else
				Easy_start_new_mode(ON_LADDER, __CLIMB_DOWN_LADDER_RIGHT);

			left_right = (uint8)(1 - left_right); // toggle
		}
	}

	log->mega->drawShadow = FALSE8; // shadows off
	MS->camera_lock = FALSE8; // stop rough room cut through effect
	was_climbing = TRUE8;
	stair_unit++; // doing another
	return (__MORE_THIS_CYCLE);
}

__mode_return _player::Player_slide_on_ladder() {
	if (stair_unit == (MS->stairs[stair_num].units + 1)) {

		MS->floor_def->Allign_with_floor(log->mega);

		log->mega->drawShadow = TRUE8; // shadows on
		Easy_start_new_mode(STOOD, __SLIDE_DOWN_LADDER_TO_STAND); // get off
		return (__MORE_THIS_CYCLE);
	}

	log->mega->drawShadow = FALSE8; // shadows off
	log->mega->actor_xyz.y -= (REAL_ONE * 24);
	MS->camera_lock = FALSE8; // stop rough room cut through effect
	stair_unit++; // doing another

	return __FINISHED_THIS_CYCLE;
}

__mode_return _player::Player_new_aim() {
	// this is the new aim mode where aim button must be held down to remain in the mode

	// check if key released
	if (!cur_state.IsButtonSet(__ARMUNARM)) { // let go of aim button

		// g_oLineOfSight->SetFieldOfView( Fetch_player_id(), 90 );

		Hard_start_reverse_new_mode(PUTTING_AWAY_GUN, __PULL_OUT_WEAPON);

		return (__FINISHED_THIS_CYCLE);
	}

	// if we are newly armed then tell all the guards who may decide to react
	if (log->cur_anim_type != __STAND)
		MS->Signal_to_guards();

	// must do this fist in-case coming from a link and diving straight out again
	log->cur_anim_type = __STAND;
	MS->Set_can_save(TRUE8); // can save

	if ((cur_state.IsButtonSet(__INVENTORY)) && (!inv_lock)) {
		if ((GetNoAmmoClips()) && (GetNoBullets() < (int32)GetBulletsPerClip())) {
			inv_lock = TRUE8; // not auto-repeat

			//			reload
			UseAmmoClips(1); // use a clip

			int32 bull_per_clip = GetBulletsPerClip();

			SetBullets(bull_per_clip); // reload

			Hard_start_new_mode(NEW_AIM, __LOAD_GUN);

			return (__FINISHED_THIS_CYCLE);
		}
	} else {
		// Stopped inventory dropping in repeatedly when you hold down the key.
		if (!(cur_state.IsButtonSet(__INVENTORY))) {
			inv_lock = FALSE8; // released
		}
	}

	// check for
	// forward step
	// step backward?
	if ((cur_state.momentum == __FORWARD_1) && (!forward_lock)) {
		Hard_start_new_mode(NEW_AIM, __STEP_FORWARD);
		player_status = GUN_LINKING;
		return (__FINISHED_THIS_CYCLE);
	} else if (cur_state.momentum == __STILL)
		forward_lock = FALSE8;

	// step backward?
	if ((cur_state.momentum == __BACKWARD_1) && (backward_lock == FALSE8)) {
		Hard_start_new_mode(NEW_AIM, __STEP_BACKWARD);
		forward_lock = FALSE8; // release a lock caused by hitting a wall
		return (__FINISHED_THIS_CYCLE);
	}
	// sidestep left?
	if ((cur_state.IsButtonSet(__SIDESTEP)) && (cur_state.turn == __LEFT)) {
		Hard_start_new_mode(NEW_AIM, __SIDESTEP_LEFT);
		forward_lock = FALSE8; // release a lock caused by hitting a wall
		backward_lock = FALSE8; // release a lock caused by hitting a wall
		return (__FINISHED_THIS_CYCLE);
	} else if ((cur_state.IsButtonSet(__SIDESTEP)) && (cur_state.turn == __RIGHT)) {
		Hard_start_reverse_new_mode(NEW_AIM, __SIDESTEP_LEFT);
		forward_lock = FALSE8; // release a lock caused by hitting a wall
		backward_lock = FALSE8; // release a lock caused by hitting a wall
		return (__FINISHED_THIS_CYCLE);
	}

	// are we turning on the spot
	if (cur_state.turn == __LEFT) {
		//  animate if frames present
		if (log->voxel_info->IsAnimTable(__TURN_ON_THE_SPOT_CLOCKWISE)) {

			log->cur_anim_type = __TURN_ON_THE_SPOT_CLOCKWISE;
			Reverse_frame_motion_and_pan(__TURN_ON_THE_SPOT_CLOCKWISE);
			log->pan += aim_turn_amount;

			forward_lock = FALSE8; // release a lock caused by hitting a wall
		}
	} else if (cur_state.turn == __RIGHT) {
		//		animate if frames present
		if (log->voxel_info->IsAnimTable(__TURN_ON_THE_SPOT_CLOCKWISE)) {
			log->cur_anim_type = __TURN_ON_THE_SPOT_CLOCKWISE;
			Advance_frame_motion_and_pan(__TURN_ON_THE_SPOT_CLOCKWISE);
			log->pan -= aim_turn_amount;

			forward_lock = FALSE8; // release a lock caused by hitting a wall
		}
	} else if (cur_state.IsButtonSet(__CROUCH)) { // crouch down
		MS->Set_pose(__CROUCH_GUN);
		MS->Change_pose_in_current_anim_set();
		Hard_start_reverse_new_mode(CROUCH_AIM, __STAND_CROUCHED_TO_STAND);
		return (__FINISHED_THIS_CYCLE);
	} else {
		log->anim_pc = 0; // just stood

		__mode_return ret;
		ret = Player_press_remora_button();
		if (ret == __FINISHED_THIS_CYCLE)
			return (ret); //
	}

	return (__FINISHED_THIS_CYCLE);
}

__mode_return _player::Player_crouch_aim() {
	// this is the new aim mode where aim button must be held down to remain in the mode

	// check if key released
	if (!cur_state.IsButtonSet(__ARMUNARM)) { // let go of aim button
		Hard_start_reverse_new_mode(PUTTING_AWAY_CROUCH_GUN, __PULL_OUT_WEAPON);

		return (__FINISHED_THIS_CYCLE);
	}

	// must do this fist in-case coming from a link and diving straight out again
	log->cur_anim_type = __STAND;
	MS->Set_can_save(TRUE8); // can save

	if ((cur_state.IsButtonSet(__INVENTORY)) && (!inv_lock)) {
		if ((GetNoAmmoClips()) && (GetNoBullets() < (int32)GetBulletsPerClip())) {
			inv_lock = TRUE8; // not auto-repeat

			//			reload
			UseAmmoClips(1); // use a clip

			int32 bull_per_clip = GetBulletsPerClip();

			SetBullets(bull_per_clip); // reload

			Hard_start_new_mode(CROUCH_AIM, __LOAD_GUN);

			return (__FINISHED_THIS_CYCLE);
		}
	} else {
		// Stopped inventory dropping in repeatedly when you hold down the key.
		if (!(cur_state.IsButtonSet(__INVENTORY))) {
			inv_lock = FALSE8; // released
		}
	}

	// check for
	// forward step
	// step backward?
	if ((cur_state.momentum == __FORWARD_1) && (!forward_lock)) {
		Hard_start_new_mode(CROUCH_AIM, __STEP_FORWARD);
		player_status = GUN_LINKING;
		return (__FINISHED_THIS_CYCLE);
	} else if (cur_state.momentum == __STILL)
		forward_lock = FALSE8;

	// step backward?
	if ((cur_state.momentum == __BACKWARD_1) && (backward_lock == FALSE8)) {
		Hard_start_new_mode(CROUCH_AIM, __STEP_BACKWARD);
		forward_lock = FALSE8; // release a lock caused by hitting a wall
		return (__FINISHED_THIS_CYCLE);
	}
	// sidestep left?
	if ((cur_state.IsButtonSet(__SIDESTEP)) && (cur_state.turn == __LEFT)) {
		Hard_start_new_mode(CROUCH_AIM, __SIDESTEP_LEFT);
		forward_lock = FALSE8; // release a lock caused by hitting a wall
		backward_lock = FALSE8; // release a lock caused by hitting a wall
		return (__FINISHED_THIS_CYCLE);
	} else if ((cur_state.IsButtonSet(__SIDESTEP)) && (cur_state.turn == __RIGHT)) {
		Hard_start_reverse_new_mode(CROUCH_AIM, __SIDESTEP_LEFT);
		forward_lock = FALSE8; // release a lock caused by hitting a wall
		backward_lock = FALSE8; // release a lock caused by hitting a wall
		return (__FINISHED_THIS_CYCLE);
	}

	// are we turning on the spot
	if (cur_state.turn == __LEFT) {
		//  animate if frames present
		if (log->voxel_info->IsAnimTable(__TURN_ON_THE_SPOT_CLOCKWISE)) {
			//			ok, frames are present so we can do this thing
			//			set type for stageDraw

			log->cur_anim_type = __TURN_ON_THE_SPOT_CLOCKWISE;
			Reverse_frame_motion_and_pan(__TURN_ON_THE_SPOT_CLOCKWISE);
			log->pan += aim_turn_amount;
			forward_lock = FALSE8; // release a lock caused by hitting a wall
		}
	} else if (cur_state.turn == __RIGHT) {
		//		animate if frames present
		if (log->voxel_info->IsAnimTable(__TURN_ON_THE_SPOT_CLOCKWISE)) {
			//			ok, frames are present so we can do this thing
			//			set type for stageDraw

			log->cur_anim_type = __TURN_ON_THE_SPOT_CLOCKWISE;
			Advance_frame_motion_and_pan(__TURN_ON_THE_SPOT_CLOCKWISE);
			log->pan -= aim_turn_amount;
			forward_lock = FALSE8; // release a lock caused by hitting a wall
		}
	} else if (!cur_state.IsButtonSet(__CROUCH)) { // crouch down
		Hard_start_new_mode(CROUCH_TO_STAND_ARMED, __STAND_CROUCHED_TO_STAND);
		return (__FINISHED_THIS_CYCLE);
	} else {
		log->anim_pc = 0; // just stood

		__mode_return ret;
		ret = Player_press_remora_button();
		if (ret == __FINISHED_THIS_CYCLE)
			return (ret); //
	}

	return (__FINISHED_THIS_CYCLE);
}

__mode_return _player::Player_crouching() {
	// must do this fist in-case coming from a link and diving straight out again
	log->cur_anim_type = __STAND;
	MS->Set_can_save(TRUE8); // can save

	MS->Process_guard_alert(__ASTOOD);

	if (log->mega->Fetch_armed_status()) {
		MS->Set_pose(__CROUCH_NOT_ARMED);
		MS->Change_pose_in_current_anim_set();
		Message_box("just caught crouching player in armed set!");
	}

	if (!cur_state.IsButtonSet(__CROUCH)) { // crouch down
		Still_start_new_mode(CROUCH_TO_STAND_UNARMED, __STAND_CROUCHED_TO_STAND);
		return (__FINISHED_THIS_CYCLE);
	}

	// starting to walk again?
	if ((!forward_lock) && (cur_state.momentum == __FORWARD_1)) {
		Start_new_mode(CROUCH_WALK /*__STAND_TO_WALK*/);

		return (__FINISHED_THIS_CYCLE);
	} else if (cur_state.momentum == __STILL)
		forward_lock = FALSE8;

	// step backward?
	if ((cur_state.momentum == __BACKWARD_1) && (backward_lock == FALSE8)) {
		Hard_start_new_mode(CROUCHING, __STEP_BACKWARD);
		forward_lock = FALSE8; // release a lock caused by hitting a wall
		return (__FINISHED_THIS_CYCLE);
	}

	// are we turning on the spot
	if (cur_state.turn == __LEFT) {
		//		animate if frames present
		if (log->voxel_info->IsAnimTable(__TURN_ON_THE_SPOT_CLOCKWISE)) {
			log->cur_anim_type = __TURN_ON_THE_SPOT_CLOCKWISE; // set type for stageDraw

			Reverse_frame_motion_and_pan(__TURN_ON_THE_SPOT_CLOCKWISE);

			log->pan += crouch_turn_amount;
			forward_lock = FALSE8; // release a lock caused by hitting a wall
		}
	} else if (cur_state.turn == __RIGHT) {
		//		animate if frames present
		if (log->voxel_info->IsAnimTable(__TURN_ON_THE_SPOT_CLOCKWISE)) { // ok, frames are present so we can do this thing
			log->cur_anim_type = __TURN_ON_THE_SPOT_CLOCKWISE; // set type for stageDraw
			Advance_frame_motion_and_pan(__TURN_ON_THE_SPOT_CLOCKWISE);
			log->pan -= crouch_turn_amount;
			forward_lock = FALSE8; // release a lock caused by hitting a wall
		}
	} else if ((has_weapon) && (cur_state.IsButtonSet(__ARMUNARM))) { // pull gun

		MS->Set_pose(__CROUCH_GUN);
		MS->Change_pose_in_current_anim_set();

		if (armedChangesMode == 1)
			Push_control_mode(ACTOR_RELATIVE);

		Hard_start_new_mode(CROUCH_AIM, __PULL_OUT_WEAPON);

		return (__FINISHED_THIS_CYCLE);

	} else {
		log->anim_pc = 0; // in-case stopping turning

		__mode_return ret = Player_press_inv_button();
		if (ret == __FINISHED_THIS_CYCLE)
			return (ret); //

		ret = Player_press_remora_button();
		if (ret == __FINISHED_THIS_CYCLE)
			return (ret); //
	}

	return (__FINISHED_THIS_CYCLE);
}

__mode_return _player::Player_stood() {

	walk_count = 0; // reset walk count

	// must do this fist in-case coming from a link and diving straight out again
	log->cur_anim_type = __STAND;
	MS->Set_motion(__MOTION_WALK); // back to walk in-case were running
	MS->Set_can_save(TRUE8); // can save
	MS->floor_def->Allign_with_floor(log->mega);

	MS->Process_guard_alert(__ASTOOD);

	//	check for new parent box and if so bring barriers
	MS->Prepare_megas_route_barriers(TRUE8);

	// attempt to catch the armed set when not really bug

	if (log->mega->Fetch_armed_status()) {
		MS->Set_pose(__NOT_ARMED);
		MS->Change_pose_in_current_anim_set();
		Message_box("just caught player in armed set!");
	}

	// starting to walk again?
	if ((!forward_lock) && (cur_state.momentum == __FORWARD_1)) {
		Hard_start_new_mode(WALKING, __STAND_TO_WALK);

		return (__FINISHED_THIS_CYCLE);
	} else if (cur_state.momentum == __STILL)
		forward_lock = FALSE8;

	// step backward?
	if ((cur_state.momentum == __BACKWARD_1) && (backward_lock == FALSE8)) {
		Hard_start_new_mode(STOOD, __STEP_BACKWARD);
		forward_lock = FALSE8; // release a lock caused by hitting a wall
		return (__FINISHED_THIS_CYCLE);
	}

	// sidestep left?
	if ((cur_state.IsButtonSet(__SIDESTEP)) && (cur_state.turn == __LEFT)) {
		Hard_start_new_mode(STOOD, __SIDESTEP_LEFT);
		forward_lock = FALSE8; // release a lock caused by hitting a wall
		backward_lock = FALSE8; // release a lock caused by hitting a wall
		return (__FINISHED_THIS_CYCLE);
	} else if ((cur_state.IsButtonSet(__SIDESTEP)) && (cur_state.turn == __RIGHT)) {
		Hard_start_reverse_new_mode(STOOD, __SIDESTEP_LEFT);
		forward_lock = FALSE8; // release a lock caused by hitting a wall
		backward_lock = FALSE8; // release a lock caused by hitting a wall
		return (__FINISHED_THIS_CYCLE);
	}

	// are we turning on the spot
	if ((cur_state.turn == __LEFT) && (log->voxel_info->IsAnimTable(__TURN_ON_THE_SPOT_CLOCKWISE))) { // animate if frames present
		log->cur_anim_type = __TURN_ON_THE_SPOT_CLOCKWISE; // set type for stageDraw
		Reverse_frame_motion_and_pan(__TURN_ON_THE_SPOT_CLOCKWISE);

		if (player_status != STOOD)
			return __FINISHED_THIS_CYCLE; // got on a ladder?

		log->pan += stood_turn_amount;
		forward_lock = FALSE8; // release a lock caused by hitting a wall
	} else if ((cur_state.turn == __RIGHT) && (log->voxel_info->IsAnimTable(__TURN_ON_THE_SPOT_CLOCKWISE))) { // animate if frames present
		log->cur_anim_type = __TURN_ON_THE_SPOT_CLOCKWISE; // set type for stageDraw
		Advance_frame_motion_and_pan(__TURN_ON_THE_SPOT_CLOCKWISE);

		if (player_status != STOOD)
			return __FINISHED_THIS_CYCLE; // got on a ladder?

		log->pan -= stood_turn_amount;
		forward_lock = FALSE8; // release a lock caused by hitting a wall
	} else if ((cur_state.turn == __HARD_LEFT) && (log->voxel_info->IsAnimTable(__TURN_ON_THE_SPOT_CLOCKWISE))) { // animate if frames present
		log->cur_anim_type = __TURN_ON_THE_SPOT_CLOCKWISE; // set type for stageDraw
		Reverse_frame_motion_and_pan(__TURN_ON_THE_SPOT_CLOCKWISE);

		if (player_status != STOOD)
			return __FINISHED_THIS_CYCLE; // got on a ladder?

		log->pan += stood_fast_turn_amount;
		forward_lock = FALSE8; // release a lock caused by hitting a wall
	} else if ((cur_state.turn == __HARD_RIGHT) && (log->voxel_info->IsAnimTable(__TURN_ON_THE_SPOT_CLOCKWISE))) { // animate if frames present
		log->cur_anim_type = __TURN_ON_THE_SPOT_CLOCKWISE; // set type for stageDraw
		Advance_frame_motion_and_pan(__TURN_ON_THE_SPOT_CLOCKWISE);

		if (player_status != STOOD)
			return __FINISHED_THIS_CYCLE; // got on a ladder?

		log->pan -= stood_fast_turn_amount;
		forward_lock = FALSE8; // release a lock caused by hitting a wall
	}

	else if ((has_weapon) && (cur_state.IsButtonSet(__ARMUNARM))) { // pull gun

		MS->Set_pose(__GUN);
		MS->Change_pose_in_current_anim_set();

		if (armedChangesMode == 1)
			Push_control_mode(ACTOR_RELATIVE);

		Hard_start_new_mode(NEW_AIM, __PULL_OUT_WEAPON);

		return (__FINISHED_THIS_CYCLE);
	} else if (cur_state.IsButtonSet(__CROUCH)) { // crouch down
		MS->Set_pose(__CROUCH_NOT_ARMED);
		MS->Change_pose_in_current_anim_set();
		Still_reverse_start_new_mode(CROUCHING, __STAND_CROUCHED_TO_STAND);
		return (__FINISHED_THIS_CYCLE);
	}

	else {
		log->anim_pc = 0; // in-case stopping turning

		__mode_return ret = Player_press_inv_button();
		if (ret == __FINISHED_THIS_CYCLE)
			return (ret); //

		ret = Player_press_remora_button();
		if (ret == __FINISHED_THIS_CYCLE)
			return (ret);
	}

	return (__FINISHED_THIS_CYCLE);
}

__mode_return _player::Player_crouch_walk() {
	// player is crouching and walking
	// see what happens this cycle

	bool8 ret;

	// uncrouch when walking
	if (!cur_state.IsButtonSet(__CROUCH)) { // crouch down
		Hard_start_new_mode(CROUCH_TO_STAND_UNARMED, __STAND_CROUCHED_TO_STAND);
		return (__FINISHED_THIS_CYCLE);
	}

	// set anim set
	log->cur_anim_type = __WALK;
	MS->Set_motion(__MOTION_WALK); // high level
	MS->Set_can_save(TRUE8); // can save
	MS->Process_guard_alert(__ASTOOD);

	// is the player still moving forward?
	if (cur_state.momentum == __FORWARD_1) {
		if (cur_state.turn == __LEFT) {
			log->pan += walk_turn_amount;
			if (log->pan >= HALF_TURN)
				log->pan -= FULL_TURN;
		}

		else if (cur_state.turn == __RIGHT) {
			log->pan -= walk_turn_amount;
			if (log->pan <= -HALF_TURN)
				log->pan += FULL_TURN;
		}

		// shift character and frame forward by the amount appropriate

		ret = MS->Advance_frame_and_motion(__WALK, TRUE8, 1);
		MS->Normalise_anim_pc();

		if (!ret) {
			//			could not walk forward

			forward_lock = TRUE8;

			Start_new_mode(CROUCHING);

			return (__FINISHED_THIS_CYCLE);
		}

		if (cur_state.IsButtonSet(__ARMUNARM)) { // pull gun
			forward_lock = TRUE8;

			MS->Set_pose(__CROUCH_GUN);
			MS->Change_pose_in_current_anim_set();
			if (armedChangesMode == 1)
				Push_control_mode(ACTOR_RELATIVE);
			Hard_start_new_mode(CROUCH_AIM, __WALK_TO_PULL_OUT_WEAPON);

			return (__FINISHED_THIS_CYCLE);
		}

		// enough for now
		return (__FINISHED_THIS_CYCLE);
	}

	Start_new_mode(CROUCHING);

	return (__FINISHED_THIS_CYCLE);
}

__mode_return _player::Player_walking() {
	// player is walking
	// see what happens this cycle

	// we have rotate left/right and forward

	// keys same as last go?

	bool8 ret;

	if (log->mega->Fetch_armed_status()) {
		MS->Set_pose(__NOT_ARMED);
		MS->Change_pose_in_current_anim_set();
		Message_box("player_walking  - just caught player in armed set!");
	}

	walk_count++; // up the walk count
	MS->Process_guard_alert(__AWALKING);

	// set anim set
	log->cur_anim_type = __WALK;
	MS->Set_motion(__MOTION_WALK); // high level
	MS->Set_can_save(TRUE8); // can save

	// arm?
	if ((has_weapon) && (cur_state.IsButtonSet(__ARMUNARM))) { // pull gun
		MS->Reset_guard_alert();
		forward_lock = TRUE8;
		MS->Set_pose(__GUN);
		MS->Change_pose_in_current_anim_set();
		if (armedChangesMode == 1)
			Push_control_mode(ACTOR_RELATIVE);
		Soft_start_new_mode(NEW_AIM, __WALK_TO_PULL_OUT_WEAPON);
		return (__FINISHED_THIS_CYCLE);
	}

	if (cur_state.IsButtonSet(__CROUCH)) { // crouch down
		MS->Reset_guard_alert();
		walk_count = 0; // cancel walk count
		MS->Set_pose(__CROUCH_NOT_ARMED);
		MS->Change_pose_in_current_anim_set();
		Hard_start_reverse_new_mode(CROUCH_WALK, __STAND_CROUCHED_TO_STAND);
		return (__FINISHED_THIS_CYCLE);
	}

	// is the player still moving forward?
	if (cur_state.momentum == __FORWARD_1) {

		if (cur_state.turn == __LEFT) {
			log->pan += walk_turn_amount;
			if (log->pan >= HALF_TURN)
				log->pan -= FULL_TURN;
		}

		else if (cur_state.turn == __RIGHT) {
			log->pan -= walk_turn_amount;
			if (log->pan <= -HALF_TURN)
				log->pan += FULL_TURN;
		}

		//		shift character and frame forward by the amount appropriate
		ret = MS->Advance_frame_and_motion(__WALK, TRUE8, 1);
		MS->Normalise_anim_pc();

		if (!ret) {
			//			could not walk forward

			forward_lock = TRUE8;

			Soft_start_new_mode(STOOD, __STEP_BACKWARD_TO_STAND, __STEP_BACKWARD_TO_OTHER_STAND_LEFT);

			return (__FINISHED_THIS_CYCLE);
		}

		// enough for now
		return (__FINISHED_THIS_CYCLE);
	} else if (cur_state.momentum == __FORWARD_2) { // run
		//		break into a run

		Start_new_mode(RUNNING); //, __WALK_TO_STAND, __WALK_TO_OTHER_STAND_LEFT_LEG);

		return (__MORE_THIS_CYCLE);
	}

	// otherwise, we must be stopping
	MS->Reset_guard_alert();
	Soft_start_new_mode(STOOD, __WALK_TO_STAND, __WALK_TO_OTHER_STAND_LEFT_LEG);

	return (__FINISHED_THIS_CYCLE);
}

__mode_return _player::Player_running() {
	// player is running
	// see what happens this cycle

	// we have rotate left/right and forward

	// keys same as last go?

	bool8 ret;

	if (log->mega->Fetch_armed_status()) {
		MS->Set_pose(__NOT_ARMED);
		MS->Change_pose_in_current_anim_set();
		Message_box("player_running  - just caught player in armed set!");
	}

	MS->Process_guard_alert(__ARUNNING);

	// set anim set
	log->cur_anim_type = __RUN;
	MS->Set_motion(__MOTION_RUN); // wtf is this for?
	MS->Set_can_save(TRUE8); // can save

	// arm?
	if ((has_weapon) && (cur_state.IsButtonSet(__ARMUNARM))) { // pull gun
		MS->Reset_guard_alert();
		forward_lock = TRUE8;
		MS->Set_pose(__GUN);
		MS->Change_pose_in_current_anim_set();
		if (armedChangesMode == 1)
			Push_control_mode(ACTOR_RELATIVE);
		Soft_start_new_mode(NEW_AIM, __RUN_TO_PULL_OUT_WEAPON);
		return (__FINISHED_THIS_CYCLE);
	}

	// is the player still moving forward?
	if (cur_state.momentum == __FORWARD_2) {

		if (cur_state.turn == __LEFT) {
			log->pan += run_turn_amount;
			if (log->pan >= HALF_TURN)
				log->pan -= FULL_TURN;
		}

		else if (cur_state.turn == __RIGHT) {
			log->pan -= run_turn_amount;
			if (log->pan <= -HALF_TURN)
				log->pan += FULL_TURN;
		}

		// shift character and frame forward by the amount appropriate
		ret = MS->Advance_frame_and_motion(__RUN, TRUE8, 1);
		MS->Normalise_anim_pc();

		if (!ret) {
			//			could not run forward

			forward_lock = TRUE8;

			Soft_start_new_mode(STOOD, __STEP_BACKWARD_TO_STAND, __STEP_BACKWARD_TO_OTHER_STAND_LEFT);

			return (__FINISHED_THIS_CYCLE);
		}

		// enough for now
		return (__FINISHED_THIS_CYCLE);
	} else if (cur_state.momentum == __FORWARD_1) { // walk
		//		break into a walk

		Soft_start_new_mode_no_link(WALKING, __WALK); //, __WALK_TO_STAND, __WALK_TO_OTHER_STAND_LEFT_LEG);

		return (__FINISHED_THIS_CYCLE);
	}

	// otherwise, we must be stopping
	MS->Reset_guard_alert();
	Hard_start_new_mode(STOOD, __RUN_TO_STAND);
	log->anim_pc = 4; // TOTAL BODGE TO CHOP OFF INITIAL FRAMES THAT PROPEL THE CHARACTER FORWARD IN UNNATURAL MANNER

	return (__FINISHED_THIS_CYCLE);
}

bool8 _player::Advance_frame_motion_and_pan(__mega_set_names anim_type) {
	// attempts to move the frame forward and move the character
	// returns fail and frame is not changed if the way forward is blocked by a barrier
	// the frame counter will be looped
	__barrier_result ret;
	PXreal xnext, znext;
	PXreal x, z;

	// get anim set
	PXanim *pAnim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(anim_type), log->voxel_info->info_name_hash[anim_type], log->voxel_info->base_path,
	                                             log->voxel_info->base_path_hash); //

	if (log->anim_pc + 1 >= pAnim->frame_qty)
		Fatal_error("Advance_frame_motion_and_pan finds [%s] has illegal frame in anim [%s] %d %d", (const char *)log->GetName(),
		            (const char *)log->voxel_info->get_info_name(anim_type), log->anim_pc, pAnim->frame_qty);

	// get pan of previous frame and next frame so we can advance the engine pan by the difference
	// update engine pan
	PXreal pan1, pan2;

	// Get the next frame from the anim
	PXframe *nextFrame = PXFrameEnOfAnim(log->anim_pc + 1, pAnim);
	// Get the current frame from the anim
	PXframe *currentFrame = PXFrameEnOfAnim(log->anim_pc, pAnim);

	nextFrame->markers[ORG_POS].GetPan(&pan1);
	currentFrame->markers[ORG_POS].GetPan(&pan2);

	log->pan += (pan1 - pan2); // update by difference

	// get motion displacement from currently displayed frame to next one
	// note that we always read frame+1 for motion of next frame even though the voxel frame itself will be looped back to 0
	PXreal x1, x2, z1, z2, unused;
	nextFrame->markers[ORG_POS].GetXYZ(&x1, &unused, &z1);
	currentFrame->markers[ORG_POS].GetXYZ(&x2, &unused, &z2);

	xnext = x1 - x2;
	znext = z1 - z2;

	// update pc
	log->anim_pc = (log->anim_pc + 1) % (pAnim->frame_qty - 1);

	// get the pan unwind value of the frame to be printed
	PXreal pan;
	PXFrameEnOfAnim(log->anim_pc, pAnim)->markers[ORG_POS].GetPan(&pan);
	log->pan_adjust = pan; // this value will be unwound from the orientation of the frame at render time in stage draw

	// calculate the new x and z coordinate from this frames motion offset
	// do the z and x together

	PXfloat ang = (log->pan - log->pan_adjust) * TWO_PI;
	PXfloat cang = (PXfloat)PXcos(ang);
	PXfloat sang = (PXfloat)PXsin(ang);

	x = log->mega->actor_xyz.x + PXfloat2PXreal(xnext * cang + znext * sang);
	z = log->mega->actor_xyz.z + PXfloat2PXreal(znext * cang - xnext * sang);
	//	x and z are the new coordinates

	PXfloat safe_pan = log->pan; // save the pan

	// ok, check for a new collision with a barrier
	ret = MS->Check_barrier_bump_and_bounce(x, log->mega->actor_xyz.y, z,                                           // new position
	                                        log->mega->actor_xyz.x, log->mega->actor_xyz.y, log->mega->actor_xyz.z, // old position
	                                        TRUE8);

	if (ret == __NUDGED)
		return TRUE8; // on ladder?

	//	did we move forward without a problem?
	if (ret == __OK) {
		//		record the new true actor position
		log->mega->actor_xyz.x = x;
		log->mega->actor_xyz.z = z;

		//		check for new parent box and if so bring barriers
		MS->Prepare_megas_route_barriers(TRUE8);

	} else {
		log->pan = safe_pan; // we bounced so cancel the pan correction that will have been done
	}

	CORRECT_PAN // stop the wrap

	    return (TRUE8);
}

bool8 _player::Reverse_frame_motion_and_pan(__mega_set_names anim_type) {
	// frames go backward
	// each frame projects its motion in the normal manner, but, obviously, the effect is of the anim moving backward

	// the frame counter will be looped

	PXreal xnext, znext;
	PXreal x, z;
	uint32 next_pc;
	__barrier_result ret;

	// get anim set
	PXanim *pAnim = (PXanim *)rs_anims->Res_open(log->voxel_info->get_info_name(anim_type), log->voxel_info->info_name_hash[anim_type], log->voxel_info->base_path,
	                                             log->voxel_info->base_path_hash); //

	// if we are on frame 0 then shift up to pretend we're coming in off the dummy frame
	if (!log->anim_pc) {
		next_pc = pAnim->frame_qty - 2;
		log->anim_pc = pAnim->frame_qty - 1;
	} else
		next_pc = (log->anim_pc - 1) % (pAnim->frame_qty - 1); // next_pc=log->anim_pc-1;

	if ((next_pc >= pAnim->frame_qty) || (log->anim_pc >= pAnim->frame_qty))
		Fatal_error("Reverse_frame_motion_and_pan finds [%s] has illegal frame in anim [%s] %d %d %d", (const char *)log->GetName(),
		            (const char *)log->voxel_info->get_info_name(anim_type), next_pc, log->anim_pc, pAnim->frame_qty);

	// get pan of previous frame and next frame so we can advance the engine pan by the difference
	// update engine pan
	PXreal pan1, pan2;

	// Get the next frame from the anim
	PXframe *nextFrame = PXFrameEnOfAnim(next_pc, pAnim);
	// Get the current frame from the anim
	PXframe *currentFrame = PXFrameEnOfAnim(log->anim_pc, pAnim);

	nextFrame->markers[ORG_POS].GetPan(&pan1);
	currentFrame->markers[ORG_POS].GetPan(&pan2);

	log->pan += (pan1 - pan2); // update by difference

	// get motion displacement from currently displayed frame to next one
	// note that we always read frame+1 for motion of next frame even though the voxel frame itself will be looped back to 0
	PXreal x1, x2, z1, z2, unused;
	nextFrame->markers[ORG_POS].GetXYZ(&x1, &unused, &z1);
	currentFrame->markers[ORG_POS].GetXYZ(&x2, &unused, &z2);

	xnext = x1 - x2;
	znext = z1 - z2;

	// update pc
	log->anim_pc = next_pc; // allready computed

	// get the pan unwind value of the frame to be printed
	PXreal pan;
	nextFrame->markers[ORG_POS].GetPan(&pan);

	log->pan_adjust = pan;

	// calculate the new x and z coordinate from this frames motion offset
	// do the z and x together
	PXfloat ang = (log->pan - log->pan_adjust) * TWO_PI;
	PXfloat cang = (PXfloat)PXcos(ang);
	PXfloat sang = (PXfloat)PXsin(ang);

	x = log->mega->actor_xyz.x + PXfloat2PXreal(xnext * cang + znext * sang);
	z = log->mega->actor_xyz.z + PXfloat2PXreal(znext * cang - xnext * sang);
	// x and z are the new coordinates

	PXfloat safe_pan = log->pan; // save the pan

	// ok, check for a new collision with a barrier
	ret = MS->Check_barrier_bump_and_bounce(x, log->mega->actor_xyz.y, z,                                           // new position
	                                        log->mega->actor_xyz.x, log->mega->actor_xyz.y, log->mega->actor_xyz.z, // old position
	                                        TRUE8);

	if (ret == __NUDGED)
		return TRUE8; // on ladder?

	// did we move forward without a problem?
	if (ret == __OK) {
		//		record the new true actor position
		log->mega->actor_xyz.x = x;
		log->mega->actor_xyz.z = z;

		//		check for new parent box and if so bring barriers
		MS->Prepare_megas_route_barriers(TRUE8);
	} else {
		log->pan = safe_pan; // we bounced so cancel the pan correction that will have been done
	}

	CORRECT_PAN // stop the wrap

	    return (TRUE8);
}

void _player::Set_player_id(uint32 id) {
	// declare object id of player to player system

	// must be legal id
	_ASSERT(id < MS->Fetch_number_of_objects());

	player_id = id;

	player_exists = TRUE8; // done for safety checks

	Zdebug("\nSet_player_id %d", player_id);

	// get player structures - we can be sure they wont get moved
	log = g_mission->session->Fetch_object_struct(player_id);

	// get initial barriers for player
	MS->Prepare_megas_route_barriers(TRUE8);

	// reset pointer to player parent barrier box
	MS->logic_structs[id]->mega->cur_parent = NULL;

	crouch = FALSE8; // not crouching

	backward_lock = FALSE8; // back repeat stop
	forward_lock = FALSE8;
	interact_lock = FALSE8;

	// set to stood
	player_status = STOOD;
}

void _player::AddMediPacks(uint32 num, bool8 bFlashIcons) {
	g_mission->num_medi += num;

	// Check if flashing icon has been requested.
	if (bFlashIcons)
		g_oIconMenu->SetAddingMedipacksCount(num);
}

void _player::AddAmmoClips(uint32 num, bool8 bFlashIcons) {
	g_mission->num_clips += num;

	uint32 maxClips = GetMaxClips();
	if (g_mission->num_clips > maxClips)
		g_mission->num_clips = maxClips;

	// Check if flashing icon has been requested.
	if (bFlashIcons)
		g_oIconMenu->SetAddingClipsCount(num);
}

void _player::SetBullets(uint32 num) {
	g_mission->num_bullets = num;
}

void _player::UseBullets(uint32 num) {
	g_mission->num_bullets -= num;
}

int32 _player::GetNoBullets() {
	return g_mission->num_bullets;
}

void _player::UseAmmoClips(uint32 num) {
	g_mission->num_clips -= num;
}

int32 _player::GetNoAmmoClips() {
	return g_mission->num_clips;
}

void _player::UseMediPacks(uint32 num) {
	g_mission->num_medi -= num;
}

int32 _player::GetNoMediPacks() {
	return g_mission->num_medi;
}

void _game_session::Reset_guard_alert() {
	// clear all alerts - call this when leaving a mode that can cause alert

	uint32 j;

	for (j = 0; j < MAX_voxel_list; j++)
		alert_list[j] = 0;
}

void _game_session::Process_guard_alert(__alert alert_type) {
	// player is walking or running
	// check if he is behind any guards and if so alert them
	// called the players cycle so can use L M etc

#define TOUCH_ALERT_DIST (70 * 70)
#define PUNCH_ALERT_DIST (200 * 200)
#define WALK_ALERT_DIST (200 * 200)
#define RUN_ALERT_DIST (400 * 400)

	static bool8 reset = FALSE8;

	uint32 j;
	PXreal len;
	int32 noise_level;

	// if first game cycle then reset list
	if (!reset)
		for (j = 0; j < MAX_voxel_list; j++)
			alert_list[j] = 0;

	reset = TRUE8;

	if ((alert_type == __AWALKING) && (player.walk_count < 8))
		return; // walking but not enough steps taken

	// we are running or have walked enough to make a sound

	noise_level = GetCurrentSoundLevel();

	Tdebug("fx.txt", "%d", noise_level);

	for (j = 0; j < number_of_voxel_ids; j++) {
		if (cur_id != voxel_id_list[j]) { // not us
			if (!g_oLineOfSight->LineOfSight(voxel_id_list[j], player.Fetch_player_id())) { // cant see
				if (PXfabs(logic_structs[voxel_id_list[j]]->mega->actor_xyz.y - M->actor_xyz.y) < (200 * REAL_ONE)) { // slack for height calc
					PXreal sub1 = logic_structs[voxel_id_list[j]]->mega->actor_xyz.x - M->actor_xyz.x;
					PXreal sub2 = logic_structs[voxel_id_list[j]]->mega->actor_xyz.z - M->actor_xyz.z;

					len = (PXreal)((sub1 * sub1) + (sub2 * sub2)); // dist away

					//					check for touching the guard
					if (len < TOUCH_ALERT_DIST) {
						alert_list[j] = TRUE8;
						Force_context_check(voxel_id_list[j]);
						return;
					}

					if (alert_type == __ARUNNING) {
						if ((!alert_list[j]) && (len < RUN_ALERT_DIST) && (noise_level < 75)) {
							alert_list[j] = TRUE8;
							//							send event
							Force_context_check(voxel_id_list[j]);
						} else if (len >= RUN_ALERT_DIST) {
							alert_list[j] = FALSE8;
						}
					} else if (alert_type == __AWALKING) { // walking
						if ((!alert_list[j]) && (len < WALK_ALERT_DIST) && (noise_level < 50)) {
							alert_list[j] = TRUE8;
							//							send event
							Force_context_check(voxel_id_list[j]);
						} else if (len >= WALK_ALERT_DIST) {
							alert_list[j] = FALSE8;
						}

					} else if (alert_type == __APUNCHING) { // punching
						if ((!alert_list[j]) && (len < PUNCH_ALERT_DIST)) {
							alert_list[j] = TRUE8;
							//							send event
							Force_context_check(voxel_id_list[j]);
						} else if (len >= PUNCH_ALERT_DIST) {
							alert_list[j] = FALSE8;
						}
					}
				}
			} else
				alert_list[j] = FALSE8; // can see so cancel a previously set alert
		}
	}
}

void _game_session::Signal_to_other_guards() {
	// players just got gun out
	// we need to tell all megas who can see us to rerun contexts because they may need to react

	uint32 j;

	for (j = 0; j < number_of_voxel_ids; j++) // run through them
		if (logic_structs[voxel_id_list[j]]->mega->is_evil) // evil ones only
			if (g_oLineOfSight->LineOfSight(voxel_id_list[j], player.Fetch_player_id())) { // can see player
				if ((player.interact_selected) && (player.cur_interact_id == voxel_id_list[j])) {
				} else {
					Force_context_check(voxel_id_list[j]); // must rerun
				}
			}
}

void _game_session::Signal_to_guards() {
	// players just got gun out
	// we need to tell all megas who can see us to rerun contexts because they may need to react

	uint32 j;

	for (j = 0; j < number_of_voxel_ids; j++) // run through them
		if (logic_structs[voxel_id_list[j]]->mega->is_evil) // evil ones only
			if (g_oLineOfSight->LineOfSight(voxel_id_list[j], player.Fetch_player_id())) // can see player
				Force_context_check(voxel_id_list[j]); // must rerun
}

mcodeFunctionReturnCodes _game_session::fn_can_hear_players_feet(int32 &result, int32 *) {
	// can this mega hear the players footsteps

	uint32 j;

	for (j = 0; j < MAX_voxel_list; j++)
		if (voxel_id_list[j] == cur_id) {
			result = alert_list[j];
			return IR_CONT;
		}

	Fatal_error("fn_can_hear_players_feet says you should never see this");

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_is_player_striking(int32 &result, int32 *) {
	// is the player punching, yes or no

	if (player.player_status == STRIKING)
		result = 1;
	else
		result = 0;

	return IR_CONT;
}

void _player::Reset_player() {
	// set the player back to standing
	// used afer gunshot, etc
	// see fn-reset-player

	crouch = FALSE8; // not crouching

	walk_count = 0; // reset walk count for guard alerting

	forward_lock = FALSE8;
	interact_lock = FALSE8;

	Start_new_mode(STOOD);
}

void _game_session::Restart_player() {
	camera_lock = FALSE8; // let the camera be working again

	// for debugging purposes

	cur_id = player.Fetch_player_id();
	L = logic_structs[cur_id];
	I = L->voxel_info;
	M = L->mega;
	MS->Set_pose(__NOT_ARMED);
	MS->Change_pose_in_current_anim_set();

	player.Reset_player();

	int32 var_num;
	c_game_object *ob;

	ob = (c_game_object *)objects->Fetch_item_by_number(player.Fetch_player_id());

	var_num = ob->GetVariable("state");
	if (var_num == -1)
		Fatal_error("Restart_player cant fetch state");
	ob->SetIntegerVariable(var_num, 0); // alive

	var_num = ob->GetVariable("hits");
	if (var_num == -1)
		Fatal_error("Restart_player cant fetch hits");
	ob->SetIntegerVariable(var_num, MAX_HITS); // another 10 hits

	L->logic_level = 0; // restart
	L->logic_ref[1] = 0;

	M->dead = 0; // not dead!!!

	player.SetBullets(9);
	player.AddAmmoClips(5, 0);
}

uint32 _player::GetBulletsPerClip() {
	uint32 bull_per_clip = g_globalScriptVariables.GetVariable("bullets_per_clip");
	return bull_per_clip;
}

uint32 _player::GetMaxClips() {
	return MAX_AMMO_CLIPS;
}

} // End of namespace ICB
