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
#include "engines/icb/common/px_floor_map.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/floors.h"
#include "engines/icb/debug.h"
#include "engines/icb/route_manager.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/session.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/res_man.h"
#include "engines/icb/mission.h"

namespace ICB {

#define ROUTE_CLOSE 10 * REAL_ONE

mcodeFunctionReturnCodes fn_route_to_marker(int32 &result, int32 *params) {
	return (MS->fn_route_to_marker(result, params));
}

mcodeFunctionReturnCodes fn_route_to_nico(int32 &result, int32 *params) {
	return (MS->fn_route_to_nico(result, params));
}

mcodeFunctionReturnCodes fn_tiny_route(int32 &result, int32 *params) {
	return (MS->fn_tiny_route(result, params));
}

mcodeFunctionReturnCodes fn_sharp_route(int32 &result, int32 *params) {
	return (MS->fn_sharp_route(result, params));
}

mcodeFunctionReturnCodes fn_route_to_near_mega(int32 &result, int32 *params) {
	return (MS->fn_route_to_near_mega(result, params));
}

mcodeFunctionReturnCodes fn_interact_near_mega(int32 &result, int32 *params) {
	return (MS->fn_interact_near_mega(result, params));
}

mcodeFunctionReturnCodes fn_sharp_route_to_near_mega(int32 &result, int32 *params) {
	return (MS->fn_sharp_route_to_near_mega(result, params));
}

mcodeFunctionReturnCodes fn_spectre_route_to_mega(int32 &result, int32 *params) {
	return (MS->fn_spectre_route_to_mega(result, params));
}

mcodeFunctionReturnCodes fn_laser_route(int32 &result, int32 *params) {
	return (MS->fn_laser_route(result, params));
}

mcodeFunctionReturnCodes fn_room_route(int32 &result, int32 *params) {
	return (MS->fn_room_route(result, params));
}

void _game_session::Reset_route_manager() {
	// reset the router object - called by session constructor

	// remove used routes - just keeps the memory limit down

	Zdebug("--new sesssion: reseting route manager service--");
}

bool8 _game_session::Is_route_required(PXreal startx, PXreal startz, PXreal destx, PXreal destz) {
	// check if route is really close or exact
	// if close then adjust

	if ((PXfabs(startx - destx) < ROUTE_CLOSE) && (PXfabs(startz - destz) < ROUTE_CLOSE)) {
		M->actor_xyz.x = destx;
		M->actor_xyz.z = destz;

		//		no route is required
		return (FALSE8);
	}

	// a route is required
	return (TRUE8);
}

void _game_session::Create_initial_route(__rtype type) {
	// Changed last parameter to int32 from bool to stop a performance warning.  Other bools are passed as int anyway,
	// so this is more consistent and faster.

	// create an initial route for character 'id'
	// this route will replace any route that maybe partway complete at this moment (chr$ can change their plans)
	// the route is of a fixed type specified by the _route_type structure
	// after building a route we return allowing fn_ functions to make any decisions/selections they wish before pressing on with animation

	_route_stat res;
	uint32 time = 0;

	if ((px.logic_timing) && (px.mega_timer))
		time = GetMicroTimer();

	Zdebug("create_initial_route");

	if (!Is_route_required(M->m_main_route.request_form.initial_x, M->m_main_route.request_form.initial_z, M->m_main_route.request_form.dest_x,
	                       M->m_main_route.request_form.dest_z)) {
		M->m_main_route.request_form.error = __RR_NO_ROUTE_REQUIRED;
		Zdebug("no route required");
		return;
	}

	//	set extrapolation size
	troute.extrap_size = M->extrap_size;

	M->m_phase = RM_MAIN;

	// reset the routers barrier list
	troute.Reset_barrier_list();

	switch (type) {
	case __FULL:
		MS->session_barriers->Form_route_barrier_list(M->m_main_route.request_form.initial_x, M->m_main_route.request_form.character_y,
		                                              M->m_main_route.request_form.initial_z, M->m_main_route.request_form.dest_x, M->m_main_route.request_form.dest_z);
		break;

	case __ENDB:
		MS->session_barriers->Form_parent_barrier_list(M->m_main_route.request_form.dest_x, M->m_main_route.request_form.character_y, M->m_main_route.request_form.dest_z);
		break;

	default:
		break;
	}

	// now make the route
	res = troute.Calc_route(M->m_main_route.request_form.initial_x, M->m_main_route.request_form.initial_z, M->m_main_route.request_form.dest_x,
	                        M->m_main_route.request_form.dest_z);

	if (res == __PRIM_ROUTE_OK) {
		Zdebug("route ok");

		// get the route - pass the address of a pointer to a _point where the route can be new'ed and copied
		troute.Give_route(&M->m_main_route);

		// get barriers for nethack diagnostics
		if (type != __LASER)
			troute.Give_barrier_list(&M->m_main_route);

		if (L->pan >= HALF_TURN)
			L->pan -= FULL_TURN;
		else if (L->pan <= -HALF_TURN)
			L->pan += FULL_TURN;

		Calc_dist_and_pan(M->actor_xyz.x, M->actor_xyz.z, &M->m_main_route);

		M->m_main_route.request_form.error = __ROUTE_REQUEST_OK;
	} else {
		Zdebug("route failed");
		M->m_main_route.request_form.error = __ROUTE_REQUEST_PRIM_FAILED;
	}

	if ((px.logic_timing) && (px.mega_timer)) {
		time = GetMicroTimer() - time;
		L->slowest_cycle_time = time;
	}
}

bool8 _game_session::Process_route() {
	// processes the route that has been built by Create_initial_route
	// animate along this characters route

	// returns   0 still going
	//				1 finished

	_route_description *route;
	bool8 res;

	route = &M->m_main_route;

	// gosub turning
	if ((M->target_pan) && (!L->auto_panning)) { // animated pan - as opposed to engine auto panning
		Animate_turn_to_pan(__TURN_ON_THE_SPOT_CLOCKWISE, 1);

		return (0);
		//      note, when this anim finishes we must snap the x,z back to what they were before the turn began
	} else if (route->arrived) {
		// must have finished route and set up a slow out anim - now run that anim
		if (route->request_form.finish_on_stand) {
			res = Play_anim();
		}

		else
			res = Play_anim_with_no_movement();

		return (res);

		// remove the phase so headup diagnostics disappear now the route is effectively done
		M->m_phase = RM_NONE;

		return (1); // no slow-out so we're done
	}

	// ok, process that point-to-point route!
	if (Animate_points(route)) {
		// route has finished
		// has a slow-out been requested?

		if (route->request_form.finish_on_stand) {
			// set up follow on animation as requested

			// set up walk-to stand + stand

			Soft_start_with_double_link(__WALK_TO_STAND, __WALK_TO_OTHER_STAND_LEFT_LEG, __STAND);

			res = Play_anim();

			// if more to do then return
			return (res);

			// else drop down and return fnished after removing the phase
		} else if (route->request_form.finish_on_null_stand) { // play link anim WITHOUT coordinate movement; for interaction accuracy
			// set up follow on animation as requested
			// set up walk-to stand + stand

			Soft_start_with_double_link(__WALK_TO_STAND, __WALK_TO_OTHER_STAND_LEFT_LEG, __STAND);

			res = Play_anim_with_no_movement();

			// if more to do then return
			return (res);

			// else drop down and return fnished after removing the phase
		}

		// remove the phase so headup diagnostics disappear now the route is effectively done
		M->m_phase = RM_NONE;

		return (1); // no slow-out so we're done
	}
	// still auto-routing
	return (0);
}

_route_description *_game_session::Fetch_route_desc(uint32 id) {
	// return a pointer to a built route
	// this is used to draw into the plan viewer
	// returns a 0 if there is no route

	// no route at all
	if (!logic_structs[id]->mega->m_phase)
		return (0);

	return (&logic_structs[id]->mega->m_main_route);
}

void _route_description::___init() {
	// nethack diagnostics
	if (diag_bars)
		delete[] diag_bars;
	diag_bars = 0;

	number_of_diag_bars = 0;

	arrived = FALSE8; // when route has been animated this is set to YES ready for optional slow-out animation

	total_points = 0; // in-case fails for viewers
}

mcodeFunctionReturnCodes _game_session::fn_tiny_route(int32 &result, int32 *params) {
	// auto-route a mega character characters x,z to specified x1,z1 from the same or adjoining floor rects
	// player or mega

	// params    0   x   INT32's to be cast to floats :| hmmm...
	//			1  z
	//			2  0='walk', else 'run'
	//			3  0=no turn-on-spot   1=yes

	// return    IR_CONT or
	//			IR_REPEAT

	int32 sub1, sub2, len;
	bool8 route_res;

	if (L->looping < 2) {

		Tdebug("route_async.txt", "%s tiny", object->GetName());

		// dont continue until all route related anims have asynced into memory
		if (!Route_async_ready((bool8)params[2], (bool8)params[3]))
			return IR_REPEAT;

		// check for free router
		if (Is_router_busy())
			return IR_REPEAT;

		sub1 = params[0] - (int32)M->actor_xyz.x;
		sub2 = params[1] - (int32)M->actor_xyz.z;

		if (sub1 < 0)
			sub1 = (0 - sub1);
		if (sub2 < 0)
			sub2 = (0 - sub2);

		len = sub1 + sub2;

		session_barriers->Set_route_barrier_mask((int32)M->actor_xyz.x - len, (int32)M->actor_xyz.x + len, (int32)M->actor_xyz.z - len, (int32)M->actor_xyz.z + len);
		route_res = Setup_route(result, params[0], params[1], params[2], __FULL, TRUE8);
		session_barriers->Clear_route_barrier_mask();
		if (!route_res) {
			if (!result) { // failed
				Setup_route(result, params[0], params[1], params[2], __LASER, TRUE8); // must succeed
			} else {
				//      not looping any int32er
				L->looping = 0;
				result = TRUE8;
				return IR_CONT; // finished so send script on
			}
		}
	}

	// animate the route
	if (Process_route()) {
		//		not looping any int32er
		L->looping = 0;
		result = TRUE8;
		return (IR_CONT); // finished so send script on
	}

	// not finished, so see you next cycle
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_room_route(int32 &result, int32 *params) {
	// auto-route a mega character characters x,z to specified x1,z1 in an adjacent room
	// player or mega
	// doesnt end on stand

	// params    0   x
	//			1  z
	//			2  0='walk', else 'run'
	//			3  end on stand (i.e. tiny or sharp)

	// return    IR_CONT or
	//			IR_REPEAT

	if (L->looping < 2) {
		//		dont continue until all route related anims have asynced into memory
		if (!Route_async_ready((bool8)params[2], (bool8)params[3]))
			return IR_REPEAT;

		// check for free router
		if (Is_router_busy()) {
			return IR_REPEAT;
		}

		if (!Setup_route(result, params[0], params[1], params[2], __ENDB, params[3])) {
			L->looping = 0;
			return (IR_CONT);
		}
	}

	// animate the route
	if (Process_route()) {
		// not looping any int32er
		L->looping = 0;
		result = TRUE8;
		return (IR_CONT); // finished so send script on
	}

	// not finished, so see you next cycle
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_sharp_route(int32 &result, int32 *params) {
	// auto-route a mega character characters x,z to specified x1,z1 from the same or adjoining floor rects
	// player or mega
	// doesnt end on stand

	// params    0   x   INT32's to be cast to floats :| hmmm...
	//			1  z
	//			2  0='walk', else 'run'
	//			3  0=no turn-on-spot   1=yes

	// return    IR_CONT or
	//			IR_REPEAT

	int32 sub1, sub2, len;
	bool8 route_res;

	if (L->looping < 2) {
		// dont continue until all route related anims have asynced into memory
		if (!Route_async_ready((bool8)params[2], (bool8)params[3]))
			return IR_REPEAT;

		// check for free router
		if (Is_router_busy()) {
			return IR_REPEAT;
		}

		sub1 = params[0] - (int32)M->actor_xyz.x;
		sub2 = params[1] - (int32)M->actor_xyz.z;

		if (sub1 < 0)
			sub1 = (0 - sub1);
		if (sub2 < 0)
			sub2 = (0 - sub2);

		len = sub1 + sub2;

		session_barriers->Set_route_barrier_mask((int32)M->actor_xyz.x - len, (int32)M->actor_xyz.x + len, (int32)M->actor_xyz.z - len, (int32)M->actor_xyz.z + len);
		route_res = Setup_route(result, params[0], params[1], params[2], __FULL, FALSE8);
		session_barriers->Clear_route_barrier_mask();
		if (!route_res) {
			if (!result) { // failed
				Setup_route(result, params[0], params[1], params[2], __LASER, FALSE8); // must succeed
			} else {
				//      not looping any int32er
				L->looping = 0;
				result = TRUE8;
				return IR_CONT; // finished so send script on
			}
		}
	}

	// animate the route
	if (Process_route()) {
		// not looping any int32er
		L->looping = 0;
		result = TRUE8;
		return (IR_CONT); // finished so send script on
	}

	// not finished, so see you next cycle
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_laser_route(int32 &result, int32 *params) {
	// auto-route a mega character characters x,z to specified x1,z1 from the same or adjoining floor rects
	// player or mega
	// doesnt end on stand

	// params    0   x   INT32's to be cast to floats :| hmmm...
	//			1  z
	//			2  0='walk', else 'run'
	//			3  end on stand (i.e. tiny or sharp)

	// return    IR_CONT or
	//			IR_REPEAT

	if (L->looping < 2) {
		// dont continue until all route related anims have asynced into memory
		if (!Route_async_ready((bool8)params[2], (bool8)params[3]))
			return IR_REPEAT;

		// check for free router
		if (Is_router_busy()) {
			return IR_REPEAT;
		}

		if (!Setup_route(result, params[0], params[1], params[2], __LASER, params[3])) {
			L->looping = 0;
			return (IR_CONT);
		}
	}

	// animate the route
	if (Process_route()) {
		// not looping any int32er
		L->looping = 0;
		result = TRUE8;
		return (IR_CONT); // finished so send script on
	}

	// not finished, so see you next cycle
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_route_to_nico(int32 &result, int32 *params) {
	// auto-route a mega character characters x,z to nico x1,z1 from the same or adjoining floor rects
	// walk to a nico marker
	// player or mega

	// params    0   nico
	//			1  0='walk', else 'run'
	//			2  0=no turn-on-spot   1=yes

	// return    IR_CONT or
	//			IR_REPEAT

	_feature_info *monica;

	const char *nico_name = NULL;
	if (params && params[0]) {
		nico_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	}

	if (L->looping < 2) {
		// dont continue until all route related anims have asynced into memory
		if (!Route_async_ready((bool8)params[1], (bool8)params[2]))
			return IR_REPEAT;

		// check for free router
		if (Is_router_busy()) {
			return IR_REPEAT;
		}

		monica = (_feature_info *)features->Try_fetch_item_by_name(nico_name);

		if (!monica)
			Fatal_error("fn_route_to_nico - object [%s] cant find nico [%s]", object->GetName(), nico_name);

		// build route
		if (!Setup_route(result, (int32)monica->x, (int32)monica->z, params[1], __FULL, TRUE8)) {
			if (result == 0)
				Message_box("fn_route_to_nico nico found but route failed");

			L->looping = 0;
			return (IR_CONT);
		}
	}

	// animate the route
	if (Process_route()) {
		// not looping any int32er
		L->looping = 0;
		result = TRUE8;
		return (IR_CONT); // finished so send script on
	}

	// not finished, so see you next cycle
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_interact_near_mega(int32 &result, int32 *params) {
	// auto-route a mega character characters x,z to nico x1,z1 from the same or adjoining floor rects
	// walk to a nico marker
	// player or mega

	// params
	//			0  0='walk', else 'run'
	//			1  0=no turn-on-spot   1=yes
	//			2  distance within which we stop

	// return    IR_CONT or
	//			IR_REPEAT

	PXreal sub1, sub2, len;

	// check we are within the distance and stop us if so
	sub1 = logic_structs[M->target_id]->mega->actor_xyz.x - M->actor_xyz.x;
	sub2 = logic_structs[M->target_id]->mega->actor_xyz.z - M->actor_xyz.z;

	//	dist
	len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

	if (L->looping < 2) {
		// dont continue until all route related anims have asynced into memory
		if (!Route_async_ready((bool8)params[2], (bool8)params[3]))
			return IR_REPEAT;

		// check for free router
		if (Is_router_busy()) {
			return IR_REPEAT;
		}

		// dont even build route if too close already
		if (len < (PXreal)(params[2] * params[2])) {
			L->looping = 0;
			result = TRUE8;
			return (IR_CONT); // finished so send script on
		}

		// build route
		if (!Setup_route(result, (int32)logic_structs[M->target_id]->mega->actor_xyz.x, (int32)logic_structs[M->target_id]->mega->actor_xyz.z, params[0], __FULL, TRUE8)) {
			if (result == 0)
				Message_box("fn_route_to_nico nico found but route failed");

			L->looping = 0;
			return (IR_CONT);
		}
	}

	// animate the route
	if (Process_route()) {
		// not looping any int32er
		L->looping = 0;
		result = TRUE8;
		return (IR_CONT); // finished so send script on
	}

	if (len < (PXreal)(params[2] * params[2])) {
		M->m_main_route.dist_left = FLOAT_ZERO;
		M->m_main_route.current_position = (M->m_main_route.total_points - 1);
	}

	// not finished, so see you next cycle
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_spectre_route_to_mega(int32 &result, int32 *params) {
	// auto-route a mega character to named mega to within specified distance - rebuild the route if the target moves away from original position

	// params    0   mega object name
	//			1  0='walk', else 'run'
	//			2  0=no turn-on-spot   1=yes
	//			3  distance within which we stop
	//			4  re-route trigger distance

	PXreal sub1, sub2, len, len2 = REAL_ONE;
	uint32 id;
#define SPECTRE_INITIAL_ROUTE_TWEEK 4242
#define SPECTRE_ROUTE_TWEEK 4269
	int32 tx;
	int32 tz;
	bool8 route_res;

	const char *mega_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// get object to check
	id = objects->Fetch_item_number_by_name(mega_name);
	if (id == 0xffffffff)
		Fatal_error("fn_spectre_route_to_mega - illegal object [%s]", mega_name);

	// check for starting off on a different floor - route straight to player
	if ((!L->looping) && (L->owner_floor_rect != logic_structs[id]->owner_floor_rect)) {
		L->list[1] = SPECTRE_INITIAL_ROUTE_TWEEK;
		// build route
		if (!Setup_route(result, (int32)logic_structs[id]->mega->actor_xyz.x, (int32)logic_structs[id]->mega->actor_xyz.z, params[1], __ENDB, 0)) { // sharp
			if (result == 0)
				Message_box("fn_spectre_route_to_mega route failed");
			return IR_CONT;
		}
	}

	// check we are within the distance and stop us if so
	sub1 = logic_structs[id]->mega->actor_xyz.x - M->actor_xyz.x;
	sub2 = logic_structs[id]->mega->actor_xyz.z - M->actor_xyz.z;
	len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

	// are we close enough to finish here?
	if (len < (PXreal)(params[3] * params[3])) {
		L->looping = 0;
		return IR_CONT; // yeah
	}

	if (L->owner_floor_rect == logic_structs[id]->owner_floor_rect) {
		L->list[0] = cur_history; // cancel tweeky catch up mode
		L->list[1] = 0; // cancel follow modes
	}

	// see if the target has moved
	// get distance from targets original position
	if (L->looping) {
		sub1 = logic_structs[id]->mega->actor_xyz.x - M->pushed_actor_xyz.x;
		sub2 = logic_structs[id]->mega->actor_xyz.z - M->pushed_actor_xyz.z;
		len2 = (PXreal)((sub1 * sub1) + (sub2 * sub2));
	}

	// normal same floor following
	if ((L->owner_floor_rect == logic_structs[id]->owner_floor_rect) && ((!L->looping) || (len2 > (PXreal)(params[4] * params[4])))) {
		M->pushed_actor_xyz.x = logic_structs[id]->mega->actor_xyz.x;
		M->pushed_actor_xyz.z = logic_structs[id]->mega->actor_xyz.z;
		M->reverse_route = FALSE8;

		//		build route
		//		get target coords
		tx = (int32)logic_structs[id]->mega->actor_xyz.x;
		tz = (int32)logic_structs[id]->mega->actor_xyz.z;

		//		route with a mask - the mask will be the retrigger dist; so should be fine but will occlude some in huge rooms
		session_barriers->Set_route_barrier_mask((int32)tx - params[4], (int32)tx + params[4], (int32)tz - params[4], (int32)tz + params[4]);
		route_res = Setup_route(result, tx, tz, params[1], __FULL, 0);
		session_barriers->Clear_route_barrier_mask();

		if (!route_res) { // sharp
			if (result == 0) {
				if (!Setup_route(result, (int32)logic_structs[id]->mega->actor_xyz.x, (int32)logic_structs[id]->mega->actor_xyz.z, params[1], __LASER, 0)) {
					if (result == 0) {
						L->looping = 0;
						return IR_CONT;
					}
				}
			}
		}
		return IR_REPEAT;
	}

	// check for different floors - route to players exit point - nice straight line
	if ((!L->list[1]) && (L->owner_floor_rect != logic_structs[id]->owner_floor_rect)) {
		// was on same floor but now gone
		// go to coordinate where he left this room

		L->list[1] = SPECTRE_ROUTE_TWEEK;
		if (!Setup_route(result, (int32)spectre_hist[L->owner_floor_rect].x, (int32)spectre_hist[L->owner_floor_rect].z, params[1], __LASER, 0)) { // sharp
			if (result == 0)
				Message_box("fn_spectre_route_to_mega route failed");

			L->looping = 0;
			return IR_CONT;
		}
	}

	// animate the route
	if (Process_route()) {

		// just finished special to-door route - now do a super route through door
		if (L->list[1] == SPECTRE_ROUTE_TWEEK) {
			// player went and we're now at the door where he left

			L->list[0] = L->list[0] + 1;
			if (L->list[0] == MAX_player_history)
				L->list[0] = 0; // wrap

			// do a super route to cross floor
			// use a mask - we can, because the coordinate is at the door
			tx = (int32)history[L->list[0]].first_x;
			tz = (int32)history[L->list[0]].first_z;
			session_barriers->Set_route_barrier_mask((int32)tx - 200, (int32)tx + 200, (int32)tz - 200, (int32)tz + 200);
			route_res = Setup_route(result, tx, tz, params[1], __ENDB, 0);
			session_barriers->Clear_route_barrier_mask();

			if (!route_res) { // sharp
				if (result == 0) {
					if (!Setup_route(result, (int32)history[L->list[0]].first_x, (int32)history[L->list[0]].first_z, params[1], __LASER, 0)) { // sharp
						if (result == 0) {
							L->looping = 0;
							return IR_CONT;
						}
					}
				}
			}
			return IR_REPEAT;
		}

		//		not looping any int32er
		L->list[1] = 0; // clear initail-route-tweek
		L->looping = 0;
		result = TRUE8;
		return IR_CONT; // finished so send script on
	}

	// not finished, so see you next cycle
	return IR_REPEAT;
}

mcodeFunctionReturnCodes _game_session::fn_sharp_route_to_near_mega(int32 &result, int32 *params) {
	// auto-route a mega character characters x,z to nico x1,z1 from the same or adjoining floor rects
	// walk to a nico marker
	// player or mega

	// params    0   mega object name
	//			1  0='walk', else 'run'
	//			2  0=no turn-on-spot   1=yes
	//			3  distance within which we stop

	// return    IR_CONT or
	//			IR_REPEAT

	const char *mega_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	return Route_to_near_mega_core(mega_name, params[1], params[2], params[3], FALSE8, result);
}

mcodeFunctionReturnCodes _game_session::fn_route_to_near_mega(int32 &result, int32 *params) {
	// auto-route a mega character characters x,z to nico x1,z1 from the same or adjoining floor rects
	// walk to a nico marker
	// player or mega

	// params    0   mega object name
	//			1  0='walk', else 'run'
	//			2  0=no turn-on-spot   1=yes
	//			3  distance within which we stop

	// return    IR_CONT or
	//			IR_REPEAT

	const char *mega_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	return Route_to_near_mega_core(mega_name, params[1], params[2], params[3], TRUE8, result);
}

mcodeFunctionReturnCodes _game_session::Route_to_near_mega_core(const char *name, int32 run, int32 initial_turn, uint32 dist, int32 end_on_stand, int32 &result) {
	PXreal sub1, sub2, x, z;
	uint32 id;
	_feature_info *monica;
	bool8 route_res;
	int32 tx;
	int32 tz;
	int32 len, mask_len;

	// get object to check

	monica = (_feature_info *)features->Try_fetch_item_by_name(name);
	if (monica) {
		x = monica->x;
		z = monica->z;

	} else {
		id = objects->Fetch_item_number_by_name(name);
		if (id == 0xffffffff)
			Fatal_error("[%s] calling Route_to_near_mega_core - finds neither object or nico named [%s]", object->GetName(), name);
		//			found mega with name!
		//			check we are within the distance and stop us if so

		x = logic_structs[id]->mega->actor_xyz.x;
		z = logic_structs[id]->mega->actor_xyz.z;
	}

	sub1 = x - M->actor_xyz.x;
	sub2 = z - M->actor_xyz.z;

	// dist
	len = (int32)((sub1 * sub1) + (sub2 * sub2));

	if (L->looping < 2) {
		// dont continue until all route related anims have asynced into memory
		if (!Route_async_ready((bool8)run, (bool8)initial_turn))
			return IR_REPEAT;

		// check for free router
		if (Is_router_busy())
			return IR_REPEAT;

		// dont even build route if too close already
		if (len < (int32)(dist * dist)) {
			result = TRUE8;
			L->looping = 0;
			return IR_CONT; // finished so send script on
		}

		// route mask length - accuracy not required

		if (sub1 < REAL_ZERO)
			sub1 = (REAL_ZERO - sub1);
		if (sub2 < REAL_ZERO)
			sub2 = (REAL_ZERO - sub2);

		mask_len = (int32)(sub1 + sub2);

		// build route
		// apply a route mask
		tx = (int32)x;
		tz = (int32)z;
		session_barriers->Set_route_barrier_mask((int32)tx - mask_len, (int32)tx + mask_len, (int32)tz - mask_len, (int32)tz + mask_len);
		route_res = Setup_route(result, (int32)x, (int32)z, run, __FULL, end_on_stand);
		session_barriers->Clear_route_barrier_mask();

		if (!route_res) { // no route built
			if (!result) { // failed
				Setup_route(result, (int32)x, (int32)z, run, __LASER, end_on_stand); // must succeed
			} else {
				//      not looping any int32er
				L->looping = 0;
				result = TRUE8;
				return IR_CONT; // finished so send script on
			}
		}
	}

	// animate the route
	if (Process_route()) {
		// not looping any int32er
		L->looping = 0;
		result = TRUE8;
		return IR_CONT; // finished so send script on
	}

	if (len < (PXreal)(dist * dist)) {
		M->m_main_route.dist_left = FLOAT_ZERO;
		M->m_main_route.current_position = (M->m_main_route.total_points - 1);

		if (!end_on_stand) {
			L->looping = 0;
			return IR_CONT;
		}
	}

	// not finished, so see you next cycle
	return IR_REPEAT;
}

mcodeFunctionReturnCodes _game_session::fn_route_to_marker(int32 &result, int32 *params) {
	// walk to a nico marker
	// player or mega

	// params    0   name of marker
	//			1  0='walk', else 'run'
	//			2  0=no turn-on-spot   1=yes

	// return    IR_CONT or
	//			IR_REPEAT

	_map_marker *marker;

	const char *marker_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (L->looping < 2) {
		// dont continue until all route related anims have asynced into memory
		if (!Route_async_ready((bool8)params[2], (bool8)params[3]))
			return IR_REPEAT;

		// check for free router
		if (Is_router_busy()) {
			return IR_REPEAT;
		}

		marker = (_map_marker *)markers.Fetch_marker_by_object_name(marker_name);

		if (!marker)
			Fatal_error("fn_route_to_marker - object [%s] cant find marker [%s]", object->GetName(), marker_name);

		// build route
		if (!Setup_route(result, (int32)marker->x, (int32)marker->z, params[1], __FULL, TRUE8)) {
			L->looping = 0;
			return (IR_CONT);
		}
	}

	// animate the route
	if (Process_route()) {
		// not looping any int32er
		L->looping = 0;
		result = TRUE8;
		return (IR_CONT); // finished so send script on
	}

	// not finished, so see you next cycle
	return (IR_REPEAT);
}

// Changed last two parameters to int32 from bool tostop a performance warning.  Other bools are passed as int anyway,
// so this is more consistent and faster.
bool8 _game_session::Setup_route(int32 &result, int32 corex, int32 corez, int32 runs, __rtype type, int32 end_on_stand) {
	// setup and create the route

	// returns   false if we should return to script - because route failed or is not required
	//				true if we need to now go on to animate along the route

	//				result=
	//				TRUE - no route required
	//				FALSE- route failed

	// only one of these per cycle
	Set_router_busy();

	// stop people routing when armed
	if ((cur_id == player.Fetch_player_id()) && (M->Fetch_armed_status())) {
		// tell it no route is required
		result = TRUE8;
		return (FALSE8);
	}

	// set up walk or run
	M->m_main_route.___init();

	// set walk or run
	if (!runs) {
		M->m_main_route.request_form.anim_type = __WALK;
		Set_motion(__MOTION_WALK); // for safety
	} else {
		M->m_main_route.request_form.anim_type = __RUN;
		Set_motion(__MOTION_RUN); // for safety
	}

	// quick CAPS check on the anim
	if ((!L->voxel_info->IsAnimTable(L->cur_anim_type)))
		Fatal_error("mega [%s] has anim [%s] missing", object->GetName(), master_anim_name_table[L->cur_anim_type].name);

	// new route do prepare a route request form!
	// initial x,z
	M->m_main_route.request_form.initial_x = M->actor_xyz.x;
	M->m_main_route.request_form.initial_z = M->actor_xyz.z;

	// target x,z
	M->m_main_route.request_form.dest_x = (PXreal)corex;
	M->m_main_route.request_form.dest_z = (PXreal)corez;

	// need characters y coordinate also
	M->m_main_route.request_form.character_y = M->actor_xyz.y;

	// this function attempts to finish on stand
	M->m_main_route.request_form.finish_on_stand = (end_on_stand == 0) ? FALSE8 : TRUE8;

	M->m_main_route.request_form.finish_on_null_stand = FALSE8; // special mode for interactions

	// set type
	M->m_main_route.request_form.rtype = ROUTE_points_only;

	// now log and create the initial route
	Create_initial_route(type);

	// if the route could not be built
	if (M->m_main_route.request_form.error == __ROUTE_REQUEST_PRIM_FAILED) {
		result = FALSE8;
		return (FALSE8);
	}

	if (M->m_main_route.request_form.error == __RR_NO_ROUTE_REQUIRED) {
		result = TRUE8;
		return (FALSE8);
	}

	// we are now looping, having done the init
	L->looping = 2;

	return (TRUE8);
}

uint32 _game_session::Animate_points(_route_description *route) {
	// animate to each point in turn

	// look at the characters available anims to judge the frames to use - this has a degree of flexibility

	// compute the pan and distance of the current line

	//	return   0 still going
	//				1 we're finished

	PXreal sub1, sub2;
	PXreal xnext, znext;
	PXreal x, z;
	PXreal dist;

	// have we any further to go on the current segment?
	if (route->dist_left == FLOAT_ZERO) {
		Zdebug(" end of seg");
		// check for end of route
		if (route->current_position == (route->total_points - 1)) {
			Zdebug("end of route!");

			route->arrived = TRUE8;

			// its the end
			return (1);
		}

		// advance to the next line segment
		route->current_position++;

		// calc distance to travel and set the pan value (force the pan for now)

		Calc_dist_and_pan(M->actor_xyz.x, M->actor_xyz.z, route);

		// if  new pan is acute we may wish to GOSUB to a turn anim?
	}

	//if   not current __WALK then choose nearest leg position from __WALK frame set

	if (L->cur_anim_type != route->request_form.anim_type) {
		// starting anew so softly into walk anim

		Soften_up_anim_file(route->request_form.anim_type, 1000000);
		L->cur_anim_type = route->request_form.anim_type;
	}

	// advance frame and motion - clipping distance if over the end

	// work out how far we wish to move
	// open the motion anim file
	ANIM_CHECK(L->cur_anim_type);

	PXanim *pAnim = (PXanim *)rs_anims->Res_open(L->voxel_info->get_info_name(L->cur_anim_type), L->voxel_info->info_name_hash[L->cur_anim_type], L->voxel_info->base_path,
	                                             L->voxel_info->base_path_hash); //

	if (L->anim_pc + 1 >= pAnim->frame_qty)
		Fatal_error("Animate_points finds [%s] has illegal frame in anim [%s] %d %d", (const char *)L->GetName(),
		            (const char *)L->voxel_info->get_info_name(L->cur_anim_type), L->anim_pc, pAnim->frame_qty);

	// get motion displacement from currently displayed frame to next one
	// note that we always read frame+1 for motion of next frame even though the voxel frame itself will be looped back to 0
	PXreal x1, z1, x2, z2, unused;
	PXFrameEnOfAnim(L->anim_pc + 1, pAnim)->markers[ORG_POS].GetXYZ(&x1, &unused, &z1);
	PXFrameEnOfAnim(L->anim_pc, pAnim)->markers[ORG_POS].GetXYZ(&x2, &unused, &z2);

	xnext = x1 - x2;
	znext = z1 - z2;

	// calculate the new x and z coordinate from this frames motion offset
	// do the z and x together
	PXfloat ang = L->pan * TWO_PI;
	PXfloat cang = (PXfloat)PXcos(ang);
	PXfloat sang = (PXfloat)PXsin(ang);

	x = M->actor_xyz.x + PXfloat2PXreal(xnext * cang + znext * sang);
	z = M->actor_xyz.z + PXfloat2PXreal(znext * cang - xnext * sang);

	// work out how far we are from the end

	// if over the end then clip the distance
	// we do this by
	// working out how far on we go this time
	// and if that is further than the distance left then we clip

	// work out how far the next frame moves us

	sub1 = x - M->actor_xyz.x;
	sub2 = z - M->actor_xyz.z;
	dist = (PXreal)PXsqrt((sub1 * sub1) + (sub2 * sub2));

	if (dist > route->dist_left) {
		// this would take us too far so clip at the end point
		M->actor_xyz.x = route->prim_route[route->current_position].x;
		M->actor_xyz.z = route->prim_route[route->current_position].z;

		// set to finished
		route->dist_left = FLOAT_ZERO;

		// in theory, we must now do extra correctional work on the actor.pos.x,z thing?
	} else {
		// ok, use the desired motion
		M->actor_xyz.x = x;
		M->actor_xyz.z = z;

		// we've moved this much further
		route->dist_left -= dist;
	}

	// advance the frame
	L->anim_pc = (L->anim_pc + 1) % (pAnim->frame_qty - 1);

	return (0);
}

void _game_session::Advance_auto_pan() {
	// move the pan closer to target

	PXfloat turn;

	if (!M->turn_dir)
		turn = -(FULL_TURN / 10); // 0.1f;
	else
		turn = (FULL_TURN / 10); // 0.1f;

	if (PXfabs(turn) >= M->target_pan) {
		//		too far so snap
		L->auto_panning = FALSE8;
		M->target_pan = ZERO_TURN; // we're done
		L->pan = M->auto_target_pan; // used for things like interact - where we just auto turn to target
	} else {
		L->auto_display_pan += turn;
		M->target_pan = M->target_pan - (PXfloat)PXfabs(turn); // distance left to travel
	}

	if (L->auto_display_pan >= HALF_TURN)
		L->auto_display_pan -= FULL_TURN;
	else if (L->auto_display_pan <= -HALF_TURN)
		L->auto_display_pan += FULL_TURN; // stop the wrap
}

void _game_session::Calc_dist_and_pan(PXreal x, PXreal z, _route_description *route) {
	// setup the next segment to route down
	// calc 'dist_left'

	PXreal sub1, sub2;

	// get length
	sub1 = x - route->prim_route[route->current_position].x;
	sub2 = z - route->prim_route[route->current_position].z;

	route->dist_left = (PXreal)PXsqrt((sub1 * sub1) + (sub2 * sub2));

	// calculate the facing pan down this track

	L->auto_panning = FALSE8; // none
	M->target_pan = ZERO_TURN; // none

	PXfloat new_pan, diff;

	if (!M->reverse_route)
		new_pan = PXAngleOfVector(route->prim_route[route->current_position].z - z, route->prim_route[route->current_position].x - x);
	else
		new_pan = PXAngleOfVector(z - route->prim_route[route->current_position].z, x - route->prim_route[route->current_position].x);

	// get difference between the two
	diff = new_pan - L->pan;

	if (PXfabs(diff) > (FULL_TURN / 10)) { // 0.1f
		                               // work out which way to turn
		if (diff > HALF_TURN)
			diff -= FULL_TURN;
		else if (diff < -HALF_TURN)
			diff += FULL_TURN;

		//		diff is now the distance to turn by and its sign denotes direction

		if (diff < FLOAT_ZERO)
			M->turn_dir = 0; // right
		else
			M->turn_dir = 1; // left

		M->target_pan = (PXfloat)PXfabs(diff); // save positive pan distance
		L->auto_display_pan = L->pan; // start where we currently are
		L->auto_panning = TRUE8;
		M->auto_target_pan = new_pan; // need this here as its snapped to at end of auto-pan cycle
	}

	// normal pan set regardless for movement
	L->pan = new_pan;
}

void _game_session::Calc_dist_and_target_pan(PXreal x, PXreal z, _route_description *route) {
	// usually called to setup the first segment to route down
	// will set target turn-to pan if current pan and pan of route are too far apart for a pan snap
	// calc 'dist_left'
	// checks for anim CAPS should have already been done before we are called

	PXreal sub1, sub2;

	// cancel any previous
	L->auto_panning = FALSE8;

	// get length
	sub1 = x - route->prim_route[route->current_position].x;
	sub2 = z - route->prim_route[route->current_position].z;

	route->dist_left = (PXreal)PXsqrt((sub1 * sub1) + (sub2 * sub2));

	Calc_target_pan_no_bones(route->prim_route[route->current_position].x, route->prim_route[route->current_position].z, x, z);
}

void _game_session::Start_new_router_game_cycle() {
	// called at start of game cycle - this allows one route to be built during the cycle
	router_busy = FALSE8;
}

void _game_session::Set_router_busy() {
	// a route has been built - so dont allow anymore

	router_busy = TRUE8;
}

bool8 _game_session::Is_router_busy() {
	// has a route been built?

	if (first_session_cycle)
		return FALSE8;

	return router_busy;
}

} // End of namespace ICB
