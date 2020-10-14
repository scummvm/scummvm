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

#include "twine/script_move.h"
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
static int32 continueMove = 0;
static int32 scriptPosition = 0;
static ActorMoveStruct *move = nullptr;
static int32 numRepeatSample = 1;

typedef int32 ScriptMoveFunc(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor);

typedef struct ScriptMoveFunction {
	const char *name;
	ScriptMoveFunc *function;
} ScriptMoveFunction;

#define MAPFUNC(name, func) \
	{ name, func }

/*0x00*/
static int32 mEND(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	continueMove = 0;
	actor->positionInMoveScript = -1;
	return 0;
}

/*0x01*/
static int32 mNOP(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	return 0;
}

/*0x02*/
static int32 mBODY(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 bodyIdx = *(scriptPtr);
	engine->_actor->initModelActor(bodyIdx, actorIdx);
	actor->positionInMoveScript++;
	return 0;
}

/*0x03*/
static int32 mANIM(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	AnimationTypes animIdx = (AnimationTypes) * (scriptPtr++);
	if (engine->_animations->initAnim(animIdx, 0, 0, actorIdx)) {
		actor->positionInMoveScript++;
	} else {
		actor->positionInMoveScript = scriptPosition;
		continueMove = 0;
	}
	return 0;
}

/*0x04*/
static int32 mGOTO_POINT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 newAngle;

	actor->positionInMoveScript++;
	engine->_scene->currentScriptValue = *(scriptPtr);

	engine->_renderer->destX = engine->_scene->sceneTracks[engine->_scene->currentScriptValue].x;
	engine->_renderer->destY = engine->_scene->sceneTracks[engine->_scene->currentScriptValue].y;
	engine->_renderer->destZ = engine->_scene->sceneTracks[engine->_scene->currentScriptValue].z;

	newAngle = engine->_movements->getAngleAndSetTargetActorDistance(actor->x, actor->z, engine->_renderer->destX, engine->_renderer->destZ);

	if (actor->staticFlags.bIsSpriteActor) {
		actor->angle = newAngle;
	} else {
		engine->_movements->moveActor(actor->angle, newAngle, actor->speed, &actor->move);
	}

	if (engine->_movements->targetActorDistance > 500) {
		continueMove = 0;
		actor->positionInMoveScript -= 2;
	}

	return 0;
}

/*0x05*/
static int32 mWAIT_ANIM(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	if (!actor->dynamicFlags.bAnimEnded) {
		continueMove = 0;
		actor->positionInMoveScript--;
	} else {
		continueMove = 0;
		engine->_movements->clearRealAngle(actor);
	}
	return 0;
}

/*0x06*/
static int32 mLOOP(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	// TODO
	return -1;
}

/*0x07*/
static int32 mANGLE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	if (!actor->staticFlags.bIsSpriteActor) {
		engine->_scene->currentScriptValue = *((int16 *)scriptPtr);
		if (actor->move.numOfStep == 0) {
			engine->_movements->moveActor(actor->angle, engine->_scene->currentScriptValue, actor->speed, move);
		}
		if (actor->angle == engine->_scene->currentScriptValue) {
			engine->_movements->clearRealAngle(actor);
			return 0;
		}
		continueMove = 0;
		actor->positionInMoveScript -= 3;
	}
	return 0;
}

/*0x08*/
static int32 mPOS_POINT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript++;
	engine->_scene->currentScriptValue = *(scriptPtr);

	engine->_renderer->destX = engine->_scene->sceneTracks[engine->_scene->currentScriptValue].x;
	engine->_renderer->destY = engine->_scene->sceneTracks[engine->_scene->currentScriptValue].y;
	engine->_renderer->destZ = engine->_scene->sceneTracks[engine->_scene->currentScriptValue].z;

	if (actor->staticFlags.bIsSpriteActor) {
		actor->speed = 0;
	}

	actor->x = engine->_renderer->destX;
	actor->y = engine->_renderer->destY;
	actor->z = engine->_renderer->destZ;

	return 0;
}

/*0x09*/
static int32 mLABEL(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->labelIdx = *(scriptPtr);
	actor->positionInMoveScript++;
	actor->currentLabelPtr = actor->positionInMoveScript - 2;
	return 0;
}

/*0x0A*/
static int32 mGOTO(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript = *((int16 *)scriptPtr);
	return 0;
}

/*0x0B*/
static int32 mSTOP(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	continueMove = 0;
	actor->positionInMoveScript = -1;
	return 0;
}

/*0x0C*/
static int32 mGOTO_SYM_POINT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 newAngle;

	actor->positionInMoveScript++;
	engine->_scene->currentScriptValue = *(scriptPtr);

	engine->_renderer->destX = engine->_scene->sceneTracks[engine->_scene->currentScriptValue].x;
	engine->_renderer->destY = engine->_scene->sceneTracks[engine->_scene->currentScriptValue].y;
	engine->_renderer->destZ = engine->_scene->sceneTracks[engine->_scene->currentScriptValue].z;

	newAngle = 0x200 + engine->_movements->getAngleAndSetTargetActorDistance(actor->x, actor->z, engine->_renderer->destX, engine->_renderer->destZ);

	if (actor->staticFlags.bIsSpriteActor) {
		actor->angle = newAngle;
	} else {
		engine->_movements->moveActor(actor->angle, newAngle, actor->speed, &actor->move);
	}

	if (engine->_movements->targetActorDistance > 500) {
		continueMove = 0;
		actor->positionInMoveScript -= 2;
	}

	return 0;
}

/*0x0D*/
static int32 mWAIT_NUM_ANIM(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;

	if (actor->dynamicFlags.bAnimEnded) {
		int32 animPos, animRepeats;

		animRepeats = *(scriptPtr);
		animPos = *(scriptPtr + 1);

		animPos++;

		if (animPos == animRepeats) {
			animPos = 0;
		} else {
			continueMove = 0;
		}

		*(scriptPtr + 1) = animPos;
	} else {
		continueMove = 0;
	}

	if (continueMove == 0) {
		actor->positionInMoveScript -= 3;
	}

	return 0;
}

/*0x0E*/
static int32 mSAMPLE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 sampleIdx = *((int16 *)scriptPtr);
	engine->_sound->playSample(sampleIdx, 0x1000, 1, actor->x, actor->y, actor->z, actorIdx);
	actor->positionInMoveScript += 2;
	return 0;
}

/*0x0F*/
static int32 mGOTO_POINT_3D(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript++;

	if (actor->staticFlags.bIsSpriteActor) {
		engine->_scene->currentScriptValue = *(scriptPtr);

		engine->_renderer->destX = engine->_scene->sceneTracks[engine->_scene->currentScriptValue].x;
		engine->_renderer->destY = engine->_scene->sceneTracks[engine->_scene->currentScriptValue].y;
		engine->_renderer->destZ = engine->_scene->sceneTracks[engine->_scene->currentScriptValue].z;

		actor->angle = engine->_movements->getAngleAndSetTargetActorDistance(actor->x, actor->z, engine->_renderer->destX, engine->_renderer->destZ);
		actor->animType = engine->_movements->getAngleAndSetTargetActorDistance(actor->y, 0, engine->_renderer->destY, engine->_movements->targetActorDistance);

		if (engine->_movements->targetActorDistance > 100) {
			continueMove = 0;
			actor->positionInMoveScript -= 2;
		} else {
			actor->x = engine->_renderer->destX;
			actor->y = engine->_renderer->destY;
			actor->z = engine->_renderer->destZ;
		}
	}

	return 0;
}

/*0x10*/
static int32 mSPEED(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	actor->speed = *((int16 *)scriptPtr);

	if (actor->staticFlags.bIsSpriteActor) {
		engine->_movements->setActorAngle(0, actor->speed, 50, move);
	}

	return 0;
}

/*0x11*/
static int32 mBACKGROUND(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript++;

	if (*(scriptPtr) != 0) {
		if (!actor->staticFlags.bIsBackgrounded) {
			actor->staticFlags.bIsBackgrounded = 1;
			if (actor->dynamicFlags.bIsVisible) {
				engine->_redraw->reqBgRedraw = 1;
			}
		}
	} else {
		if (actor->staticFlags.bIsBackgrounded) {
			actor->staticFlags.bIsBackgrounded = 0;
			if (actor->dynamicFlags.bIsVisible) {
				engine->_redraw->reqBgRedraw = 1;
			}
		}
	}

	return 0;
}

/*0x12*/
static int32 mWAIT_NUM_SECOND(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 numSeconds, currentTime;
	actor->positionInMoveScript += 5;

	numSeconds = *(scriptPtr++);
	currentTime = *((int32 *)scriptPtr);

	if (currentTime == 0) {
		currentTime = engine->lbaTime + numSeconds * 50;
		*((int32 *)scriptPtr) = currentTime;
	}

	if (engine->lbaTime < currentTime) {
		continueMove = 0;
		actor->positionInMoveScript -= 6;
	} else {
		*((int32 *)scriptPtr) = 0;
	}

	return 0;
}

/*0x13*/
static int32 mNO_BODY(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_actor->initModelActor(-1, actorIdx);
	return 0;
}

/*0x14*/
static int32 mBETA(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int16 beta;

	beta = *((int16 *)scriptPtr);
	actor->positionInMoveScript += 2;

	actor->angle = beta;

	if (actor->staticFlags.bIsSpriteActor) {
		engine->_movements->clearRealAngle(actor);
	}

	return 0;
}

/*0x15*/
static int32 mOPEN_LEFT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	if (actor->staticFlags.bIsSpriteActor && actor->staticFlags.bUsesClipping) {
		actor->angle = 0x300;
		actor->doorStatus = *((int16 *)scriptPtr);
		actor->dynamicFlags.bIsSpriteMoving = 1;
		actor->speed = 1000;
		engine->_movements->setActorAngle(0, 1000, 50, move);
	}
	return 0;
}

/*0x16*/
static int32 mOPEN_RIGHT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	if (actor->staticFlags.bIsSpriteActor && actor->staticFlags.bUsesClipping) {
		actor->angle = 0x100;
		actor->doorStatus = *((int16 *)scriptPtr);
		actor->dynamicFlags.bIsSpriteMoving = 1;
		actor->speed = 1000;
		engine->_movements->setActorAngle(0, 1000, 50, move);
	}
	return 0;
}

/*0x17*/
static int32 mOPEN_UP(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	if (actor->staticFlags.bIsSpriteActor && actor->staticFlags.bUsesClipping) {
		actor->angle = 0x200;
		actor->doorStatus = *((int16 *)scriptPtr);
		actor->dynamicFlags.bIsSpriteMoving = 1;
		actor->speed = 1000;
		engine->_movements->setActorAngle(0, 1000, 50, move);
	}
	return 0;
}

/*0x18*/
static int32 mOPEN_DOWN(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	if (actor->staticFlags.bIsSpriteActor && actor->staticFlags.bUsesClipping) {
		actor->angle = 0;
		actor->doorStatus = *((int16 *)scriptPtr);
		actor->dynamicFlags.bIsSpriteMoving = 1;
		actor->speed = 1000;
		engine->_movements->setActorAngle(0, 1000, 50, move);
	}
	return 0;
}

/*0x19*/
static int32 mCLOSE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	if (actor->staticFlags.bIsSpriteActor && actor->staticFlags.bUsesClipping) {
		actor->doorStatus = 0;
		actor->dynamicFlags.bIsSpriteMoving = 1;
		actor->speed = -1000;
		engine->_movements->setActorAngle(0, -1000, 50, move);
	}
	return 0;
}

/*0x1A*/
static int32 mWAIT_DOOR(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	if (actor->staticFlags.bIsSpriteActor && actor->staticFlags.bUsesClipping) {
		if (actor->speed) {
			continueMove = 0;
			actor->positionInMoveScript--;
		}
	}
	return 0;
}

/*0x1B*/
static int32 mSAMPLE_RND(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 freq = engine->getRandomNumber(2048) + 2048;
	int32 sampleIdx = *((int16 *)scriptPtr);
	engine->_sound->playSample(sampleIdx, freq, 1, actor->x, actor->y, actor->z, actorIdx);
	actor->positionInMoveScript += 2;
	return 0;
}

/*0x1C*/
static int32 mSAMPLE_ALWAYS(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 sampleIdx = *((int16 *)scriptPtr);
	if (!engine->_sound->isSamplePlaying(sampleIdx)) { // if its not playing
		engine->_sound->playSample(sampleIdx, 0x1000, -1, actor->x, actor->y, actor->z, actorIdx);
	}
	actor->positionInMoveScript += 2;
	return 0;
}

/*0x1D*/
static int32 mSAMPLE_STOP(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 sampleIdx = *((int16 *)scriptPtr);
	engine->_sound->stopSample(sampleIdx);
	actor->positionInMoveScript += 2;
	return 0;
}

/*0x1E*/
static int32 mPLAY_FLA(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	// TODO
	return -1;
}

/*0x1F*/
static int32 mREPEAT_SAMPLE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	numRepeatSample = *((int16 *)scriptPtr);
	actor->positionInMoveScript += 2;
	return 0;
}

/*0x20*/
static int32 mSIMPLE_SAMPLE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 sampleIdx = *((int16 *)scriptPtr);
	engine->_sound->playSample(sampleIdx, 0x1000, numRepeatSample, actor->x, actor->y, actor->z, actorIdx);
	numRepeatSample = 1;
	actor->positionInMoveScript += 2;
	return 0;
}

/*0x21*/
static int32 mFACE_HERO(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	if (!actor->staticFlags.bIsSpriteActor) {
		engine->_scene->currentScriptValue = *((int16 *)scriptPtr);
		if (engine->_scene->currentScriptValue == -1 && actor->move.numOfStep == 0) {
			engine->_scene->currentScriptValue = engine->_movements->getAngleAndSetTargetActorDistance(actor->x, actor->z, engine->_scene->sceneHero->x, engine->_scene->sceneHero->z);
			engine->_movements->moveActor(actor->angle, engine->_scene->currentScriptValue, actor->speed, &actor->move);
			*((int16 *)scriptPtr) = engine->_scene->currentScriptValue;
		}

		if (actor->angle != engine->_scene->currentScriptValue) {
			continueMove = 0;
			actor->positionInMoveScript -= 3;
		} else {
			engine->_movements->clearRealAngle(actor);
			*((int16 *)scriptPtr) = -1;
		}
	}
	return 0;
}

/*0x22*/
static int32 mANGLE_RND(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 newAngle;

	actor->positionInMoveScript += 4;
	if (!actor->staticFlags.bIsSpriteActor) {
		engine->_scene->currentScriptValue = *((int16 *)scriptPtr + 2);

		if (engine->_scene->currentScriptValue == -1 && actor->move.numOfStep == 0) {
			if (engine->getRandomNumber() & 1) {
				engine->_scene->currentScriptValue = *((int16 *)scriptPtr);
				newAngle = actor->angle + 0x100 + (ABS(engine->_scene->currentScriptValue) >> 1);
				engine->_scene->currentScriptValue = (newAngle - engine->getRandomNumber(engine->_scene->currentScriptValue)) & 0x3FF;
			} else {
				engine->_scene->currentScriptValue = *((int16 *)scriptPtr);
				newAngle = actor->angle - 0x100 + (ABS(engine->_scene->currentScriptValue) >> 1);
				engine->_scene->currentScriptValue = (newAngle - engine->getRandomNumber(engine->_scene->currentScriptValue)) & 0x3FF;
			}

			engine->_movements->moveActor(actor->angle, engine->_scene->currentScriptValue, actor->speed, &actor->move);
			*((int16 *)scriptPtr + 2) = engine->_scene->currentScriptValue;
		}

		if (actor->angle != engine->_scene->currentScriptValue) {
			continueMove = 0;
			actor->positionInMoveScript -= 5;
		} else {
			engine->_movements->clearRealAngle(actor);
			*((int16 *)scriptPtr + 2) = -1;
		}
	}
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
	continueMove = 0;
	scriptPosition = 0;
	move = nullptr;
	numRepeatSample = 1;
}

void ScriptMove::processMoveScript(int32 actorIdx) {
	int32 scriptOpcode;
	ActorStruct *actor;

	continueMove = 1;
	actor = &_engine->_scene->sceneActors[actorIdx];
	move = &actor->move;

	do {
		scriptPosition = actor->positionInMoveScript;
		scriptPtr = actor->moveScript + scriptPosition;
		scriptOpcode = *(scriptPtr++);

		actor->positionInMoveScript++;

		if (scriptOpcode >= 0 && scriptOpcode < ARRAYSIZE(function_map)) {
			function_map[scriptOpcode].function(_engine, actorIdx, actor);
		} else {
			error("Wrong opcode %i", scriptOpcode);
		}
	} while (continueMove);
}

} // namespace TwinE
