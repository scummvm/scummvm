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
#include "engines/icb/p4_generic.h"
#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/text.h"
#include "engines/icb/direct_input.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/surface_manager.h"
#include "engines/icb/stage_draw.h"
#include "engines/icb/actor.h"
#include "engines/icb/actor_pc.h"
#include "engines/icb/res_man.h"
#include "engines/icb/common/px_capri_maths.h"
#include "engines/icb/gfx/psx_anims.h"

namespace ICB {

void PolyStageDraw(SDactor *actors, uint32 actorQty);

void _game_session::Stage_draw_poly() {
	// The actors to draw
	SDactor actors[MAXIMUM_POTENTIAL_ON_SCREEN_ACTOR_QUANTITY];

	// Get back which mega the actor could interact with
	int32 sel_id = GetSelectedMegaId();

	// Fill in the actors that need drawing an update thier positions
	uint32 actorsToDraw = 0;
	int32 id;
	for (uint32 j = 0; j < number_of_voxel_ids; j++) {
		//		fetch the logic structure for the game object that has a voxel image to render
		id = (int32)voxel_id_list[j];
		_logic *log = logic_structs[id];

		// Only clip by HITHER_PLANE, ActorDraw will clip image to film boundaries
		if (Object_visible_to_camera(id)) {
			g_mission->nActorsConsidered++;

			bool8 result = TRUE8;
			PXvector filmPosition;
			PXWorldToFilm(log->mega->actor_xyz, set.GetCamera(), result, filmPosition);

			if (filmPosition.z < -g_actor_hither_plane) {
				VECTOR v;
				v.vx = (int32)log->mega->actor_xyz.x;
				v.vy = (int32)log->mega->actor_xyz.y;
				v.vz = (int32)log->mega->actor_xyz.z;

				SVECTOR orient;
				orient.vx = 0;
				orient.vy = 0;
				orient.vz = 0;

				// Ignore characters who are off screen
				if (QuickActorCull((psxCamera *)&MSS.GetCamera(), &v, &orient) == 1)
					continue;

				SDactor &thisActor = actors[actorsToDraw];

				// do a high level anim file existence check!
				if (!log->voxel_info->IsAnimTable(log->cur_anim_type))
					Fatal_error("Illegal anim type [%s %s %s] for [%s]", master_anim_name_table[log->cur_anim_type].name, log->mega->chr_name,
					            log->mega->anim_set, log->GetName());

				thisActor.anim_name =
				    log->voxel_info->anim_name[log->cur_anim_type]; // this should do the trick now that anim mega set.cpp sets names correct for polygons...

				// setup act element

				thisActor.frame = log->anim_pc;

				psxActor &act = thisActor.psx_actor;

				uint32 hash1 = NULL_HASH;
				uint32 hash2 = NULL_HASH;

				PXanim_PSX *pAnim = (PXanim_PSX *)rs_anims->Res_open(log->voxel_info->info_name[log->cur_anim_type], hash1, log->voxel_info->base_path, hash2);

				int f = log->anim_pc;
				if ((f < 0) || (f >= (int)pAnim->frame_qty)) {
					f = (pAnim->frame_qty - 1);
				}
				PXmarker_PSX &marker = PXFrameEnOfAnim(f, pAnim)->markers[ORG_POS];
				float dx, dy, dz;
				marker.GetXYZ(&dx, &dy, &dz);

				// Make the actors orientation matrix
				int pan;
				if (log->auto_panning == FALSE8)
					pan = (int)((log->pan - log->pan_adjust) * 4096);
				else
					pan = (int)((log->auto_display_pan - log->pan_adjust) * 4096);

				act.rot.vx = 0;
				act.rot.vy = (short)pan;
				act.rot.vz = 0;

				// Because only one level in the hierarchy this is the lw matrix
				RotMatrix_gte(&act.rot, &act.lw);
				act.lw.t[0] = (int)log->mega->actor_xyz.x;
				act.lw.t[1] = (int)log->mega->actor_xyz.y;
				act.lw.t[2] = (int)log->mega->actor_xyz.z;

				// the bone local frame has the hip at 0,0,0
				// so put the hip at correct world height
				// uint32 dy = ( marker.pos.y == 0 ) ? 100 : marker.pos.y;
				// if ( log->cur_anim_type == __BEING_SHOT_DEAD ) dy = 0;
				act.lw.t[1] += (int)dy;

				// Set the actors true rotation & position values
				act.truePos.x = (int)log->mega->actor_xyz.x;
				act.truePos.y = (int)log->mega->actor_xyz.y;
				act.truePos.z = (int)log->mega->actor_xyz.z;

				act.truePos.y += (int)dy;

				act.trueRot.vx = 0;

				if (log->auto_panning == FALSE8)
					act.trueRot.vy = (short)(log->pan * 4096);
				else
					act.trueRot.vy = (short)(log->auto_display_pan * 4096);
				act.trueRot.vz = 0;

				//
				thisActor.log = log;

				// if selected then set ambient to be ambientSelect otherwise it's ambient normal...
				u_char r = 0;
				u_char g = 0;
				u_char b = 0;

				if (id == sel_id) {
					GetSelectedMegaRGB(r, g, b);
				}

				// mega RGB is 0-256
				thisActor.r = r;
				thisActor.g = g;
				thisActor.b = b;

				//  must be reset
				log->pan_adjust = FLOAT_ZERO;

				// And on to the next actor
				actorsToDraw++;
				g_mission->nActorsDrawn++;
			}
		}
	}

	StageDrawPoly(actors, actorsToDraw);
}

} // End of namespace ICB
