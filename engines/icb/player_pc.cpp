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

#include "engines/icb/p4.h" //for machine version
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/player.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/common/px_route_barriers.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_switches.h"

namespace ICB {

#define FACING_FORWARDS ((70 * 4096) / 360)
#define FACING_BACKWARDS ((135 * 4096) / 360)

#define STOOD_TURN_TOLERANCE ((10 * 4096) / 360)
#define WALK_TURN_TOLERANCE ((20 * 4096) / 360)
#define STAIR_TURN_TOLERANCE ((130 * 4096) / 360)
#define STOOD_HARD_TURN_TOLERANCE ((60 * 4096) / 360)
#define NO_TURN_TOLERANCE (2 * 4096)
#define RUN_TURN_TOLERANCE ((30 * 4096) / 360)

#define NEAREST_INT(X) (X) > 0.0 ? int((X) + 0.5) : int((X)-0.5)
#define FIXED_POINT_ANG(X) NEAREST_INT(((X)*4096.0f) / (2 * M_PI))

#define SECOND_ZONE 80

#define R_LIMIT 50

#define CHECKDXDY(dx, dy)                                                                                                                                                          \
	{                                                                                                                                                                          \
		int32 r2 = (dx) * (dx) + (dy) * (dy);                                                                                                                                \
		if (r2 < R_LIMIT * R_LIMIT) {                                                                                                                                      \
			if (r2 < 1)                                                                                                                                                \
				r2 = 1;                                                                                                                                            \
			int32 scale = NEAREST_INT(PXsqrt((double)(128 * 128 * R_LIMIT * R_LIMIT) / r2));                                                                             \
			(dx) = ((dx)*scale) / 128;                                                                                                                                 \
			(dy) = ((dy)*scale) / 128;                                                                                                                                 \
		}                                                                                                                                                                  \
	}

// Which pan value the joystick is pointing at
uint32 g_targetPan = 0;
int32 g_playerPan;
int32 g_joyPan;

uint8 _DRAWCOMPASS_ = 0;

// pc device switch
__pc_input device = __KEYS; // default to keys

uint32 fire_key;
uint32 interact_key;
uint32 inventory_key;
uint32 arm_key;
uint32 remora_key;
uint32 crouch_key;
uint32 sidestep_key;
uint32 run_key;
uint32 up_key;
uint32 down_key;
uint32 left_key;
uint32 right_key;
uint32 pause_key;

void _player::Update_input_state() {
	// looks at the input devices in use and returns a single user-is-doing state

	// note - analogue controllers return a direction which is different from forward-and-rotate schemes

	// this routine requires separate pc/psx coding

	//				pc devices
	//				----------
	//				keys
	//				joysticks
	//				gamepads
	//				analogue devices

	// Are we trying to find a new pan value
	g_targetPan = 0;
	g_playerPan = NEAREST_INT(log->pan * 4096.0f);
	int32 clastCameraPan = NEAREST_INT(lastCameraPan * 4096.0f);
	int32 cdeltaCameraPan = NEAREST_INT(deltaCameraPan * 4096.0f);

	int32 stairs = 0;
	int32 ladders = 0;

	// Defaults
	cur_state.turn = __NO_TURN;
	cur_state.bitflag = 0;

	// Button controls are universal for all control modes

	if (Read_DI_keys(fire_key)) // Attack
		cur_state.SetButton(__ATTACK);

	if (Read_DI_keys(interact_key)) // Interact
		cur_state.SetButton(__INTERACT);

	if (Read_DI_keys(inventory_key)) // Inventory
		cur_state.SetButton(__INVENTORY);

	if (Read_DI_keys(arm_key)) // Arm/unarm
		cur_state.SetButton(__ARMUNARM);

	if (Read_DI_keys(remora_key)) // Remora
		cur_state.SetButton(__REMORA);

	if (Read_DI_keys(crouch_key)) // Crouch
		cur_state.SetButton(__CROUCH);

	if (Read_DI_keys(sidestep_key)) // Sidestep
		cur_state.SetButton(__SIDESTEP);

	if (Read_DI_keys(run_key)) // Jog
		cur_state.SetButton(__JOG);

	// No control whilst sliding down a ladder !
	if (player_status == SLIP_SLIDIN_AWAY)
		return;

	// No control whilst getting onto a ladder
	if ((player_status == EASY_LINKING) && (stat_after_link == BEGIN_DOWN_LADDER)) {
		cur_state.momentum = __STILL;
		return;
	}

	// Have different turn tolerances & control method for being on or off stairs
	if ((stat_after_link == ON_STAIRS) || (player_status == ON_STAIRS) || (stat_after_link == STOOD_ON_STAIRS) || (player_status == STOOD_ON_STAIRS) ||
	    (stat_after_link == REVERSE_ON_STAIRS) || (player_status == REVERSE_ON_STAIRS) || (stat_after_link == RUNNING_ON_STAIRS) || (player_status == RUNNING_ON_STAIRS)) {
		stairs = 1;
	}

	// Have different bigger tolerances & control method for being on or off ladders
	if (((player_status == EASY_LINKING) && ((stat_after_link == ON_LADDER) || (stat_after_link == SLIP_SLIDIN_AWAY) || (stat_after_link == LEAVE_LADDER) ||
	                                         (stat_after_link == BEGIN_DOWN_LADDER) || (stat_after_link == LEAVE_LADDER_BOTTOM))) ||
	    (player_status == ON_LADDER) || (player_status == SLIP_SLIDIN_AWAY) || (player_status == LEAVE_LADDER) || (player_status == BEGIN_DOWN_LADDER) ||
	    (player_status == LEAVE_LADDER_BOTTOM)) {
		ladders = 1;
		g_playerPan = NEAREST_INT(4096.0f * MS->stairs[stair_num].pan_ref);
	}

	// Keyboard support (generic controls)

	// If running then always running
	if (cur_state.IsButtonSet(__JOG)) {
		if (player_status == STOOD)
			cur_state.momentum = __FORWARD_1;
		else
			cur_state.momentum = __FORWARD_2;
	} else {
		// Forward/backward - one or the other
		if (Read_DI_keys(up_key)) {
			// Can't move forward with sidestep held (unless on stairs or ladders)
			if (cur_state.IsButtonSet(__SIDESTEP) && stairs == 0 && ladders == 0) {
				cur_state.momentum = __STILL;
			} else {
				// We want to walk
				cur_state.momentum = __FORWARD_1;
			}
		} else if (Read_DI_keys(down_key))
			cur_state.momentum = __BACKWARD_1;
		else
			cur_state.momentum = __STILL;
	}

	// Left/right
	if (Read_DI_keys(left_key)) {
		cur_state.turn = __LEFT;
	} else if (Read_DI_keys(right_key)) {
		cur_state.turn = __RIGHT;
	} else
		cur_state.turn = __NO_TURN;

	// Set the floating point version of the fixed point variables we've used
	lastCameraPan = ((float)clastCameraPan / 4096.0f);
	deltaCameraPan = ((float)cdeltaCameraPan / 4096.0f);
}

void _player::DrawCompass() {
}

} // End of namespace ICB
