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

#include "engines/icb/p4.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/common/ptr_util.h"
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
#include "engines/icb/session.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/res_man.h"
#include "engines/icb/floors.h"

namespace ICB {

mcodeFunctionReturnCodes fn_generic_prop_interact(int32 &result, int32 *params) { return (MS->fn_generic_prop_interact(result, params)); }

mcodeFunctionReturnCodes fn_custom_prop_interact(int32 &result, int32 *params) { return (MS->fn_custom_prop_interact(result, params)); }

mcodeFunctionReturnCodes fn_is_there_interact_object(int32 &result, int32 *params) { return (MS->fn_is_there_interact_object(result, params)); }

mcodeFunctionReturnCodes fn_get_interact_object_id(int32 &result, int32 *params) { return (MS->fn_get_interact_object_id(result, params)); }

mcodeFunctionReturnCodes fn_is_object_interact_object(int32 &result, int32 *params) { return (MS->fn_is_object_interact_object(result, params)); }

mcodeFunctionReturnCodes fn_register_for_auto_interaction(int32 &result, int32 *params) { return (MS->fn_register_for_auto_interaction(result, params)); }

mcodeFunctionReturnCodes fn_route_to_custom_prop_interact(int32 &result, int32 *params) { return (MS->fn_route_to_custom_prop_interact(result, params)); }

mcodeFunctionReturnCodes fn_route_to_generic_prop_interact(int32 &result, int32 *params) { return (MS->fn_route_to_generic_prop_interact(result, params)); }

mcodeFunctionReturnCodes fn_sony_door_interact(int32 &result, int32 *params) { return (MS->fn_sony_door_interact(result, params)); }

mcodeFunctionReturnCodes fn_unregister_for_auto_interaction(int32 &result, int32 *params) { return (MS->fn_unregister_for_auto_interaction(result, params)); }

mcodeFunctionReturnCodes fn_wandering_custom_prop_interact(int32 &result, int32 *params) { return (MS->fn_wandering_custom_prop_interact(result, params)); }

mcodeFunctionReturnCodes fn_wandering_generic_prop_interact(int32 &result, int32 *params) { return (MS->fn_wandering_generic_prop_interact(result, params)); }

#define SONY_DOOR_STEP_BACK_DIST ((50 * REAL_ONE) * (50 * REAL_ONE))
#define SONY_DOOR_PRESS_DIST ((100 * REAL_ONE) * (100 * REAL_ONE))

mcodeFunctionReturnCodes _game_session::fn_set_interacting(int32 &, int32 *params) {
	// set interting and id of target
	// so we can run prop interact type animation functions outside of an interaction

	// params    0   name of target

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	uint32 id = objects->Fetch_item_number_by_name(object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_set_interacting - illegal object [%s]", object_name);

	M->target_id = id;

	M->interacting = TRUE8;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_clear_interacting(int32 &, int32 *) {
	M->interacting = FALSE8;

	return (IR_STOP); // this is vital as currently the object will be
}

mcodeFunctionReturnCodes _game_session::fn_route_to_generic_prop_interact(int32 &result, int32 *params) {
	// WALK-TO interact with a prop BUT DOESNT play a generic animation
	// will call a trigger script if finds marker and script

	// params        0   name of generic animation

	if (L->looping == 2) {
		L->looping = FALSE8;
		L->pan = logic_structs[M->target_id]->prop_interact_pan;

		// force to stand, frame 0, restore pre anim coordinates
		POST_INTERACTION // fix coords and set to stand

		    return (IR_CONT);
	}

	return (Core_prop_interact(result, params, FALSE8, FALSE8));
}

mcodeFunctionReturnCodes _game_session::fn_route_to_custom_prop_interact(int32 &result, int32 *params) {
	// WALK-TO interact with a prop BUT DOESNT play a custom non generic animation
	// then return to script

	// params        0   name of custom animation

	if (L->looping == 2) {
		L->looping = FALSE8;
		L->pan = logic_structs[M->target_id]->prop_interact_pan;

		// force to stand, frame 0, restore pre anim coordinates
		POST_INTERACTION // fix coords and set to stand

		    Reset_cur_megas_custom_type();

		return (IR_CONT);
	}

	return (Core_prop_interact(result, params, TRUE8, FALSE8));
}

mcodeFunctionReturnCodes _game_session::fn_sony_door_interact(int32 &result, int32 *params) {
	// special door situation whereby we are passed the names of two buttons and we need to work out which one to interact with

	// params        0   name of first button
	//				1 name of second button
	//				2  number of buttons

	PXfloat new_pan, diff;
	uint32 id;
	uint32 but_floor;

	const char *button1_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *button2_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	if ((!params[2]) || (params[2] > 2))
		Fatal_error("fn_sony_door_interact - %d is illegal number of buttons, can be 1 or 2", params[2]);

	result = FALSE8; // no button was pressed

	if (!L->looping) {
		// work out which button to interact with

		id = objects->Fetch_item_number_by_name(button1_name);
		if (id == 0xffffffff)
			Fatal_error("fn_sony_door_interact - illegal object [%s]", button1_name);

		but_floor = floor_def->Return_floor_rect(logic_structs[id]->prop_xyz.x, logic_structs[id]->prop_xyz.z, M->actor_xyz.y, 0);

		// angle
		new_pan = logic_structs[id]->prop_interact_pan; // get targets pan

		// get difference between the two
		diff = L->pan - new_pan;

		// correct
		if (diff > HALF_TURN)
			diff -= FULL_TURN;
		else if (diff < -HALF_TURN)
			diff += FULL_TURN;

		if ((L->owner_floor_rect == but_floor) && (PXfabs(diff) < (FULL_TURN / 5))) { // 36 deg = +/- 18 deg
			// facing the same so this must be the button
			M->target_id = id; // change the target

			if (prop_interact_dist < SONY_DOOR_STEP_BACK_DIST)
				M->reverse_route = TRUE8;

			result = TRUE8; // button 1
		} else {
			// wanst button 1 - so do nothing if that was only button
			if (params[2] == 1) {
				return IR_CONT;
			}

			// there is another button so lets take a look to see it is named correctly
			id = objects->Fetch_item_number_by_name(button2_name);
			if (id == 0xffffffff)
				Fatal_error("fn_sony_door_interact - illegal object [%s]", button2_name);

			but_floor = floor_def->Return_floor_rect(logic_structs[id]->prop_xyz.x, logic_structs[id]->prop_xyz.z, M->actor_xyz.y, 0);

			if (L->owner_floor_rect != but_floor)
				return IR_CONT;

			M->target_id = id; // change the target

			if (prop_interact_dist < SONY_DOOR_STEP_BACK_DIST)
				M->reverse_route = TRUE8;

			result = TRUE8; // button 2
		}
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_custom_prop_interact(int32 &result, int32 *params) {
	// interact with a prop and play a custom non generic animation
	return (Core_prop_interact(result, params, TRUE8, TRUE8));
}

mcodeFunctionReturnCodes _game_session::fn_generic_prop_interact(int32 &result, int32 *params) {
	// interact with a prop and play a generic animation
	// will call a trigger script if finds marker and script

	// params        0   name of generic animation

	return (Core_prop_interact(result, params, FALSE8, TRUE8));
}

mcodeFunctionReturnCodes _game_session::fn_wandering_custom_prop_interact(int32 &result, int32 *params) {
	// interact with a prop and play a custom non generic animation

	return (Core_prop_interact(result, params, TRUE8, FALSE8));
}

mcodeFunctionReturnCodes _game_session::fn_wandering_generic_prop_interact(int32 &result, int32 *params) {
	// interact with a prop and play a generic animation
	// will call a trigger script if finds marker and script

	// params        0   name of generic animation

	return (Core_prop_interact(result, params, FALSE8, FALSE8));
}

mcodeFunctionReturnCodes _game_session::Core_prop_interact(int32 & /*result*/, int32 *params, bool8 custom, bool8 coord_correction) {
	bool8 initial_turn;
	bool8 res = FALSE8;
	__mega_set_names anim;
	PXreal destx, destz;
	PXfloat diff;
	int32 retval;
	PXreal sub1, sub2, len, len2;
	uint32 j;

	// looping   0 init route
	//				1 process route
	//				2 init turn to pan
	//				3 async wait
	//				4 play target anim
	//				5

	const char *anim_name = NULL;
	if (params && params[0]) {
		anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	}

	// set up first time in

	if (!L->looping) {

		// setup autoroute to coordinate

		if (!custom) {
			Zdebug("calc *generic* target anim [%s]", anim_name);

			// get anim type
			res = I->Find_anim_type(&anim, anim_name);
			if (!res)
				Fatal_error("Core_prop_interact cant indentify animation %s", anim_name);

			if (!I->IsAnimTable(anim))
				Fatal_error("Core_prop_interact finds [%s] doesnt have a [%s] animation", object->GetName(), params[0]);
		} else {
			Zdebug("calc *custom* target anim [%s]", anim_name);
			I->Init_custom_animation(anim_name);
			anim = __NON_GENERIC;
		}

		// start psx asyncing the anim - may already be doing so if scripts are written properly!
		if (rs_anims->Res_async_open(I->get_info_name(anim), I->info_name_hash[anim], I->base_path, I->base_path_hash) == 0)
			return IR_REPEAT;

		// we are now looping, having done the init
		L->looping = 1;

		// calculate the coordinate
		Compute_target_interaction_coordinate(anim, &destx, &destz); // uses target_id to derive initial target coord

		// save target coord for later post animation correction
		M->target_xyz.x = destx;
		M->target_xyz.z = destz;

		// first lets see if we are really quite close to the interact coordinate - if we are we'll snap
		sub1 = (PXreal)(destx - L->mega->actor_xyz.x);
		sub2 = (PXreal)(destz - L->mega->actor_xyz.z);
		len = (PXreal)((sub1 * sub1) + (sub2 * sub2)); // dist
		if (len < (35 * 35)) {
			L->mega->actor_xyz.x = destx;
			L->mega->actor_xyz.z = destz;
			L->looping = 2;
			return (IR_REPEAT);
		}

		// lets see if the interact coordinate is further away than we are - which is bad news

		// first, our coordinate to the prop
		sub1 = (PXreal)(logic_structs[M->target_id]->prop_xyz.x - L->mega->actor_xyz.x);
		sub2 = (PXreal)(logic_structs[M->target_id]->prop_xyz.z - L->mega->actor_xyz.z);
		len = (PXreal)((sub1 * sub1) + (sub2 * sub2)); // dist

		// second, the interact point to the prop
		sub1 = (PXreal)(destx - logic_structs[M->target_id]->prop_xyz.x);
		sub2 = (PXreal)(destz - logic_structs[M->target_id]->prop_xyz.z);
		len2 = (PXreal)((sub1 * sub1) + (sub2 * sub2)); // dist

		M->m_main_route.___init();

		// set motion type
		if ((len2 > len) || (M->reverse_route == TRUE8)) { // if further away OR already set to reverse - must have been by fn-sony-door
			M->m_main_route.request_form.anim_type = __STEP_BACKWARD;
			M->reverse_route = TRUE8;
			initial_turn = FALSE8;
		} else {
			initial_turn = TRUE8;

			if (M->motion == __MOTION_WALK)
				M->m_main_route.request_form.anim_type = __WALK;
			else
				M->m_main_route.request_form.anim_type = __RUN; // form.anim_type=__RUN;
		}

		// new route do prepare a route request form!
		// initial x,z
		M->m_main_route.request_form.initial_x = M->actor_xyz.x;
		M->m_main_route.request_form.initial_z = M->actor_xyz.z;

		// target x,z
		M->m_main_route.request_form.dest_x = (PXreal)destx;
		M->m_main_route.request_form.dest_z = (PXreal)destz;

		Zdebug("PLAYER INTERACT to %3.2f,%3.2f from %3.2f,%3.2f", destx, destz, M->actor_xyz.x, M->actor_xyz.z);

		// need characters y coordinate also
		M->m_main_route.request_form.character_y = M->actor_xyz.y;

		// this function attempts to finish on stand
		M->m_main_route.request_form.finish_on_null_stand = TRUE8;
		M->m_main_route.request_form.finish_on_stand = FALSE8;

		// set type
		M->m_main_route.request_form.rtype = ROUTE_points_only;

		// now log and create the initial route
		// set a barrier mask :(
		session_barriers->Set_route_barrier_mask((int32)destx - 500, (int32)destx + 500, (int32)destz - 500, (int32)destz + 500);
		Create_initial_route(__FULL);
		session_barriers->Clear_route_barrier_mask();

		// only one of these per cycle - we may have cheated and done a second route here but at least we can stop another if we
		// were first
		Set_router_busy();

		// if the route could not be built
		if (M->m_main_route.request_form.error == __ROUTE_REQUEST_PRIM_FAILED) {
			Create_initial_route(__LASER); // lets get out of this the easy way!
		}

		// we may not actually need a route if we are very close
		if (M->m_main_route.request_form.error == __RR_NO_ROUTE_REQUIRED) {
			Zdebug("skipping route");
			L->looping = 2; // bypass the route
			return (IR_REPEAT);
		}
	}

	// routing
	if (L->looping == 1) {
		if (Process_route()) {
			// not looping any longer
			// set to turn phase
			L->looping = 2;
			return (IR_REPEAT);
		}
	}

	// set up auto turning ready for anim play
	if (L->looping == 2) {
		diff = logic_structs[M->target_id]->prop_interact_pan - L->pan;

		// work out which way to turn
		if (diff > HALF_TURN)
			diff -= FULL_TURN;
		else if (diff < -HALF_TURN)
			diff += FULL_TURN;

		// diff is now the distance to turn by and its sign denotes direction

		if (diff < FLOAT_ZERO)
			M->turn_dir = 0; // right
		else
			M->turn_dir = 1; // left

		M->target_pan = (PXfloat)PXfabs(diff);                               // save positive pan distance
		M->auto_target_pan = logic_structs[M->target_id]->prop_interact_pan; // actual target which we may clip to
		L->auto_display_pan = L->pan;                                        // start where we currently are
		L->auto_panning = TRUE8;

		L->looping = 3; // go straight to play anim
		return (IR_REPEAT);
	}

	// check anim in memory
	if (L->looping == 3) {

		if (custom)
			anim = __NON_GENERIC;
		else
			anim = Fetch_generic_anim_from_ascii(anim_name);

		// in memory yet?
		if (rs_anims->Res_async_open(I->get_info_name(anim), I->info_name_hash[anim], I->base_path, I->base_path_hash)) {
			L->cur_anim_type = anim;
			L->anim_pc = 0;
			L->looping = 4; // go straight to play anim
		}

		return IR_REPEAT;
	}

	// running target animation
	if (L->looping == 4) {
		// get animation
		PXanim *pAnim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash); //

		// last frame is currently displayed?
		if ((int)(L->anim_pc + M->anim_speed) >= (pAnim->frame_qty - 1)) {
			L->looping = FALSE8;
			M->reverse_route = FALSE8;

			// force to stand, frame 0
			if (coord_correction) {
				POST_INTERACTION // fix coords and set to stand
			} else {                 // was a wandering finish-where-we-finish interaction
				L->cur_anim_type = __STAND;
				L->anim_pc = 0;
			}

			Reset_cur_megas_custom_type();
			return (IR_CONT);
		}

		// shift character and frame forward by the amount appropriate
		if (!MS->Easy_frame_and_motion(L->cur_anim_type, 0, M->anim_speed)) {
			L->looping = FALSE8;

			M->reverse_route = FALSE8;

			// force to stand, frame 0, restore pre anim coordinates
			if (coord_correction) {
				POST_INTERACTION // fix coords and set to stand
			} else {
				L->cur_anim_type = __STAND;
				L->anim_pc = 0;
			}

			Reset_cur_megas_custom_type();
			return (IR_CONT);
		}

		// is the interact marker on this frame ?
		for (j = 0; j < M->anim_speed; j++) {
			PXframe *frame = PXFrameEnOfAnim(L->anim_pc + j, pAnim);

			if ((frame->marker_qty > INT_POS) && (INT_TYPE == (frame->markers[INT_POS].GetType()))) {
				//          run the trigger anim
				if (!MS->Call_socket(M->target_id, "trigger", &retval)) {
					Message_box("[%s] interact marker but no trigger script", (const char *)L->GetName());
					Message_box("anim %s Target ID %d [%s]", master_anim_name_table[L->cur_anim_type].name, M->target_id, Fetch_object_name(M->target_id));
				}

				break; // done it
			}
		}
	}

	// not finished, so see you next cycle
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_is_there_interact_object(int32 &result, int32 *) {
	// return yes or no for whether or not an interact object exists

	result = player.Fetch_player_interact_status();

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_get_interact_object_id(int32 &result, int32 *) {
	// return yes or no for whether or not an interact object exists

	result = player.Fetch_player_interact_id();

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_is_object_interact_object(int32 &result, int32 *params) {
	// return yes or no for whether or not an interact object exists

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	uint32 id = objects->Fetch_item_number_by_name(object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_is_object_interact_object - object [%s] does not exist", object_name);

	if (id == player.Fetch_player_interact_id())
		result = TRUE8;
	else
		result = FALSE8;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_unregister_for_auto_interaction(int32 &, int32 *) {
	// as the name says - for stairs, session joins, etc.
	uint32 j;

	for (j = 0; j < MAX_auto_interact; j++) {
		if (auto_interact_list[j] == (uint8)(cur_id + 1)) {
			Tdebug("auto_interact.txt", "- [%s] %d", object->GetName(), j);
			auto_interact_list[j] = 0; // slot not empty
			return IR_CONT;
		}
	}

	Fatal_error("fn_unregister_for_auto_interaction cant unregister non registered object [%s]", object->GetName());

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_register_for_auto_interaction(int32 &, int32 *) {
	// as the name says - for stairs, session joins, etc.

	uint32 j;

	for (j = 0; j < MAX_auto_interact; j++) {
		if (auto_interact_list[j] == (uint8)(cur_id + 1))
			Fatal_error("fn_register_for_auto_interaction finds double registration of %s", object->GetName());

		if (!auto_interact_list[j]) { // empty slot
			auto_interact_list[j] = (uint8)(cur_id + 1);
			Tdebug("auto_interact.txt", "+ [%s] %d", object->GetName(), j);
			return IR_CONT;
		}
	}

	Fatal_error("fn_register_for_auto_interaction - list full - [%s]", object->GetName());

	return IR_CONT;
}

} // End of namespace ICB
