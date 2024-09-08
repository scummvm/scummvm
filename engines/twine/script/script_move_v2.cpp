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

#include "twine/script/script_move_v2.h"
#include "twine/resources/resources.h"
#include "twine/twine.h"

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
	{"ANGLE_RND", ScriptMove::mANGLE_RND},
	{"REM", ScriptMove::mEMPTY}, // unused
	{"WAIT_NB_DIZIEME", ScriptMoveV2::mWAIT_NB_DIZIEME},
	{"DO", ScriptMove::mEMPTY}, // unused
	{"SPRITE", ScriptMoveV2::mSPRITE},
	{"WAIT_NB_SECOND_RND", ScriptMoveV2::mWAIT_NB_SECOND_RND},
	{"AFF_TIMER", ScriptMove::mEMPTY}, // unused
	{"SET_FRAME", ScriptMoveV2::mSET_FRAME},
	{"SET_FRAME_3DS", ScriptMoveV2::mSET_FRAME_3DS},
	{"SET_START_3DS", ScriptMoveV2::mSET_START_3DS},
	{"SET_END_3DS", ScriptMoveV2::mSET_END_3DS},
	{"START_ANIM_3DS", ScriptMoveV2::mSTART_ANIM_3DS},
	{"STOP_ANIM_3DS", ScriptMoveV2::mSTOP_ANIM_3DS},
	{"WAIT_ANIM_3DS", ScriptMoveV2::mWAIT_ANIM_3DS},
	{"WAIT_FRAME_3DS", ScriptMoveV2::mWAIT_FRAME_3DS},
	{"WAIT_NB_DIZIEME_RND", ScriptMoveV2::mWAIT_NB_DIZIEME_RND},
	{"DECALAGE", ScriptMoveV2::mOFFSET},
	{"FREQUENCY", ScriptMoveV2::mFREQUENCY},
	{"VOLUME", ScriptMoveV2::mVOLUME}
};

int32 ScriptMoveV2::mWAIT_NB_DIZIEME(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int32 numSeconds = ctx.stream.readByte();
	int32 currentTime = ctx.stream.readSint32LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::WAIT_NB_DIZIEME(%i, %i)", (int)numSeconds, currentTime);

	if (currentTime == 0) {
		currentTime = engine->timerRef + engine->toSeconds(numSeconds) / 10;
		ctx.stream.rewind(4);
		ctx.stream.writeSint32LE(currentTime);
	}

	if (engine->timerRef < currentTime) {
		ctx.undo(5);
		return 1;
	}

	ctx.stream.rewind(4);
	ctx.stream.writeSint32LE(0);

	return 0;
}

int32 ScriptMoveV2::mWAIT_NB_DIZIEME_RND(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int32 numSeconds = engine->getRandomNumber(ctx.stream.readByte());
	int32 currentTime = ctx.stream.readSint32LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::WAIT_NB_DIZIEME(%i, %i)", (int)numSeconds, currentTime);

	if (currentTime == 0) {
		currentTime = engine->timerRef + engine->toSeconds(numSeconds) / 10;
		ctx.stream.rewind(4);
		ctx.stream.writeSint32LE(currentTime);
	}

	if (engine->timerRef < currentTime) {
		ctx.undo(5);
		return 1;
	}

	ctx.stream.rewind(4);
	ctx.stream.writeSint32LE(0);

	return 0;
}

int32 ScriptMoveV2::mWAIT_NB_SECOND_RND(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int32 numSeconds = engine->getRandomNumber(ctx.stream.readByte());
	int32 currentTime = ctx.stream.readSint32LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::WAIT_NB_SECOND_RND(%i, %i)", (int)numSeconds, currentTime);

	if (currentTime == 0) {
		currentTime = engine->timerRef + engine->toSeconds(numSeconds);
		ctx.stream.rewind(4);
		ctx.stream.writeSint32LE(currentTime);
	}

	if (engine->timerRef < currentTime) {
		ctx.undo(5);
		return 1;
	}

	ctx.stream.rewind(4);
	ctx.stream.writeSint32LE(0);

	return 0;
}

int32 ScriptMoveV2::mSPRITE(TwinEEngine *engine, MoveScriptContext &ctx) {
	int16 num = ctx.stream.readSint16LE();
	if (ctx.actor->_staticFlags.bSprite3D) {
		engine->_actor->initSprite(num, ctx.actorIdx);
	}
	return 0;
}

int32 ScriptMoveV2::mSET_FRAME(TwinEEngine *engine, MoveScriptContext &ctx) {
	const uint8 num = ctx.stream.readByte();
	if (!ctx.actor->_staticFlags.bSprite3D) {
		engine->_actor->setFrame(ctx.actorIdx, num);
	}
	return 0;
}

int32 ScriptMoveV2::mSET_FRAME_3DS(TwinEEngine *engine, MoveScriptContext &ctx) {
	int32 num = ctx.stream.readByte();
	if (ctx.actor->_staticFlags.bHasSpriteAnim3D) {
		const T_ANIM_3DS *anim = engine->_resources->getAnim(ctx.actor->A3DS.Num);
		if (num > (anim->Fin - anim->Deb)) {
			num = anim->Fin - anim->Deb;
		}

		num += anim->Deb;

		engine->_actor->initSprite(num, ctx.actorIdx);
	}
	return 0;
}

int32 ScriptMoveV2::mSET_START_3DS(TwinEEngine *engine, MoveScriptContext &ctx) {
	return -1;
}

int32 ScriptMoveV2::mSET_END_3DS(TwinEEngine *engine, MoveScriptContext &ctx) {
	return -1;
}

int32 ScriptMoveV2::mSTART_ANIM_3DS(TwinEEngine *engine, MoveScriptContext &ctx) {
	return -1;
}

int32 ScriptMoveV2::mSTOP_ANIM_3DS(TwinEEngine *engine, MoveScriptContext &ctx) {
	return -1;
}

int32 ScriptMoveV2::mWAIT_ANIM_3DS(TwinEEngine *engine, MoveScriptContext &ctx) {
	return -1;
}

int32 ScriptMoveV2::mWAIT_FRAME_3DS(TwinEEngine *engine, MoveScriptContext &ctx) {
	return -1;
}

// DECALAGE
int32 ScriptMoveV2::mOFFSET(TwinEEngine *engine, MoveScriptContext &ctx) {
	return -1;
}

// FREQUENCE
int32 ScriptMoveV2::mFREQUENCY(TwinEEngine *engine, MoveScriptContext &ctx) {
	return -1;
}

int32 ScriptMoveV2::mVOLUME(TwinEEngine *engine, MoveScriptContext &ctx) {
	return -1;
}

ScriptMoveV2::ScriptMoveV2(TwinEEngine *engine) : ScriptMove(engine, function_map, ARRAYSIZE(function_map)) {
}

} // namespace TwinE
