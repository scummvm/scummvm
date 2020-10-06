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
#include "engines/icb/text.h"
#include "engines/icb/session.h"
#include "engines/icb/global_switches.h"

namespace ICB {

mcodeFunctionReturnCodes fn_start_player_interaction(int32 &result, int32 *params) { return (MS->fn_start_player_interaction(result, params)); }

#define INTERACT_DISTANCE (250 * REAL_ONE)
#define MIN_INTERACT_DISTANCE (5 * REAL_ONE)

// distance to point head at - 15m
#define LOOK_AT_DISTANCE (500 * REAL_ONE)

//( ( FULL_TURN*n)/360 )
#define NEAR_PROP_DISTANCE (70 * REAL_ONE)

// was 130 original EU psx release
#define DEAD_MEGA_DISTANCE (230 * REAL_ONE)

//+/- 90deg
#define NEAR_PROP_ANGLE (FULL_TURN / 4)

void _player::Find_current_player_interact_object() {
	// find the nico that represents our current interactable object

	// search nicos on our level
	// find within distance
	// find within angle
	// the name of the nico will be the name of the object

	uint32 j;
	PXreal sub1, sub2, len;
	PXreal nearest = REAL_LARGE * REAL_LARGE; // high number to be bettered
	PXreal nearest_mega = REAL_LARGE * REAL_LARGE; // high number to be bettered
	PXfloat new_pan, diff;

	uint32 prop_id = 0;
	uint32 mega_id = 0;
	uint32 look_at_prop_id = 0;
	uint32 pl_id = Fetch_player_id();
	bool8 armed_status = log->mega->Fetch_armed_status();
	uint8 crouch_status = log->mega->Is_crouched();

	interact_selected = FALSE8;
	look_at_selected = FALSE8;
	dead_mega = FALSE8;
	bool8 evil_chosen = FALSE8;

	// ** check first for megas as indicated by line-of-sight-manager - these get priority over prop nicos **

	// run through all the objects calling their logic
	for (j = 0; j < MS->total_objects; j++) { // object 0 is used
		//		object must be alive and interactable
		if ((MS->logic_structs[j]->ob_status != OB_STATUS_HELD) && (MS->logic_structs[j]->player_can_interact)) { // not if the object has been manually switched out

			if ((MS->logic_structs[j]->image_type == PROP) && (!armed_status) &&
			    (crouch_status == (MS->logic_structs[j]->three_sixty_interact & PROP_CROUCH_INTERACT))) {
				if ((MS->logic_structs[j]->prop_xyz.y >= log->mega->actor_xyz.y) && (MS->logic_structs[j]->owner_floor_rect == log->owner_floor_rect))
					if ((MS->logic_structs[j]->prop_xyz.y - log->mega->actor_xyz.y) < (190 * REAL_ONE)) {
						sub1 = (PXreal)MS->logic_structs[j]->prop_xyz.x - log->mega->actor_xyz.x;
						sub2 = (PXreal)MS->logic_structs[j]->prop_xyz.z - log->mega->actor_xyz.z;

						//              dist
						len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

						//              less than n centimeters away
						if ((len > MIN_INTERACT_DISTANCE * MIN_INTERACT_DISTANCE) && (len < LOOK_AT_DISTANCE * LOOK_AT_DISTANCE) && (len < nearest)) {
							//                      check for prop being a 360deg type
							if (MS->logic_structs[j]->three_sixty_interact & THREE_SIXTY_INTERACT) {

								new_pan = PXAngleOfVector(sub2, sub1); // work out vector
								//                          get difference between the two
								diff = new_pan - log->pan;
								//                      correct
								if (diff > HALF_TURN)
									diff -= FULL_TURN;
								else if (diff < -HALF_TURN)
									diff += FULL_TURN;

								if (PXfabs(diff) < (FULL_TURN / 10)) { // 0.1f
									MS->prop_interact_dist = len; // can be used later in core_prop_interact
									nearest = len;
									prop_id = j + 1;
								}
							} else { // are we a similar pan to target object
								//                      angle
								new_pan = MS->logic_structs[j]->prop_interact_pan; // get targets pan

								//                      get difference between the two
								diff = log->pan - new_pan;

								//                      correct
								if (diff > HALF_TURN)
									diff -= FULL_TURN;
								else if (diff < -HALF_TURN)
									diff += FULL_TURN;

								if (((len < NEAR_PROP_DISTANCE * NEAR_PROP_DISTANCE) && (PXfabs(diff) < NEAR_PROP_ANGLE)) /* OR */ ||
								    (PXfabs(diff) < (FULL_TURN / 8))) { //*
									//                          ok, we are facing the right direction - i.e. nearly same as interact pan
									//                          but
									//                          are we behind or infront - need another check

									PXreal dx = (PXreal)PXsin((log->pan + (FULL_TURN / 4)) * TWO_PI);
									PXreal dz = (PXreal)PXcos((log->pan + (FULL_TURN / 4)) * TWO_PI);

									PXreal mx = MS->logic_structs[j]->prop_xyz.x;
									PXreal mz = MS->logic_structs[j]->prop_xyz.z;

									if ((dz * (mx - log->mega->actor_xyz.x)) <= (dx * (mz - log->mega->actor_xyz.z))) {
										MS->prop_interact_dist = len; // can be used later in core_prop_interact
										nearest = len;
										prop_id = j + 1;
									}
								}
							}
						}
					}
			} else if ((MS->logic_structs[j]->image_type == VOXEL) && (MS->logic_structs[j]->mega->actor_xyz.y == log->mega->actor_xyz.y)) { // mega character
				// we have targeted an evil but this one is not evil then skip it - regardless of proximity
				if ((evil_chosen) && (!MS->logic_structs[j]->mega->is_evil))
					continue;

				// if there is a chi then we can target her when armed
				if ((MS->is_there_a_chi) && (j == MS->chi_id) && (armed_status))
					continue;

				if ((g_oLineOfSight->LineOfSight(pl_id, j)) && (MS->Object_visible_to_camera(j))) { // must be on screen
					sub1 = (PXreal)MS->logic_structs[j]->mega->actor_xyz.x - log->mega->actor_xyz.x;
					sub2 = (PXreal)MS->logic_structs[j]->mega->actor_xyz.z - log->mega->actor_xyz.z;

					// dist
					len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

					// nearer or the current is dead or armed and the current is non-evil
					if (((armed_status) && (!evil_chosen) && (MS->logic_structs[j]->mega->is_evil)) || (dead_mega) || (len < nearest_mega)) {
						// we are nearer or the current is dead
						// see if object is dead
						if ((MS->logic_structs[j]->mega->dead) &&
						    (crouch_status)) { // this mega is dead and we're crouched - only register him if there isnt another
							if ((!mega_id) && (len < DEAD_MEGA_DISTANCE * DEAD_MEGA_DISTANCE)) { // dead mega chosen - must be within prop type range
								nearest_mega = len;
								mega_id = j + 1;
								dead_mega = TRUE8; // chosen a dead mega
							}
						} else if (!MS->logic_structs[j]->mega->dead) { // must belive if we're stood
							evil_chosen = MS->logic_structs[j]->mega->is_evil;
							nearest_mega = len;
							mega_id = j + 1;
							dead_mega = FALSE8; // chosen a live mega
						}
					}
				}
			}
		}
	}

	// if crouching and targeting a mega the mega must be dead
	// crouch props are filtered out above
	if ((crouch_status) && (mega_id)) {
		// if dead AND not armed   or    alive AND armed
		if (((dead_mega) && (!armed_status)) || ((armed_status) && (!dead_mega))) {
			cur_interact_id = (mega_id - 1);
			interact_selected = TRUE8;
		}
		return;
	}

	// mega if armed, nearest prop or live mega, dead mega
	if ((prop_id) && (nearest < nearest_mega)) { // UNARMED prop nearer than mega (wont be a prop if armed)
		cur_interact_id = (prop_id - 1);
		interact_selected = TRUE8;
	} else if ((mega_id) && (!dead_mega)) { // live mega
		cur_interact_id = (mega_id - 1);
		interact_selected = TRUE8;
	} else if (prop_id) { // prop
		cur_interact_id = (prop_id - 1);
		interact_selected = TRUE8;
	}

	// look at
	if ((!interact_selected) && (look_at_prop_id)) {
		look_at_id = look_at_prop_id - 1;
		look_at_selected = TRUE8;
	}
}

#if CD_MODE == 0

void _player::Render_crude_interact_highlight() {
	uint32 pitch; // backbuffer pitch
	uint8 *ad;

	_rgb pen = {255, 0, 0, 0};

	// anything highlighted?
	if (interact_selected == FALSE8)
		return;

	// cross hair is now a development option
	if (px.cross_hair == FALSE8)
		return;

	ad = surface_manager->Lock_surface(working_buffer_id);
	pitch = surface_manager->Get_pitch(working_buffer_id);

	// setup camera
	PXcamera &camera = MS->GetCamera();

	// set up nico world coords
	PXvector pos;

	if (MS->logic_structs[cur_interact_id]->image_type == PROP) {
		pos.x = MS->logic_structs[cur_interact_id]->prop_xyz.x;
		pos.y = MS->logic_structs[cur_interact_id]->prop_xyz.y;
		pos.z = MS->logic_structs[cur_interact_id]->prop_xyz.z;
	} else {
		pos.x = MS->logic_structs[cur_interact_id]->mega->actor_xyz.x;
		pos.y = MS->logic_structs[cur_interact_id]->mega->actor_xyz.y;
		pos.z = MS->logic_structs[cur_interact_id]->mega->actor_xyz.z;
	}

	// screen pos
	PXvector filmpos;

	// yesno
	bool8 result = FALSE8;

	// compute screen coord
	PXWorldToFilm(pos, camera, result, filmpos);

	// print name if on screen
	if (result) {
		Clip_text_print(&pen, (int32)(filmpos.x + (SCREEN_WIDTH / 2)), (int32)((SCREEN_DEPTH / 2) - filmpos.y), ad, pitch, "+");
	}

	surface_manager->Unlock_surface(working_buffer_id);
}

#else

void _player::Render_crude_interact_highlight() {}

#endif // #if CD_MODE == 0

__mode_return _player::Player_interact() {
	// check if the player has pressed the interact button
	// if so see if there's a current interact object and if so setup the interaction

	// return
	//				__FINISHED_THIS_CYCLE, or
	//				__MORE_THIS_CYCLE

	c_game_object *iobject;
	uint32 j;

	// first check for auto-interact objects

	if ((interact_selected) && ((log->cur_anim_type == __WALK) || ((log->cur_anim_type == __RUN))))
		for (j = 0; j < MAX_auto_interact; j++)
			if (MS->auto_interact_list[j] == (cur_interact_id + 1)) {
				//      try to fetch the object
				iobject = (c_game_object *)MS->objects->Fetch_item_by_number(cur_interact_id);

				Zdebug("  INTERACT with %s", iobject->GetName());

				//      get the address of the script we want to run
				const char *pc = (const char *)MS->scripts->Try_fetch_item_by_hash(iobject->GetScriptNameFullHash(OB_ACTION_CONTEXT)); //

				if (pc == NULL)
					Fatal_error("Object [%s] has no interact script", iobject->GetName());

				//      now run the action context script which may or may not set a new script on level 1
				RunScript(pc, iobject);

				//      stop for a cycle regardless
				return (__FINISHED_THIS_CYCLE);
			}

	// check for interact button AND there being an object to interact with
	if ((cur_state.IsButtonSet(__INTERACT)) && (interact_selected) && (!interact_lock) && (!stood_on_lift)) {
		// try to fetch the object
		iobject = (c_game_object *)MS->objects->Fetch_item_by_number(cur_interact_id);

		// get the address of the script we want to run
		const char *pc = (const char *)MS->scripts->Try_fetch_item_by_hash(iobject->GetScriptNameFullHash(OB_ACTION_CONTEXT)); //

		if (pc == NULL)
			Fatal_error("Object [%s] has no interact script", iobject->GetName());

		interact_lock = TRUE8; // switch the lock on

		// reset player to either stood or stood armed
		if (MS->logic_structs[Fetch_player_id()]->mega->Is_crouched())
			Set_player_status(CROUCHING);
		else if (MS->logic_structs[Fetch_player_id()]->mega->Fetch_armed_status())
			Set_player_status(NEW_AIM);
		else
			Set_player_status(STOOD);

		Push_player_stat();

		// now run the action context script which may or may not set a new script on level 1
		RunScript(pc, iobject);

		// stop for a cycle regardless
		return (__FINISHED_THIS_CYCLE);
	} else if (!cur_state.IsButtonSet(__INTERACT)) // release the interact lock
		interact_lock = FALSE8; // let go

	return (__MORE_THIS_CYCLE);
}

mcodeFunctionReturnCodes _game_session::fn_start_player_interaction(int32 &, int32 *params) {
	// do check to see if script running

	// if not set it up on level 2 and change script level

	// then call the new script as if from logic because, remember, this is being called from a script that is being called
	// from a function ; fn_player

	// S player::logic
	//	fn_player()
	//		S interact_context script
	//			fn_start_player_interact
	//				S new script

	//	** if we start a new script we should write a flag for _player::Player_interact **

	char *ad;

	// set target id
	M->target_id = player.Fetch_player_interact_id();

	// set this flag to avoid interact with id=0 based problems
	M->interacting = TRUE8;

	// fetch action script
	ad = (char *)scripts->Try_fetch_item_by_hash(params[0] /*(uint32)params*/);

	//	write actual offset
	L->logic[1] = ad;

	//	write reference for change script checks later - i.e. FN_context_chosen_script
	L->logic_ref[1] = ad;

	L->logic_level = 1; // reset to level 2
	// action script will fall back to looping level 1

	L->looping = 0; // reset to 0 for new logics

	// script interpretter shouldnt write a pc back
	return (IR_TERMINATE);
}

bool8 _game_session::Engine_start_interaction(const char *script, uint32 id) {
	// set the current mega object interacting named 'script' in target object 'id'

	c_game_object *iobject;
	uint32 script_hash;

	script_hash = HashString(script);

	// get target object
	iobject = (c_game_object *)MS->objects->Fetch_item_by_number(id);
	if (!iobject)
		Fatal_error("Engine_start_interaction - named object dont exist"); // should never happen

	// now try and find a script with the passed extention i.e. ???::looping
	for (uint32 k = 0; k < iobject->GetNoScripts(); k++) {

		if (script_hash == iobject->GetScriptNamePartHash(k)) {
			//			script k is the one to run
			//			get the address of the script we want to run
			char *pc = (char *)scripts->Try_fetch_item_by_hash(iobject->GetScriptNameFullHash(k));

			// set target id
			M->target_id = id;

			// set this flag to avoid interact with id=0 based problems
			M->interacting = TRUE8;

			//  write actual offset
			L->logic[1] = pc;

			//  write reference for change script checks later - i.e. FN_context_chosen_script
			L->logic_ref[1] = pc;

			L->logic_level = 1; // reset to level 2
			// action script will fall back to looping level 1

			L->looping = 0; // reset to 0 for new logics

			return (TRUE8);
		}
	}

	// didnt find the named script

	return (FALSE8);
}

} // End of namespace ICB
