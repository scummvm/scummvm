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
#include "engines/icb/res_man.h"

#include "common/textconsole.h"

namespace ICB {

bool8 _game_session::Find_interact_marker_in_anim(__mega_set_names animation, PXreal *xoff, PXreal *zoff) {
	// find the first instance of the interact marker - which will be on every frame or none at all which is an error
	// then find the offset from the feet ORG_MARKET to the INT interaction marker
	// this difference is the relative position from the nico that the character must route to to run this animation
	// the idea being that the animations can therefore change without any effect on the engine - its all data driven

	// first check anim file will exist
	if (!I->IsAnimTable(animation))
		Fatal_error("Find_interact_marker_in_anim finds [%s] doesn't have a [%s] animation", CGameObject::GetName(object), master_anim_name_table[animation].name);

	// open the file
	PXanim *pAnim = (PXanim *)rs_anims->Res_open(I->get_info_name(animation), I->info_name_hash[animation], I->base_path, I->base_path_hash);

	// New code block

	// Loop over animation frames looking for interact if it is found then
	// take the position from where it is found
	// i.e. replace the hard coded look on frame 0 with search for it
	//
	// The INTERACT marker comes in two forms
	// INT0_TYPE : which means it was a forced export on frame 0 only, because the anim never made the INT marker visible
	// INT_TYPE : which means it was a real export of the INT marker on the frame when the anim made the INT marker visible

	PXreal x_org, z_org, unused;
	PXmarker_PSX_Object::GetXYZ(&PXFrameEnOfAnim(0, pAnim)->markers[ORG_POS], &x_org, &unused, &z_org);

	for (uint16 frame = 0; frame < pAnim->frame_qty; frame++) {
		PXframe *frm = PXFrameEnOfAnim(frame, pAnim);
		// Check the interact marker is present

		if (frm->marker_qty > INT_POS) {
			PXmarker *marker = &(frm->markers[INT_POS]);
			uint8 mtype = (uint8)PXmarker_PSX_Object::GetType(marker);

			if ((INT0_TYPE == mtype) || (INT_TYPE == mtype)) {
				// The interact marker exists
				PXreal x_int, z_int;

				PXmarker_PSX_Object::GetXYZ(marker, &x_int, &unused, &z_int);

				xoff[0] = x_int - x_org;
				zoff[0] = z_int - z_org;

				return (TRUE8);
			}
		}
	}
	// did not find an INT marker!
	Message_box("Warning couldn't find INT marker for '%s' %X %s", I->get_info_name(animation), I->info_name_hash[animation], master_anim_name_table[animation].name);

	xoff[0] = REAL_ZERO;
	zoff[0] = REAL_ZERO;
	return (TRUE8);

	// This is an error and should be re-enabled, it is made a warning so implementation can continue
	// and they can get the anims fixed as time proceeds
	// We must then get an interaction marker put into the animation
}

bool8 _game_session::Compute_target_interaction_coordinate(__mega_set_names anim, PXreal *destx, PXreal *destz) {
	// get target coordinate
	// this is for prop interaction
	// used L->target_id

	// we're passed TYPE of anim
	// the type may be __CUSTOM

	PXreal int_x, int_z; // for the marker
	bool8 ret;
	PXfloat tpan;

	// get interact marker offset
	ret = Find_interact_marker_in_anim(anim, &int_x, &int_z);

	if (!ret) {
		Zdebug("no int marker found in %s", (const char *)(I->get_info_name(anim)));
		Fatal_error("Compute_target_interaction_coordinate: no int marker error object %s anim %s", (const char *)(L->GetName()), (const char *)(I->get_info_name(anim)));
	}

	Zdebug("INT markers %3.2f, %3.2f", int_x, int_z);

	// get coord of target object and sub from it the offset from the anim

	// reverse the target pan so it faces 180deg back around
	tpan = logic_structs[M->target_id]->prop_interact_pan + HALF_TURN;

	// now project the interaction offset back away from the target nico
	// this will give us our interact coordinate
	PXfloat ang = tpan * TWO_PI;
	PXfloat cang = (PXfloat)PXcos(ang);
	PXfloat sang = (PXfloat)PXsin(ang);

	destx[0] = logic_structs[M->target_id]->prop_xyz.x + PXfloat2PXreal(int_x * cang + int_z * sang);
	destz[0] = logic_structs[M->target_id]->prop_xyz.z + PXfloat2PXreal(int_z * cang - int_x * sang);

	return (TRUE8);
}

void _game_session::Snap_to_ladder(_stair *lad, uint32 dist) {
	// snap our coordinate to passed ladder
	// take the ladder marker, reverse its pan, and push backward to get our coordinate
	// sadly we need to push back by a different amount for top or bottom

	PXfloat tpan;

	tpan = lad->pan + HALF_TURN; // reverse the ladder nico

	PXfloat ang = tpan * TWO_PI;
	PXfloat cang = (PXfloat)PXcos(ang);
	PXfloat sang = (PXfloat)PXsin(ang);

	warning("Snap:: lad %f %f ang %f", lad->x, lad->z, (ang * 360) / FULL_TURN);

	M->actor_xyz.x = lad->x + PXfloat2PXreal((PXfloat)(dist)*sang);
	M->actor_xyz.z = lad->z + PXfloat2PXreal((PXfloat)(dist)*cang);

	warning("Snap:: new pos %f %f", M->actor_xyz.x, M->actor_xyz.z);
}

} // End of namespace ICB
