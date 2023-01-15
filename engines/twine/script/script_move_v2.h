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

#ifndef TWINE_SCRIPTMOVEV2_H
#define TWINE_SCRIPTMOVEV2_H

#include "twine/script/script_move.h"

namespace TwinE {

class TwinEEngine;

class ScriptMoveV2 : public ScriptMove {
public:
	static int32 mWAIT_NB_DIZIEME(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mSPRITE(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mWAIT_NB_SECOND_RND(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mSET_FRAME(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mSET_FRAME_3DS(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mSET_START_3DS(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mSET_END_3DS(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mSTART_ANIM_3DS(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mSTOP_ANIM_3DS(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mWAIT_ANIM_3DS(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mWAIT_FRAME_3DS(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mWAIT_NB_DIZIEME_RND(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mDECALAGE(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mFREQUENCE(TwinEEngine *engine, MoveScriptContext &ctx);
	static int32 mVOLUME(TwinEEngine *engine, MoveScriptContext &ctx);

	ScriptMoveV2(TwinEEngine *engine);
};

} // namespace TwinE

#endif
