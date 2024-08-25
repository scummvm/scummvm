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
#include "engines/icb/debug.h"
#include "engines/icb/p4_generic.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/common/px_game_object.h"
#include "engines/icb/icb.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/floors.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/sound.h"
#include "engines/icb/res_man.h"

namespace ICB {

mcodeFunctionReturnCodes fn_apply_bullet(int32 &result, int32 *params) { return (g_mission->session->fn_apply_bullet(result, params)); }

mcodeFunctionReturnCodes fn_set_to_last_frame_generic_anim(int32 &result, int32 *params) { return (MS->fn_set_to_last_frame_generic_anim(result, params)); }

mcodeFunctionReturnCodes fn_play_custom_anim(int32 &result, int32 *params) { return (MS->fn_play_custom_anim(result, params)); }

mcodeFunctionReturnCodes fn_easy_play_generic_anim(int32 &result, int32 *params) { return (MS->fn_easy_play_generic_anim(result, params)); }

mcodeFunctionReturnCodes fn_easy_play_custom_anim(int32 &result, int32 *params) { return (MS->fn_easy_play_custom_anim(result, params)); }

mcodeFunctionReturnCodes fn_snap_face_object(int32 &result, int32 *params) { return (MS->fn_snap_face_object(result, params)); }

mcodeFunctionReturnCodes fn_face_coord(int32 &result, int32 *params) { return (MS->fn_face_coord(result, params)); }

mcodeFunctionReturnCodes fn_face_object(int32 &result, int32 *params) { return (MS->fn_face_object(result, params)); }

mcodeFunctionReturnCodes fn_set_to_first_frame_custom_anim(int32 &result, int32 *params) { return (MS->fn_set_to_first_frame_custom_anim(result, params)); }

mcodeFunctionReturnCodes fn_set_to_last_frame_custom_anim(int32 &result, int32 *params) { return (MS->fn_set_to_last_frame_custom_anim(result, params)); }

mcodeFunctionReturnCodes fn_new_apply_bullet(int32 &result, int32 *params) { return (MS->fn_new_apply_bullet(result, params)); }

mcodeFunctionReturnCodes fn_play_generic_anim(int32 &result, int32 *params) { return (MS->fn_play_generic_anim(result, params)); }

mcodeFunctionReturnCodes fn_reverse_generic_anim(int32 &result, int32 *params) { return (MS->fn_reverse_generic_anim(result, params)); }

mcodeFunctionReturnCodes fn_apply_anim_y(int32 &result, int32 *params) { return (MS->fn_apply_anim_y(result, params)); }

mcodeFunctionReturnCodes fn_set_to_first_frame_generic_anim(int32 &result, int32 *params) { return (MS->fn_set_to_first_frame_generic_anim(result, params)); }

mcodeFunctionReturnCodes fn_easy_play_generic_anim_with_pan(int32 &result, int32 *params) { return (MS->fn_easy_play_generic_anim_with_pan(result, params)); }

mcodeFunctionReturnCodes fn_fast_face_object(int32 &result, int32 *params) { return (MS->fn_fast_face_object(result, params)); }

mcodeFunctionReturnCodes fn_face_nicos_pan(int32 &result, int32 *params) { return (MS->fn_face_nicos_pan(result, params)); }

mcodeFunctionReturnCodes fn_add_y(int32 &result, int32 *params) { return (MS->fn_add_y(result, params)); }

mcodeFunctionReturnCodes fn_reverse_custom_anim(int32 &result, int32 *params) { return (MS->fn_reverse_custom_anim(result, params)); }

mcodeFunctionReturnCodes fn_fast_face_coord(int32 &result, int32 *params) { return (MS->fn_fast_face_coord(result, params)); }

mcodeFunctionReturnCodes fn_easy_play_custom_anim_with_pan(int32 &result, int32 *params) { return (MS->fn_easy_play_custom_anim_with_pan(result, params)); }

mcodeFunctionReturnCodes fn_prime_custom_anim(int32 &result, int32 *params) { return (MS->fn_prime_custom_anim(result, params)); }

mcodeFunctionReturnCodes fn_sync_with_mega(int32 &result, int32 *params) { return (MS->fn_sync_with_mega(result, params)); }

mcodeFunctionReturnCodes fn_set_feet_to_pan(int32 &result, int32 *params) { return (MS->fn_set_feet_to_pan(result, params)); }

mcodeFunctionReturnCodes fn_hard_load_generic_anim(int32 &result, int32 *params) { return (MS->fn_hard_load_generic_anim(result, params)); }

mcodeFunctionReturnCodes fn_hard_load_custom_anim(int32 &result, int32 *params) { return (MS->fn_hard_load_custom_anim(result, params)); }

mcodeFunctionReturnCodes fn_face_camera(int32 &result, int32 *params) { return (MS->fn_face_camera(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_face_camera(int32 &, int32 *params) {
	//	params   0   face away from camera
	//				1 face toward camera

	PXfloat new_pan, diff;
	PXcamera currentCamera;

	if (!L->looping) {
		currentCamera = GetCamera();
		new_pan = (PXfloat)currentCamera.pan;

		// reverse 180deg?
		if (params[1])
			new_pan += HALF_TURN;

		if (new_pan > HALF_TURN)
			new_pan -= FULL_TURN;

		else if (new_pan < -HALF_TURN)
			new_pan += FULL_TURN;

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

			L->looping = TRUE8;
		} else {
			// shallow angle so snap and continue as normal
			L->pan = new_pan;
			return IR_CONT;
		}
	}

	// still got some to go
	if (M->target_pan) {
		Animate_turn_to_pan(__TURN_ON_THE_SPOT_CLOCKWISE, 1);
		return (IR_REPEAT);
	}

	// we're done
	L->looping = FALSE8;

	// set to stand
	L->cur_anim_type = __STAND;
	L->anim_pc = 0;

	return (IR_CONT);
}

// fn_set_feet_to_pan()
// ensures actual pan is what we are looking at
mcodeFunctionReturnCodes _game_session::fn_set_feet_to_pan(int32 &, int32 *) {
	// we face straight ahead
	I->lookBone.boneTarget.vz = (int16)(0);

	// and our pan is set to the looking_pan
	L->pan = M->looking_pan;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_face_coord(int32 &, int32 *params) {
	//	params   0 target x
	//				1 target z

	// return    IR_CONT or
	//			IR_REPEAT

	if (!L->looping) {
		// setup

		if (Calc_target_pan((PXreal)params[0], (PXreal)params[1], L->mega->actor_xyz.x, L->mega->actor_xyz.z)) {
			// turn required

			L->looping = TRUE8;
		} else
			return (IR_CONT); // will have possibly made a tiny snap
	}

	// still got some to go
	if (M->target_pan) {
		Animate_turn_to_pan(__TURN_ON_THE_SPOT_CLOCKWISE, 1);
		return (IR_REPEAT);
	}

	// we're done
	L->looping = FALSE8;

	// set to stand
	L->cur_anim_type = __STAND;
	L->anim_pc = 0;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_face_nicos_pan(int32 &, int32 *params) {
	//	params   0   target nico
	//				1 reserved for future use

	// return    IR_CONT or
	//			IR_REPEAT

	_feature_info *start_pos;
	PXfloat new_pan, diff;

	const char *nico_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) {
		// setup
		start_pos = (_feature_info *)LinkedDataObject::Try_fetch_item_by_name(features, nico_name);
		if (!start_pos)
			Fatal_error("no NICO marker (fn_face_nico) ob %s, nico %s", CGameObject::GetName(object), nico_name);

		new_pan = start_pos->direction;

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

			L->looping = TRUE8;
		} else {
			//      shallow angle so snap and continue as normal
			L->pan = new_pan;
			return IR_CONT;
		}
	}

	// still got some to go
	if (M->target_pan) {
		Animate_turn_to_pan(__TURN_ON_THE_SPOT_CLOCKWISE, 1);
		return (IR_REPEAT);
	}

	// we're done
	L->looping = FALSE8;

	// set to stand
	L->cur_anim_type = __STAND;
	L->anim_pc = 0;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_face_object(int32 &, int32 *params) {
	//	params   target object

	// return    IR_CONT or
	//			IR_REPEAT

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) {
		// setup
		_logic *log;

		uint32 id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);

		log = Fetch_object_struct(id);

		if (log->image_type == PROP) {
			if (Calc_target_pan(log->prop_xyz.x, log->prop_xyz.z, M->actor_xyz.x, M->actor_xyz.z)) {
				// turn required
				L->looping = TRUE8;
			} else
				return (IR_CONT); // will have possibly made a tiny snap

		} else {
			if (Calc_target_pan(log->mega->actor_xyz.x, log->mega->actor_xyz.z, L->mega->actor_xyz.x, L->mega->actor_xyz.z)) {
				// turn required
				L->looping = TRUE8;
			} else
				return (IR_CONT); // will have possibly made a tiny snap
		}
	}

	// still got some to go
	if (M->target_pan) {
		Animate_turn_to_pan(__TURN_ON_THE_SPOT_CLOCKWISE, 1);
		return (IR_REPEAT);
	}

	// we're done
	L->looping = FALSE8;

	// set to stand
	L->cur_anim_type = __STAND;
	L->anim_pc = 0;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_fast_face_object(int32 &, int32 *params) {
	//	params   0 target object
	//				1 speed up

	// return    IR_CONT or
	//			IR_REPEAT

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) {
		// setup
		_logic *log;

		uint32 id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);

		log = Fetch_object_struct(id);

		if (log->image_type == PROP) {
			if (Calc_target_pan(log->prop_xyz.x, log->prop_xyz.z, M->actor_xyz.x, M->actor_xyz.z)) {
				//          turn required
				L->looping = TRUE8;
			} else
				return (IR_CONT); // will have possibly made a tiny snap
		} else {
			if (Calc_target_pan(log->mega->actor_xyz.x, log->mega->actor_xyz.z, L->mega->actor_xyz.x, L->mega->actor_xyz.z)) {
				//          turn required
				L->looping = TRUE8;
			} else
				return (IR_CONT); // will have possibly made a tiny snap
		}
	}

	// still got some to go
	if (M->target_pan) {
		Animate_turn_to_pan(__TURN_ON_THE_SPOT_CLOCKWISE, params[1]);
		return (IR_REPEAT);
	}

	// we're done
	L->looping = FALSE8;

	// set to stand
	L->cur_anim_type = __STAND;
	L->anim_pc = 0;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_fast_face_coord(int32 &, int32 *params) {
	//	params   0 x
	//				1 z
	//				2 speed up

	// return    IR_CONT or
	//			IR_REPEAT

	if (!L->looping) {
		if (Calc_target_pan((PXreal)params[0], (PXreal)params[1], L->mega->actor_xyz.x, L->mega->actor_xyz.z)) {
			// turn required
			L->looping = TRUE8;
		} else
			return (IR_CONT); // will have possibly made a tiny snap
	}

	// still got some to go
	if (M->target_pan) {
		Animate_turn_to_pan(__TURN_ON_THE_SPOT_CLOCKWISE, params[2]);
		return (IR_REPEAT);
	}

	// we're done
	L->looping = FALSE8;

	// set to stand
	L->cur_anim_type = __STAND;
	L->anim_pc = 0;

	return (IR_CONT);
}

bool8 _game_session::Need_to_turn_to_face_object(uint32 id) {
	// is a turn required to face an object
	// used by chi

	_logic *log;

	log = Fetch_object_struct(id);

	if (log->image_type == PROP)
		Fatal_error("fast_face_object = target must be mega");

	if (Calc_target_pan(log->mega->actor_xyz.x, log->mega->actor_xyz.z, L->mega->actor_xyz.x, L->mega->actor_xyz.z)) {
		// turn required
		return TRUE8;
	}

	return (FALSE8); // may even have made a tiny snap
}

bool8 _game_session::fast_face_object(uint32 id, uint32 speed) {
	// called in engine
	// for example from chi logic

	if (!L->looping) {
		// setup
		_logic *log;

		log = Fetch_object_struct(id);

		if (log->image_type == PROP)
			Fatal_error("fast_face_object = target must be mega");

		if (Calc_target_pan(log->mega->actor_xyz.x, log->mega->actor_xyz.z, L->mega->actor_xyz.x, L->mega->actor_xyz.z)) {
			// turn required
			L->looping = TRUE8;
		} else
			return (TRUE8); // will have possibly made a tiny snap
	}

	// still got some to go
	if (M->target_pan) {
		Animate_turn_to_pan(__TURN_ON_THE_SPOT_CLOCKWISE, speed);
		return (FALSE8);
	}

	// we're done
	L->looping = FALSE8;

	// set to stand
	L->cur_anim_type = __STAND;
	L->anim_pc = 0;

	return (TRUE8);
}

bool8 _game_session::fast_face_rnd(uint32 speed) {
	PXfloat new_pan, diff;

	if (!L->looping) {
		// pick a random pan
		new_pan = (FULL_TURN * (g_icb->getRandomSource()->getRandomNumber(359 - 1))) / 360;

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

			L->looping = TRUE8;

		} else
			return TRUE8; // random was too tiny to bother
	}

	// still got some to go
	if (M->target_pan) {
		Animate_turn_to_pan(__TURN_ON_THE_SPOT_CLOCKWISE, speed);
		return (FALSE8);
	}

	// we're done
	L->looping = FALSE8;

	// set to stand
	L->cur_anim_type = __STAND;
	L->anim_pc = 0;

	return (TRUE8);
}

mcodeFunctionReturnCodes _game_session::fn_snap_face_object(int32 &, int32 *params) {
	// force pan - no animation

	// params        0 target object
	// return        IR_CONT

	_logic *log;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("fn_snap_face_object [%s]", object_name);

	uint32 id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);

	if (id == 0xffffffff)
		Fatal_error("fn_snap_face_object can't find target object %s", object_name);

	log = Fetch_object_struct(id);

	if (log->image_type == PROP) {
		if (Calc_target_pan(log->prop_xyz.x, log->prop_xyz.z, M->actor_xyz.x, M->actor_xyz.z)) {
			L->pan = M->actual_target_pan;
			M->actual_target_pan = REAL_ZERO;
		}
	} else {
		if (Calc_target_pan(log->mega->actor_xyz.x, log->mega->actor_xyz.z, L->mega->actor_xyz.x, L->mega->actor_xyz.z)) {
			L->pan = M->actual_target_pan;
			M->actual_target_pan = REAL_ZERO;
		}
	}

	return (IR_CONT);
}

bool8 _game_session::speech_face_object(uint32 tar_id) {
	// custom system for speech

	// return    TRUE8 - more to do
	//			FALSE - done
	bool8 res;

	if (!L->looping) {
		// setup
		_logic *log;

		log = Fetch_object_struct(tar_id);

		if (log->image_type == VOXEL)
			res = Calc_target_pan(log->mega->actor_xyz.x, log->mega->actor_xyz.z, L->mega->actor_xyz.x, L->mega->actor_xyz.z);

		else
			res = Calc_target_pan(log->prop_xyz.x, log->prop_xyz.z, L->mega->actor_xyz.x, L->mega->actor_xyz.z);

		if (res) {
			// turn required
			L->looping = TRUE8;
		} else { // will have possibly made a tiny snap
			// set to stand
			L->cur_anim_type = __STAND;
			L->anim_pc = 0;

			return (FALSE8); // done!
		}
	}

	// still got some to go
	if (M->target_pan) {
		Animate_turn_to_pan(__TURN_ON_THE_SPOT_CLOCKWISE, 1);
		return (TRUE8); // more to do
	}

	// we're done
	L->looping = FALSE8;

	// set to stand
	L->cur_anim_type = __STAND;
	L->anim_pc = 0;

	Zdebug(" finished");

	return (FALSE8); // done
}

mcodeFunctionReturnCodes _game_session::fn_reverse_generic_anim(int32 &, int32 *params) {
	//	params   0 ascii name of anim

	// return    IR_CONT or
	//			IR_REPEAT

	bool8 ret;

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) {
		// setup
		M->next_anim_type = Fetch_generic_anim_from_ascii(anim_name);
		L->looping = 100;
		ANIM_CHECK(M->next_anim_type);
		L->list[0] = HashString(anim_name);
	}

	if (L->looping == 100) {
		// setup
		// psx async loading check - is file in memory
		if (!rs_anims->Res_open(I->get_info_name(M->next_anim_type), I->info_name_hash[M->next_anim_type], I->base_path, I->base_path_hash))
			return IR_REPEAT;

		if ((Object_visible_to_camera(cur_id)) &&
		    (!rs_anims->Res_open(I->get_anim_name(M->next_anim_type), I->anim_name_hash[M->next_anim_type], I->base_path, I->base_path_hash)))
			return IR_REPEAT;

		L->cur_anim_type = M->next_anim_type; // anim now in memory

		L->looping = TRUE8;

		PXanim *anim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash); //

		// set last frame
		L->anim_pc = anim->frame_qty - 2;

		return (IR_REPEAT);
	}

	// ok, we are simply animating through the frames
	// last frame is currently displayed?
	if (!L->anim_pc) {
		L->looping = FALSE8;
		return (IR_CONT);
	}

	// shift character and frame forward by the amount appropriate
	ret = MS->Reverse_frame_and_motion(L->cur_anim_type, 0, M->anim_speed);
	if (!ret) { // could not move forward?
		L->looping = FALSE8;
		return (IR_CONT);
	}

	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_play_generic_anim(int32 &, int32 *params) {
	//	params   0 ascii name of anim

	// return    IR_CONT or
	//			IR_REPEAT

	bool8 ret;

	const char *anim_name = nullptr;
	if (params && params[0]) {
		anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	}

	if (!L->looping) {
		// setup
		M->next_anim_type = Fetch_generic_anim_from_ascii(anim_name);
		L->looping = 100;
		ANIM_CHECK(M->next_anim_type);
		L->list[0] = HashString(anim_name);
	}

	if (L->looping == 100) {
		// psx async loading check - is file in memory
		if (!rs_anims->Res_open(I->get_info_name(M->next_anim_type), I->info_name_hash[M->next_anim_type], I->base_path, I->base_path_hash))
			return IR_REPEAT;

		if ((Object_visible_to_camera(cur_id)) &&
		    (!rs_anims->Res_open(I->get_anim_name(M->next_anim_type), I->anim_name_hash[M->next_anim_type], I->base_path, I->base_path_hash)))
			return IR_REPEAT;

		// anim found and started ok
		L->looping = TRUE8;

		L->cur_anim_type = M->next_anim_type; // anim now in memory

		// get animation
		ANIM_CHECK(L->cur_anim_type);

		PXanim *anim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash); //

		// advance the frame
		L->anim_pc = anim->frame_qty - 2;
		Advance_frame_and_motion(L->cur_anim_type, 0, 1);
		L->anim_pc = 0;

		return (IR_REPEAT);
	}

	// ok, we are simply animating through the frames

	PXanim *anim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash); //

	// last frame is currently displayed?
	if ((int32)(L->anim_pc + M->anim_speed) >= (anim->frame_qty - 1)) {
		L->looping = FALSE8;
		return (IR_CONT);
	}

	// shift character and frame forward by the amount appropriate
	ret = Advance_frame_and_motion(L->cur_anim_type, 0, M->anim_speed);

	if (!ret) { // could not move forward?
		L->looping = FALSE8;
		return (IR_CONT);
	}

	// more to do - come back again next cycle
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_easy_play_generic_anim(int32 &, int32 *params) {
	// barriers are ignored!

	//	params   0 ascii name of anim

	// return    IR_CONT or
	//			IR_REPEAT

	const char *anim_name = nullptr;
	if (params && params[0]) {
		anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	}

	if (!L->looping) {
		// setup
		M->next_anim_type = Fetch_generic_anim_from_ascii(anim_name);
		L->looping = 100;
		ANIM_CHECK(M->next_anim_type);
		L->list[0] = HashString(anim_name);
	}

	if (L->looping == 100) {
		// setup
		// psx async loading check - is file in memory
		if (!rs_anims->Res_open(I->get_info_name(M->next_anim_type), I->info_name_hash[M->next_anim_type], I->base_path, I->base_path_hash))
			return IR_REPEAT;

		if ((Object_visible_to_camera(cur_id)) &&
		    (!rs_anims->Res_open(I->get_anim_name(M->next_anim_type), I->anim_name_hash[M->next_anim_type], I->base_path, I->base_path_hash)))
			return IR_REPEAT;

		L->cur_anim_type = M->next_anim_type; // anim now in memory
		L->looping = TRUE8;

		// advance the frame
		// get animation
		ANIM_CHECK(L->cur_anim_type);

		PXanim *anim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash); //

		L->anim_pc = anim->frame_qty - 2;
		Easy_frame_and_motion(L->cur_anim_type, 0, 1);
		L->anim_pc = 0;

		return (IR_REPEAT);
	}

	// ok, we are simply animating through the frames

	// get animation
	ANIM_CHECK(L->cur_anim_type);

	PXanim *anim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash);

	// last frame is currently displayed?
	if ((int32)(L->anim_pc + M->anim_speed) >= (anim->frame_qty - 1)) {
		L->looping = FALSE8;
		return (IR_CONT);
	}

	// shift character and frame forward by the amount appropriate
	MS->Easy_frame_and_motion(L->cur_anim_type, 0, M->anim_speed);

	// more to do - come back again next cycle
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_easy_play_generic_anim_with_pan(int32 &, int32 *params) {
	// barriers are ignored!

	//	params   0 ascii name of anim

	// return    IR_CONT or
	//			IR_REPEAT

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) {
		// setup
		M->next_anim_type = Fetch_generic_anim_from_ascii(anim_name);
		L->looping = 100;
		ANIM_CHECK(M->next_anim_type);
		L->list[0] = HashString(anim_name);
	}

	if (L->looping == 100) {
		// setup

		// psx async loading check - is file in memory
		if (!rs_anims->Res_open(I->get_info_name(M->next_anim_type), I->info_name_hash[M->next_anim_type], I->base_path, I->base_path_hash))
			return IR_REPEAT;

		if ((Object_visible_to_camera(cur_id)) &&
		    (!rs_anims->Res_open(I->get_anim_name(M->next_anim_type), I->anim_name_hash[M->next_anim_type], I->base_path, I->base_path_hash)))
			return IR_REPEAT;

		L->cur_anim_type = M->next_anim_type; // anim now in memory

		L->looping = TRUE8;

		// advance the frame
		// get animation
		ANIM_CHECK(L->cur_anim_type);

		PXanim *anim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash); //

		L->anim_pc = anim->frame_qty - 2;
		Easy_frame_motion_and_pan(L->cur_anim_type, 0);
		L->anim_pc = 0;

		return (IR_REPEAT);
	}

	// ok, we are simply animating through the frames

	// get animation
	ANIM_CHECK(L->cur_anim_type);

	PXanim *anim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash); //

	// last frame is currently displayed?
	if ((int32)(L->anim_pc + 1) == (anim->frame_qty - 1)) {
		L->looping = FALSE8;
		return (IR_CONT);
	}

	// shift character and frame forward by the amount appropriate
	MS->Easy_frame_motion_and_pan(L->cur_anim_type, 0);

	// more to do - come back again next cycle
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_easy_play_custom_anim_with_pan(int32 &, int32 *params) {
	// barriers are ignored!
	// pan is updated

	//	params   0 ascii name of anim

	// return    IR_CONT or
	//			IR_REPEAT

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	Zdebug("fn_easy_play_custom_anim_with_pan %s %s", CGameObject::GetName(object), anim_name);

	if (!L->looping) {
		// set anim up
		I->Init_custom_animation(anim_name);
		Reset_cur_megas_custom_type();
		L->looping = 100; // have to distinguish between first time in and first cycle with anim in memory
		ANIM_CHECK(__NON_GENERIC);
		L->list[0] = HashString(anim_name);
	}

	// anim is in memory so do first frame then pass on to normal logic path
	if (L->looping == 100) {
		// psx async loading check - is file in memory
		if (!rs_anims->Res_open(I->get_info_name(__NON_GENERIC), I->info_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash))
			return IR_REPEAT;

		if ((Object_visible_to_camera(cur_id)) &&
		    (!rs_anims->Res_open(I->get_anim_name(__NON_GENERIC), I->anim_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash)))
			return IR_REPEAT;

		I->Promote_non_generic(); // swap out the _NON_GENERIC to safe place
		L->cur_anim_type = __PROMOTED_NON_GENERIC;

		L->anim_pc = 0;
		L->looping = TRUE8;

		return (IR_REPEAT);
	}

	// ok, we are simply animating through the frames

	PXanim *anim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash); //

	// last frame is currently displayed?
	if ((int32)(L->anim_pc + 1) == (anim->frame_qty - 1)) {
		L->looping = FALSE8;

		return (IR_CONT);
	}

	// shift character and frame forward by the amount appropriate
	MS->Easy_frame_motion_and_pan(L->cur_anim_type, 0);

	// more to do - come back again next cycle
	return (IR_REPEAT);
}

mcodeFunctionReturnCodes _game_session::fn_set_to_last_frame_generic_anim(int32 &, int32 *params) {
	//	params   0 ascii name of anim

	// return    IR_CONT or IR_STOP if error

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) {
		M->next_anim_type = Fetch_generic_anim_from_ascii(anim_name);
		L->looping = 100;
		ANIM_CHECK(M->next_anim_type);
		L->list[0] = HashString(anim_name);
	}

	// psx async loading check - is file in memory
	if (!rs_anims->Res_open(I->get_info_name(M->next_anim_type), I->info_name_hash[M->next_anim_type], I->base_path, I->base_path_hash))
		return IR_REPEAT;

	if ((Object_visible_to_camera(cur_id)) &&
	    (!rs_anims->Res_open(I->get_anim_name(M->next_anim_type), I->anim_name_hash[M->next_anim_type], I->base_path, I->base_path_hash)))
		return IR_REPEAT;

	L->cur_anim_type = M->next_anim_type; // anim now in memory

	// ok, set to last frame
	// get animation
	ANIM_CHECK(L->cur_anim_type);

	PXanim *anim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash); //

	// set to last frame
	L->anim_pc = anim->frame_qty - 2; // if 10 frames then 10+1 (looper) == 11 meaning 9 is last displayable frame number
	L->looping = 0;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_set_to_first_frame_generic_anim(int32 &, int32 *params) {
	//	params   0 ascii name of anim

	// return    IR_CONT or IR_STOP if error

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) {
		M->next_anim_type = Fetch_generic_anim_from_ascii(anim_name);
		L->looping = 100;
		ANIM_CHECK(M->next_anim_type);
	}

	// psx async loading check - is file in memory
	if (!rs_anims->Res_open(I->get_info_name(M->next_anim_type), I->info_name_hash[M->next_anim_type], I->base_path, I->base_path_hash))
		return IR_REPEAT;

	if ((Object_visible_to_camera(cur_id)) &&
	    (!rs_anims->Res_open(I->get_anim_name(M->next_anim_type), I->anim_name_hash[M->next_anim_type], I->base_path, I->base_path_hash)))
		return IR_REPEAT;

	L->cur_anim_type = M->next_anim_type; // anim now in memory

	// set to last frame
	L->anim_pc = 0; // first
	L->looping = 0;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_to_first_frame_custom_anim(int32 &, int32 *params) {
	// set to first frame of a custom animation
	// must use fn-set-custom before this

	// params        0   name of anim

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) { // once
		I->Init_custom_animation(anim_name);
		L->looping = 1;
		ANIM_CHECK(__NON_GENERIC);
	}

	// psx async loading check - is file in memory
	if (!rs_anims->Res_open(I->get_info_name(__NON_GENERIC), I->info_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash))
		return IR_REPEAT;

	if ((Object_visible_to_camera(cur_id)) && (!rs_anims->Res_open(I->get_anim_name(__NON_GENERIC), I->anim_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash)))
		return IR_REPEAT;

	I->Promote_non_generic(); // swap out the _NON_GENERIC to safe place
	L->cur_anim_type = __PROMOTED_NON_GENERIC;

	L->anim_pc = 0;
	L->looping = 0;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_set_to_last_frame_custom_anim(int32 &, int32 *params) {
	// set to last frame of a custom animation
	// must use fn-set-custom before this

	// params        0   name of anim

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) { // once
		I->Init_custom_animation(anim_name);
		L->looping = 1;
		ANIM_CHECK(__NON_GENERIC);
	}

	// psx async loading check - is file in memory
	if (!rs_anims->Res_open(I->get_info_name(__NON_GENERIC), I->info_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash))
		return IR_REPEAT;

	if ((Object_visible_to_camera(cur_id)) && (!rs_anims->Res_open(I->get_anim_name(__NON_GENERIC), I->anim_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash)))
		return IR_REPEAT;

	I->Promote_non_generic(); // swap out the _NON_GENERIC to safe place
	L->cur_anim_type = __PROMOTED_NON_GENERIC;

	PXanim *anim = (PXanim *)rs_anims->Res_open(I->get_info_name(L->cur_anim_type), I->info_name_hash[L->cur_anim_type], I->base_path, I->base_path_hash);

	// set to last frame
	L->anim_pc = anim->frame_qty - 2; // if 10 frames then 10+1 (looper) == 11 meaning 9 is last displayable frame number

	L->looping = 0;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_hard_load_generic_anim(int32 &, int32 *params) {
	// get generic anim into memory NOW

	// params    0   name of anim
	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	__mega_set_names load;

	load = Fetch_generic_anim_from_ascii(anim_name);
	ANIM_CHECK(load);
	rs_anims->Res_open(I->get_info_name(load), I->info_name_hash[load], I->base_path, I->base_path_hash);

	if (Object_visible_to_camera(cur_id))
		rs_anims->Res_open(I->get_anim_name(load), I->anim_name_hash[load], I->base_path, I->base_path_hash);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_hard_load_custom_anim(int32 &, int32 *params) {
	// get custom anim into memory NOW

	// params    0   name of anim

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	I->Init_custom_animation(anim_name);
	Reset_cur_megas_custom_type();

	ANIM_CHECK(__NON_GENERIC);
	rs_anims->Res_open(I->get_info_name(__NON_GENERIC), I->info_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash); //

	if (Object_visible_to_camera(cur_id))
		rs_anims->Res_open(I->get_anim_name(__NON_GENERIC), I->anim_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_prime_custom_anim(int32 &, int32 *params) {
	// get custom anim into memory

	// params    0   name of anim

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) {
		I->Init_custom_animation(anim_name);
		Reset_cur_megas_custom_type();

		L->looping = 100;

		ANIM_CHECK(__NON_GENERIC);
	}

	// anim is in memory so do first frame then pass on to normal logic path
	if (L->looping == 100) {
		// psx async loading check - is file in memory
		if (!rs_anims->Res_open(I->get_info_name(__NON_GENERIC), I->info_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash))
			return IR_REPEAT;

		if ((Object_visible_to_camera(cur_id)) &&
		    (!rs_anims->Res_open(I->get_anim_name(__NON_GENERIC), I->anim_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash)))
			return IR_REPEAT;
	}

	// anim is now in memory
	L->looping = 0;
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_play_custom_anim(int32 &result, int32 *params) {
	// a mega character plays an anim
	// the anim IS NOT part of the generic set
	// barriers are checked

	// params    0   name of anim

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) {
		I->Init_custom_animation(anim_name);
		Reset_cur_megas_custom_type();
		L->looping = 100;
		ANIM_CHECK(__NON_GENERIC);
		L->list[0] = HashString(anim_name);
	}

	// anim is in memory so do first frame then pass on to normal logic path
	if (L->looping == 100) {
		// psx async loading check - is file in memory
		if (!rs_anims->Res_open(I->get_info_name(__NON_GENERIC), I->info_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash))
			return IR_REPEAT;

		if ((Object_visible_to_camera(cur_id)) &&
		    (!rs_anims->Res_open(I->get_anim_name(__NON_GENERIC), I->anim_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash)))
			return IR_REPEAT;

		I->Promote_non_generic(); // swap out the _NON_GENERIC to safe place
		L->cur_anim_type = __PROMOTED_NON_GENERIC;

		L->anim_pc = 0;
		L->looping = TRUE8;

		return IR_REPEAT;
	}

	return (fn_play_generic_anim(result, nullptr));
}

mcodeFunctionReturnCodes _game_session::fn_reverse_custom_anim(int32 &, int32 *params) {
	// a mega character plays an anim backward
	// the anim IS NOT part of the generic set
	// barriers are checked

	// params    0   name of anim

	bool8 ret;

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) {
		I->Init_custom_animation(anim_name);
		Reset_cur_megas_custom_type();
		L->looping = 100;
		ANIM_CHECK(__NON_GENERIC);
		L->list[0] = HashString(anim_name);
	}

	// anim is in memory so do first frame then pass on to normal logic path
	if (L->looping == 100) {
		// psx async loading check - is file in memory
		if (!rs_anims->Res_open(I->get_info_name(__NON_GENERIC), I->info_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash))
			return IR_REPEAT;

		if ((Object_visible_to_camera(cur_id)) &&
		    (!rs_anims->Res_open(I->get_anim_name(__NON_GENERIC), I->anim_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash)))
			return IR_REPEAT;

		I->Promote_non_generic(); // swap out the _NON_GENERIC to safe place
		L->cur_anim_type = __PROMOTED_NON_GENERIC;

		PXanim *anim = (PXanim *)rs_anims->Res_open(I->get_info_name(__NON_GENERIC), I->info_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash); //
		L->anim_pc = anim->frame_qty - 2;
		L->looping = TRUE8;

		return IR_REPEAT;
	}

	// last frame is currently displayed?
	if (!L->anim_pc) {
		L->looping = FALSE8;
		return IR_CONT;
	}

	// shift character and frame forward by the amount appropriate
	ret = MS->Reverse_frame_and_motion(L->cur_anim_type, 0, M->anim_speed);
	if (!ret) { // could not move forward?
		L->looping = FALSE8;
		return IR_CONT;
	}

	// more to do - come back again next cycle
	return IR_REPEAT;
}

mcodeFunctionReturnCodes _game_session::fn_easy_play_custom_anim(int32 &result, int32 *params) {
	// a mega character plays an anim
	// the anim IS NOT part of the generic set
	// barriers are ignored

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) {
		I->Init_custom_animation(anim_name);
		Reset_cur_megas_custom_type();
		L->looping = 100; // have to distinguish between first time in and first cycle with anim in memory
		ANIM_CHECK(__NON_GENERIC);
		L->list[0] = HashString(anim_name);
	}

	// anim is in memory so do first frame then pass on to normal logic path
	if (L->looping == 100) {
		// psx async loading check - is file in memory
		if (!rs_anims->Res_open(I->get_info_name(__NON_GENERIC), I->info_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash))
			return IR_REPEAT;

		if ((Object_visible_to_camera(cur_id)) &&
		    (!rs_anims->Res_open(I->get_anim_name(__NON_GENERIC), I->anim_name_hash[__NON_GENERIC], I->base_path, I->base_path_hash)))
			return IR_REPEAT;

		I->Promote_non_generic(); // swap out the _NON_GENERIC to safe place
		L->cur_anim_type = __PROMOTED_NON_GENERIC;

		L->anim_pc = 0;
		L->looping = TRUE8;
		return (IR_REPEAT);
	}

	return (fn_easy_play_generic_anim(result, nullptr));
}

mcodeFunctionReturnCodes _game_session::fn_apply_bullet(int32 &, int32 *) {
	// a bullet is to hit an object
	// we simply call its gun_shot socket
	// this system circumvents the logic context switch
	// this should be used when guards shoot the player... and when chi shoots guards

	// params        0 name of target

	M->SetDynamicLight(1, 255, 255, 255, 0, 150, 100, 200); // 2 metres

	// Hey we are shooting someone (muzzle flash on / cartridge case on (we my want to split this!)
	M->is_shooting = TRUE8;

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_sync_with_mega(int32 &, int32 *params) {
	//	params   0   name of target mega
	//				1 nowt

	const char *mega_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	if (!L->looping) {
		L->list[0] = LinkedDataObject::Fetch_item_number_by_name(objects, mega_name);
		L->list[1] = 42;    // we are here
		L->looping = TRUE8; // don't do this again
	}

	if (logic_structs[L->list[0]]->list[1] == 42) {
		// we are going first
		L->list[1] = 43;
		L->looping = 0;
		return IR_CONT; // byeee
	} else if (logic_structs[L->list[0]]->list[1] == 43) {
		// its gone first
		logic_structs[L->list[0]]->list[1] = 0; // clean it up
		L->list[1] = 0;                         // clean us up
		L->looping = 0;
		return IR_CONT;
	}

	return IR_REPEAT;
}

mcodeFunctionReturnCodes _game_session::fn_new_apply_bullet(int32 &, int32 *params) {
	// a bullet is to hit an object with a percentage chance of hitting
	// on a hit simply call its gun_shot socket
	// this system circumvents the logic context switch
	// this should be used when guards shoot the player... and when chi shoots guards

	// params        0 name of target
	//				1 percentage chance

	uint32 tid;
	int32 retval;
	int32 rnd;
	PXreal sub1, sub2, len;
	bool8 crouched = FALSE8;

	const char *target_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// gun sound
	if (logic_structs[cur_id]->sfxVars[GUNSHOT_SFX_VAR] != 0)
		RegisterSound(cur_id, nullptr, logic_structs[cur_id]->sfxVars[GUNSHOT_SFX_VAR], gunDesc, (int8)127); // have to use full version so we can give hash instead of string
	else
		RegisterSound(cur_id, defaultGunSfx, gunDesc); // use small version as we have string not hash

	// shot types
	// 0 - nothing on screen
	// 1 - normal light/muzzleflash/cartridge case
	// default is 1
	int32 shotType = CGameObject::GetIntegerValueOrDefault(object, "gun_effects", 1);

	// if mega then do dynamic light (only if shotType isn't 0)
	if ((logic_structs[cur_id]->image_type == VOXEL) && (shotType == 1)) {
		// dynamic light
		M->SetDynamicLight(1, 255, 255, 255, 0, 150, 100, 200); // 2 metres
		// Hey we are shooting someone (muzzle flash on / cartridge case on) - we may want to split this!
		M->is_shooting = TRUE8;
	}

	// get id
	tid = LinkedDataObject::Fetch_item_number_by_name(objects, target_name);

	// how near
	if (L->image_type == PROP) { // we are prop
		sub1 = (PXreal)L->prop_xyz.x - logic_structs[tid]->mega->actor_xyz.x;
		sub2 = (PXreal)L->prop_xyz.z - logic_structs[tid]->mega->actor_xyz.z;
	} else {
		crouched = M->Is_crouched();
		sub1 = (PXreal)M->actor_xyz.x - logic_structs[tid]->mega->actor_xyz.x;
		sub2 = (PXreal)M->actor_xyz.z - logic_structs[tid]->mega->actor_xyz.z;
	}

	// dist
	len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

	// hit chance
	rnd = g_icb->getRandomSource()->getRandomNumber(100 - 1);

	// we didn't miss
	int32 missed = 0;

	// user hit chance of 0 means always miss
	if (params[1]) {
		// age chance true, or within distance, or crouching
		if ((rnd < params[1]) || (len < (PXreal)(ALWAYS_HIT_DIST * ALWAYS_HIT_DIST)) || (crouched)) { // hit

			// if prop is firing then target must be on A floor
			// ELSE
			// if mega firing then both y coords must be the same
			if (((L->image_type == PROP) && (floor_def->On_a_floor(logic_structs[tid]->mega))) ||
			    ((L->image_type == VOXEL) && (M->actor_xyz.y == logic_structs[tid]->mega->actor_xyz.y))) {

				// kick kinematic for the player if we are shooting the player
				if (tid == player.Fetch_player_id()) {
					MS->player.being_shot = 3;            // can't shoot for 3 cycles (engine anim over three frames)
					MS->player.shot_by_id = (int8)cur_id; // shot by us...!

					CGame *ob = (CGame *)LinkedDataObject::Fetch_item_by_number(objects, player.Fetch_player_id());
					int32 ret = CGameObject::GetVariable(ob, "hits");
					uint32 hits = CGameObject::GetIntegerVariable(ob, ret);

					PXreal subp1, subp2;
					if (L->image_type == PROP) { // we are prop
						subp1 = logic_structs[tid]->mega->actor_xyz.x - L->prop_xyz.x;
						subp2 = logic_structs[tid]->mega->actor_xyz.z - L->prop_xyz.z;
					} else {
						subp1 = logic_structs[tid]->mega->actor_xyz.x - M->actor_xyz.x;
						subp2 = logic_structs[tid]->mega->actor_xyz.z - M->actor_xyz.z;
					}
					PXreal dist = ((subp1 * subp1) + (subp2 * subp2));

					if (dist < PXreal(200 * 200)) {
						if (hits >= 4)
							hits -= 4;
						else
							hits = 0;
					} else if (dist < PXreal(500 * 500)) {
						if (hits >= 2)
							hits -= 2;
						else
							hits = 0;
					} else if (hits)
						hits--;

					CGameObject::SetIntegerVariable(ob, ret, hits);
				}

				MS->Call_socket(tid, "gun_shot", &retval); // the hit takes

				// cancel any speech
				Exit_speech(tid);
			}
		} else {
			// ricochet sound
			missed = 1;
		}
	} else { // missed
		missed = 1;
	}

	if (missed) {
		// gun sound
		if (logic_structs[cur_id]->sfxVars[RICOCHET_SFX_VAR] != 0)
			RegisterSound(cur_id, nullptr, logic_structs[cur_id]->sfxVars[RICOCHET_SFX_VAR], ricochetDesc,
			              (int8)127); // have to use full version so we can give hash instead of string
		else
			RegisterSound(cur_id, defaultRicochetSfx, ricochetDesc); // use small version as we have string not hash
	}

	// if a guard, adjust the bullet/clip figures
	if (logic_structs[cur_id]->image_type == VOXEL)
		if (M->is_evil) {
			// take chi and player out of the equation

			int32 ret = CGameObject::GetVariable(object, "cur_bullets");
			if (ret != -1) {
				int32 result = CGameObject::GetIntegerVariable(object, ret);

				if (!result) { // no bullets
					int32 clipret = CGameObject::GetVariable(object, "number_of_clips");
					int32 no_clips = CGameObject::GetIntegerVariable(object, clipret);
					if (no_clips == -1)
						Fatal_error("object has no 'number_of_clips' variable");

					if (no_clips) {     // has clips left
						no_clips--; // 1 less
						CGameObject::SetIntegerVariable(object, clipret, no_clips);

						int32 bull_per_clip = MS->player.GetBulletsPerClip();

						CGameObject::SetIntegerVariable(object, ret, bull_per_clip); // reload the gun
					}
				} else { // has bullets, fire one
					result--;
					CGameObject::SetIntegerVariable(object, ret, result); // reload
				}
			}
		}

	return (IR_CONT);
}

mcodeFunctionReturnCodes _game_session::fn_apply_anim_y(int32 &, int32 *params) {
	// add the y offset from frame 0 to end frame
	// used for climbing ladders and stairs and so on where the height gets moved

	// params        0 generic anim name

	uint32 k;
	PXreal y_next;

	const char *anim_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// search for the named generic anim - can't use __ANIM_NAME from script unfortunately
	for (k = 0; k < __TOTAL_ANIMS; k++) {
		// we must search the table
		if (!strcmp(anim_name, master_anim_name_table[k].name)) {
			// found!
			ANIM_CHECK(k);

			PXanim *pAnim = (PXanim *)rs_anims->Res_open(I->get_info_name(k), I->info_name_hash[k], I->base_path, I->base_path_hash); //
			PXreal x, z;
			PXreal yend;
			PXreal ystart;

			PXmarker_PSX_Object::GetXYZ(&PXFrameEnOfAnim(pAnim->frame_qty - 1, pAnim)->markers[ORG_POS], &x, &yend, &z);

			PXmarker_PSX_Object::GetXYZ(&PXFrameEnOfAnim(0, pAnim)->markers[ORG_POS], &x, &ystart, &z);

			y_next = yend - ystart;

			Tdebug("y_apply.txt", "%s offset - %3.1f", (const char *)I->get_info_name(k), y_next);

			M->actor_xyz.y += y_next;
			return IR_CONT;
		}
	}

	Fatal_error("fn_apply_anim_y [%s] can't find generic anim [%s]", CGameObject::GetName(object), anim_name);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_add_y(int32 &, int32 *params) {
	// add y value to a mega

	// params        0   value

	if (L->image_type == PROP)
		Fatal_error("fn_add_y can't be used on a prop - %s", CGameObject::GetName(object));

	M->actor_xyz.y += params[0];

	Tdebug("fn_add_y.txt", "%s +%d to %3.1f", CGameObject::GetName(object), params[0], M->actor_xyz.y);

	return IR_CONT;
}

} // End of namespace ICB
