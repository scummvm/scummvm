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

#include "engines/icb/common/px_common.h"
#include "engines/icb/session.h"
#include "engines/icb/mission.h"
#include "engines/icb/p4.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/custom_logics.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/common/px_prop_anims.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/sound.h"

namespace ICB {

mcodeFunctionReturnCodes fn_set_custom_simple_animator(int32 &result, int32 *params) { return (MS->fn_set_custom_simple_animator(result, params)); }

mcodeFunctionReturnCodes fn_set_custom_button_operated_door(int32 &result, int32 *params) { return (MS->fn_set_custom_button_operated_door(result, params)); }

mcodeFunctionReturnCodes fn_set_custom_auto_door(int32 &result, int32 *params) { return (MS->fn_set_custom_auto_door(result, params)); }

mcodeFunctionReturnCodes fn_set_cad_lock_status(int32 &result, int32 *params) { return (MS->fn_set_cad_lock_status(result, params)); }

mcodeFunctionReturnCodes fn_get_cad_state_flag(int32 &result, int32 *params) { return (MS->fn_get_cad_state_flag(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_set_custom_simple_animator(int32 &, int32 *) {
	// chnage the object to be a special simple animator type where all scripts are circumvented
	// we need to check here, first, for legality - i.e. we clear the way for run-time assumptions to be made
	_animating_prop *index;
	_animation_entry *anim;

	// find entry for this object via its name
	index = (_animating_prop *)prop_anims->Try_fetch_item_by_name(object->GetName());

	// get anim
	anim = (_animation_entry *)(((char *)index) + index->anims[0]);

	// check for no frame
	if (!anim->num_frames) {
		Tdebug("objects_that_died.txt", "fn_set_custom_simple_animator [%s] loop anim has 0 frames", object->GetName());
		Shut_down_object("by fn_set_custom_simple_animator");
		return (IR_STOP);
	}

	// start at frame 0
	L->anim_pc = 0;

	L->big_mode = __CUSTOM_SIMPLE_ANIMATE;

	SA_INDEX = prop_anims->Fetch_item_number_by_name(object->GetName());

	// object will pause when off screen
	L->hold_mode = prop_camera_hold;

	Tdebug("logic_modes.txt", "fn_set_custom_simple_animator switching [%s]", object->GetName());

	return (IR_CONT);
}

void _game_session::Custom_simple_animator() {
	// this special _big_mode logic replaces the logic script of a lib_simple_animator object
	// this is a rationalisation designed to aid speed up on psx
	_animating_prop *index;
	_animation_entry *anim;

	// get index for object
	index = (_animating_prop *)prop_anims->Fetch_item_by_number(SA_INDEX);

	// now then, lets make the assumption that anim 0 will be the 'looping' one
	anim = (_animation_entry *)(((char *)index) + index->anims[0]);

	if ((uint8)L->anim_pc == (anim->num_frames - 1))
		L->anim_pc = 0;
	else
		L->anim_pc++; // advance current pc

	// set frame
	prop_state_table[cur_id] = anim->frames[L->anim_pc];
}

mcodeFunctionReturnCodes _game_session::fn_set_custom_button_operated_door(int32 &, int32 *params) {
	// set to special custom door logic

	// params    0       initial state value
	// prime
	BOD_STATE = params[0];

	// starts non animating
	BOD_CONTROL = 0; // not opening or closing

	// switch out of script mode
	L->big_mode = __CUSTOM_BUTTON_OPERATED_DOOR;

	BOD_INDEX = prop_anims->Fetch_item_number_by_name(object->GetName());

	BOD_OPEN_NO = Validate_prop_anim("opening");
	BOD_CLOSE_NO = Validate_prop_anim("closing");

	Tdebug("logic_modes.txt", "fn_set_custom_button_operated_door switching [%s]", object->GetName());

	// Set a symbol type so the Remora knows what to draw.
	L->object_type = __BUTTON_OPERATED_DOOR;

	// push the y up for Cord to look at
	L->prop_xyz.y += 179;

	return IR_CONT;
}

void _game_session::Custom_button_operated_door() {
	// this special _big_mode logic replaces the logic script of a lib_simple_animator object
	// this is a rationalisation designed to aid speed up on psx

	_animating_prop *index;
	_animation_entry *anim;
	uint32 var_num;
	int32 params, result;

	do { // do while because when we switch in to new anim we need to do an anim
		if (BOD_CONTROL == BOD_WAITING) {
			// we've opened, now we wait until no one in the list is near to us
			if (!BOD_WAIT_COUNT) {
				params = 300;
				fn_near_list(result, &params);

				if (result == TRUE8) {
					BOD_WAIT_COUNT = 36; // count again
					return;
				}
				// else, close

				L->anim_pc = 0;
				BOD_CONTROL = BOD_CLOSING;
				BOD_STATE = 1; // closed

				// set state flag to 1
				var_num = object->GetVariable("state");
				object->SetIntegerVariable(var_num, 1);
			} else {
				BOD_WAIT_COUNT--;
				return;
			}
		}

		if (BOD_CONTROL == BOD_OPENING) {
			// get index for object
			index = (_animating_prop *)prop_anims->Fetch_item_by_number(BOD_INDEX);
			anim = (_animation_entry *)(((char *)index) + index->anims[BOD_OPEN_NO]);
			prop_state_table[cur_id] = anim->frames[L->anim_pc];

			if ((uint8)L->anim_pc == (anim->num_frames - 1)) {
				BOD_CONTROL = BOD_WAITING; // just opened - set to wait for list of people to not be here
				BOD_WAIT_COUNT = 36;
				BOD_STATE = 0; // open
				// set state flag to 0
				var_num = object->GetVariable("state");
				object->SetIntegerVariable(var_num, 0);

			} else
				L->anim_pc++; // frame on
			return;
		} else if (BOD_CONTROL == BOD_CLOSING) {
			// get index for object
			index = (_animating_prop *)prop_anims->Try_fetch_item_by_name(object->GetName());
			anim = (_animation_entry *)(((char *)index) + index->anims[BOD_CLOSE_NO]);

			// set frame
			prop_state_table[cur_id] = anim->frames[L->anim_pc];

			if ((uint8)L->anim_pc == (anim->num_frames - 1))
				BOD_CONTROL = 0; // cancel mode
			else
				L->anim_pc++; // frame on

			return;
		}

		// ok, we're not animating so just check state
		//  what state are we in?
		if (!BOD_STATE) { // open
			// ok, check to see if opened
			// get state variable number
			var_num = object->GetVariable("state");
			// get value
			if (!object->GetIntegerVariable(var_num)) {
				BOD_CONTROL = BOD_WAITING; // just opened - set to wait for list of people to not be here
				BOD_WAIT_COUNT = 36;
				return; // zero so still open
			}

			L->anim_pc = 0;

			BOD_CONTROL = BOD_CLOSING;

			BOD_STATE = 1; // closed

			// close sound
			if (logic_structs[cur_id]->sfxVars[CLOSE_SFX_VAR] != 0)
				RegisterSound(cur_id, NULL, logic_structs[cur_id]->sfxVars[CLOSE_SFX_VAR], closeDesc,
				              (int8)127); // have to use full version so we can give hash instead of string
			else
				RegisterSound(cur_id, defaultCloseSfx, closeDesc); // use small version as we have string not hash
		} else {                                                           // closed
			// check for
			// a. someone pressing button
			// b. chi off screen being near

			bool8 open = FALSE8;

			if (is_there_a_chi) {
				// if chi is on a different floor from player then assume she is following and so auto open the door for her
				// - sneaky
				if (logic_structs[chi_id]->owner_floor_rect != logic_structs[player.Fetch_player_id()]->owner_floor_rect) {
					PXreal sub1, sub2;

					sub1 = logic_structs[chi_id]->mega->actor_xyz.x - L->prop_xyz.x;
					sub2 = logic_structs[chi_id]->mega->actor_xyz.z - L->prop_xyz.z;

					// dist
					if (((sub1 * sub1) + (sub2 * sub2)) < (200 * 200)) {
						open = TRUE8;
					}
				}
			}
			// get state variable number
			var_num = object->GetVariable("state");

			// get value
			if (!object->GetIntegerVariable(var_num)) {
				open = TRUE8; // now 0 so start opening
			}
			if (open) {
				L->anim_pc = 0;
				BOD_CONTROL = BOD_OPENING;
				BOD_STATE = 0; // open

				// open sound
				if (logic_structs[cur_id]->sfxVars[OPEN_SFX_VAR] != 0)
					RegisterSound(cur_id, NULL, logic_structs[cur_id]->sfxVars[OPEN_SFX_VAR], openDesc,
					              (int8)127); // have to use full version so we can give hash instead of string
				else
					RegisterSound(cur_id, defaultOpenSfx, openDesc); // use small version as we have string not hash
			} else {                                                         // still closed - see if we can go to sleep
				if (!L->prop_on_this_screen) {
					// closed and no on screen
					L->camera_held = TRUE8; // not on screen
					L->cycle_time = 0;      // accurate for displays
				}
				return;
			}
		}
	} while (1);
}

mcodeFunctionReturnCodes _game_session::fn_set_custom_auto_door(int32 &, int32 *params) {
	// set to special custom door logic

	// params    0       initial state value
	// params 1      dist for detection
	// params 2      locked or not
	CAD_STATE = params[0];
	CAD_DIST = params[1];
	CAD_LOCKED = params[2]; // starts not locked
	L->list[9] = params[1];
	// switch out of script mode
	L->big_mode = __CUSTOM_AUTO_DOOR;

	// anim presets
	CAD_INDEX = prop_anims->Fetch_item_number_by_name(object->GetName());
	CAD_OPEN_NO = Validate_prop_anim("opening");
	CAD_CLOSE_NO = Validate_prop_anim("closing");

	Tdebug("logic_modes.txt", "fn_set_custom_auto_door switching [%s]", object->GetName());

	// Set a symbol type so the Remora knows what to draw.
	L->object_type = __AUTO_DOOR;

	return (IR_CONT);
}

void _game_session::Custom_auto_door() {
	// this special _big_mode logic replaces the logic script of a lib_auto_slide_door object
	// this is a rationalisation designed to aid speed up on psx

	_animating_prop *index;
	_animation_entry *anim;
	uint32 j, id;
	bool8 sensed = FALSE8;

	L->list[8] = 0;

	// check if someone is here or not
	j = 0;
	while ((j < number_of_voxel_ids) && (!sensed)) { // object 0 is used
		id = voxel_id_list[j++];

		if ((!logic_structs[id]->mega->dead) && (logic_structs[id]->ob_status != OB_STATUS_HELD)) {    // still alive
			if (PXfabs(L->prop_xyz.y - logic_structs[id]->mega->actor_xyz.y) < (200 * REAL_ONE)) { // slack for height calc
				if ((PXfabs(L->prop_xyz.x - logic_structs[id]->mega->actor_xyz.x) < (PXreal)CAD_DIST) &&
				    (PXfabs(L->prop_xyz.z - logic_structs[id]->mega->actor_xyz.z) < (PXreal)CAD_DIST)) {
					// yes
					sensed = TRUE8;
					L->list[8]++;
				}
			}
		}
	}

	if (CAD_STATE == CAD_OPENING) { // doors opening
		// get index for object
		index = (_animating_prop *)prop_anims->Fetch_item_by_number(CAD_INDEX);

		anim = (_animation_entry *)(((char *)index) + index->anims[CAD_OPEN_NO]);
		prop_state_table[cur_id] = anim->frames[L->anim_pc];

		if ((uint8)L->anim_pc == (anim->num_frames - 1)) {
			CAD_STATE = CAD_OPEN; // cancel mode
			CAD_WAIT = CAD_TIMER; // when we sense somewhat we reset the timer
		} else
			L->anim_pc++; // frame on

		return;
	} else if (CAD_STATE == CAD_OPEN) {
		if ((!CAD_LOCKED) && (!sensed)) { // no one here anymore so switch modes
			if (CAD_WAIT) {           // now wait for time up
				CAD_WAIT--;       // 1 less
				return;
			}

			CAD_STATE = CAD_CLOSING;
			// close sound
			if (logic_structs[cur_id]->sfxVars[CLOSE_SFX_VAR] != 0)
				RegisterSound(cur_id, NULL, logic_structs[cur_id]->sfxVars[CLOSE_SFX_VAR], closeDesc,
				              (int8)127); // have to use full version so we can give hash instead of string
			else
				RegisterSound(cur_id, defaultCloseSfx, closeDesc); // use small version as we have string not hash
		}

		CAD_WAIT = CAD_TIMER; // when we sense somewhat we reset the timer
		return;
	} else if (CAD_STATE == CAD_CLOSING) { // doors closing
		if (sensed) {                  // sensed someone so switch modes
			CAD_STATE = CAD_OPENING;
			return;
		}
		// get index for object
		index = (_animating_prop *)prop_anims->Fetch_item_by_number(CAD_INDEX);

		// when closing we reverse the opening anim - until the done when we set to last frame of closing
		anim = (_animation_entry *)(((char *)index) + index->anims[CAD_OPEN_NO]);
		prop_state_table[cur_id] = anim->frames[L->anim_pc];

		if (!L->anim_pc) {
			CAD_STATE = CAD_CLOSED; // cancel mode
			anim = (_animation_entry *)(((char *)index) + index->anims[CAD_CLOSE_NO]);
			prop_state_table[cur_id] = anim->frames[anim->num_frames - 1];
		} else
			L->anim_pc--; // frame on

		return;
	} else if (CAD_STATE == CAD_CLOSED) {
		if ((!CAD_LOCKED) && (sensed)) { // sensed someone so switch modes
			CAD_STATE = CAD_OPENING;
			L->anim_pc = 0;

			// open sound
			if (logic_structs[cur_id]->sfxVars[OPEN_SFX_VAR] != 0)
				RegisterSound(cur_id, NULL, logic_structs[cur_id]->sfxVars[OPEN_SFX_VAR], openDesc,
				              (int8)127); // have to use full version so we can give hash instead of string
			else
				RegisterSound(cur_id, defaultOpenSfx, openDesc); // use small version as we have string not hash
		}

		if (!L->prop_on_this_screen) {
			// closed and no on screen
			L->camera_held = TRUE8; // not on screen
			L->cycle_time = 0;      // accurate for displays
		}

		return;
	}
}

mcodeFunctionReturnCodes _game_session::fn_set_cad_lock_status(int32 &, int32 *params) {
	// set the locked status of an auto slide door
	// only the object should call this

	// params    0   0 name of door
	// params    0   0 or non zero
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	uint32 id;

	id = (uint32)objects->Fetch_item_number_by_name(object_name);
	logic_structs[id]->list[5] = params[1];

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_get_cad_state_flag(int32 &result, int32 *params) {
	// return custom auto door state to button

	// params    0   name of door
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	uint32 id;

	id = (uint32)objects->Fetch_item_number_by_name(object_name);
	if (logic_structs[id]->EXT_CAD_STATE == CAD_OPEN)
		result = 1;
	else
		result = 0;

	return IR_CONT;
}

} // End of namespace ICB
