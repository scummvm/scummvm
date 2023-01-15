/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "twine/script/script_move_v1.h"


namespace TwinE {

static const ScriptMoveFunction function_map[] = {
	{"END", ScriptMove::mEND},
	{"NOP", ScriptMove::mNOP},
	{"BODY", ScriptMove::mBODY},
	{"ANIM", ScriptMove::mANIM},
	{"GOTO_POINT", ScriptMove::mGOTO_POINT},
	{"WAIT_ANIM", ScriptMove::mWAIT_ANIM},
	{"LOOP", ScriptMove::mLOOP},
	{"ANGLE", ScriptMove::mANGLE},
	{"POS_POINT", ScriptMove::mPOS_POINT},
	{"LABEL", ScriptMove::mLABEL},
	{"GOTO", ScriptMove::mGOTO},
	{"STOP", ScriptMove::mSTOP},
	{"GOTO_SYM_POINT", ScriptMove::mGOTO_SYM_POINT},
	{"WAIT_NUM_ANIM", ScriptMove::mWAIT_NUM_ANIM},
	{"SAMPLE", ScriptMove::mSAMPLE},
	{"GOTO_POINT_3D", ScriptMove::mGOTO_POINT_3D},
	{"SPEED", ScriptMove::mSPEED},
	{"BACKGROUND", ScriptMove::mBACKGROUND},
	{"WAIT_NUM_SECOND", ScriptMove::mWAIT_NUM_SECOND},
	{"NO_BODY", ScriptMove::mNO_BODY},
	{"BETA", ScriptMove::mBETA},
	{"OPEN_LEFT", ScriptMove::mOPEN_LEFT},
	{"OPEN_RIGHT", ScriptMove::mOPEN_RIGHT},
	{"OPEN_UP", ScriptMove::mOPEN_UP},
	{"OPEN_DOWN", ScriptMove::mOPEN_DOWN},
	{"CLOSE", ScriptMove::mCLOSE},
	{"WAIT_DOOR", ScriptMove::mWAIT_DOOR},
	{"SAMPLE_RND", ScriptMove::mSAMPLE_RND},
	{"SAMPLE_ALWAYS", ScriptMove::mSAMPLE_ALWAYS},
	{"SAMPLE_STOP", ScriptMove::mSAMPLE_STOP},
	{"PLAY_FLA", ScriptMove::mPLAY_FLA},
	{"REPEAT_SAMPLE", ScriptMove::mREPEAT_SAMPLE},
	{"SIMPLE_SAMPLE", ScriptMove::mSIMPLE_SAMPLE},
	{"FACE_HERO", ScriptMove::mFACE_HERO},
	{"ANGLE_RND", ScriptMove::mANGLE_RND}
};

ScriptMoveV1::ScriptMoveV1(TwinEEngine *engine) : ScriptMove(engine, function_map, ARRAYSIZE(function_map)) {
}

} // namespace TwinE
