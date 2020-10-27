/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "twine/script_move_v1.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "twine/actor.h"
#include "twine/animations.h"
#include "twine/movements.h"
#include "twine/redraw.h"
#include "twine/renderer.h"
#include "twine/scene.h"
#include "twine/sound.h"
#include "twine/twine.h"

namespace TwinE {

static uint8 *scriptPtr = nullptr;
static int32 scriptPosition = 0;
static int32 numRepeatSample = 1;

struct ScriptContext {
	int32 actorIdx;
	ActorStruct *actor;
};

/** Returns:
	   -1 - Need implementation
		0 - Completed
		1 - Break script */
typedef int32 ScriptMoveFunc(TwinEEngine *engine, ScriptContext& ctx);

struct ScriptMoveFunction {
	const char *name;
	ScriptMoveFunc *function;
};

#define MAPFUNC(name, func) \
	{ name, func }

/*0x00*/
static int32 mEND(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript = -1;
	return 1;
}

/*0x01*/
static int32 mNOP(TwinEEngine *engine, ScriptContext& ctx) {
	return 0;
}

/*0x02*/
static int32 mBODY(TwinEEngine *engine, ScriptContext& ctx) {
	int32 bodyIdx = *(scriptPtr);
	engine->_actor->initModelActor(bodyIdx, ctx.actorIdx);
	ctx.actor->positionInMoveScript++;
	return 0;
}

/*0x03*/
static int32 mANIM(TwinEEngine *engine, ScriptContext& ctx) {
	AnimationTypes animIdx = (AnimationTypes) * (scriptPtr++);
	if (engine->_animations->initAnim(animIdx, 0, 0, ctx.actorIdx)) {
		ctx.actor->positionInMoveScript++;
		return 0;
	}
	ctx.actor->positionInMoveScript = scriptPosition;
	return 1;
}

/*0x04*/
static int32 mGOTO_POINT(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript++;
	engine->_scene->currentScriptValue = *(scriptPtr);

	const ScenePoint &sp = engine->_scene->sceneTracks[engine->_scene->currentScriptValue];
	engine->_renderer->destX = sp.x;
	engine->_renderer->destY = sp.y;
	engine->_renderer->destZ = sp.z;

	const int32 newAngle = engine->_movements->getAngleAndSetTargetActorDistance(ctx.actor->x, ctx.actor->z, sp.x, sp.z);

	if (ctx.actor->staticFlags.bIsSpriteActor) {
		ctx.actor->angle = newAngle;
	} else {
		engine->_movements->moveActor(ctx.actor->angle, newAngle, ctx.actor->speed, &ctx.actor->move);
	}

	if (engine->_movements->targetActorDistance > 500) {
		ctx.actor->positionInMoveScript -= 2;
		return 1;
	}

	return 0;
}

/*0x05*/
static int32 mWAIT_ANIM(TwinEEngine *engine, ScriptContext& ctx) {
	if (!ctx.actor->dynamicFlags.bAnimEnded) {
		ctx.actor->positionInMoveScript--;
	} else {
		engine->_movements->clearRealAngle(ctx.actor);
	}
	return 1;
}

/*0x06*/
static int32 mLOOP(TwinEEngine *engine, ScriptContext& ctx) {
	// TODO
	return -1;
}

/*0x07*/
static int32 mANGLE(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript += 2;
	if (ctx.actor->staticFlags.bIsSpriteActor) {
		return 0;
	}
	engine->_scene->currentScriptValue = *((int16 *)scriptPtr);
	if (ctx.actor->move.numOfStep == 0) {
		engine->_movements->moveActor(ctx.actor->angle, engine->_scene->currentScriptValue, ctx.actor->speed, &ctx.actor->move);
	}
	if (ctx.actor->angle == engine->_scene->currentScriptValue) {
		engine->_movements->clearRealAngle(ctx.actor);
		return 0;
	}
	ctx.actor->positionInMoveScript -= 3;
	return 1;
}

/*0x08*/
static int32 mPOS_POINT(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript++;
	engine->_scene->currentScriptValue = *(scriptPtr);

	const ScenePoint &sp = engine->_scene->sceneTracks[engine->_scene->currentScriptValue];
	engine->_renderer->destX = sp.x;
	engine->_renderer->destY = sp.y;
	engine->_renderer->destZ = sp.z;

	if (ctx.actor->staticFlags.bIsSpriteActor) {
		ctx.actor->speed = 0;
	}

	ctx.actor->x = sp.x;
	ctx.actor->y = sp.y;
	ctx.actor->z = sp.z;

	return 0;
}

/*0x09*/
static int32 mLABEL(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->labelIdx = *(scriptPtr);
	ctx.actor->positionInMoveScript++;
	ctx.actor->currentLabelPtr = ctx.actor->positionInMoveScript - 2;
	return 0;
}

/*0x0A*/
static int32 mGOTO(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript = *((int16 *)scriptPtr);
	return 0;
}

/*0x0B*/
static int32 mSTOP(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript = -1;
	return 1;
}

/*0x0C*/
static int32 mGOTO_SYM_POINT(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript++;
	engine->_scene->currentScriptValue = *(scriptPtr);

	const ScenePoint &sp = engine->_scene->sceneTracks[engine->_scene->currentScriptValue];
	engine->_renderer->destX = sp.x;
	engine->_renderer->destY = sp.y;
	engine->_renderer->destZ = sp.z;

	const int32 newAngle = 0x200 + engine->_movements->getAngleAndSetTargetActorDistance(ctx.actor->x, ctx.actor->z, sp.x, sp.z);

	if (ctx.actor->staticFlags.bIsSpriteActor) {
		ctx.actor->angle = newAngle;
	} else {
		engine->_movements->moveActor(ctx.actor->angle, newAngle, ctx.actor->speed, &ctx.actor->move);
	}

	if (engine->_movements->targetActorDistance > 500) {
		ctx.actor->positionInMoveScript -= 2;
		return 1;
	}

	return 0;
}

/*0x0D*/
static int32 mWAIT_NUM_ANIM(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript += 2;

	bool abortMove = 0;
	if (ctx.actor->dynamicFlags.bAnimEnded) {
		int32 animPos, animRepeats;

		animRepeats = *(scriptPtr);
		animPos = *(scriptPtr + 1);

		animPos++;

		if (animPos == animRepeats) {
			animPos = 0;
		} else {
			abortMove = 1;
		}

		*(scriptPtr + 1) = animPos;
	} else {
		abortMove = 1;
	}

	if (abortMove == 1) {
		ctx.actor->positionInMoveScript -= 3;
	}

	return abortMove;
}

/*0x0E*/
static int32 mSAMPLE(TwinEEngine *engine, ScriptContext& ctx) {
	int32 sampleIdx = *((int16 *)scriptPtr);
	engine->_sound->playSample(sampleIdx, 0x1000, 1, ctx.actor->x, ctx.actor->y, ctx.actor->z, ctx.actorIdx);
	ctx.actor->positionInMoveScript += 2;
	return 0;
}

/*0x0F*/
static int32 mGOTO_POINT_3D(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript++;

	if (!ctx.actor->staticFlags.bIsSpriteActor) {
		return 0;
	}

	engine->_scene->currentScriptValue = *(scriptPtr);

	const ScenePoint &sp = engine->_scene->sceneTracks[engine->_scene->currentScriptValue];
	engine->_renderer->destX = sp.x;
	engine->_renderer->destY = sp.y;
	engine->_renderer->destZ = sp.z;

	ctx.actor->angle = engine->_movements->getAngleAndSetTargetActorDistance(ctx.actor->x, ctx.actor->z, sp.x, sp.z);
	ctx.actor->animType = engine->_movements->getAngleAndSetTargetActorDistance(ctx.actor->y, 0, sp.y, engine->_movements->targetActorDistance);

	if (engine->_movements->targetActorDistance > 100) {
		ctx.actor->positionInMoveScript -= 2;
		return 1;
	}
	ctx.actor->x = sp.x;
	ctx.actor->y = sp.y;
	ctx.actor->z = sp.z;

	return 0;
}

/*0x10*/
static int32 mSPEED(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript += 2;
	ctx.actor->speed = *((int16 *)scriptPtr);

	if (ctx.actor->staticFlags.bIsSpriteActor) {
		engine->_movements->setActorAngle(0, ctx.actor->speed, 50, &ctx.actor->move);
	}

	return 0;
}

/*0x11*/
static int32 mBACKGROUND(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript++;

	if (*scriptPtr != 0) {
		if (!ctx.actor->staticFlags.bIsBackgrounded) {
			ctx.actor->staticFlags.bIsBackgrounded = 1;
			if (ctx.actor->dynamicFlags.bIsVisible) {
				engine->_redraw->reqBgRedraw = true;
			}
		}
	} else {
		if (ctx.actor->staticFlags.bIsBackgrounded) {
			ctx.actor->staticFlags.bIsBackgrounded = 0;
			if (ctx.actor->dynamicFlags.bIsVisible) {
				engine->_redraw->reqBgRedraw = true;
			}
		}
	}

	return 0;
}

/*0x12*/
static int32 mWAIT_NUM_SECOND(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript += 5;

	const int32 numSeconds = *(scriptPtr++);
	int32 currentTime = *((int32 *)scriptPtr);

	if (currentTime == 0) {
		currentTime = engine->lbaTime + numSeconds * 50;
		*((int32 *)scriptPtr) = currentTime;
	}

	if (engine->lbaTime < currentTime) {
		ctx.actor->positionInMoveScript -= 6;
		return 1;
	} else {
		*((int32 *)scriptPtr) = 0;
	}

	return 0;
}

/*0x13*/
static int32 mNO_BODY(TwinEEngine *engine, ScriptContext& ctx) {
	engine->_actor->initModelActor(-1, ctx.actorIdx);
	return 0;
}

/*0x14*/
static int32 mBETA(TwinEEngine *engine, ScriptContext& ctx) {
	const int16 beta = *((int16 *)scriptPtr);
	ctx.actor->positionInMoveScript += 2;

	ctx.actor->angle = beta;

	if (ctx.actor->staticFlags.bIsSpriteActor) {
		engine->_movements->clearRealAngle(ctx.actor);
	}

	return 0;
}

/*0x15*/
static int32 mOPEN_LEFT(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript += 2;
	if (ctx.actor->staticFlags.bIsSpriteActor && ctx.actor->staticFlags.bUsesClipping) {
		ctx.actor->angle = 0x300;
		ctx.actor->doorStatus = *((int16 *)scriptPtr);
		ctx.actor->dynamicFlags.bIsSpriteMoving = 1;
		ctx.actor->speed = 1000;
		engine->_movements->setActorAngle(0, 1000, 50, &ctx.actor->move);
	}
	return 0;
}

/*0x16*/
static int32 mOPEN_RIGHT(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript += 2;
	if (ctx.actor->staticFlags.bIsSpriteActor && ctx.actor->staticFlags.bUsesClipping) {
		ctx.actor->angle = 0x100;
		ctx.actor->doorStatus = *((int16 *)scriptPtr);
		ctx.actor->dynamicFlags.bIsSpriteMoving = 1;
		ctx.actor->speed = 1000;
		engine->_movements->setActorAngle(0, 1000, 50, &ctx.actor->move);
	}
	return 0;
}

/*0x17*/
static int32 mOPEN_UP(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript += 2;
	if (ctx.actor->staticFlags.bIsSpriteActor && ctx.actor->staticFlags.bUsesClipping) {
		ctx.actor->angle = 0x200;
		ctx.actor->doorStatus = *((int16 *)scriptPtr);
		ctx.actor->dynamicFlags.bIsSpriteMoving = 1;
		ctx.actor->speed = 1000;
		engine->_movements->setActorAngle(0, 1000, 50, &ctx.actor->move);
	}
	return 0;
}

/*0x18*/
static int32 mOPEN_DOWN(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript += 2;
	if (ctx.actor->staticFlags.bIsSpriteActor && ctx.actor->staticFlags.bUsesClipping) {
		ctx.actor->angle = 0;
		ctx.actor->doorStatus = *((int16 *)scriptPtr);
		ctx.actor->dynamicFlags.bIsSpriteMoving = 1;
		ctx.actor->speed = 1000;
		engine->_movements->setActorAngle(0, 1000, 50, &ctx.actor->move);
	}
	return 0;
}

/*0x19*/
static int32 mCLOSE(TwinEEngine *engine, ScriptContext& ctx) {
	if (ctx.actor->staticFlags.bIsSpriteActor && ctx.actor->staticFlags.bUsesClipping) {
		ctx.actor->doorStatus = 0;
		ctx.actor->dynamicFlags.bIsSpriteMoving = 1;
		ctx.actor->speed = -1000;
		engine->_movements->setActorAngle(0, -1000, 50, &ctx.actor->move);
	}
	return 0;
}

/*0x1A*/
static int32 mWAIT_DOOR(TwinEEngine *engine, ScriptContext& ctx) {
	if (ctx.actor->staticFlags.bIsSpriteActor && ctx.actor->staticFlags.bUsesClipping) {
		if (ctx.actor->speed) {
			ctx.actor->positionInMoveScript--;
			return 1;
		}
	}
	return 0;
}

/*0x1B*/
static int32 mSAMPLE_RND(TwinEEngine *engine, ScriptContext& ctx) {
	int32 freq = engine->getRandomNumber(2048) + 2048;
	int32 sampleIdx = *((int16 *)scriptPtr);
	engine->_sound->playSample(sampleIdx, freq, 1, ctx.actor->x, ctx.actor->y, ctx.actor->z, ctx.actorIdx);
	ctx.actor->positionInMoveScript += 2;
	return 0;
}

/*0x1C*/
static int32 mSAMPLE_ALWAYS(TwinEEngine *engine, ScriptContext& ctx) {
	int32 sampleIdx = *((int16 *)scriptPtr);
	if (!engine->_sound->isSamplePlaying(sampleIdx)) { // if its not playing
		engine->_sound->playSample(sampleIdx, 0x1000, -1, ctx.actor->x, ctx.actor->y, ctx.actor->z, ctx.actorIdx);
	}
	ctx.actor->positionInMoveScript += 2;
	return 0;
}

/*0x1D*/
static int32 mSAMPLE_STOP(TwinEEngine *engine, ScriptContext& ctx) {
	int32 sampleIdx = *((int16 *)scriptPtr);
	engine->_sound->stopSample(sampleIdx);
	ctx.actor->positionInMoveScript += 2;
	return 0;
}

/*0x1E*/
static int32 mPLAY_FLA(TwinEEngine *engine, ScriptContext& ctx) {
	// TODO
	return -1;
}

/*0x1F*/
static int32 mREPEAT_SAMPLE(TwinEEngine *engine, ScriptContext& ctx) {
	numRepeatSample = *((int16 *)scriptPtr);
	ctx.actor->positionInMoveScript += 2;
	return 0;
}

/*0x20*/
static int32 mSIMPLE_SAMPLE(TwinEEngine *engine, ScriptContext& ctx) {
	int32 sampleIdx = *((int16 *)scriptPtr);
	engine->_sound->playSample(sampleIdx, 0x1000, numRepeatSample, ctx.actor->x, ctx.actor->y, ctx.actor->z, ctx.actorIdx);
	numRepeatSample = 1;
	ctx.actor->positionInMoveScript += 2;
	return 0;
}

/*0x21*/
static int32 mFACE_HERO(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript += 2;
	if (ctx.actor->staticFlags.bIsSpriteActor) {
		return 0;
	}
	engine->_scene->currentScriptValue = *((int16 *)scriptPtr);
	if (engine->_scene->currentScriptValue == -1 && ctx.actor->move.numOfStep == 0) {
		engine->_scene->currentScriptValue = engine->_movements->getAngleAndSetTargetActorDistance(ctx.actor->x, ctx.actor->z, engine->_scene->sceneHero->x, engine->_scene->sceneHero->z);
		engine->_movements->moveActor(ctx.actor->angle, engine->_scene->currentScriptValue, ctx.actor->speed, &ctx.actor->move);
		*((int16 *)scriptPtr) = engine->_scene->currentScriptValue;
	}

	if (ctx.actor->angle != engine->_scene->currentScriptValue) {
		ctx.actor->positionInMoveScript -= 3;
		return 1;
	}
	engine->_movements->clearRealAngle(ctx.actor);
	*((int16 *)scriptPtr) = -1;
	return 0;
}

/*0x22*/
static int32 mANGLE_RND(TwinEEngine *engine, ScriptContext& ctx) {
	ctx.actor->positionInMoveScript += 4;
	if (ctx.actor->staticFlags.bIsSpriteActor) {
		return 0;
	}
	engine->_scene->currentScriptValue = *((int16 *)scriptPtr + 2);

	if (engine->_scene->currentScriptValue == -1 && ctx.actor->move.numOfStep == 0) {
		if (engine->getRandomNumber() & 1) {
			engine->_scene->currentScriptValue = *((int16 *)scriptPtr);
			const int32 newAngle = ctx.actor->angle + 0x100 + (ABS(engine->_scene->currentScriptValue) >> 1);
			engine->_scene->currentScriptValue = (newAngle - engine->getRandomNumber(engine->_scene->currentScriptValue)) & 0x3FF;
		} else {
			engine->_scene->currentScriptValue = *((int16 *)scriptPtr);
			const int32 newAngle = ctx.actor->angle - 0x100 + (ABS(engine->_scene->currentScriptValue) >> 1);
			engine->_scene->currentScriptValue = (newAngle - engine->getRandomNumber(engine->_scene->currentScriptValue)) & 0x3FF;
		}

		engine->_movements->moveActor(ctx.actor->angle, engine->_scene->currentScriptValue, ctx.actor->speed, &ctx.actor->move);
		*((int16 *)scriptPtr + 2) = engine->_scene->currentScriptValue;
	}

	if (ctx.actor->angle != engine->_scene->currentScriptValue) {
		ctx.actor->positionInMoveScript -= 5;
		return 1;
	}
	engine->_movements->clearRealAngle(ctx.actor);
	*((int16 *)scriptPtr + 2) = -1;
	return 0;
}

static const ScriptMoveFunction function_map[] = {
    /*0x00*/ MAPFUNC("END", mEND),
    /*0x01*/ MAPFUNC("NOP", mNOP),
    /*0x02*/ MAPFUNC("BODY", mBODY),
    /*0x03*/ MAPFUNC("ANIM", mANIM),
    /*0x04*/ MAPFUNC("GOTO_POINT", mGOTO_POINT),
    /*0x05*/ MAPFUNC("WAIT_ANIM", mWAIT_ANIM),
    /*0x06*/ MAPFUNC("LOOP", mLOOP),
    /*0x07*/ MAPFUNC("ANGLE", mANGLE),
    /*0x08*/ MAPFUNC("POS_POINT", mPOS_POINT),
    /*0x09*/ MAPFUNC("LABEL", mLABEL),
    /*0x0A*/ MAPFUNC("GOTO", mGOTO),
    /*0x0B*/ MAPFUNC("STOP", mSTOP),
    /*0x0C*/ MAPFUNC("GOTO_SYM_POINT", mGOTO_SYM_POINT),
    /*0x0D*/ MAPFUNC("WAIT_NUM_ANIM", mWAIT_NUM_ANIM),
    /*0x0E*/ MAPFUNC("SAMPLE", mSAMPLE),
    /*0x0F*/ MAPFUNC("GOTO_POINT_3D", mGOTO_POINT_3D),
    /*0x10*/ MAPFUNC("SPEED", mSPEED),
    /*0x11*/ MAPFUNC("BACKGROUND", mBACKGROUND),
    /*0x12*/ MAPFUNC("WAIT_NUM_SECOND", mWAIT_NUM_SECOND),
    /*0x13*/ MAPFUNC("NO_BODY", mNO_BODY),
    /*0x14*/ MAPFUNC("BETA", mBETA),
    /*0x15*/ MAPFUNC("OPEN_LEFT", mOPEN_LEFT),
    /*0x16*/ MAPFUNC("OPEN_RIGHT", mOPEN_RIGHT),
    /*0x17*/ MAPFUNC("OPEN_UP", mOPEN_UP),
    /*0x18*/ MAPFUNC("OPEN_DOWN", mOPEN_DOWN),
    /*0x19*/ MAPFUNC("CLOSE", mCLOSE),
    /*0x1A*/ MAPFUNC("WAIT_DOOR", mWAIT_DOOR),
    /*0x1B*/ MAPFUNC("SAMPLE_RND", mSAMPLE_RND),
    /*0x1C*/ MAPFUNC("SAMPLE_ALWAYS", mSAMPLE_ALWAYS),
    /*0x1D*/ MAPFUNC("SAMPLE_STOP", mSAMPLE_STOP),
    /*0x1E*/ MAPFUNC("PLAY_FLA", mPLAY_FLA),
    /*0x1F*/ MAPFUNC("REPEAT_SAMPLE", mREPEAT_SAMPLE),
    /*0x20*/ MAPFUNC("SIMPLE_SAMPLE", mSIMPLE_SAMPLE),
    /*0x21*/ MAPFUNC("FACE_HERO", mFACE_HERO),
    /*0x22*/ MAPFUNC("ANGLE_RND", mANGLE_RND)};

ScriptMove::ScriptMove(TwinEEngine *engine) : _engine(engine) {
	scriptPtr = nullptr;
	scriptPosition = 0;
	numRepeatSample = 1;
}

void ScriptMove::processMoveScript(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	numRepeatSample = 1;

	int32 end = -2;

	ScriptContext ctx{actorIdx, actor};
	do {
		scriptPosition = actor->positionInMoveScript;
		// TODO: use Common::MemoryReadStream for the script parsing
		scriptPtr = actor->moveScript + actor->positionInMoveScript;
		int32 scriptOpcode = *(scriptPtr++);

		actor->positionInMoveScript++;

		if (scriptOpcode >= 0 && scriptOpcode < ARRAYSIZE(function_map)) {
			end = function_map[scriptOpcode].function(_engine, ctx);
		} else {
			error("Actor %d with wrong offset/opcode - Offset: %d (opcode: %i)", actorIdx, actor->positionInLifeScript, scriptOpcode);
		}

		if (end < 0) { // show error message
			warning("Actor %d Life script [%s] not implemented", actorIdx, function_map[scriptOpcode].name);
		}
	} while (end != 1);
}

} // namespace TwinE
