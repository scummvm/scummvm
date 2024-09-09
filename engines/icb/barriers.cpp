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

#include "engines/icb/p4.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_floor_map.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/common/px_route_barriers.h"
#include "engines/icb/common/px_prop_anims.h"
#include "engines/icb/mission.h"
#include "engines/icb/session.h"
#include "engines/icb/debug.h"
#include "engines/icb/floors.h"
#include "engines/icb/barriers.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/res_man.h"

#include "common/textconsole.h"

namespace ICB {

// how much you get nudge along a nudge barrier per cycle
#define NUDGE_DISTANCE (10)

// How close you have to get to barriers to consider a collision
// Note : the run anim has a delta movement of 30cm hence the 35cm figure
#define BARRIER_CLOSE (20 * FLOAT_ONE)
#define RUN_BARRIER_CLOSE (35 * FLOAT_ONE)

// Only do fort-knox solution on barriers closer than this distance
#define IGNORE_BARRIER_CLOSE (100 * FLOAT_ONE)

// made bigger than barrier close so we hit stairs before atual barriers that surround them
#define STAIR_CLOSE 25 * FLOAT_ONE
#define LADDER_TOP_CLOSE 50 * FLOAT_ONE

// Ignore barriers that are more than this away from players height
#define BARRIER_TOO_HIGH (100 * REAL_ONE)

// above this angle the mega is blocked, below this angle it is aligned
#define BARRIER_TOLERANCE ((FULL_TURN * 70) / 360) // 70 deg

// For Sony testing make these variables for altering at run-time
// in long term they will become constants

// How much you get reflected away from a barrier when you are aligned with it
PXreal REPEL_TURN = ((FULL_TURN * 6) / 360); // 6 deg

// how much you get repulsed away from a barrier per cycle when you
// are being aligned with it
PXreal REPULSE_DISTANCE = (15 * REAL_ONE); // 15 cm

__barrier_result _game_session::Check_barrier_bump_and_bounce(PXreal newx, PXreal newy, PXreal newz, PXreal oldx, PXreal /* oldy */, PXreal oldz, bool8 pl) {
	// see if the coordinates passed are close to the players current barriers
	// returns   0 no barrier found that's too near
	//				1 a barrier was too close

	RouteBarrier *bar;
	PXreal pdist, dist;
	uint32 j;
	PXfloat barrier_tolerance = BARRIER_TOLERANCE; // 1/8 of a turn = 45 degress
	PXfloat diff;
	int32 ignoreThis;

	// 1 = means don't do fort knox solution on this barrier
	// 0 = means do fort knox solution on this barrier
	int32 ignoreBarrier[MAX_barriers];

	PXreal bar_close = BARRIER_CLOSE; // changes if player running or walking

	if (pl) {
		// is player running
		if (Get_motion() == __MOTION_RUN) // running
			bar_close = RUN_BARRIER_CLOSE;

		// check for stair entry complience - oh yes
		for (j = 0; j < num_stairs; j++) {

			bar = &stairs[j].bar;

			// blocked if stair/ladder is disabled
			if (!stairs[j].live)
				continue;

			if (newy != bar->m_bottom)
				continue; // not on our floor so continue with next

			pdist = ((newx * bar->m_bcm.m_lpx) + (newz * bar->m_bcm.m_lpz)) - bar->m_bcm.m_linedist;

			if (((PXfloat)PXfabs(pdist) < STAIR_CLOSE) ||                                                    // stair
			    ((!stairs[j].is_stair) && (!stairs[j].up) && ((PXfloat)PXfabs(pdist) < LADDER_TOP_CLOSE))) { // top of stairs
				// we are near the plane so now we must check the end points
				// check the left end of the line
				dist = ((newx * BarrierCollisionMathsObject::alpx(&bar->m_bcm)) + (newz * BarrierCollisionMathsObject::alpz(&bar->m_bcm))) - bar->m_bcm.m_alinedist;

				// check the right end
				if (dist >= 0) {
					dist = ((newx * BarrierCollisionMathsObject::blpx(&bar->m_bcm)) + (newz * BarrierCollisionMathsObject::blpz(&bar->m_bcm))) - bar->m_bcm.m_blinedist;

					if (dist >= 0) {
						// ok, its a hit
						// we need to crash into the barrier if we're crouched

						if (M->Is_crouched())
							return __BLOCKED;

						// and crash if we're armed
						if (M->Fetch_armed_status())
							return __BLOCKED;

						// must be walking or running upright - so off we go

						diff = L->pan - stairs[j].pan;
						// correct
						if (diff > HALF_TURN)
							diff -= FULL_TURN;
						else if (diff < -HALF_TURN)
							diff += FULL_TURN;

						if (PXfabs(diff) < (FULL_TURN / 10)) { // 36 deg = +/- 18 deg
							L->pan = stairs[j].pan;
							MS->player.stair_num = (uint8)j;           // actual stair index number
							MS->player.stair_unit = 0;                 // units into cycle
							MS->player.stair_dir = stairs[j].up;       // 1 is up, 0 is down
							MS->player.begun_at_bottom = stairs[j].up; // 1 is started at bottom
							MS->player.was_climbing = FALSE8;          // no y movement into the cycle

							if (stairs[j].is_stair) {     // stair or ladder
								M->on_stairs = TRUE8; // for shadow correction system

								// stairs
								if (Get_motion() == __MOTION_RUN) // running
									MS->player.Set_player_status(RUNNING_ON_STAIRS);

								else
									MS->player.Set_player_status(ON_STAIRS);

								if (MS->player.stair_dir)               // going up
									MS->player.step_sample_num = 0; // starting at bottom so sampling starts at 0
								else
									MS->player.step_sample_num = TOP_stair_num; // starting at top

								// set the stairway coordinate correction system
								for (uint32 k = 0; k < MAX_stair_length; k++)
									MS->player.step_samples[k].stepped_on_step = FALSE8;
							} else { // ladder
								MS->player.left_right = 0;
								if (MS->player.stair_dir) { // going up
#define SNAP_BACK_DOWN 70
#define SNAP_BACK_UP 25
									Snap_to_ladder(&stairs[j], SNAP_BACK_UP);
									MS->player.was_climbing = TRUE8; // into anim has movement
									MS->player.Easy_start_new_mode(ON_LADDER, __CORD_STAND_TO_CLIMB_UP_LADDER);
								} else {
									M->drawShadow = FALSE8; // shadows off
									Snap_to_ladder(&stairs[j], SNAP_BACK_DOWN);
									camera_lock = TRUE8; // stop rough room cut through effect
									MS->player.Easy_start_new_mode(BEGIN_DOWN_LADDER, __STAND_TO_CLIMB_DOWN_LADDER_RIGHT);
								}
								return (__NUDGED);
							}
							return (__OK);
						}
					}
				}
			}
		}

		// check for collision with nudge barriers
		for (j = 0; j < M->number_of_nudge; j++) {
			int32 b = Fetch_megas_nudge_barrier_number(j);
			bar = session_barriers->Fetch_barrier(b);

			pdist = ((newx * bar->m_bcm.m_lpx) + (newz * bar->m_bcm.m_lpz)) - bar->m_bcm.m_linedist;
			if ((PXfloat)PXfabs(pdist) < BARRIER_CLOSE) {
				// we are near the plane so now we must check the end points
				// check the left end of the line
				dist = ((newx * BarrierCollisionMathsObject::alpx(&bar->m_bcm)) + (newz * BarrierCollisionMathsObject::alpz(&bar->m_bcm))) - bar->m_bcm.m_alinedist;
				// check the right end
				// Make barrier a bit longer to nudge player through the doorway nicely
				if (dist > -bar_close) {
					dist = ((newx * BarrierCollisionMathsObject::blpx(&bar->m_bcm)) + (newz * BarrierCollisionMathsObject::blpz(&bar->m_bcm))) - bar->m_bcm.m_blinedist;
					// Make barrier a bit longer to nudge player through the doorway nicely
					if (dist > -bar_close) {
						// check angle - narrow ones are ignored

						PXfloat delta = remainder(L->pan - bar->m_pan, FULL_TURN, HALF_TURN);
						PXfloat delta2 = delta;

						if (delta < -QUARTER_TURN)
							delta2 += HALF_TURN;
						if (delta > QUARTER_TURN)
							delta2 -= HALF_TURN;
						PXfloat fd = (PXfloat)PXfabs(delta2);
						if (fd >= barrier_tolerance) {
							// ok, we are close to the barrier and at an acceptable angle - now nudge along
							// work out pan of barrier

							// we have our coordinate and a direction to shift in
							PXfloat ang = bar->m_pan * TWO_PI;

							PXfloat cang = (PXfloat)PXcos(ang);
							PXfloat sang = (PXfloat)PXsin(ang);

							// Let's ignore the zero*something as it is always zero
							M->actor_xyz.x += PXfloat2PXreal(NUDGE_DISTANCE * REAL_ONE * sang);
							M->actor_xyz.z += PXfloat2PXreal(NUDGE_DISTANCE * REAL_ONE * cang);

							return (__NUDGED); // we've avoided the hit
						}
					}
				}
			}
		} // nudge for loop
	}         // if player

	adjusted_pan = FLOAT_ZERO;
	made_adjust = FALSE8;
	normalAngle = 3 * FULL_TURN; // place holder to mean not set
	int32 nFortKnox = 0;
	uint32 nBarriers = (M->number_of_barriers + M->number_of_animating);

	for (j = 0; j < nBarriers; j++) {
		int32 b = Fetch_megas_barrier_number(j);
		bar = session_barriers->Fetch_barrier(b);
		ignoreBarrier[j] = 1;

		if ((PXfloat)PXfabs(newy - bar->m_bottom) > BARRIER_TOO_HIGH)
			continue; // ignore abars that are now too high

		__barrier_result result = Check_this_barrier(bar, newx, newz, oldx, oldz, bar_close, &ignoreThis);

		// Ignore barrier culling isn't working - so do them all except animating barriers
		ignoreThis = 0;

		ignoreBarrier[j] = ignoreThis;
		if (ignoreThis == 0)
			nFortKnox++;

		if (result != __OK)
			return (result);
	}
	if ((!made_adjust) && (MS->player.player_status != RUNNING) && (MS->player.player_status != WALKING)) {
		// didn't hit a normal barrier so check if we hit a stair or ladder barrier

		// check for stair entry complience - oh yes
		for (j = 0; j < num_stairs; j++) {
			bar = &stairs[j].bar;

			if (newy != bar->m_bottom)
				continue; // not on our floor so continue with next

			__barrier_result result = Check_this_barrier(bar, newx, newz, oldx, oldz, bar_close, &ignoreThis);
			if (result != __OK)
				return (result);
		}
	}

	// if we hit a single corectable barrier then we can make that adjustment now
	int32 repulsed = 0;
	PXfloat destx = FLOAT_ZERO;
	PXfloat destz = FLOAT_ZERO;

	if (made_adjust) {
		L->pan = adjusted_pan;

		if (normalAngle < 2 * FULL_TURN) {
			// Repulse the mega back a bit
			PXfloat ang = normalAngle * TWO_PI;
			PXfloat cang = (PXfloat)PXcos(ang);
			PXfloat sang = (PXfloat)PXsin(ang);

			// Let's ignore the zero*something as it is always zero
			// the stored angle normalAngle is a normal angle pointing away
			// from the barrier by 90 deg AND towards the players side of the
			// barrier
			repulsed = 1;
			destx = M->actor_xyz.x + PXfloat2PXreal(REPULSE_DISTANCE * REAL_ONE * sang);
			destz = M->actor_xyz.z + PXfloat2PXreal(REPULSE_DISTANCE * REAL_ONE * cang);
		}
	} else {
		destx = newx;
		destz = newz;
	}

	// Right so finally do a line intersection between the old position and the "final" new position
	// old position is : oldx, oldz
	// new position is : destx, destz
	int32 hit = 0;

	// Special treatment for the player
	if (pl) {
		for (j = 0; j < nBarriers; j++) {
			int32 b = Fetch_megas_barrier_number(j);
			bar = session_barriers->Fetch_barrier(b);

			// Ignore barriers which are in the ignore list
			if (ignoreBarrier[j] == 1)
				continue;

			hit = troute.Get_intersect(oldx, oldz, destx, destz, bar->m_x1, bar->m_z1, bar->m_x2, bar->m_z2);

			if (hit == 1) {
				warning("Player crossed the line nBars %d nFortKnox %d : player %f %f -> %f %f bar: %f %f -> %f %f", nBarriers, nFortKnox, oldx, oldz, destx, destz,
						bar->m_x1, bar->m_z1, bar->m_x2, bar->m_z2);
				break;
			}
		}
		// Oh dear we went through a barrier
		if (hit == 1)
			return (__BLOCKED); // conflict, so finish
	}

	// The repulsed position looks good - so use it !
	if (repulsed) {
		M->actor_xyz.x = destx;
		M->actor_xyz.z = destz;
	}

	// Return the correct values
	if (made_adjust)
		return (__CORRECTED);

	// finally, if player is still looking good then do a check for him hitting other megas
	if (pl) {
		/*
		                                           /
		                              mega        /
		                position : mx, mz        /
		                              *         /
		                                       /
		                                      /
		                                     /
		                                    /
		                                   /
		                                  /
		                                 /
		                                /
		                               / - his pan direction
		                    #
		player @ position : px, pz direction: pan

		dx = PXsin( pan )
		dz = PXcos( pan )

		then the mega is on the left-hand side of the line (or on the line)
		if ( (dz * ( mx - px ))  <= ( dx * ( mz - pz )) )
		else
		// he is on the right-hand side of the line
		*/

		static int32 total_adjusts = 0;

		if ((!total_adjusts) && (MS->player.interact_selected) && (logic_structs[MS->player.cur_interact_id]->image_type == VOXEL)) {
			// player is highlighting a mega

			// check nearness

			PXreal sub1, sub2;

			sub1 = logic_structs[MS->player.cur_interact_id]->mega->actor_xyz.x - M->actor_xyz.x;
			sub2 = logic_structs[MS->player.cur_interact_id]->mega->actor_xyz.z - M->actor_xyz.z;

			//  dist
			PXreal distance = ((sub1 * sub1) + (sub2 * sub2));

			// near
			if (distance < (120 * 120)) {

				if ((MS->player.cur_state.momentum != __FORWARD_1) && (MS->player.cur_state.momentum != __FORWARD_2))
					return __OK;

				PXreal dx = (PXreal)PXsin(L->pan * TWO_PI);
				PXreal dz = (PXreal)PXcos(L->pan * TWO_PI);

				PXreal mx = logic_structs[MS->player.cur_interact_id]->mega->actor_xyz.x;
				PXreal mz = logic_structs[MS->player.cur_interact_id]->mega->actor_xyz.z;

				if ((dz * (mx - M->actor_xyz.x)) <= (dx * (mz - M->actor_xyz.z))) {
					// right
					if ((distance > (50 * 50)) && (distance < (120 * 120)))
						L->pan += 0.03f;
				} else {
					// left
					if ((distance > (50 * 50)) && (distance < (120 * 120)))
						L->pan -= 0.03f;
				}

				total_adjusts++;

				return (__OK);
			}
		}
		total_adjusts = 0; // reset
	}
	return (__OK);
}

__barrier_result _game_session::Check_this_barrier(RouteBarrier *bar, PXreal newx, PXreal newz, PXreal /* oldx */, PXreal /* oldz */, PXreal bar_close, int32 *ignoreThis) {
	PXfloat delta;
	PXfloat delta2;
	PXfloat barrier_tolerance = BARRIER_TOLERANCE; // 1/8 of a turn = 45 degress
	PXreal pdist, dist;
	PXreal ignore_bar_close = IGNORE_BARRIER_CLOSE;

	*ignoreThis = 1;

	pdist = ((newx * bar->m_bcm.m_lpx) + (newz * bar->m_bcm.m_lpz)) - bar->m_bcm.m_linedist;

	if ((PXfloat)PXfabs(pdist) < bar_close) {
		// we are near the plane so now we must check the end points
		// check the left end of the line

		dist = ((newx * BarrierCollisionMathsObject::alpx(&bar->m_bcm)) + (newz * BarrierCollisionMathsObject::alpz(&bar->m_bcm))) - bar->m_bcm.m_alinedist;

		// check the right end
		if (dist >= 0) {
			dist = ((newx * BarrierCollisionMathsObject::blpx(&bar->m_bcm)) + (newz * BarrierCollisionMathsObject::blpz(&bar->m_bcm))) - bar->m_bcm.m_blinedist;

			if (dist >= 0) {
				*ignoreThis = 0;

				// we are going to hit this barrier
				// but, if the angle is narrow we can acquire the barriers pan and continue unmolested
				delta = remainder(L->pan - bar->m_pan, FULL_TURN, HALF_TURN);
				delta2 = delta;

				if (delta < -QUARTER_TURN)
					delta2 += HALF_TURN;
				if (delta > QUARTER_TURN)
					delta2 -= HALF_TURN;
				if (PXfabs(delta2) < barrier_tolerance) {
					if (made_adjust)
						return (__BLOCKED); // conflict, so finish

					if ((delta > QUARTER_TURN) || (delta < -QUARTER_TURN)) {
						adjusted_pan = remainder(bar->m_pan + HALF_TURN, FULL_TURN, HALF_TURN);
					} else {
						adjusted_pan = bar->m_pan;
					}
					made_adjust = TRUE8;
					if (adjusted_pan > L->pan) {
						adjusted_pan += REPEL_TURN;
					} else if (adjusted_pan < L->pan) {
						adjusted_pan -= REPEL_TURN;
					}
					if (pdist > 0)
						normalAngle = bar->m_pan + QUARTER_TURN;
					else if (pdist < 0)
						normalAngle = bar->m_pan - QUARTER_TURN;
				} else {
					// can't adjust
					return (__BLOCKED);
				}
			} else
				*ignoreThis = 1;
		} else
			*ignoreThis = 1;
	} else if ((PXfloat)PXfabs(pdist) < ignore_bar_close) {
		*ignoreThis = 0;
	}

	return __OK;
}

void _barrier_handler::___init() {
	RoutingSlice *slice;
	uint32 *num_bars;
	uint32 len;

	Zdebug("_barrier_handler");
	Zdebug("\n+init _barrier_handler %s", MS->Fetch_session_name());

	// load the raw barrier file for this session
	// When clustered the session files have the base stripped
	len = Common::sprintf_s(temp_buf, "%s", PX_FILENAME_BARRIERLIST);
	if (len > ENGINE_STRING_LEN)
		Fatal_error("_barrier_handler::___init string len error");

	Tdebug("barriers.txt", "%s", (const char *)temp_buf);
	uint32 buf_hash = NULL_HASH;
	uint32 cluster_hash = MS->Fetch_session_cluster_hash();
	raw_barriers = (LinkedDataFile *)private_session_resman->Res_open(temp_buf, buf_hash, MS->Fetch_session_cluster(), cluster_hash);

	num_bars = (uint32 *)LinkedDataObject::Fetch_item_by_name(raw_barriers, "Count");

	total_barriers = READ_LE_U32(num_bars);

	Tdebug("barriers.txt", "%d raw barriers", total_barriers);

	// load in the routing wrapper
	// When clustered the session files have the base stripped
	len = Common::sprintf_s(temp_buf, "%s", PX_FILENAME_ROUTING);
	if (len > ENGINE_STRING_LEN)
		Fatal_error("_barrier_handler::___init string len error");

	Tdebug("barriers.txt", "%s", temp_buf);
	buf_hash = NULL_HASH;
	route_wrapper = (LinkedDataFile *)private_session_resman->Res_open(temp_buf, buf_hash, MS->Fetch_session_cluster(), cluster_hash);

	total_slices = LinkedDataObject::Fetch_number_of_items(route_wrapper);

	if (total_slices > MAX_slices)
		Fatal_error("_barrier_handler::___init finds too many slices - %d but only %d allowed", total_slices, MAX_slices);

	Tdebug("slice.txt", "%d routing levels", total_slices);

	if (!total_slices) {
		Zdebug("[%s]", (const char *)temp_buf);
		Fatal_error("no parent routing levels (no parent boxes) engine cannot proceed");
	}

	uint32 j;
	for (j = 0; j < total_slices; j++) {
		slice = (RoutingSlice *)LinkedDataObject::Fetch_item_by_number(route_wrapper, j);
		Tdebug("slice.txt", "bottom %3.1f top %3.1f", FROM_LE_FLOAT32(slice->bottom), FROM_LE_FLOAT32(slice->top));
		Tdebug("slice.txt", "%d parents", FROM_LE_32(slice->num_parent_boxes));
	}

	// reset prop list for each
	// fully reset abar systems
	for (j = 0; j < MAX_slices; j++) {
		anim_slices[j].num_props_in_slice = 0;
		for (uint32 l = 0; l < MAX_parents_per_anim_slice; l++)
			anim_slices[j].anim_parents[l] = nullptr; // unasigned pointer
	}
	for (j = 0; j < MAX_props; j++) {
		anim_prop_info[j].barriers_per_state = 0;
		anim_prop_info[j].total_states = 0;
	}
	for (j = 0; j < MAX_floors; j++) // reset the unassigned parents
		anim_parent_table[j].num_props = 0;
	parents_used = 0; // no parents have been assigned

	Zdebug("anim bars");

	Prepare_animating_barriers();

	Zdebug("done barriers");
}

#define ADD_CHILD                                                                                                                                                                  \
	if (clist[j]->num_barriers)                                                                                                                                                \
		for (k = 0; k < clist[j]->num_barriers; k++) {                                                                                                                     \
			bar = Fetch_barrier(clist[j]->barriers[k]);                                                                                                                \
			if (bar->m_bottom == y) {                                                                                                                                  \
				if (barrier_mask) {                                                                                                                                \
					if (MS->troute.LineIntersectsRect(mask, (int32)bar->m_x1, (int32)bar->m_z1, (int32)bar->m_x2, (int32)bar->m_z2))                           \
						MS->troute.Add_barrier(bar);                                                                                                       \
				} else                                                                                                                                             \
					MS->troute.Add_barrier(bar);                                                                                                               \
			}                                                                                                                                                          \
		}

#define EXPAND_ROUTE_BOX                                                                                                                                                           \
	if (CHILDL < RBL)                                                                                                                                                          \
		RBL = CHILDL;                                                                                                                                                      \
	if (CHILDR > RBR)                                                                                                                                                          \
		RBR = CHILDR;                                                                                                                                                      \
	if (CHILDT < RBT)                                                                                                                                                          \
		RBT = CHILDT;                                                                                                                                                      \
	if (CHILDB > RBB)                                                                                                                                                          \
		RBB = CHILDB;                                                                                                                                                      \
	expanded_this_go++;

#define CHILDL clist[j]->left
#define CHILDR clist[j]->right
#define CHILDT clist[j]->back
#define CHILDB clist[j]->front
#define RBL rb.x1
#define RBR rb.x2
#define RBT rb.z1
#define RBB rb.z2

void _barrier_handler::Form_parent_barrier_list(PXreal x, PXreal y, PXreal z) {
	// we are routing into a room - just get the parent barriers
	ParentBox *endb;
	uint32 parent_a, slice_a, k;
	RouteBarrier *bar;
	uint32 *array;

	endb = Fetch_parent_box_for_xyz(x, y, z, parent_a, slice_a);
	if (!endb)
		return; // will be because of between floor gap - not to worry

	if (endb->num_barriers) {
		array = (uint32 *)(((char *)endb) + (endb->barriers));
		for (k = 0; k < endb->num_barriers; k++) {
			bar = Fetch_barrier(array[k]);
			MS->troute.Add_barrier(bar);
		}
	}
}

void _barrier_handler::Form_route_barrier_list(PXreal x, PXreal y, PXreal z, PXreal x2, PXreal z2) {
	// add all barriers to the prim route building system for the route x,z,x2,z2
	// after this the route_manager can call the prim route builder for the final route to be made
	// the restriction is that we can only route from one place to another within a single floor-rect/parent-box
	// or
	// from within one parent-box/floor-rect to an adjacent parent-box/floor-rect
	// it is a higher level job to divide long routes up into rect to rect chunks before calling here
	// but this is quite sensible anyway as we only want to be auto-routing across tiny areas at a time

	ParentBox *startb;
	ParentBox *endb;
	_rect rb; // rb meaning 'Route-Box'
	uint32 j;
	ChildGroup *clist[MAX_child_groups_per_parent * 2];
	uint32 total_childs = 0; // separate total for safety
	int32 expanded_this_go;
	RouteBarrier *bar;
	uint32 k;
	uint32 parent_a, parent_b;
	uint32 slice_a, slice_b;

	// which floors are the start and end positions in?
	// list child boxes for each floor - ignoring the parent box concept?
	// build route
	// add parent boundaries for all parents that the final route box intersects - !?!

	// find the parent box
	// first, find _parent_box for start point and end points

	startb = Fetch_parent_box_for_xyz(x, y, z, parent_a, slice_a);
	endb = Fetch_parent_box_for_xyz(x2, y, z2, parent_b, slice_b);

	if ((!startb) && (endb))
		startb = endb; // no start but end
	if ((!endb) && (startb))
		endb = startb; // no end but start

	if (!startb)
		Fatal_error("_barrier_handler::Form_route_barrier_list start and end not on floor - %s", MS->Fetch_object_name(MS->Fetch_cur_id()));
	// make a list of pointers to the child groups

	if (startb != endb) { // oh dear - not going to allow this anymore
		Form_parent_barrier_list(x2, y, z2);
		Form_parent_barrier_list(x, y, z);
		return;
	}

	Zdebug("%3.1f %3.1f %3.1f %3.1f", startb->back, startb->left, startb->front, startb->right);

	if (startb->num_childgroups) {
		for (j = 0; j < startb->num_childgroups; j++) {
			if (total_childs == (MAX_child_groups_per_parent * 2))
				Fatal_error("_barrier_handler::Form_route_barrier_list - clist ran out of space");
			clist[total_childs++] = Fetch_child_box(startb, j);
		}
	}

	// we now know the _parent_box that our start point is on - we therefore have the list of child boxes
	// same or different?
	if (startb != endb) { // different
		Zdebug("different parent for end box");
		if (endb->num_childgroups) {
			Zdebug("adding %d child boxes for end point", endb->num_childgroups);
			for (j = 0; j < endb->num_childgroups; j++) {
				if (total_childs == (MAX_child_groups_per_parent * 2))
					Fatal_error("_barrier_handler::Form_route_barrier_list - clist ran out of space");
				clist[total_childs++] = Fetch_child_box(endb, j);
			}
		}
	}

	// ok, we now have a list of pointers to all the child groups relevant

	// create the initial route box
	if (x < x2) {
		rb.x1 = x;
		rb.x2 = x2;
	} else {
		rb.x1 = x2;
		rb.x2 = x;
	}
	if (z < z2) {
		rb.z1 = z;
		rb.z2 = z2;
	} else {
		rb.z1 = z2;
		rb.z2 = z;
	}

	// we now have our initial route box

	// enter our main loop
	do {
		expanded_this_go = 0; // flag to say whether or no the route-box got expanded this loop

		for (j = 0; j < total_childs; j++) {
			if (clist[j]) {
				// child not deleted

				if (((CHILDR < RBL) || (CHILDL > RBR)) && ((CHILDB < RBT) || (CHILDT > RBB))) {
					// child is wholly outside the route box - do nothing except prime the ELSE
				} else if (((CHILDR >= RBL) && (CHILDL <= RBR)) && ((CHILDB >= RBT) && (CHILDT <= RBB))) {
					// child is wholly inside route box - add it and delete child
					ADD_CHILD
					clist[j] = nullptr; // delete the child now it has been absorbed
				} else if (((RBR >= CHILDL) && (RBL <= CHILDR)) && ((RBT >= CHILDT) && (RBB <= CHILDB))) {
					// route box is wholly inside child box - expand route box, add and delete child
					ADD_CHILD
					clist[j] = nullptr; // delete the child now it has been absorbed
					EXPAND_ROUTE_BOX
				} else {
					if ((CHILDL > RBL) && (CHILDL < RBR) && ((CHILDT > RBT) && (CHILDT < RBB))) {
						// child top/left is within route box - expand route box, add and delete child
						ADD_CHILD
						clist[j] = nullptr; // delete the child now it has been absorbed
						EXPAND_ROUTE_BOX
					} else if ((CHILDR > RBL) && (CHILDR < RBR) && ((CHILDT > RBT) && (CHILDT < RBB))) {
						// child top/right is within route box - expand route box, add and delete child
						ADD_CHILD
						clist[j] = nullptr; // delete the child now it has been absorbed
						EXPAND_ROUTE_BOX
					} else if ((CHILDL > RBL) && (CHILDL < RBR) && ((CHILDB > RBT) && (CHILDB < RBB))) {
						// child bottom/left is within route box - expand route box, add and delete child
						ADD_CHILD
						clist[j] = nullptr; // delete the child now it has been absorbed
						EXPAND_ROUTE_BOX
					} else if ((CHILDR > RBL) && (CHILDR < RBR) && ((CHILDB > RBT) && (CHILDB < RBB))) {
						// child bottom/right is within route box - expand route box, add and delete child
						ADD_CHILD
						clist[j] = nullptr; // delete the child now it has been absorbed
						EXPAND_ROUTE_BOX
					} else {
						// we must check for our route line intersecting a horizontal and vertical child box edge
					}
				}
			}
		}

		// if this loop we did not expand the route_box then we quit this DO-WHILE loop
		if (!expanded_this_go)
			break;
	} while (1);

	// if there are two parent boxes we must add the parent barriers from each to the list
	if (startb != endb) {
		// add both parent box barriers in
		uint32 *array;

		if (startb->num_barriers) {
			array = (uint32 *)(((char *)startb) + (startb->barriers));
			for (k = 0; k < startb->num_barriers; k++) {
				bar = Fetch_barrier(array[k]);
				MS->troute.Add_barrier(bar);
			}
		}
		if (endb->num_barriers) {
			array = (uint32 *)(((char *)endb) + (endb->barriers));
			for (k = 0; k < endb->num_barriers; k++) {
				bar = Fetch_barrier(array[k]);
				MS->troute.Add_barrier(bar);
			}
		}
	} else { // route lies within a single floor - so draw a box around it
		RouteBarrier newbar;

		// left hand barrier
		newbar.m_x1 = startb->left;
		newbar.m_z1 = startb->back;
		newbar.m_x2 = startb->left;
		newbar.m_z2 = startb->front;
		MS->troute.Add_barrier(&newbar);

		// right hand barrier
		newbar.m_x1 = startb->right;
		newbar.m_z1 = startb->back;
		newbar.m_x2 = startb->right;
		newbar.m_z2 = startb->front;
		MS->troute.Add_barrier(&newbar);

		// top barrier
		newbar.m_x1 = startb->left;
		newbar.m_z1 = startb->back;
		newbar.m_x2 = startb->right;
		newbar.m_z2 = startb->back;
		MS->troute.Add_barrier(&newbar);

		// bottom barrier
		newbar.m_x1 = startb->left;
		newbar.m_z1 = startb->front;
		newbar.m_x2 = startb->right;
		newbar.m_z2 = startb->front;
		MS->troute.Add_barrier(&newbar);
	}
}

ParentBox *_barrier_handler::Fetch_parent_num_on_slice_y(uint32 requested_parent, PXreal y) {
	// fetch the parent of the number passed for a given y level
	// ie 0 means first, 1 means second, etc
	// this is called by the plan-viewer which just keeps asking for the next one until we say there are no more
	// by passing back a 0 instead of a pointer to a parent
	static RoutingSlice *slice;
	uint32 cur_slice = 0;

	// first time in so compute the slice
	if (!requested_parent) {
		while (1) {
			slice = (RoutingSlice *)LinkedDataObject::Fetch_item_by_number(route_wrapper, cur_slice);

			if ((y >= slice->bottom) && (y < slice->top))
				break;

			// safety
			cur_slice++;
			if (cur_slice == total_slices) // if so then must be last slice :O
				Fatal_error("Fetch_parent_num_on_slice_y ran out of slices");

			// next
			slice++;
		}
	}

	// ok, we have the slice
	// return the parent of the requested number - or, 0 if there is no more

	// reached total?
	if (requested_parent == slice->num_parent_boxes)
		return (nullptr);

	// simply return the pointer

	return ((ParentBox *)(((uint8 *)slice) + slice->parent_boxes[requested_parent]));
}

RouteBarrier *_barrier_handler::Fetch_barrier(uint32 num) {
	// return a pointer to numbered barrier
	RouteBarrier *bar;

	assert(num < total_barriers);

	if (num >= total_barriers)
		Fatal_error("illegal barrier request %d", num);

	bar = (RouteBarrier *)LinkedDataObject::Fetch_item_by_name(raw_barriers, "Data");

	return &bar[num];
}

ParentBox *_barrier_handler::Fetch_parent_box_for_xyz(PXreal x, PXreal y, PXreal z, uint32 &par_num, uint32 &slice_num) {
	// return a pointer to the parent box of a point in world space
	// returns 0 if the point does not lie within a parent box

	RoutingSlice *slice = nullptr;
	ParentBox *parent = nullptr;

	// find correct slice according to height
	// fetch first

	slice_num = 0;

	while (1) {
		slice = (RoutingSlice *)LinkedDataObject::Fetch_item_by_number(route_wrapper, slice_num);

		if ((y >= slice->bottom) && (y < slice->top))
			break;

		// safety
		slice_num++;
		if (slice_num == total_slices) { // if so then must be last slice :O
			Fatal_error("_barrier_handler::Fetch_parent_box_for_xyz ran out of slices: object [%s] (%3.1f %3.1f %3.1f) has an "
			            "illegal marker",
			            MS->Fetch_object_name(MS->Fetch_cur_id()), x, y, z);
		}
		// next
		slice++;
	}

	// ok, we found the right y slice
	// now find the right parent box
	if (!slice->num_parent_boxes)
		Fatal_error("_barrier_handler::Fetch_parent_box_for_xyz slice has no parent boxes");

	for (par_num = 0; par_num < slice->num_parent_boxes; par_num++) {
		parent = (ParentBox *)(((uint8 *)slice) + slice->parent_boxes[par_num]);

		// do we lie within the box?
		if ((x > parent->left) && (x < parent->right) && (z > parent->back) && (z < parent->front)) {
			return (parent);
		}
	}

	return (nullptr);
}

void _game_session::Prepare_megas_route_barriers(bool8 pl) {
	// see which parent box we're owned by
	// if different from previous fetch all the barriers for the new parent
	// this system is custom for the player object - routing megas use their own system
	// this routine fecthes the 'special' player only line-of-sight barriers too

	ParentBox *par = nullptr;
	ChildGroup *pchild;
	uint32 total_childs;
	uint32 j, k;
	uint32 *list;
	RouteBarrier *bar;
	uint32 parent_number;
	RoutingSlice *slice;
	PXreal x, y, z;

	x = M->actor_xyz.x;
	y = floor_def->Return_true_y(M->actor_xyz.y);
	z = M->actor_xyz.z;

	// on previous slice?
	slice = (RoutingSlice *)LinkedDataObject::Fetch_item_by_number(session_barriers->route_wrapper, M->cur_slice);
	if ((y >= slice->bottom) && (y < slice->top) && (M->cur_parent))
		if ((x > M->cur_parent->left) && (x < M->cur_parent->right) && (z > M->cur_parent->back) && (z < M->cur_parent->front)) {
			// nothing has changed
			Prepare_megas_abarriers(M->cur_slice, M->par_number);

			// player should have added the barriers of stood megas
			if (pl)
				Fetch_mega_barriers_for_player();

			return;
		}

	M->cur_slice = 0;
	while (1) {
		slice = (RoutingSlice *)LinkedDataObject::Fetch_item_by_number(session_barriers->route_wrapper, M->cur_slice);
		if ((y >= slice->bottom) && (y < slice->top))
			break;

		// safety
		M->cur_slice++;
		if (M->cur_slice == session_barriers->total_slices) { // if so then must be last slice :O
			M->cur_slice--;
			slice = (RoutingSlice *)LinkedDataObject::Fetch_item_by_number(session_barriers->route_wrapper, M->cur_slice);
			break;
		}
	}
	// ok, we found the right y slice
	// now find the right parent box
	if (!slice->num_parent_boxes)
		Fatal_error("Prepare_megas_route_barriers slice has no parent boxes");

	for (parent_number = 0; parent_number < slice->num_parent_boxes; parent_number++) {
		par = (ParentBox *)(((uint8 *)slice) + slice->parent_boxes[parent_number]);

		// do we lie within the box?
		if ((x > par->left) && (x < par->right) && (z > par->back) && (z < par->front)) {
			break; // found
		}
	}

	if (parent_number == slice->num_parent_boxes) {
		// not on a legal position - can happen
		M->cur_parent = nullptr; // null pointer
		M->number_of_barriers = 0;
		M->number_of_nudge = 0;
		M->number_of_animating = 0;
		return;
	}

	// has a parent box different from the current one been found?
	{
		// new one
		M->cur_parent = par;
		M->par_number = parent_number;

		// reset list of barrier ids
		M->number_of_barriers = 0;
		M->number_of_animating = 0;

		// firstly, drag out the parents bounding barriers if it has any
		if (par->num_barriers) {
			list = (uint32 *)((uint8 *)par + par->barriers);
			// get all the barriers
			for (j = 0; j < par->num_barriers; j++) {
				// fetch each barrier and check that its bottom edge is on the floor - otherwise we ignore it
				bar = session_barriers->Fetch_barrier(*(list));
				if (bar->m_bottom == slice->bottom) { // M->actor_xyz.y)
					// ok, this barrier is on the floor so we add it to our list
					M->barrier_list[M->number_of_barriers++] = *(list++);
				}
			}
		}

		if (pl)
			M->number_of_nudge = 0; // reset now - in case there are none

		// now player only specials
		if ((pl) && (par->num_specials)) {
#ifdef VERBOSE
			Zdebug("getting player specials");
#endif

			list = (uint32 *)((uint8 *)par + par->specials);
			// get all the barriers
			for (j = 0; j < par->num_specials; j++) {
				// fetch each barrier and check that its bottom edge is on the floor - otherwise we ignore it
				bar = session_barriers->Fetch_barrier(*(list));

				if (bar->m_material == VIEW_FIELD) {
					if (bar->m_bottom == slice->bottom) { // M->actor_xyz.y)
						//                  ok, this barrier is on the floor so we add it to our list
						M->barrier_list[M->number_of_barriers++] = *(list++);
					}
				} else if (bar->m_material >= LEFT_NUDGE) {
					if (bar->m_bottom == slice->bottom) { // M->actor_xyz.y)
						//                  ok, this barrier is on the floor so we add it to our list
						M->nudge_list[M->number_of_nudge++] = *(list++);
					}
				} else {
					Fatal_error("illegal barrier [%d], special list - type %d, x1 %3.2f, x2 %3.2f, z1 %3.2f, z2 %3.2f", *(list), bar->m_material, bar->m_x1,
					            bar->m_x2, bar->m_z1, bar->m_z2);
				}
			}
		}

		// find out have many child boxes this parent has
		total_childs = session_barriers->Fetch_number_of_child_boxes(par);

		for (j = 0; j < total_childs; j++) {
			pchild = session_barriers->Fetch_child_box(par, j);

			for (k = 0; k < pchild->num_barriers; k++) {
				bar = session_barriers->Fetch_barrier(pchild->barriers[k]);
				if (bar->m_bottom == slice->bottom) { // M->actor_xyz.y)
					M->barrier_list[M->number_of_barriers++] = pchild->barriers[k];
				}
			}
		}
	}

	// now build the animating barrier list - regardless of whether or not the parent has changed
	Prepare_megas_abarriers(M->cur_slice, M->par_number);

	if (M->number_of_barriers + M->number_of_animating > MAX_bars)
		Fatal_error("[%s] finds too many barriers - found %d + %d animating, total max %d", CGameObject::GetName(object), M->number_of_barriers, M->number_of_animating, MAX_bars);

	if (M->number_of_nudge > MAX_bars)
		Fatal_error("too many player nudge barriers");
}

void _barrier_handler::Prepare_animating_barriers() {
	// the nightmare that is animating barriers
	// search through all prop animations getting out the animating barriers - checking for duplicates

#define MAX_anim_barriers 400
	uint32 j;
	uint16 barrier_table[MAX_anim_barriers];
	uint32 total_anim_bars = 0;
	RouteBarrier *bar;
	RoutingSlice *slice;
	ParentBox *parent;
	uint32 cur_slice = 0;
	uint32 l, f, pbar_num;
	uint32 abar_index = 0;

	Tdebug("anim_barriers.txt", "Preparing animating barriers");

	for (j = 0; j < LinkedDataObject::Fetch_number_of_items(MS->prop_anims); j++) {
		Tdebug("anim_barriers.txt", "\n%d %s", j, LinkedDataObject::Fetch_items_name_by_number(MS->prop_anims, j));

		_animating_prop *index;
		_animation_entry *anim;

		index = (_animating_prop *)LinkedDataObject::Fetch_item_by_number(MS->prop_anims, j);

		Tdebug("anim_barriers.txt", " has %d anims", index->num_anims);

		// loop through all looking for our named anim
		if (index->num_anims) {
			// get the prop number

			for (uint32 k = 0; k < index->num_anims; k++) {
				anim = (_animation_entry *)(((char *)index) + index->anims[k]);
				Tdebug("anim_barriers.txt", "  '%s' - %d frames", (((char *)index) + anim->name), anim->num_frames);

				if (anim->num_barriers_per_frame) {
					Tdebug("anim_barriers.txt", "   has %d anim barriers per frame", anim->num_barriers_per_frame);
					uint16 *bars = (uint16 *)(((char *)index) + anim->offset_barriers);

					for (uint32 i = 0; i < (uint32)(anim->num_barriers_per_frame * anim->num_frames); i++) {
						if (bars[i] != 0xffff) {
							// check each barrier for duplication and add in to correct slice, parent list

							for (l = 0; l < total_anim_bars; l++) {
								if (barrier_table[l] == bars[i]) {
									Tdebug("anim_barriers.txt", "    %d in list already - index %d", bars[i], i);
									break; // found this bar
								}
							}
							// i is barrier index

							bar = Fetch_barrier(bars[i]);

							if (l == total_anim_bars) { // didn't find in list
								Tdebug("anim_barriers.txt", "     new barrier %d  x%3.2f y%3.2f z%3.2f", bars[i], bar->m_x1, bar->m_bottom,
								       bar->m_z1);

								barrier_table[total_anim_bars++] = (uint16)bars[i]; // write the bar down

								if (total_anim_bars == MAX_anim_barriers)
									Fatal_error("Prepare_animating_barriers finds too many barriers "
									            "for scratch table");
							}
							cur_slice = 0;

							do {
								slice = (RoutingSlice *)LinkedDataObject::Fetch_item_by_number(route_wrapper, cur_slice);

								if ((bar->m_bottom >= slice->bottom) && (bar->m_bottom < slice->top))
									break;

								cur_slice++;
							} while (cur_slice != total_slices);

							if (cur_slice == total_slices) { // if so then must be last slice :O
								Tdebug("anim_barriers.txt", "      **ran out of slices :O\n");
							} else {
								Tdebug("anim_barriers.txt", "      exists on slice %d", cur_slice);

								// check legality of number of parents
								if (slice->num_parent_boxes > MAX_parents_per_anim_slice)
									Fatal_error("prepare anim barriers finds too many parents in slice "
									            "- %d parents",
									            slice->num_parent_boxes);

								int32 prop_number = LinkedDataObject::Fetch_item_number_by_name(MS->prop_anims, (const char *)LinkedDataObject::Fetch_items_name_by_number(MS->prop_anims, j));

								if (prop_number == -1) {
									Tdebug("anim_barriers.txt", "       !!associated prop [%s] not a game object - so ignoring",
									       (const char *)LinkedDataObject::Fetch_items_name_by_number(MS->prop_anims, j));
								} else {
									// now find parent the barrier would belong to
									for (f = 0; f < slice->num_parent_boxes; f++) {
										Tdebug("anim_barriers.txt", "       check parent %d", f);

										parent = (ParentBox *)(((uint8 *)slice) + slice->parent_boxes[f]);

										// do we lie within the box?
										if ((bar->m_x1 > parent->left) && (bar->m_x1 < parent->right) && (bar->m_z1 > parent->back) &&
										    (bar->m_z1 < parent->front)) {
											char *props_name = (char *)LinkedDataObject::Fetch_items_name_by_number(MS->prop_anims, j);
											uint32 props_number = LinkedDataObject::Fetch_item_number_by_name(MS->objects, props_name);

											if (!anim_slices[cur_slice].anim_parents[f]) {
												anim_slices[cur_slice].anim_parents[f] = &anim_parent_table[parents_used++];
												Tdebug("anim_barriers.txt", "        new parent");
											}

											pbar_num = anim_slices[cur_slice].anim_parents[f]->num_props;

											uint32 n;
											for (n = 0; n < pbar_num; n++)
												if (anim_slices[cur_slice].anim_parents[f]->prop_number[n] == props_number)
													break;

											if (n == pbar_num) { // new
												anim_slices[cur_slice].anim_parents[f]->num_props++;

												if (pbar_num == MAX_props_per_parent)
													Fatal_error("too many props in "
													            "parent - max = %d",
													            MAX_props_per_parent);

												Tdebug("anim_barriers.txt", "        found - slice %d, parent %d "
												                            "[%d bars so far], prop [%s, %d] state "
												                            "%d\n",
												       cur_slice, f, anim_slices[cur_slice].anim_parents[f]->num_props, props_name,
												       props_number, anim->frames[i / anim->num_barriers_per_frame]);

												anim_slices[cur_slice].anim_parents[f]->prop_number[pbar_num] = (uint8)props_number;
											}
											break;
										}
									}
									if (f == slice->num_parent_boxes)
										Tdebug("anim_barriers.txt", "       !!barrier not located within a parent box!!\n");
								}
							}
						} else
							Tdebug("anim_barriers.txt", "     !!barrier is type 0xffffffff");
					}
				} else
					Tdebug("anim_barriers.txt", "   0 anim bars");
			}
		}
	}

	// now reverse engineer the target lists
	for (j = 0; j < total_slices; j++) {
		Tdebug("anim_barriers.txt", " slice %d", j);

		for (uint32 k = 0; k < MAX_parents_per_anim_slice; k++) {
			if (!anim_slices[j].anim_parents[k])
				Tdebug("anim_barriers.txt", "   par %d free", k);
			else {
				Tdebug("anim_barriers.txt", "  parent %d has %d animating props", k, anim_slices[j].anim_parents[k]->num_props);
				for (uint32 i = 0; i < anim_slices[j].anim_parents[k]->num_props; i++)
					Tdebug("anim_barriers.txt", "   prop num %d", anim_slices[j].anim_parents[k]->prop_number[i]);
			}
		}
	}

	Tdebug("anim_barriers.txt", "\n\n\n\n\n--------preparing prop slice format---------------\n");

	uint32 total_states, i;

	// now compute the special prop table for run-time LOS injection
	// first reset special prop fixed list
	for (j = 0; j < MAX_props; j++)
		anim_prop_info[j].barriers_per_state = 0; // do it here so it can double up as 0 being an unused entry

	for (j = 0; j < LinkedDataObject::Fetch_number_of_items(MS->prop_anims); j++) {
		Tdebug("anim_barriers.txt", "\n**%d %s", j, LinkedDataObject::Fetch_items_name_by_number(MS->prop_anims, j));
		_animating_prop *index;
		_animation_entry *anim;

		index = (_animating_prop *)LinkedDataObject::Fetch_item_by_number(MS->prop_anims, j);

		// loop through all looking for our named anim
		if (index->num_anims) {
			// got a prop
			// it has anims
			// is it a prop with animating barriers?

			// get first anim
			anim = (_animation_entry *)(((char *)index) + index->anims[0]);

			// does it have anim barriers
			if (anim->num_barriers_per_frame) {
				// ok, we're on

				Tdebug("anim_barriers.txt", " has %d anims and %d per frame", index->num_anims, anim->num_barriers_per_frame);

				// reset our total number of states for this prop counter
				total_states = 0;

				// find the first legal barrier in this anim
				uint16 *bars = (uint16 *)(((char *)index) + anim->offset_barriers);
				i = 0;
				while ((bars[i] == 0xffff) && (i < (uint32)(anim->num_barriers_per_frame * anim->num_frames)))
					i++;

				// better make sure the anim has a legal barrier - if not then assume the prop is well knackered and leave
				// it out of the anim barrier system
				if (i < (uint32)(anim->num_barriers_per_frame * anim->num_frames)) {
					// get our chosen sample barrier
					bar = Fetch_barrier(bars[i]);

					// now compute slice for sample barrier
					cur_slice = 0;
					do {
						slice = (RoutingSlice *)LinkedDataObject::Fetch_item_by_number(route_wrapper, cur_slice);
						if ((bar->m_bottom >= slice->bottom) && (bar->m_bottom < slice->top))
							break;
						cur_slice++;
					} while (cur_slice != total_slices);

					// check for not on a slice - not good
					if (cur_slice == total_slices) { // if so then must be last slice :O
						Tdebug("anim_barriers.txt", "      **ran out of slices :O\n");
					} else {
						Tdebug("anim_barriers.txt", "  sample bar puts prop in slice %d", cur_slice);

						// get the prop number
						uint32 prop_number = LinkedDataObject::Fetch_item_number_by_name(MS->prop_anims, (const char *)LinkedDataObject::Fetch_items_name_by_number(MS->prop_anims, j));

						// gotta check for anims with no equivalent game objects
						if (prop_number != 0xffffffff) {
							Tdebug("anim_barriers.txt", "  listing as prop number %d", anim_slices[cur_slice].num_props_in_slice);
							// ok, add the prop to the slices list of props
							anim_slices[cur_slice].prop_list[anim_slices[cur_slice].num_props_in_slice++] = (uint8)prop_number;

							if (anim_slices[cur_slice].num_props_in_slice >= MAX_props)
								Fatal_error("form anim barrier list found too many props in the slice");

							// set number of barriers per frame for this prop
							anim_prop_info[prop_number].barriers_per_state = anim->num_barriers_per_frame; // set from our sample first anim

							// now loop through all the anims filling in the state table
							// in theory, having done this, there will be entries for each state referenced
							for (uint32 k = 0; k < index->num_anims; k++) {
								// get anim k
								anim = (_animation_entry *)(((char *)index) + index->anims[k]);

								// get anims list of barriers
								uint16 *barriers = (uint16 *)(((char *)index) + anim->offset_barriers);

								// go through each frame/state
								for (uint32 c = 0; c < anim->num_frames; c++) {
									// get state
									uint32 state = anim->frames[c];

									// keep computing the total number of states for this prop
									if (state > total_states)
										total_states = state;

									// for each state go through all the barriers per frame/state
									for (i = 0; i < anim->num_barriers_per_frame; i++) {
										// gotta check for funny illegal barriers
										if (barriers[(c * anim->num_barriers_per_frame) + i] != 0xffff) {
											// legal barrier
											barrier_table[(state * anim->num_barriers_per_frame) + i] =
											    (uint16)barriers[(c * anim->num_barriers_per_frame) + i];
										} else {
											// dodgy barrier
											// setting to barrier 0 but this is not a proper
											// solution
											barrier_table[(state * anim->num_barriers_per_frame) + i] = 0;
										}
									}
								}
							}
							// ok, we've set up the anim barriers for this prop

							Tdebug("anim_barriers.txt", "    prop [%s] highest state %d, %d bars per state",
							       (const char *)LinkedDataObject::Fetch_items_name_by_number(MS->prop_anims, j), total_states,
							       anim_prop_info[prop_number].barriers_per_state);

							// note down total states
							anim_prop_info[prop_number].total_states = (uint8)(total_states + 1);

							// create space for table
							Tdebug("anim_barriers.txt", "prop %d needs %d uint16's", prop_number,
							       (total_states + 1) * anim_prop_info[prop_number].barriers_per_state);

							if (abar_index >= MAX_prop_abars)
								Fatal_error("too many animating barriers - current max = %d", MAX_prop_abars);

							// move the barrier table
							memcpy(&prop_abar_table[abar_index], barrier_table,
							       (sizeof(uint16) * ((total_states + 1) * anim_prop_info[prop_number].barriers_per_state)));

							anim_prop_info[prop_number].barrier_list = &prop_abar_table[abar_index];

							abar_index += ((total_states + 1) * anim_prop_info[prop_number].barriers_per_state);

						} else { // if prop number legal
							Tdebug("anim_barriers.txt", "    prop has no object - ignoring");
						}
					}
				}
			}
		}
	}

	Tdebug("anim_barriers.txt", " ");

	for (j = 0; j < MS->Fetch_number_of_objects(); j++)
		if (anim_prop_info[j].barriers_per_state) {
			Tdebug("anim_barriers.txt", "prop %d", j);
			Tdebug("anim_barriers.txt", "prop %d [%s] has %d anim barriers, %d per frame", j, (const char *)LinkedDataObject::Fetch_items_name_by_number(MS->objects, j),
			       anim_prop_info[j].total_states, anim_prop_info[j].barriers_per_state);
			Tdebug("anim_barriers.txt", "total %d", anim_prop_info[j].barriers_per_state * (anim_prop_info[j].total_states));
			for (uint16 k = 0; k < anim_prop_info[j].barriers_per_state * (anim_prop_info[j].total_states); k++)
				Tdebug("anim_barriers.txt", "%d %d", k, anim_prop_info[j].barrier_list[k]);
		}

	Tdebug("anim_barriers.txt", "\nDone");
}

uint32 _barrier_handler::Get_anim_barriers(uint32 n, uint32 *oThisCubesBarriers, uint32 slice) {
	// gives you all the 'current' anim barriers on an ENTIRE slice
	// used by LOS, planview,
	uint32 prop_id;
	uint32 prop_state;
	uint32 bars_per_state;
	uint32 bar_index;
	uint16 *bars;
	uint32 j, k;

	// how many props in this slice
	uint32 num_props = anim_slices[slice].num_props_in_slice; // prop_list.GetNoItems();

	for (j = 0; j < num_props; j++) {
		// get id of prop
		prop_id = anim_slices[slice].prop_list[j];

		// get prop state
		prop_state = MS->prop_state_table[prop_id];

		// get number of barriers per state
		bars_per_state = anim_prop_info[prop_id].barriers_per_state;

		// index into barrier list to first barrier for this state
		bar_index = (prop_state * bars_per_state);

		bars = (uint16 *)&anim_prop_info[prop_id].barrier_list[0];
		bars += bar_index;

		for (k = 0; k < bars_per_state; k++) {
			if (*(bars) >= total_barriers)
				Fatal_error("Get_anim_barriers - illegal barrier request %d", *(bars));

			oThisCubesBarriers[n++] = (uint32) * (bars++);
		}
	}

	return n;
}

void _game_session::Prepare_megas_abarriers(uint32 slice_number, uint32 parent_number) {
	// fetch abars for the current parent ONLY
	M->number_of_animating = 0;
	uint32 num_props;
	uint32 prop_id;
	uint32 prop_state;
	uint32 bars_per_state;
	uint32 bar_index;
	uint16 *bars;
	uint32 k;

	if (!session_barriers->anim_slices[slice_number].anim_parents[parent_number])
		return; // parent has none at all

	num_props = session_barriers->anim_slices[slice_number].anim_parents[parent_number]->num_props;

	for (uint32 i = 0; i < num_props; i++) {
		// yup - our parent does have animating props

		// get prop id on this parent
		prop_id = session_barriers->anim_slices[slice_number].anim_parents[parent_number]->prop_number[i];

		// get prop state
		prop_state = prop_state_table[prop_id];

		// get number of barriers per state
		bars_per_state = session_barriers->anim_prop_info[prop_id].barriers_per_state;

		// index into barrier list to first barrier for this state
		bar_index = (prop_state * bars_per_state);

		bars = (uint16 *)&session_barriers->anim_prop_info[prop_id].barrier_list[0];
		bars += bar_index;

		for (k = 0; k < bars_per_state; k++)
			M->barrier_list[M->number_of_barriers + M->number_of_animating++] = (uint32) * (bars++);
	}
}

void _game_session::Fetch_mega_barriers_for_player() {
	// find all the megas on our floor
	// then add in barriers for each of them that is stood
	// add the barriers on the end of the anim barrier list which MUST have already been computed
}

} // End of namespace ICB
