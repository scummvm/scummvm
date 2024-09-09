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
#include "engines/icb/common/px_anims.h"
#include "engines/icb/session.h"
#include "engines/icb/animation_mega_set.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/debug.h"
#include "engines/icb/player.h"
#include "engines/icb/res_man.h"

namespace ICB {

bool8 _game_session::Easy_frame_motion_and_pan(__mega_set_names anim_type, bool8 /*player*/) {
	// advances frame and motion AND PAN but does not check for barrier collisions
	PXreal xnext, znext;
	PXreal x, z;

	ANIM_CHECK(anim_type);

	PXanim *pAnim = (PXanim *)rs_anims->Res_open(I->get_info_name(anim_type), I->info_name_hash[anim_type], I->base_path, I->base_path_hash);

	if (L->anim_pc + 1 >= pAnim->frame_qty)
		Fatal_error("Easy_frame_and_motion finds [%s] has illegal frame in anim [%s] %d %d", (const char *)L->GetName(), (const char *)I->get_info_name(anim_type),
		            L->anim_pc, pAnim->frame_qty);

	// add the change in pan from the last frame to the current frame of the anim to the logic objects pan
	PXreal pan1, pan2;

	// Get next frame from the anim
	PXframe *nextFrame = PXFrameEnOfAnim(L->anim_pc + 1, pAnim);

	// Get current frame from the anim
	PXframe *currentFrame = PXFrameEnOfAnim(L->anim_pc, pAnim);

	PXmarker_PSX_Object::GetPan(&nextFrame->markers[ORG_POS], &pan1);
	PXmarker_PSX_Object::GetPan(&currentFrame->markers[ORG_POS], &pan2);

	L->pan += (pan1 - pan2); // update by difference

	// get motion displacement from currently displayed frame to next one
	// note that we always read frame+1 for motion of next frame even though the voxel frame itself will be looped back to 0
	PXreal x1, x2, z1, z2, unused;

	PXmarker_PSX_Object::GetXYZ(&nextFrame->markers[ORG_POS], &x2, &unused, &z2);
	PXmarker_PSX_Object::GetXYZ(&currentFrame->markers[ORG_POS], &x1, &unused, &z1);

	xnext = x2 - x1;
	znext = z2 - z1;

	// advance the frame
	L->anim_pc = (L->anim_pc + 1) % (pAnim->frame_qty - 1);

	// get the pan unwind value of the frame to be printed
	PXreal pan;

	// Get the new current frame, using a new reference !
	// as GCC got it all wrong if the same reference was re-used
	currentFrame = PXFrameEnOfAnim(L->anim_pc, pAnim);

	PXmarker_PSX_Object::GetPan(&currentFrame->markers[ORG_POS], &pan);

	L->pan_adjust = pan; // this value will be unwound from the orientation of the frame at render time in stage draw

	// calculate the new x and z coordinate from this frames motion offset
	// do the z and x together
	PXfloat ang = (L->pan - L->pan_adjust) * TWO_PI;
	PXfloat cang = (PXfloat)PXcos(ang);
	PXfloat sang = (PXfloat)PXsin(ang);

	x = M->actor_xyz.x + PXfloat2PXreal(xnext * cang + znext * sang);
	z = M->actor_xyz.z + PXfloat2PXreal(znext * cang - xnext * sang);

	// x and z are the new coordinates

	// record the new true actor position
	M->actor_xyz.x = x;
	M->actor_xyz.z = z;

	// check for new parent box and if so bring barriers
	if (L->pan >= HALF_TURN)
		L->pan -= FULL_TURN;
	else if (L->pan <= -HALF_TURN)
		L->pan += FULL_TURN;

	return (TRUE8);
}

bool8 _game_session::Easy_frame_and_motion(__mega_set_names anim_type, bool8 /*player*/, uint8 nFrames) {
	// advances frame and motion but does not check for barrier collisions
	PXreal xnext, znext;
	PXreal x, z;

	ANIM_CHECK(anim_type);

	PXanim *pAnim = (PXanim *)rs_anims->Res_open(I->get_info_name(anim_type), I->info_name_hash[anim_type], I->base_path, I->base_path_hash);

	if ((L->anim_pc + nFrames) >= pAnim->frame_qty)
		Fatal_error("Easy_frame_and_motion finds [%s] has illegal frame in anim [%s] %d %d", (const char *)L->GetName(), (const char *)I->get_info_name(anim_type),
		            L->anim_pc, pAnim->frame_qty);

	// get motion displacement from currently displayed frame to next one
	// note that we always read frame+1 for motion of next frame even though the voxel frame itself will be looped back to 0
	PXreal x1, x2, z1, z2, unused;

	// Get next frame from the anim
	PXframe *nextFrame = PXFrameEnOfAnim(L->anim_pc + nFrames, pAnim);
	// Get current frame from the anim
	PXframe *currentFrame = PXFrameEnOfAnim(L->anim_pc, pAnim);

	PXmarker_PSX_Object::GetXYZ(&nextFrame->markers[ORG_POS], &x2, &unused, &z2);
	PXmarker_PSX_Object::GetXYZ(&currentFrame->markers[ORG_POS], &x1, &unused, &z1);

	xnext = x2 - x1;
	znext = z2 - z1;

	// calculate the new x and z coordinate from this frames motion offset
	// do the z and x together
	PXfloat ang = L->pan * TWO_PI;
	PXfloat cang = (PXfloat)PXcos(ang);
	PXfloat sang = (PXfloat)PXsin(ang);

	x = M->actor_xyz.x + PXfloat2PXreal(xnext * cang + znext * sang);
	z = M->actor_xyz.z + PXfloat2PXreal(znext * cang - xnext * sang);
	// x and z are the new coordinates

	// advance the frame
	L->anim_pc = (L->anim_pc + nFrames);

	// record the new true actor position
	M->actor_xyz.x = x;
	M->actor_xyz.z = z;

	return (TRUE8);
}

bool8 _game_session::Advance_frame_and_motion(__mega_set_names anim_type, bool8 pl, uint8 nFrames) {
	// attempts to move the frame forward and move the character
	// returns fail and frame is not changed if the way forward is blocked by a barrier
	// the frame counter will be looped
	// handles shallow barrier bump correction!

	__barrier_result ret;
	uint32 initial_pc = L->anim_pc;

	ANIM_CHECK(anim_type);

	ret = Core_advance(anim_type, pl, nFrames);

	if (ret == __BLOCKED)
		return (FALSE8);

	if ((ret == __OK) || (ret == __NUDGED))
		return (TRUE8);

	// pan was corrected - go around again

	L->anim_pc = initial_pc;
	ret = Core_advance(anim_type, pl, nFrames);

	if (ret == __BLOCKED) // v odd indeed
		return (FALSE8);

	if ((ret == __OK) || (ret == __NUDGED))
		return (TRUE8); // good

	// corrected again - this is strange

	return (FALSE8);
}

__barrier_result _game_session::Core_advance(__mega_set_names anim_type, bool8 pl, uint8 nFrames) {
	PXreal xnext, znext;
	PXreal x, z;
	__barrier_result ret;

	PXanim *pAnim = (PXanim *)rs_anims->Res_open(I->get_info_name(anim_type), I->info_name_hash[anim_type], I->base_path, I->base_path_hash);

	if ((L->anim_pc + nFrames) >= pAnim->frame_qty)
		Fatal_error("Core_advance finds [%s] has illegal frame in anim [%s] %d %d", (const char *)L->GetName(), (const char *)I->get_info_name(anim_type), L->anim_pc,
		            pAnim->frame_qty);

	// get motion displacement from currently displayed frame to next one
	// note that we always read frame+1 for motion of next frame even though the voxel frame itself will be looped back to 0
	PXreal x1, x2, z1, z2, unused;

	// Get next frame from the anim
	PXframe *nextFrame = PXFrameEnOfAnim(L->anim_pc + nFrames, pAnim);
	// Get current frame from the anim
	PXframe *currentFrame = PXFrameEnOfAnim(L->anim_pc, pAnim);

	PXmarker_PSX_Object::GetXYZ(&nextFrame->markers[ORG_POS], &x2, &unused, &z2);
	PXmarker_PSX_Object::GetXYZ(&currentFrame->markers[ORG_POS], &x1, &unused, &z1);

	xnext = x2 - x1;
	znext = z2 - z1;

	// calculate the new x and z coordinate from this frames motion offset
	// do the z and x together
	PXfloat ang = L->pan * TWO_PI;
	PXfloat cang = (PXfloat)PXcos(ang);
	PXfloat sang = (PXfloat)PXsin(ang);

	x = M->actor_xyz.x + PXfloat2PXreal(xnext * cang + znext * sang);
	z = M->actor_xyz.z + PXfloat2PXreal(znext * cang - xnext * sang);
	// x and z are the new coordinates

	// but, are we walking into a barrier?
	ret = Check_barrier_bump_and_bounce(x, M->actor_xyz.y, z,                           // new position
	                                    M->actor_xyz.x, M->actor_xyz.y, M->actor_xyz.z, // old position
	                                    pl);

	L->anim_pc = (L->anim_pc + nFrames);

	// did we move forward without a problem?
	if (ret == __OK) {
		// advance the frame

		// record the new true actor position
		M->actor_xyz.x = x;
		M->actor_xyz.z = z;

		// check for new parent box and if so bring barriers
		Prepare_megas_route_barriers(pl);
	}

	return (ret);
}

void _game_session::Normalise_anim_pc() {
	// routines that make raw calls to advance_frame_and_motion need to call this to reset anim pc
	// this is because of changes required by the multi-speed capabilities

	PXanim *pAnim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash); //
	L->anim_pc = (L->anim_pc) % (pAnim->frame_qty - 1);
}

bool8 _game_session::Reverse_frame_and_motion(__mega_set_names anim_type, bool8 pl, uint8 nFrames) {
	// as Advance_frame_and_motion but frames work backward
	// the frame counter will be looped
	// handles shallow barrier bump correction!

	__barrier_result ret;

	ANIM_CHECK(anim_type);

	ret = Core_reverse(anim_type, pl, nFrames);

	if (ret == __BLOCKED)
		return (FALSE8);

	if ((ret == __OK) || (ret == __NUDGED))
		return (TRUE8);

	// pan was corrected - go around again
	ret = Core_reverse(anim_type, pl, nFrames);

	if (ret == __BLOCKED) // v odd indeed
		return (FALSE8);

	if ((ret == __OK) || (ret == __NUDGED))
		return (TRUE8); // good

	// corrected again - this is strange

	return (FALSE8);
}

__barrier_result _game_session::Core_reverse(__mega_set_names anim_type, bool8 pl, uint8 nFrames) {
	// as Advance_frame_and_motion but frames work backward
	// the frame counter will be looped
	// handles shallow barrier bump correction!

	PXreal xnext, znext;
	PXreal x, z;
	__barrier_result ret;
	uint32 next_pc;

	ANIM_CHECK(anim_type);

	PXanim *pAnim = (PXanim *)rs_anims->Res_open(I->get_info_name(anim_type), I->info_name_hash[anim_type], I->base_path, I->base_path_hash);

	// if we are on frame 0 then shift up to pretend we're coming in off the dummy frame
	if (!L->anim_pc) {
		next_pc = pAnim->frame_qty - 2;
		L->anim_pc = pAnim->frame_qty - 1;
	} else if (L->anim_pc < nFrames)
		next_pc = 0;
	else
		next_pc = L->anim_pc - nFrames;

	if ((next_pc >= pAnim->frame_qty) || (L->anim_pc >= pAnim->frame_qty))
		Fatal_error("Core_reverse finds [%s] has illegal frame in anim [%s] %d %d %d", (const char *)L->GetName(), (const char *)I->get_info_name(anim_type), next_pc,
		            L->anim_pc, pAnim->frame_qty);

	// get motion displacement from currently displayed frame to next one
	// note that we always read frame+1 for motion of next frame even though the voxel frame itself will be looped back to 0
	PXreal x1, x2, z1, z2, unused;

	// Get next frame from the anim
	PXframe *nextFrame = PXFrameEnOfAnim(next_pc, pAnim);
	// Get current frame from the anim
	PXframe *currentFrame = PXFrameEnOfAnim(L->anim_pc, pAnim);

	PXmarker_PSX_Object::GetXYZ(&nextFrame->markers[ORG_POS], &x2, &unused, &z2);
	PXmarker_PSX_Object::GetXYZ(&currentFrame->markers[ORG_POS], &x1, &unused, &z1);

	xnext = x2 - x1;
	znext = z2 - z1;

	// calculate the new x and z coordinate from this frames motion offset
	// do the z and x together
	PXfloat ang = L->pan * TWO_PI;
	PXfloat cang = (PXfloat)PXcos(ang);
	PXfloat sang = (PXfloat)PXsin(ang);

	x = M->actor_xyz.x + PXfloat2PXreal(xnext * cang + znext * sang);
	z = M->actor_xyz.z + PXfloat2PXreal(znext * cang - xnext * sang);
	// x and z are the new coordinates

	// but, are we walking into a barrier?
	ret = Check_barrier_bump_and_bounce(x, M->actor_xyz.y, z,                           // new position
	                                    M->actor_xyz.x, M->actor_xyz.y, M->actor_xyz.z, // old position
	                                    pl);

	L->anim_pc = next_pc;

	// did we move forward without a problem?
	if (ret == __OK) {
		// record the new true actor position
		M->actor_xyz.x = x;
		M->actor_xyz.z = z;

		// check for new parent box and if so bring barriers
		Prepare_megas_route_barriers(pl);
	}

	return (ret);
}

void _game_session::Animate_turn_to_pan(__mega_set_names anim_type, uint32 speedup) {
	// route manager turns character on the spot until facing route->target_pan
	// final pan may need to be snapped to avoid overrun
	// once done, x,z are restored to pre-turn animations - this is essential if character is to follow route correctly
	// there is no barrier check!

	// do a frames turn anim
	// have we finished
	// snap pan
	// restore x,z

	uint32 next_pc, info_pc;
	PXfloat this_pan_change;

	// set type
	L->cur_anim_type = anim_type;

	ANIM_CHECK(anim_type);

	// get anim set
	PXanim *pAnim = (PXanim *)rs_anims->Res_open(L->voxel_info->get_info_name(anim_type), L->voxel_info->info_name_hash[anim_type], L->voxel_info->base_path,
	                                             L->voxel_info->base_path_hash); //

	// anim pc may be illegal so neutralise it
	L->anim_pc = (L->anim_pc) % (pAnim->frame_qty - 1);

	// adjust the frame PC in the direction we're going to turn
	if (!M->turn_dir) {
		// turning right (clockwise)
		// anims are clockwise to run naturally through frames
		next_pc = (L->anim_pc + 1) % (pAnim->frame_qty - 1); // next frame to display
		info_pc = L->anim_pc + 1;                            // from to take movement info from
	} else {                                                     // pos
		// turning left
		// reverse the anim

		if (!L->anim_pc) {                                // pc==0
			info_pc = next_pc = pAnim->frame_qty - 2; // take movement info from dummy marker frame
			L->anim_pc = pAnim->frame_qty - 1;        // display last legal frame - have to force this for math to work
		} else {                                          // pc is non zero
			info_pc = next_pc = L->anim_pc - 1;       // display and movement info from next frame down
		}
	}

	if ((info_pc >= pAnim->frame_qty) || (next_pc >= pAnim->frame_qty))
		Fatal_error("Animate_turn_to_pan [%s] using illegal frame", CGameObject::GetName(object));

	// update engine pan with the difference between pan of previous frame and next frame
	PXreal pan1, pan2;

	// Get info_pc frame from the anim
	PXframe *infoFrame = PXFrameEnOfAnim(info_pc, pAnim);
	// Get current frame from the anim
	PXframe *currentFrame = PXFrameEnOfAnim(L->anim_pc, pAnim);

	PXmarker_PSX_Object::GetPan(&infoFrame->markers[ORG_POS], &pan1);
	PXmarker_PSX_Object::GetPan(&currentFrame->markers[ORG_POS], &pan2);

	this_pan_change = (pan1 - pan2); // update by difference

	// times the speedup for fast turn functions - normally 0
	this_pan_change *= speedup;

	if (this_pan_change >= HALF_TURN) {
		this_pan_change -= FULL_TURN;
	} else if (this_pan_change <= -HALF_TURN) {
		this_pan_change += FULL_TURN; // stop the wrap
	}

	if (PXfabs(this_pan_change) > M->target_pan) {
		// we are going to turn too much this go so clip

		L->pan = M->actual_target_pan; // clip to actual left

		M->target_pan = ZERO_TURN; // we're done

	} else {
		// still further to go

		L->pan += this_pan_change;

		M->target_pan = M->target_pan - (PXfloat)PXfabs(this_pan_change);
	}

	// get motion displacement from currently displayed frame to next one
	// note that we always read frame+1 for motion of next frame even though the voxel frame itself will be looped back to 0
	PXreal x1, x2, z1, z2, unused;

	// Get the next frame from the anim
	PXframe *nextFrame = PXFrameEnOfAnim(next_pc, pAnim);

	PXmarker_PSX_Object::GetXYZ(&nextFrame->markers[ORG_POS], &x2, &unused, &z2);

	// Note, assumes current frame hasn't changed i.e L->info_pc is same
	PXmarker_PSX_Object::GetXYZ(&currentFrame->markers[ORG_POS], &x1, &unused, &z1);

	// FIXME: xnext and znext are not used currently...
	//PXreal xnext = x2 - x1;
	//PXreal znext = z2 - z1;

	// update pc
	L->anim_pc = next_pc; // already computed

	// get the pan unwind value of the frame to be printed
	PXreal pan;

	// Note, L->anim_pc = next_pc
	PXmarker_PSX_Object::GetPan(&nextFrame->markers[ORG_POS], &pan);

	L->pan_adjust = pan;

	// FIXME: ang, cang and sang are not used currently...
	// calculate the new x and z coordinate from this frames motion offset
	// do the z and x together
	// PXfloat ang = (L->pan - L->pan_adjust) * TWO_PI;
	//PXfloat cang = (PXfloat)PXcos(ang);
	//PXfloat sang = (PXfloat)PXsin(ang);

	// FIXME: x and z are not used currently...
	//PXreal x = M->actor_xyz.x + PXfloat2PXreal(xnext * cang + znext * sang);
	//PXreal z = M->actor_xyz.z + PXfloat2PXreal(znext * cang - xnext * sang);
	// x and z are the new coordinates

	if (L->pan >= HALF_TURN)
		L->pan -= FULL_TURN;
	else if (L->pan <= -HALF_TURN)
		L->pan += FULL_TURN; // stop the wrap
}

bool8 _game_session::Calc_target_pan(PXreal x, PXreal z, PXreal x2, PXreal z2) {
	// will LOOK TOWARDS a shallow angle or set M->target_pan
	// not to be used by routing routines!

	// x,z are source coordinate
	// x2,z2 are target coordinate

	// returns   FALSE no turn required
	//				TRUE turn required

	PXfloat new_pan, diff;

	new_pan = PXAngleOfVector(z - z2, x - x2); // work out vector

	M->looking_pan = new_pan;

	// get difference between the two
	diff = new_pan - L->pan;

	if (PXfabs(diff) > (FULL_TURN / 10)) { // 0.1f
		// work out which way to turn
		if (diff > HALF_TURN)
			diff -= FULL_TURN;

		else if (diff < -HALF_TURN)
			diff += FULL_TURN;

		// diff is now the distance to turn by and its sign denotes direction

		if (diff < FLOAT_ZERO) {
			M->turn_dir = 0; // right
		} else {
			M->turn_dir = 1; // left
		}

		M->target_pan = (PXfloat)PXfabs(diff); // save positive pan distance

		M->actual_target_pan = new_pan; // actual target which we may clip to

		L->anim_pc = 0; // legal pc for first frame in turn anim - this needs thought

		// we face straight ahead
		I->lookBone.boneTarget.vz = (int16)(0);

		return (TRUE8);
	} else {
		// face towards object or whatever with upper body
		I->lookBone.boneNumber = 1;
		I->lookBone.boneSpeed = 128;
		I->lookBone.boneTarget.vz = (int16)(diff * (4096 / FULL_TURN)); // on psx diff is 0-4096 (full_turn), so we want diff*1     = diff * (4096/4096)
		// on pc diff is 0-1 (full_turn), we we want diff*4096      = diff * (4096/1)

		return (FALSE8);
	}
}

bool8 _game_session::Calc_target_pan_no_bones(PXreal x, PXreal z, PXreal x2, PXreal z2) {
	// will snap a shallow angle or set M->target_pan

	//	x,z are source coordinate
	// x2,z2 are target coordinate

	// returns   FALSE no turn required
	//				TRUE turn required

	PXfloat new_pan, diff;

	new_pan = PXAngleOfVector(z - z2, x - x2); // work out vector

	// get difference between the two
	diff = new_pan - L->pan;

	if (PXfabs(diff) > (FULL_TURN / 10)) { // 0.1f
		// work out which way to turn
		if (diff > HALF_TURN)
			diff -= FULL_TURN;

		else if (diff < -HALF_TURN)
			diff += FULL_TURN;

		// diff is now the distance to turn by and its sign denotes direction

		if (diff < FLOAT_ZERO) {
			M->turn_dir = 0; // right
		} else {
			M->turn_dir = 1; // left
		}

		M->target_pan = (PXfloat)PXfabs(diff); // save positive pan distance

		M->actual_target_pan = new_pan; // actual target which we may clip to

		L->anim_pc = 0; // legal pc for first frame in turn anim - this needs thought

		// we face straight ahead
		I->lookBone.boneTarget.vz = (int16)(0);

		return (TRUE8);
	} else {
		// shallow angle so snap and continue as normal
		L->pan = new_pan;

		return FALSE8;
	}
}

void _game_session::Soft_start_with_double_link(__mega_set_names link_one, __mega_set_names link_two, __mega_set_names next_anim) {
	// soft start from best os 2 link anims (if CAPS available) and push follow on animation

	int32 diff = 1000000; // a big number

	// is the link anim available?
	if (I->IsAnimTable(link_one)) {
		// push hard start follow on animation
		M->next_anim_type = next_anim;

		// will pick best
		diff = Soften_up_anim_file(link_one, diff);

		if (I->IsAnimTable(link_two)) {
			// may pick best
			Soften_up_anim_file(link_two, diff);
		}
	} else { // first link anim requested not available so skip the second and jump straight in
		// set anim type
		L->anim_pc = 0;
		L->cur_anim_type = next_anim;
	}
}

void _game_session::Soft_start_with_single_link(__mega_set_names link_anim, __mega_set_names next_anim) {
	// soft start a link anim (if CAPS available) and push follow on animation

	int32 diff = 1000000; // a big number

	// is the link anim available?
	if (I->IsAnimTable(link_anim)) {
		// will pick best
		Soften_up_anim_file(link_anim, diff);

		// push hard start follow on animation
		M->next_anim_type = next_anim;
	} else { // link anim requested but not available
		// set anim type
		L->anim_pc = 0;
		L->cur_anim_type = next_anim;
	}
}

void _game_session::Soft_start_single_anim(__mega_set_names next_anim) {
	// select best leg position for specified animation
	// there is no optional link
	// it HAS to be ok to skip past frames in the new anim
	// also sets cur_anim_type

	int32 diff = 1000000; // a big number

	// next anim not in CAPS
	if ((!L->voxel_info->IsAnimTable(next_anim))) {
		// can't go on without a major anim - this is not a link so can't just be skipped
		Shut_down_object("by Soft_start_single_anim next anim don't exist");
		return;
	}

	// will pick best
	Soften_up_anim_file(next_anim, diff);

	// set anim type
	L->cur_anim_type = next_anim;

	// nothing to move onto
	M->next_anim_type = __NO_ANIM;
}

void _game_session::Hard_start_single_anim(__mega_set_names next_anim) {
	// jumps straight into new anim on frame 0
	// sets cur_anim_type

	// next anim not in CAPS
	if ((!L->voxel_info->IsAnimTable(next_anim))) {
		// can't go on without a major anim - this is not a link so can't just be skipped
		Shut_down_object("by Hard_start_single_anim next anim don't exist");
		return;
	}

	// hard start on frame 0
	L->anim_pc = 0;

	// set anim type
	L->cur_anim_type = next_anim;

	// nothing to move onto
	M->next_anim_type = __NO_ANIM;
}

int32 _game_session::Soften_up_anim_file(__mega_set_names link, int32 diff) {
	// pick best frame in passed anim compared to best so far passed in 'diff'
	uint32 old_leg_pos;
	int32 j;

	// Jake check the anim exists / make its name
	ANIM_CHECK(L->cur_anim_type);

	// Do this first to make the whole res_open defrag thing work nicely
	PXanim *pCur_Anim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash);

	// find out leg position for current frame
	old_leg_pos = FROM_LE_16(PXFrameEnOfAnim(L->anim_pc, pCur_Anim)->left_foot_distance);

	// Jake check the link anim exists / make its name
	ANIM_CHECK(link);
	PXanim *pLnk_Anim = (PXanim *)rs_anims->Res_open(I->get_info_name(link), I->info_name_hash[link], I->base_path, I->base_path_hash); //
	// special check for only one frame
	if (pLnk_Anim->frame_qty == 1) {
		L->cur_anim_type = link; // set type
		L->anim_pc = 0;          // first frame
		return (diff);
	}

	// see which has the closest leg position
	for (j = 0; j < (pLnk_Anim->frame_qty - 1); j++) {
		int32 foot = FROM_LE_16(PXFrameEnOfAnim(j, pLnk_Anim)->left_foot_distance);
		int32 d = twabs(foot - old_leg_pos);

		if (d < diff) {
			diff = d;
			L->cur_anim_type = link; // set type
			L->anim_pc = j;          // this frame is best so far
		}
	}

	// return best diff so far incase we're coming back on a multiple link anim scheme
	return (diff);
}

bool8 _game_session::Play_anim() {
	// plays anim until finished
	// keeps playing anim even if motion has been stopped
	// when current is done it will play the queued M->next_anim_type if it exists. When that's done we're done

	// returns   1 when done
	//				0 we need to come back next cycle

	// get animation
	PXanim *pAnim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash);

	// last frame currently displayed?
	if ((int32)(L->anim_pc + 1) == (pAnim->frame_qty - 1)) {
		// we displayed the last frame last cycle - so display first of new mode this cycle
		// we may have been playing a link anim - so check for a new anim waiting in M->next_anim_type
		if (M->next_anim_type == __NO_ANIM) {
			// we're finished
			return (TRUE8);
		}

		// reset pc ready for whatever starts next
		// current must be link so we go into new on frame 0
		L->anim_pc = 0;

		// ok, there is a link anim
		L->cur_anim_type = M->next_anim_type;
		M->next_anim_type = __NO_ANIM; // must clear this or it'll play forever

		// need to come back
		return (FALSE8);
	}

	// shift character and frame forward by the amount appropriate
	Advance_frame_and_motion(L->cur_anim_type, 0, 1);

	// need to come back
	return (FALSE8);
}

bool8 _game_session::Play_reverse_anim() {
	// plays anim until finished
	// keeps playing anim even if motion has been stopped
	// when current is done it will play the queued M->next_anim_type if it exists. When that's done we're done

	// returns   1 when done
	//				0 we need to come back next cycle
	Zdebug("Play_reverse_anim");

	// last frame currently displayed?
	if (!L->anim_pc) {
		// we displayed the last frame last cycle - so display first of new mode this cycle

		// we may have been playing a link anim - so check for a new anim waiting in M->next_anim_type
		if (M->next_anim_type == __NO_ANIM) {
			Zdebug("done");
			// we're finished
			return (TRUE8);
		}

		// reset pc ready for whatever starts next
		// current must be link so we go into new on frame 0
		L->anim_pc = 0;

		// ok, there is a link anim
		L->cur_anim_type = M->next_anim_type;
		M->next_anim_type = __NO_ANIM; // must clear this or it'll play forever

		// need to come back
		return (FALSE8);
	}

	// shift character and frame forward by the amount appropriate
	Reverse_frame_and_motion(L->cur_anim_type, 0, 1);

	Zdebug("~Reverse");

	// need to come back
	return (FALSE8);
}

bool8 _game_session::Play_anim_with_no_movement() {
	// plays anim until finished
	// keeps playing anim even if motion has been stopped
	// when current is done it will play the queued M->next_anim_type if it exists. When that's done we're done

	// returns   1 when done
	//			0 we need to come back next cycle

	// get animation
	PXanim *pAnim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash); //

	// last frame currently displayed?
	if ((int32)(L->anim_pc + 1) == (pAnim->frame_qty - 1)) {
		// we displayed the last frame last cycle - so display first of new mode this cycle

		// we may have been playing a link anim - so check for a new anim waiting in M->next_anim_type
		if (M->next_anim_type == __NO_ANIM) {
			// we're finished
			return (TRUE8);
		}

		// reset pc ready for whatever starts next
		// current must be link so we go into new on frame 0
		L->anim_pc = 0;

		// ok, there is a link anim
		L->cur_anim_type = M->next_anim_type;
		M->next_anim_type = __NO_ANIM; // must clear this or it'll play forever

		// need to come back
		return (FALSE8);
	}

	// shift character and frame forward by the amount appropriate

	// ok, advance frame but not position
	L->anim_pc = (L->anim_pc + 1) % (pAnim->frame_qty - 1);

	// need to come back
	return (FALSE8);
}

void _game_session::Set_motion(__motion motion) {
	// set motion type for cur object
	M->motion = motion;
}

__motion _game_session::Get_motion() {
	// get motion type for cur object
	return (M->motion);
}

void _game_session::Set_pose(__weapon weapon) {
	// set weapon type for cur object
	M->weapon = weapon;
}

void _game_session::Change_pose_in_current_anim_set() {
	// change the weapon set
	// delete an old one
	// we very much expect a set to already have been picked by the script
	// its likely that this will only be called by the player arm/unarm code

	// create _vox_image object
	logic_structs[cur_id]->voxel_info->___init(M->chr_name, M->anim_set, logic_structs[cur_id]->mega->Fetch_pose()); // we pass the person, set names through
}

uint32 _game_session::Fetch_last_frame(__mega_set_names anima) {
	// return final frame number in animation
	if ((!I->IsAnimTable(anima))) {
		Fatal_error("Fetch_last_frame can't access illegal anim [%s]", master_anim_name_table[anima].name);
	}

	PXanim *pAnim = (PXanim *)rs_anims->Res_open(I->get_info_name(anima), I->info_name_hash[anima], I->base_path, I->base_path_hash);

	return (pAnim->frame_qty - 1);
}

} // End of namespace ICB
