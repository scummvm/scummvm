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

#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/bone.h"
#include "engines/icb/sound.h"
#include "engines/icb/icb.h"

#include "common/textconsole.h"

namespace ICB {

#define NECK_PERCENT 16
#define NECK_RANGE 96
#define NECK_SPEED 8

#define JAW_PERCENT 40
#define JAW_MAX 256
#define JAW_SPEED 32

#define STANDARD_LOOK_SPEED 128

// useful function to cap a short value at min-max range
void LimitShort(short &v, short min, short max) {
	if (v < min)
		v = min;
	else if (v > max)
		v = max;
}

#define BULLET_COL 200
#define BULLET_DC -8

#define BULLET_G -8  // dy+=g
#define BULLET_DX 4  // initial dy
#define BULLET_DY 16 // initial dy

void _mega::InitCartridgeCase(SVECTOR *initPos, short initialHeight) {
	bulletInitialHeight = initialHeight;
	bulletOn = TRUE8;
	bulletPos = *initPos;
	bulletDX = BULLET_DX;
	bulletDY = BULLET_DY;
	bulletColour = (uint8)BULLET_COL;
	bulletBounced = (uint8)0;
}

void _game_session::UpdateCartridgeCase() {
	if (M->bulletOn) {
		// gravity acceleration
		M->bulletDY = (int16)(M->bulletDY + BULLET_G);

		// movement
		M->bulletPos.vx = (int16)(M->bulletPos.vx + M->bulletDX);
		M->bulletPos.vy = (int16)(M->bulletPos.vy + M->bulletDY);

		// only reduce colour if >0
		if (M->bulletColour)
			M->bulletColour = (uint8)(M->bulletColour + BULLET_DC);

		if ((M->bulletPos.vy < -M->bulletInitialHeight) && (M->bulletBounced >= 1)) { // we are at ground height and have already bounced enough (1) times, turn us off
			M->bulletOn = FALSE8;
		} else if (M->bulletPos.vy < -M->bulletInitialHeight) {     // otherwise if we are at floor height then bounce us back up...
			M->bulletPos.vy = (int16)(-M->bulletInitialHeight); // clip
			M->bulletDY = (int16)(-M->bulletDY / 2);
			M->bulletDX = (int16)(M->bulletDX / 2);
			M->bulletBounced++;

			// this is where we make the bouncing sound...
			RegisterSound(cur_id, CGameObject::GetStringValueOrDefault(object, tinkleSfxVar, defaultTinkleSfx), tinkleDesc);
		}
	}
}

// update talking character log
// using there rap file to get bones
// for jaw and neck. This sets random
// movement on neck (x,y,z) and random
// movement for jaw (just in x)
void UpdateTalking(_logic *log, RapAPI *rap) {
	// check for -1 in rap
	if (rap->jawBone == (int8)-1)
		Tdebug("bones.txt", "mega %s speaking but has no jaw bone!", log->mega->chr_name);
	if (rap->neckBone == (int8)-1)
		Tdebug("bones.txt", "mega %s speaking but has no neck bone!", log->mega->chr_name);

	// set deformations....
	BoneDeformation *neckBone = &(log->voxel_info->neckBone);
	BoneDeformation *jawBone = &(log->voxel_info->jawBone);

	// set speed
	neckBone->boneSpeed = NECK_SPEED;
	jawBone->boneSpeed = JAW_SPEED;

	// set number of bone from rap file
	jawBone->boneNumber = (int16)(rap->jawBone);
	neckBone->boneNumber = (int16)(rap->neckBone);

	// unless it's -1 (no bone) we want the bone above this...
	if (neckBone->boneNumber != (int16)-1) {
		neckBone->boneNumber++; // add one to the value
	}

	// random neck movement (all three planes)
	if ((g_icb->getRandomSource()->getRandomNumber(100 - 1)) < NECK_PERCENT) {
		neckBone->boneTarget.vx = (int16)((g_icb->getRandomSource()->getRandomNumber(2 * NECK_RANGE - 1)) - NECK_RANGE);
		neckBone->boneTarget.vz = (int16)((g_icb->getRandomSource()->getRandomNumber(2 * NECK_RANGE - 1)) - NECK_RANGE);
		neckBone->boneTarget.vy = (int16)((g_icb->getRandomSource()->getRandomNumber(2 * NECK_RANGE - 1)) - NECK_RANGE);
	}

	// random jaw movement (just the v moves)
	if ((g_icb->getRandomSource()->getRandomNumber(100 - 1)) < JAW_PERCENT) {
		jawBone->boneTarget.vx = (int16)(g_icb->getRandomSource()->getRandomNumber(JAW_MAX - 1));
		jawBone->boneTarget.vz = (int16)0;
		jawBone->boneTarget.vy = (int16)0;
	}
}

// player was shot by obj
// needs some work to  get right
void SetPlayerShotBone(int32 obj_id) {
	_logic *player_log = MS->player.log;
	_logic *obj_log = MS->logic_structs[obj_id];

	PXfloat px, py, pz, ox, oy, oz;

	// get locations (only really interested in x and z
	player_log->GetPosition(px, py, pz);
	obj_log->GetPosition(ox, oy, oz);

	PXfloat dx, dz;

	dx = ox - px;
	dz = oz - pz;

	// get player facing direction...

	PXfloat player_pan;

	if (MS->player.log->auto_panning == FALSE8)
		player_pan = MS->player.log->pan;
	else
		player_pan = MS->player.log->auto_display_pan;

	int32 direction;

// get direction got shot from...

	direction = (int32)(4096.0 * (PXAngleOfVector(-dz, -dx) - player_pan));

	// make sure it is -2048 - 2048
	if (direction > 2047)
		direction -= 4096;

	if (direction < -2048)
		direction += 4096;

	// bone number 2 set to (512,0,512)

	if (abs(direction) < 1024)                             // shot from somewhere behind...
		MS->player.shotDeformation.boneValue.vx = 256; // front
	else
		MS->player.shotDeformation.boneValue.vx = -256; // back

	if ((g_icb->getRandomSource()->getRandomNumber(100 - 1)) < 50) { // 50% chance of going either way left or right
		MS->player.shotDeformation.boneValue.vy = 32;            // turn a bit just for variation
		MS->player.shotDeformation.boneValue.vz = 32;            // turn a bit just for variation
	} else {
		MS->player.shotDeformation.boneValue.vy = -32; // turn
		MS->player.shotDeformation.boneValue.vz = -32; // turn a bit just for variation
	}

	MS->player.shotDeformation.Target0();

	MS->player.shotDeformation.boneNumber = 1;
	MS->player.shotDeformation.boneSpeed = 128;
}

#define STATUS_NONE 0
#define STATUS_BODY 1
#define STATUS_HEAD 2

#define PLAYER_STOOD_EYE 180
#define PLAYER_CROUCH_EYE 65

#define MEGA_STOOD_EYE 170
#define MEGA_CROUCH_EYE 55
#define MEGA_DEAD_EYE 0
#define ROBOT_EYE 40

#define STANDARD_MARKER_HEIGHT 170

void ResetPlayerLook() {
	_logic *log = MS->player.log;
	BoneDeformation *b = &(log->voxel_info->lookBone);

	b->boneTarget.vx = b->boneTarget.vy = b->boneTarget.vz = 0;
}

// update the neck bone
// should only be called with player as logic
void UpdatePlayerLook() {
	static int32 status = STATUS_NONE;

	_logic *log = MS->player.log;
	BoneDeformation *b = &(log->voxel_info->lookBone);

	// start off with manually set values
	b->boneTarget = log->voxel_info->scriptedLookBoneTarget;

	// interact object, or look at object if we don't have an interact one
	bool8 has_interact = MS->player.interact_selected;
	uint32 sel_id = MS->player.cur_interact_id;

	// we have no interact so try getting a look at...
	if (!has_interact) {
		has_interact = MS->player.look_at_selected;
		sel_id = MS->player.look_at_id;
	}

	// if we have a specific look set then we set bone number to 23
	if ((b->boneTarget.vx != 0) || (b->boneTarget.vy != 0) || (b->boneTarget.vz != 0)) {
		status = STATUS_NONE;
		b->boneNumber = 23;
		b->boneSpeed = STANDARD_LOOK_SPEED;
	}

	// we only override these things if we do not have a bone target set (ie bone target is <0,0,0>)
	// also we only do this if the player is looking at something... (otherwise the targets are <0,0,0>
	// also we only do this when not playing a custom anim...
	if ((b->boneTarget.vx == 0) && (b->boneTarget.vy == 0) && (b->boneTarget.vz == 0) && // bone override not set
	    (has_interact) &&                                                                // are looking at something
	    (log->cur_anim_type != __PROMOTED_NON_GENERIC) &&                                // we are not on a special non-generic (custom)
	    (log->cur_anim_type != __NON_GENERIC)                                            // we are not on a non-generic (custom)
	    ) {
		_logic *target;
		PXfloat player_pan;
		PXreal ox, oy, oz, px, py, pz;
		PXreal dx, dy, dz;
		int32 playerEye;

		// get position of players head

		// raw position (foot)
		log->GetPosition(px, py, pz);

		if (log->mega->Is_crouched())
			playerEye = PLAYER_CROUCH_EYE; // crouch
		else
			playerEye = PLAYER_STOOD_EYE; // stand

		// height increases to eye...
		py += playerEye;

		// get position of targeting object

		target = MS->logic_structs[sel_id];

		// raw position
		target->GetPosition(ox, oy, oz);

		// target is an actor so need adjustment for eye height...
		if (target->image_type == VOXEL) {
			CGame *pGameObject = (CGame *)LinkedDataObject::Fetch_item_by_number(MS->objects, sel_id);
			int32 dead = CGameObject::GetIntegerVariable(pGameObject, CGameObject::GetVariable(pGameObject, "state"));

			if (target->object_type == __NON_ORGANIC_MEGA) { // robot (look down)
				oy += ROBOT_EYE;
			} else if (dead) { // dead
				oy += MEGA_DEAD_EYE;
			} else if (target->mega->Is_crouched()) { // crouch
				oy += MEGA_CROUCH_EYE;
			} else {
				oy += MEGA_STOOD_EYE; // standing
			}

			// difference in x,y,z
			dx = (px - ox);
			dy = (py - oy);
			dz = (pz - oz);

		}
		// is an interact object, check for height.
		else {
			int32 height = STANDARD_MARKER_HEIGHT; // standard prop height

			// if look_height set for this marker set
			if (target->look_height != -1)
				height = target->look_height;

			dx = (px - ox);
			// difference between player eye and marker height
			dy = (PXreal)(playerEye - height);
			dz = (pz - oz);
		}

		// get player pan (for calculating how far round the head moves)

		if (MS->player.log->auto_panning == FALSE8)
			player_pan = MS->player.log->pan;
		else
			player_pan = MS->player.log->auto_display_pan;

// Now find angles for neck bone...

		b->boneTarget.vz = (int16)(4096.0 * (PXAngleOfVector(-dz, -dx) - player_pan));
		b->boneTarget.vx = (int16)(4096.0 * PXAngleOfVector((PXfloat)PXsqrt(dx * dx + dz * dz), dy));

		// make sure vz is in range -2048 - 2048... this might not be because of subtracting off player_pan

		while (b->boneTarget.vz > 2048)
			b->boneTarget.vz -= 4096;

		while (b->boneTarget.vz < -2048)
			b->boneTarget.vz += 4096;

		if (b->boneTarget.vz > 1024)
			b->boneTarget.vz = 1024;
		if (b->boneTarget.vz < -1024)
			b->boneTarget.vz = -1024;

		// armed unarmed

		int32 armed = MS->player.log->mega->Fetch_armed_status();

		// from NONE to a status

		// if status is none and we are armed and we have returned to central then start looking with upper body...
		if ((status == STATUS_NONE) && (armed) && (b->boneValue.vz == 0) && (b->boneValue.vx == 0))
			status = STATUS_BODY;

		// if status is none and we are not armed and we have returned to central then start looking with head...
		if ((status == STATUS_NONE) && (!armed) && (b->boneValue.vz == 0) && (b->boneValue.vx == 0))
			status = STATUS_HEAD;

		// from wrong status to NONE

		// if aiming with body but not armed then status is none (in prep of going to status_body)
		if ((status == STATUS_BODY) && (!armed))
			status = STATUS_NONE;

		// if looking with head but armed then status is none (in preparation for going to STATUS_HEAD)
		if ((status == STATUS_HEAD) && (armed))
			status = STATUS_NONE;

		// now what to do in each status...

		// no status, target nothing...
		if (status == STATUS_NONE) {
			b->boneTarget.vx = 0;
			b->boneTarget.vy = 0;
			b->boneTarget.vz = 0;
			b->boneSpeed = STANDARD_LOOK_SPEED * 2;
		}
		// look with head...
		else if (status == STATUS_HEAD) {
			// limit pan and pitch
			LimitShort(b->boneTarget.vz, -512, 384);
			LimitShort(b->boneTarget.vx, -256, 256);

			// for turning it looks better if you look up slightly (vx=vx-abs(vz)/2)
			b->boneTarget.vx = (int16)(b->boneTarget.vx - (abs(b->boneTarget.vz) / 3));

			// we need to set the speed to be STANDARD_LOOK_SPEED
			b->boneSpeed = STANDARD_LOOK_SPEED;

			// should not be hard coded, neck bone
			b->boneNumber = 23;
		}
		// look with body...
		else if (status == STATUS_BODY) {
			// no bend when aimed
			b->boneTarget.vy = 0;

			// limit pitch (pan can be any value so gun is always pointing at target...)
			LimitShort(b->boneTarget.vx, -256, 256);

			b->boneTarget.vy = (int16)((b->boneTarget.vx * b->boneTarget.vz) / 1024);

			// we need to set the speed to be STANDARD_LOOK_SPEED
			b->boneSpeed = STANDARD_LOOK_SPEED * 2;

			// should not be hard coded, body bone
			b->boneNumber = 1;
		}

	} else {
		// if still in body mode and we have gone back to straight then set mode to none and bone goes to 23 (neck) for specific
		// looks...
		if ((b->boneTarget.vx == 0) && (b->boneValue.vz == 0) && (status == STATUS_BODY)) {
			status = STATUS_NONE;
			b->boneNumber = 23;
			b->boneSpeed = STANDARD_LOOK_SPEED;
		}
	}

	// don't do an update here...
}

mcodeFunctionReturnCodes fn_set_neck_bone(int32 &result, int32 *params) { return (MS->fn_set_neck_bone(result, params)); }

mcodeFunctionReturnCodes fn_set_neck_vector(int32 &result, int32 *params) { return (MS->fn_set_neck_vector(result, params)); }

mcodeFunctionReturnCodes speak_set_neck_vector(int32 &result, int32 *params) { return (MS->speak_set_neck_vector(result, params)); }

mcodeFunctionReturnCodes fn_simple_look(int32 &result, int32 *params) { return (MS->fn_simple_look(result, params)); }

mcodeFunctionReturnCodes speak_simple_look(int32 &result, int32 *params) { return (MS->speak_simple_look(result, params)); }

mcodeFunctionReturnCodes fn_set_mega_height(int32 &result, int32 *params) { return (MS->fn_set_mega_height(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_set_mega_height(int32 &, int32 *params) {
	if (!L->mega)
		Fatal_error("fn_set_mega_height called for %s which is not a mega!", L->GetName());

	L->mega->height = params[0];

	return IR_CONT;
}


// the array of standard look coords

#define LOOK_RIGHT (int16)(-384)
#define LOOK_UP (int16)(-196)

#define LOOK_LEFT (int16)(-LOOK_RIGHT)
#define LOOK_DOWN (int16)(-LOOK_UP)

const short looks[9][3] = {
	{0, 0, 0},                  // ahead
	{0, 0, LOOK_UP},            // up
	{LOOK_RIGHT, 0, LOOK_UP},   // up/right
	{LOOK_RIGHT, 0, 0},         // right
	{LOOK_RIGHT, 0, LOOK_DOWN}, // down/right
	{0, 0, LOOK_DOWN},          // down
	{LOOK_LEFT, 0, LOOK_DOWN},  // down/left
	{LOOK_LEFT, 0, 0},          // left
	{LOOK_LEFT, 0, LOOK_UP}     // up/left
};

// look
// 0    - Ahead
// 1    - Up
// 2    - Up/Right
// 3    - Right
// 4    - Right/Down
// 5    - Down
// 6    - Down/Left
// 7    - Left
// 8    - Left/Up

// simple look
mcodeFunctionReturnCodes _game_session::fn_simple_look(int32 &, int32 *params) {
	int32 l = params[0]; // which direction

	if (!logic_structs[cur_id]->mega)
		Fatal_error("fn_simple_look called by non mega %s", L->GetName());

	int32 callingParams[2];

	callingParams[0] = MemoryUtil::encodePtr((uint8 *)const_cast<char *>(L->GetName()));
	callingParams[1] = l;

	int32 ret;
	return speak_simple_look(ret, callingParams);
}

// simple look from speech scripts...

mcodeFunctionReturnCodes _game_session::speak_simple_look(int32 &, int32 *params) {
	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// object
	int32 object_id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);

	// direction
	int32 l = params[1];

	if (!logic_structs[object_id]->mega)
		Fatal_error("speak_simple_look called by non mega %s", logic_structs[object_id]->GetName());

	if (logic_structs[object_id]->voxel_info->lookBone.boneNumber == (int8)-1)
		Fatal_error("speak_simple_look called but no fn_set_neck_bone() has been called for object %s", logic_structs[object_id]->GetName());

	Tdebug("bones.txt", "%s: Simple look %d <%d,%d,%d> at speed %d", object_name, l, looks[l][2], looks[l][1], looks[l][0], STANDARD_LOOK_SPEED);

	// sorry looks is wrong way round! (z,y,x)
	logic_structs[object_id]->voxel_info->scriptedLookBoneTarget.vx = looks[l][2];
	logic_structs[object_id]->voxel_info->scriptedLookBoneTarget.vy = looks[l][1];
	logic_structs[object_id]->voxel_info->scriptedLookBoneTarget.vz = looks[l][0];
	logic_structs[object_id]->voxel_info->lookBone.boneSpeed = STANDARD_LOOK_SPEED;

	warning("doing a look direction: %d bone: %d", l, logic_structs[object_id]->voxel_info->lookBone.boneNumber);

	return IR_CONT;
}

// set neck bone of current object
mcodeFunctionReturnCodes _game_session::fn_set_neck_bone(int32 &, int32 *params) {
	int32 bone = params[0];

	if (!logic_structs[cur_id]->mega)
		Fatal_error("fn_set_neck_bone called by non mega %s", L->GetName());

	Tdebug("bones.txt", "%s: Neck bone is %d", L->GetName(), bone);

	logic_structs[cur_id]->voxel_info->lookBone.boneNumber = (int8)bone;
	logic_structs[cur_id]->voxel_info->neckBone.boneNumber = (int8)bone;

	return IR_CONT;
}

// set neck vector
// params
// 0    - x
// 1    - y
// 2    - z
// 3    - speed
// equiverlant to speech_set_neck_vector(NAME,x,y,z,speed)
// where NAME is object name...
//
mcodeFunctionReturnCodes _game_session::fn_set_neck_vector(int32 &, int32 *params) {
	int32 x, y, z, speed;

	x = params[0];
	y = params[1];
	z = params[2];
	speed = params[3];

	if (!logic_structs[cur_id]->mega)
		Fatal_error("fn_set_neck_vector called by non mega %s", L->GetName());

	int32 callingParams[5];

	callingParams[0] = MemoryUtil::encodePtr((uint8 *)const_cast<char *>(L->GetName()));
	callingParams[1] = x;
	callingParams[2] = y;
	callingParams[3] = z;
	callingParams[4] = speed;

	int32 ret;
	return speak_set_neck_vector(ret, callingParams);
}

// set neck vector
// params
// 0    - "object"
// 1    - x
// 2    - y
// 3    - z
// 4    - speed
//
mcodeFunctionReturnCodes _game_session::speak_set_neck_vector(int32 &, int32 *params) {
	int32 object_id;
	int32 x, y, z, speed;

	const char *object_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	object_id = LinkedDataObject::Fetch_item_number_by_name(objects, object_name);
	x = params[1];
	y = params[2];
	z = params[3];
	speed = params[4];

	if (L == player.log) {
		warning("player set neck vector...");
		logic_structs[object_id]->voxel_info->lookBone.boneNumber = 23;
	}

	if (!logic_structs[object_id]->mega)
		Fatal_error("fn_set_neck_vector called by non mega %s", logic_structs[object_id]->GetName());

	if (logic_structs[object_id]->voxel_info->lookBone.boneNumber == (int8)-1)
		Fatal_error("fn_set_neck_vector called but no fn_set_neck_bone() has been called for object %s", logic_structs[object_id]->GetName());

	Tdebug("bones.txt", "%s: Setting bone <%d,%d,%d> at speed %d", object_name, x, y, z, speed);

	logic_structs[object_id]->voxel_info->scriptedLookBoneTarget.vx = (int16)x;
	logic_structs[object_id]->voxel_info->scriptedLookBoneTarget.vy = (int16)y;
	logic_structs[object_id]->voxel_info->scriptedLookBoneTarget.vz = (int16)z;
	logic_structs[object_id]->voxel_info->lookBone.boneSpeed = (int16)speed;

	return IR_CONT;
}

} // End of namespace ICB
