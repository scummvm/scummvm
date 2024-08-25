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
#include "engines/icb/session.h"
#include "engines/icb/mission.h"
#include "engines/icb/p4.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/sound.h"
#include "engines/icb/res_man.h"

namespace ICB {

mcodeFunctionReturnCodes fn_simple_chi(int32 &result, int32 *params) { return (MS->fn_simple_chi(result, params)); }

mcodeFunctionReturnCodes fn_chi(int32 &result, int32 *params) { return (MS->fn_chi(result, params)); }

mcodeFunctionReturnCodes fn_start_chi_following(int32 &result, int32 *params) { return (MS->fn_start_chi_following(result, params)); }

mcodeFunctionReturnCodes fn_record_player_interaction(int32 &result, int32 *params) { return (MS->fn_record_player_interaction(result, params)); }

mcodeFunctionReturnCodes fn_fetch_chi_mode(int32 &result, int32 *params) { return (MS->fn_fetch_chi_mode(result, params)); }

mcodeFunctionReturnCodes fn_check_for_chi(int32 &result, int32 *params) { return (MS->fn_check_for_chi(result, params)); }

mcodeFunctionReturnCodes fn_wait_for_chi(int32 &result, int32 *params) { return (MS->fn_wait_for_chi(result, params)); }

mcodeFunctionReturnCodes fn_send_chi_to_this_object(int32 &result, int32 *params) { return (MS->fn_send_chi_to_this_object(result, params)); }

mcodeFunctionReturnCodes fn_chi_wait_for_player_to_move(int32 &result, int32 *params) { return (MS->fn_chi_wait_for_player_to_move(result, params)); }

mcodeFunctionReturnCodes fn_stop_chi_following(int32 &result, int32 *params) { return (MS->fn_stop_chi_following(result, params)); }

mcodeFunctionReturnCodes fn_register_chi(int32 &result, int32 *params) { return (MS->fn_register_chi(result, params)); }

mcodeFunctionReturnCodes fn_send_chi_to_named_object(int32 &result, int32 *params) { return (MS->fn_send_chi_to_named_object(result, params)); }

mcodeFunctionReturnCodes fn_chi_heard_gunshot(int32 &result, int32 *params) { return (MS->fn_chi_heard_gunshot(result, params)); }

mcodeFunctionReturnCodes fn_calibrate_chi(int32 &result, int32 *params) { return (MS->fn_calibrate_chi(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_simple_chi(int32 &, int32 *) {
	// simple chi that teleports around

	Fatal_error("fn_simple_chi not supported");

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_start_chi_following(int32 &, int32 *) {
	// the real chi

	g_mission->chi_following = TRUE8;

	chi_history = cur_history; // chi gets same point as cord

	chi_think_mode = __FOLLOWING; // set mode

	chi_do_mode = __THINKING; // thinking about following :)

	chi_next_move = 100; // 100 cycles

	Tdebug("chi.txt", "-+fn_start_chi_following - history=%d +-", chi_history);

	permission_to_fire = FALSE8; // reset

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_stop_chi_following(int32 &, int32 *) {
	// stop her
	// do this before changing her to custom logics

	g_mission->chi_following = FALSE8;

	chi_think_mode = __NOTHING; // nowt

	Tdebug("chi.txt", "-+fn_stop_chi_following");

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_record_player_interaction(int32 &, int32 *) {
	// write the id of interact object to player history list
	// advance history
	cur_history++;
	if (cur_history == MAX_player_history)
		cur_history = 0; // wrap

	// record it
	history[cur_history].interaction = TRUE8;
	history[cur_history].id = M->target_id;

	Tdebug("history.txt", "-> [%s] %d", LinkedDataObject::Fetch_items_name_by_number(objects, M->target_id), M->target_id);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_send_chi_to_named_object(int32 &, int32 *params) {
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	uint32 id;

	id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	if (id == 0xffffffff)
		Fatal_error("fn_send_chi_to_named_object - illegal object [%s]", object_name);

	// advance history
	cur_history++;
	if (cur_history == MAX_player_history)
		cur_history = 0; // wrap

	// record it
	history[cur_history].interaction = TRUE8;
	history[cur_history].id = id;

	Tdebug("history.txt", ">> [%s] %d", CGameObject::GetName(object), cur_id);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_send_chi_to_this_object(int32 &, int32 *) {
	// write the id of calling object to player history list

	// advance history
	cur_history++;
	if (cur_history == MAX_player_history)
		cur_history = 0; // wrap

	// record it
	history[cur_history].interaction = TRUE8;
	history[cur_history].id = cur_id;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_fetch_chi_mode(int32 &result, int32 *) {
	// returns the chi mode (__chi_think_mode) to script

	result = (int32)chi_think_mode;

	return IR_CONT;
}

__chi_think_mode _game_session::fn_fetch_chi_mode() {
	// returns the chi mode (__chi_think_mode) to engine
	return chi_think_mode;
}

mcodeFunctionReturnCodes _game_session::fn_chi_heard_gunshot(int32 &, int32 *) {
	// called when chi hears a gunshot

	// if we can see the player and same history - or, we're already in armed mode
	if (((chi_history == cur_history) && (g_oLineOfSight->LineOfSight(cur_id, player.Fetch_player_id()))) || (chi_do_mode == __FIGHT_HELP)) {
		permission_to_fire = 1; // chi can start fighting
	}

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_calibrate_chi(int32 &, int32 *params) {
	// set chi's near and far distances

	// params        0       catch up distance
	//				1     lost player distance

	chi_catch_up_dist = (PXreal)(params[0] * params[0]);

	chi_lost_dist = (PXreal)(params[1] * params[1]);

	return IR_CONT;
}

void _game_session::Set_chi_permission() {
	//	called by players gunshot code

	permission_to_fire = 1; // chi can start fighting
}

bool8 _game_session::Find_a_chi_target() {
	// find an ememy for chi
	uint32 j;

	for (j = 0; j < number_of_voxel_ids; j++) {
		if ((!logic_structs[voxel_id_list[j]]->mega->dead) &&                 // alive
		    (logic_structs[voxel_id_list[j]]->ob_status != OB_STATUS_HELD) && // not held
		    (logic_structs[voxel_id_list[j]]->mega->is_evil) &&               // is evil
		    (Object_visible_to_camera(voxel_id_list[j])) &&                   // on screen
		    (g_oLineOfSight->ObjectToObject(cur_id, voxel_id_list[j], LIGHT, 0, _line_of_sight::USE_OBJECT_VALUE,
		                                    TRUE8))) { // can see
			chi_target_id = voxel_id_list[j];
			chi_has_target = TRUE8;
			Tdebug("chi_targets.txt", "chi selects [%s] as target", logic_structs[chi_target_id]->GetName());
			return TRUE8;
		}
	}

	chi_has_target = FALSE8;

	// no
	return FALSE8;
}

void _game_session::Chi_leaves_fight_mode() {
	// set animation
	L->cur_anim_type = __PULL_OUT_WEAPON;
	I->IsAnimTable(L->cur_anim_type);

	// get animation
	PXanim *anim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash); //
	// set to last frame

	L->anim_pc = anim->frame_qty - 2; // if 10 frames then 10+1 (looper) == 11 meaning 9 is last displayable frame number
	M->next_anim_type = __NO_ANIM;
	chi_do_mode = __DISARM_TO_THINK; // back to thinking next cycle
}

mcodeFunctionReturnCodes _game_session::fn_chi(int32 &, int32 *) {
	// the real chi

	while (!Process_chi()) {
	};

	return IR_REPEAT;
}

bool8 _game_session::Process_chi() {
	uint32 next_room;
	PXreal x, z;
	int32 result;
	bool8 res;
	PXreal sub1, sub2, cord_dist;
	bool8 route_res;

	switch (chi_do_mode) {
	case __ANIMATE_TO_THINK:
		if (Play_anim()) {
			chi_do_mode = __THINKING;
		}
		return TRUE8;
		break;

	case __DISARM_TO_THINK:
		if (Play_reverse_anim()) {

			chi_do_mode = __THINKING;
			Set_pose(__NOT_ARMED);
			Change_pose_in_current_anim_set();

			L->cur_anim_type = __STAND;
			I->IsAnimTable(L->cur_anim_type);
		}
		return TRUE8;
		break;

	case __GET_WEAPON_OUT:
		if (fight_pause)
			fight_pause--;
		else {
			Set_pose(__GUN);
			Change_pose_in_current_anim_set();

			L->anim_pc = 0; // start here
			L->cur_anim_type = __PULL_OUT_WEAPON;
			M->next_anim_type = __STAND;

			I->IsAnimTable(L->cur_anim_type);

			Find_a_chi_target(); // select a target if there is one available

			chi_do_mode = __ANIMATE_TO_FIGHT_HELP;
		}

		return TRUE8;
		break;

	case __ANIMATE_TO_FIGHT_HELP:
		if (Play_anim()) {
			chi_do_mode = __FIGHT_HELP;
		}
		return TRUE8;
		break;

	case __TURN_RND:
		if (fast_face_rnd(2))
			chi_do_mode = __FIGHT_HELP;
		return TRUE8;
		break;

	case __TURN_TO_FACE_OBJECT:
		if (fast_face_object(chi_target_id, 3)) {
			chi_do_mode = __FIGHT_HELP;
		}

		return TRUE8;
		break;

	case __INTERACT_FOLLOW: // return from interact with ladder, stair, etc., here
		// we then move the position on and think again
		// arrived
		chi_do_mode = __THINKING;

		// move on to next position
		chi_history += 1;
		if (chi_history == MAX_player_history)
			chi_history = 0; // wrap
		chi_history += 1;
		if (chi_history == MAX_player_history)
			chi_history = 0; // wrap

		chi_next_move = 12;

		return TRUE8;
		break;

	case __ROUTING:
		// first do a check to see if players wandered back onto our floor
		// if we're on same floor as we started AND we meet player then stop
		if ((history[chi_history].id == L->owner_floor_rect) && (L->owner_floor_rect == logic_structs[player.Fetch_player_id()]->owner_floor_rect)) {
			chi_do_mode = __ANIMATE_TO_THINK;
			chi_history = cur_history; // chi gets same point as cord

			L->anim_pc = 0; // start here
			L->cur_anim_type = __WALK_TO_STAND;
			M->next_anim_type = __STAND;

			return TRUE8;
		}

		// animate the route
		if (Process_route()) {
			// arrived
			chi_do_mode = __THINKING;

			// in-case last move pick a time to next move
			chi_next_move = g_icb->getRandomSource()->getRandomNumber(200 - 1);
			chi_next_move += 50;

			// move on to next position
			chi_history += 1;
			if (chi_history == MAX_player_history)
				chi_history = 0; // wrap

			// cancel looping flag from the routing
			L->looping = 0;

			return TRUE8;
		}
		break;

	case __CHASING:
		// running after cord but on same floor
		cord_dist = Cord_dist();

		// animate the route
		if ((Process_route()) || (chi_history != cur_history) ||
		    (cord_dist < (PXreal)(chi_catch_up_dist))) { // check for route done or player moved on in which case we quit the route and get following
			// cancel looping flag from the routing
			L->looping = 0;

			if (chi_history == cur_history) {
				// still on same floor - route has just finished naturally
				// if we are within the zone then we come to a stand
				if (cord_dist < (PXreal)(chi_lost_dist)) {
					// within distance - so come to a stand
					chi_do_mode = __ANIMATE_TO_THINK;
					L->anim_pc = 0; // start here
					L->cur_anim_type = __WALK_TO_STAND;
					M->next_anim_type = __STAND;
					chi_next_move = g_icb->getRandomSource()->getRandomNumber(200 - 1);
					chi_next_move += 50;
					return TRUE8;
				}
			}
			// players gone or we're going to keep running
			chi_do_mode = __THINKING;
			chi_next_move = 1;

			return FALSE8;
		}
		break;

	case __BUMBLING:
		// just ambling about the current room

		cord_dist = Cord_dist();

		// animate the route
		if ((Process_route()) || ((chi_history != cur_history)) ||
		    (cord_dist < (PXreal)(chi_catch_up_dist / 2))) { // check for route done or player moved on in which case we quit the route and get following
			// arrived
			chi_do_mode = __THINKING;

			// in-case last move pick a time to next move
			chi_next_move = g_icb->getRandomSource()->getRandomNumber(200 - 1);
			chi_next_move += 50;

			// cancel looping flag from the routing
			L->looping = 0;

			if (chi_history == cur_history) {
				// still on same floor - route has just finished naturally
				// if we are within the zone then we come to a stand
				if (cord_dist < (PXreal)(chi_catch_up_dist)) {
					// within distance - so come to a stand
					chi_do_mode = __ANIMATE_TO_THINK;
					L->anim_pc = 0; // start here
					L->cur_anim_type = __WALK_TO_STAND;
					M->next_anim_type = __STAND;
					return TRUE8;
				}
			}
			return TRUE8;
		}
		break;

	case __GO_CORD_GO:
		// wait for player to moveout of a lift
		sub1 = logic_structs[player.Fetch_player_id()]->mega->actor_xyz.x - M->actor_xyz.x;
		sub2 = logic_structs[player.Fetch_player_id()]->mega->actor_xyz.z - M->actor_xyz.z;

		// dist
		if (((chi_history != cur_history)) || (((sub1 * sub1) + (sub2 * sub2)) > (PXreal)(100 * 100))) {
			// players moved
			Message_box("bye then");
			chi_next_move = 1;
			chi_do_mode = __THINKING;
		}

		return TRUE8;
		break;

	case __PAUSING:
		break;

	case __FIGHT_HELP: // help the player in a fight
		// first see if players gone
		if (chi_history != cur_history) {
			Chi_leaves_fight_mode();
			return TRUE8;
		} else {
			if (!fight_pause) {
				// not got permission, can see player and player not armed - exit armed mode
				if ((!permission_to_fire) && (!MS->logic_structs[player.Fetch_player_id()]->mega->Fetch_armed_status()) &&
				    (g_oLineOfSight->ObjectToObject(cur_id, player.Fetch_player_id(), LIGHT, 0, _line_of_sight::USE_OBJECT_VALUE, TRUE8))) {
					// exit this mode
					Chi_leaves_fight_mode();
					return TRUE8;
				}

				// not on camera -  then quit fight mode and catch up with player
				if (!Object_visible_to_camera(chi_id)) {
					// exit this mode
					Chi_leaves_fight_mode();
					return TRUE8;
				}

				// if can't see the player but would expect to (he's moved) then forget this and catch him up
				if (!g_oLineOfSight->ObjectToObject(cur_id, player.Fetch_player_id(), LIGHT, 0, _line_of_sight::USE_OBJECT_VALUE, TRUE8)) {
					// exit this mode
					PXreal x2 = logic_structs[player.Fetch_player_id()]->mega->actor_xyz.x;
					PXreal z2 = logic_structs[player.Fetch_player_id()]->mega->actor_xyz.z;
					PXreal vect = PXAngleOfVector(z2 - M->actor_xyz.z, x2 - M->actor_xyz.x); // work out vector

					if (PXfabs(L->pan - vect) < (FULL_TURN / 5)) {
						Chi_leaves_fight_mode();
						return TRUE8;
					}
				}

				// got target but target is dead or not seeable - set no target
				if (chi_has_target) {
					if (logic_structs[chi_target_id]->mega->dead)
						chi_has_target = 0; // dead so no target

					if (!Object_visible_to_camera(chi_target_id))
						chi_has_target = 0; // target is now off camera so forget it
				}

				// do something!
				if ((chi_has_target) && (permission_to_fire)) {
					// if right angle, shoot
					// else turn to face

					if (Need_to_turn_to_face_object(chi_target_id))
						chi_do_mode = __TURN_TO_FACE_OBJECT;
					else {
						// shoot!

						// play gun sound
						if (MS->logic_structs[cur_id]->sfxVars[GUNSHOT_SFX_VAR] != 0)
							RegisterSound(cur_id, nullptr, MS->logic_structs[cur_id]->sfxVars[GUNSHOT_SFX_VAR], gunDesc,
							              (int8)127); // have to use full version so we can give hash instead of string
						else
							RegisterSound(cur_id, defaultGunSfx, gunDesc); // use small version as we have string not hash

						// dynamic light
						M->SetDynamicLight(1, 255, 255, 255, 0, 150, 100, 200);

						// Hey we are shooting
						M->is_shooting = TRUE8;

						L->anim_pc = 0; // start here
						L->cur_anim_type = __STAND_AND_SHOOT;
						M->next_anim_type = __STAND;
						I->IsAnimTable(L->cur_anim_type);
						chi_do_mode = __ANIMATE_TO_FIGHT_HELP;
						fight_pause = (uint8)(1 + g_icb->getRandomSource()->getRandomNumber(10 - 1));
						Call_socket(chi_target_id, "gun_shot", &result);

						return TRUE8;
					}
				} else if (chi_has_target) {
					// can't shoot
					chi_do_mode = __TURN_TO_FACE_OBJECT;
				} else {
					// no target
					// try to find new target
					// yes, then set to turn
					// no, turn to player

					if (!Find_a_chi_target()) {

						if (!fast_face_rnd(2))
							chi_do_mode = __TURN_RND;

						// cancel permission
						permission_to_fire = 0;
					} else {                                     // found a new target
						chi_do_mode = __TURN_TO_FACE_OBJECT; // turns to face new  target
					}
				}

				fight_pause = (uint8)(6 + g_icb->getRandomSource()->getRandomNumber(10 - 1));
			} else {
				fight_pause--;
			}
		}
		break;

	case __THINKING:
		switch (chi_think_mode) {
		case __FOLLOWING:
			if (M->reverse_route)
				Message_box("chi in reverse");

			if (chi_history != cur_history) {
				// player must have moved on one or more rooms but we can be certain the rooms are properly adjacent

				// get next room
				next_room = chi_history + 1;
				if (next_room == MAX_player_history)
					next_room = 0; // wrap

				// is next entry a new floor or an object to interact with
				if (history[next_room].interaction == TRUE8) {
					// players interacted with something - a floor, lift, stairway, etc.

					if (L->looping) {
						L->looping = 0;
					}

					Set_motion(__MOTION_RUN);

					res = chi_interacts(history[next_room].id, "chi");
					if (!res)
						res = chi_interacts(history[next_room].id, "interact");

					if (!res) {                      // no interaction portal for chi - actually not possible now she can use the normal
						                         // 'interact' script
						chi_think_mode = __LOST; // oh dear, we must be lost. Not legal but lets handle it for now
						Tdebug("chi.txt", "chi can't follow player via %d", history[next_room].id);
						return TRUE8;
					} else {
						// ok, back to script
						// script interpreter shouldn't write a pc back

						chi_do_mode = __INTERACT_FOLLOW;

						return (IR_GOSUB);
					}
				} else { // players gone to an adjacent floor
					x = history[next_room].first_x;
					z = history[next_room].first_z;

					// if player is running then chi should run
					// if chi is more than one behind chi should run
					bool8 run = TRUE8;
					if (Cord_dist() < (400 * 400))
						run = FALSE8; // walk if near

					// 0  result
					// 1  x
					// 2  z
					// 3  0='walk', else 'run'
					// 4  0=no turn-on-spot   1=yes
					// 5  end on stand
					if (!Setup_route(result, (int32)x, (int32)z, run, __ENDB, TRUE8)) {
						// route failed or was no route required which in theory can't happen - so we take it as
						// route failed to build
						Tdebug("chi.txt", "  route failed");
						Setup_route(result, (int32)x, (int32)z, 1, __LASER, TRUE8);
					}

					Tdebug("chi.txt", "  route made");

					chi_do_mode = __ROUTING;
					return TRUE8;
				}
			} else {
				// same room - unless we started incorrectly, which we'll notice when we move
				// decide on something to do then?

				if (!chi_next_move) {
					// create a coordinate to route to
					bool8 ret = Make_floor_coordinate(&x, &z);
					if (!ret) {
						chi_next_move = 36; // try again in a second
						return TRUE8;       // make another next cycle and try again
					}

					session_barriers->Set_route_barrier_mask((int32)x - 300, (int32)x + 300, (int32)z - 300, (int32)z + 300);
					route_res = Setup_route(result, (int32)x, (int32)z, 0, __FULL, 1); // walk
					session_barriers->Clear_route_barrier_mask();

					if (!route_res) {
						// route failed or was no route required which in theory can't happen - so we take it as
						// route failed to build
						Tdebug("chi.txt", "  bumble route failed");

						chi_next_move = 36; // try again in a second
						return TRUE8;       // make another next cycle and try again
					} else {
						if (M->reverse_route)
							Message_box("chi is reversing! do it - do it now");
						chi_do_mode = __BUMBLING;
						return TRUE8;
					}
				} else {
					chi_next_move--; // reduce time to next

					// see if we should fight help
					// either we just heard player gunshot or we can see him and he's armed
					if ((Object_visible_to_camera(chi_id)) && (Cord_dist() < (PXreal)(chi_lost_dist)) &&
					    (logic_structs[player.Fetch_player_id()]->mega->actor_xyz.y == M->actor_xyz.y))
						if ((permission_to_fire) || ((g_oLineOfSight->LineOfSight(cur_id, player.Fetch_player_id())) &&
						                             (MS->logic_structs[player.Fetch_player_id()]->mega->Fetch_armed_status()))) {
							// has he got gun out?
							chi_do_mode = __GET_WEAPON_OUT;

							if (!permission_to_fire)
								fight_pause = 3; // shes seen player arming - wait a bit
							else
								fight_pause = 0; // heard a shot - arm straight away

							permission_to_fire = 0; // when armed - must see player shoot someone before she will shoot

							return TRUE8;
						}

					// move if player a distance away
					if ((Cord_dist() > (PXreal)(chi_lost_dist)) && (logic_structs[player.Fetch_player_id()]->mega->actor_xyz.y == M->actor_xyz.y) &&
					    (prev_save_state)) {
						//                          0  result
						//                          1   x
						//                          2   z
						//                          3   0='walk', else 'run'
						//                          4   0=no turn-on-spot   1=yes
						//                          5  end on stand
						x = logic_structs[player.Fetch_player_id()]->mega->actor_xyz.x;
						z = logic_structs[player.Fetch_player_id()]->mega->actor_xyz.z;

						bool8 run = TRUE8;
						bool8 eos = FALSE8;
						if (Cord_dist() < (400 * 400)) {
							run = FALSE8; // walk if near
							eos = TRUE8;  // end on stand
						}

						// set a barrier mask :(
						session_barriers->Set_route_barrier_mask((int32)x - 450, (int32)x + 450, (int32)z - 450, (int32)z + 450);
						route_res = Setup_route(result, (int32)x, (int32)z, run, __FULL, eos);
						session_barriers->Clear_route_barrier_mask();

						if (!route_res) {
							// route failed or was no route required which in theory can't happen - so we take it
							// as route failed to build
							if (result == FALSE8)
								Setup_route(result, (int32)x, (int32)z, run, __LASER, eos);
							else
								return TRUE8; // make another next cycle and try again
						} else {
							Tdebug("chi.txt", "  route made");
							if (M->reverse_route)
								Message_box("chi is reversing! do it - do it now");
							chi_do_mode = __CHASING;
							return FALSE8;
						}
					}

					// cancel permission - so we catch player gunshots immediately
					permission_to_fire = 0;
				}
			}
			break;

		case __LOST:
			// we're lost - but if the player turns up then we can restart
			if (logic_structs[player.Fetch_player_id()]->owner_floor_rect == L->owner_floor_rect) {
				chi_history = cur_history;    // chi gets same point as cord
				chi_think_mode = __FOLLOWING; // set mode
				Tdebug("chi.txt", "chi is finds cord again - chi=%d, player=%d", chi_history, cur_history);
			}
			/* fall through */

		case __NOTHING:
			// dummy mode for when switched to custom logics

			L->cur_anim_type = __STAND;
			L->anim_pc = 0;

			return (TRUE8);

			break;

		default:
			Fatal_error("ilegal chi mode");
			break;
		}
		break;
	}

	return (TRUE8);
}

PXreal _game_session::Cord_dist() {
	// return cords distance from chi (or other mega depending upon where called)
	PXreal sub1,
	sub2;

	sub1 = logic_structs[player.Fetch_player_id()]->mega->actor_xyz.x - M->actor_xyz.x;
	sub2 = logic_structs[player.Fetch_player_id()]->mega->actor_xyz.z - M->actor_xyz.z;

	// dist
	return ((sub1 * sub1) + (sub2 * sub2));
}

mcodeFunctionReturnCodes _game_session::fn_check_for_chi(int32 &result, int32 *) {
	// player wants to go down a lift so we must check to see if we need to tell chi to come over

	// returns   0 no need to wait for chi
	//				1 need to wait for chi

	// default to no
	result = 0;

	// check there is a following chi
	if (!is_there_a_chi)
		return IR_CONT;

	// not if held
	if (logic_structs[chi_id]->ob_status == OB_STATUS_HELD)
		return IR_CONT;

	if (g_mission->chi_following) {
		if (logic_structs[chi_id]->mega->dead) {
			// chi is dead
			result = 0;
			return IR_CONT;
		}

		result = 1;
		return IR_CONT;
		// chi's got to come over and onto the platform
	}

	// not in follow mode
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_wait_for_chi(int32 &, int32 *) {
	// waits for chi to register as having arrived
	// this happens when she resets her following variables

	if (chi_history == cur_history) {
		// she's arrived
		return IR_CONT;
	}

	return IR_REPEAT;
}

mcodeFunctionReturnCodes _game_session::fn_chi_wait_for_player_to_move(int32 &, int32 *) {
	// set to go-cord-go wait mode
	// used after lift movement

	chi_history = cur_history; // chi gets same point as cord

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_register_chi(int32 &, int32 *) {
	// tell engine there is a chi object

	if (is_there_a_chi)
		Fatal_error("double call to fn_register_chi");

	Tdebug("chi.txt", "%s registers as chi", CGameObject::GetName(object));

	is_there_a_chi = TRUE8;
	chi_id = cur_id;

	return IR_CONT;
}

bool8 _game_session::Make_floor_coordinate(PXreal *x, PXreal *z) {
	// make a coordinate for the floor

	if (!local_history_count)
		return FALSE8;

	int32 choice = g_icb->getRandomSource()->getRandomNumber(local_history_count - 1);

	*x = local_history[choice].x;
	*z = local_history[choice].z;

	return TRUE8;
}

} // End of namespace ICB
