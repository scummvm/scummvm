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

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/session.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/sound.h"
#include "engines/icb/floors.h"
#include "engines/icb/res_man.h"

namespace ICB {

#define MIN_FOOTSTEP_DISTANCE_XZ 2000 // 20m
#define MIN_FOOTSTEP_DISTANCE_Y 200   // 2m

mcodeFunctionReturnCodes fn_set_default_footstep_sfx(int32 &result, int32 *params) { return (MS->fn_set_default_footstep_sfx(result, params)); }

mcodeFunctionReturnCodes fn_set_floor_footstep_sfx(int32 &result, int32 *params) { return (MS->fn_set_floor_footstep_sfx(result, params)); }

mcodeFunctionReturnCodes fn_set_footstep_weight(int32 &result, int32 *params) { return (MS->fn_set_footstep_weight(result, params)); }

mcodeFunctionReturnCodes fn_set_special_footstep(int32 &result, int32 *params) { return (MS->fn_set_special_footstep(result, params)); }

// fn_set_default_footstep_sfx("sfx");
// set the default sfx for footsteps
mcodeFunctionReturnCodes _game_session::fn_set_default_footstep_sfx(int32 &, int32 *params) {
	const char *sfx_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	defaultFootSfx = HashString(sfx_name);

	return IR_CONT;
}

// fn_set_floor_footstep_sfx("floor","sfx");
// set a specific sound effect for a named floor footstep
mcodeFunctionReturnCodes _game_session::fn_set_floor_footstep_sfx(int32 &, int32 *params) {
	const char *floor_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *sfx_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	if (numFloorFootSfx == MAX_footstepFloors)
		Fatal_error("trying to set more than the maximum (%d) amount of specific floor footsteps (trying %s=%s)", MAX_footstepFloors, floor_name, sfx_name);

	if (strcmp(floor_name, "special") == 0)
		specialFootSfx = HashString(sfx_name);
	else if (strcmp(floor_name, "ladder") == 0)
		ladderFootSfx = HashString(sfx_name);
	else {
		floorFootSfx[numFloorFootSfx][0] = floor_def->Fetch_floor_number_by_name(floor_name);
		floorFootSfx[numFloorFootSfx][1] = HashString(sfx_name);
		numFloorFootSfx++;
	}

	return IR_CONT;
}

// fn_set_footstep_weight(weight);
// set weight of mega for footstep: % of cords weight alowable 0-200
mcodeFunctionReturnCodes _game_session::fn_set_footstep_weight(int32 &, int32 *params) {
	int w;

	w = params[0];

	// check must be mega
	if (!M)
		Fatal_error("Cant set footstep weight for non-mega %s\n", L->GetName());

	// check allow
	if ((w < 0) || (w > 200))
		Fatal_error("Cant set %s footstep weight to %d, allowed range is 0-200%%", L->GetName(), w);

	M->footstep_weight = (uint8)w;

	return IR_CONT;
}

// fn_set_special_footstep(on/off)
// turn on special footstep production for this mega
mcodeFunctionReturnCodes _game_session::fn_set_special_footstep(int32 &, int32 *params) {
	// error checking
	// must be mega
	if (!M)
		Fatal_error("Cant set special footsteps for non-mega %s\n", L->GetName());

	// must have special footstep set
	if ((specialFootSfx == 0) && (params[0]))
		Fatal_error("Mega %s set special footstep on but no special footstep has been specified", L->GetName());

	// set
	if (params[0])
		M->footstep_special = TRUE8;
	else
		M->footstep_special = FALSE8;

	return IR_CONT;
}

void _game_session::UpdateFootstep() {
	PXreal x, y, z, px, py, pz;

	// get position and players position...
	L->GetPosition(x, y, z);
	player.log->GetPosition(px, py, pz);

	px -= x;
	py -= y;
	pz -= z;

	// check in range...
	if ((PXfabs(px) < MIN_FOOTSTEP_DISTANCE_XZ) && (PXfabs(py) < MIN_FOOTSTEP_DISTANCE_Y) && (PXfabs(pz) < MIN_FOOTSTEP_DISTANCE_XZ) &&
	    ((px * px + pz * pz) < (MIN_FOOTSTEP_DISTANCE_XZ * MIN_FOOTSTEP_DISTANCE_XZ))) {

		// Try to find anim file
		PXanim *pAnim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash);

		// get the frame...
		PXframe *frm = PXFrameEnOfAnim(L->anim_pc, pAnim);

		int8 step = 0;

		// step is true if either left foor or right foot hit the ground
		if ((frm->leftFootStep) || (frm->rightFootStep))
			step = 1;

		// if last was true and so was this one then keep status the same and set step to zero
		if ((M->footstep_status) && (step)) {
			// status stays true
			step = 0;
		}
		// otherwise step can be set to either true or false depending on anim and status gets set to whatever it is
		else {
			M->footstep_status = step;
			// step stays the same
		}

		// do the footstep
		if (step) {
			// volume
			int vol;

			// volume from weight (0-128, standard is 64)
			vol = (M->footstep_weight * 63) / 100;

			// if crouched reduce by factor of 4
			if (M->Is_crouched())
				vol /= 4;
			// otherwise if not running then reduce by factor of 2
			else if (L->cur_anim_type != __RUN)
				vol /= 2;

			// if walking add a boast (*1.5)
			if (L->cur_anim_type == __WALK) {
				vol = vol + (vol / 2);
			}

			// do step...

			// check for special mega who just has single sfx
			if (M->footstep_special) {
				RegisterSound(cur_id, NULL, specialFootSfx, "foot", (int8)(vol));
			}
			// we are an ordinary mega look for sfx
			else {
				// try and find our floor in the overrides...
				uint32 sfx = 0;
				int floor = L->owner_floor_rect;
				int i = 0;

				// loop through or until we have a sfx
				while ((i < numFloorFootSfx) && (sfx == 0)) {
					if (floorFootSfx[i][0] == (uint32)floor)
						sfx = floorFootSfx[i][1];

					// next one
					i++;
				}

				// if on ladder then set to ladder sfx...
				if ((L->cur_anim_type == __CLIMB_UP_LADDER_LEFT) || (L->cur_anim_type == __CLIMB_UP_LADDER_RIGHT) ||
				    (L->cur_anim_type == __CLIMB_DOWN_LADDER_LEFT) || (L->cur_anim_type == __CLIMB_DOWN_LADDER_RIGHT))
					sfx = ladderFootSfx;

				// if sfx not found in specific floors then use default
				if (sfx == 0)
					sfx = defaultFootSfx;

				if (sfx)
					RegisterSound(cur_id, NULL, sfx, "foot", (int8)(vol));
			}
		}
	}
}

} // End of namespace ICB
