/** @file script.move.cpp
	@brief
	This file contains movies routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "script.move.h"
#include "scene.h"
#include "actor.h"
#include "movements.h"
#include "animations.h"
#include "scene.h"
#include "renderer.h"
#include "sound.h"
#include "redraw.h"
#include "lbaengine.h"

uint8 *scriptPtr;
int32 continueMove;
int32 scriptPosition;
ActorMoveStruct *move;
int32 numRepeatSample = 1;

typedef int32 ScriptMoveFunc(int32 actorIdx, ActorStruct *actor);

typedef struct ScriptMoveFunction {
	const uint8 *name;
	ScriptMoveFunc *function;
} ScriptMoveFunction;

#define MAPFUNC(name, func) {(uint8*)name, func}


/*0x00*/
int32 mEND(int32 actorIdx, ActorStruct *actor) {
	continueMove = 0;
	actor->positionInMoveScript = -1;
	return 0;
}

/*0x01*/
int32 mNOP(int32 actorIdx, ActorStruct *actor) {
	return 0;
}

/*0x02*/
int32 mBODY(int32 actorIdx, ActorStruct *actor) {
	int32 bodyIdx = *(scriptPtr);
	initModelActor(bodyIdx, actorIdx);
	actor->positionInMoveScript++;
	return 0;
}

/*0x03*/
int32 mANIM(int32 actorIdx, ActorStruct *actor) {
	int32 animIdx = *(scriptPtr++);
	if (initAnim(animIdx, 0, 0, actorIdx)) {
		actor->positionInMoveScript++;
	} else {
		actor->positionInMoveScript = scriptPosition;
		continueMove = 0;
	}
	return 0;
}

/*0x04*/
int32 mGOTO_POINT(int32 actorIdx, ActorStruct *actor) {
	int32 newAngle;

	actor->positionInMoveScript++;
	currentScriptValue = *(scriptPtr);

	destX = sceneTracks[currentScriptValue].X;
	destY = sceneTracks[currentScriptValue].Y;
	destZ = sceneTracks[currentScriptValue].Z;

	newAngle = getAngleAndSetTargetActorDistance(actor->X, actor->Z, destX, destZ);

	if (actor->staticFlags.bIsSpriteActor) {
		actor->angle = newAngle;
	} else {
		moveActor(actor->angle, newAngle, actor->speed, &actor->move);
	}

	if (targetActorDistance > 500) {
		continueMove = 0;
		actor->positionInMoveScript -= 2;
	}

	return 0;
}

/*0x05*/
int32 mWAIT_ANIM(int32 actorIdx, ActorStruct *actor) {
	if (!actor->dynamicFlags.bAnimEnded) {
		continueMove = 0;
		actor->positionInMoveScript--;
	} else {
		continueMove = 0;
		clearRealAngle(actor);
	}
	return 0;
}

/*0x06*/
int32 mLOOP(int32 actorIdx, ActorStruct *actor) {
	// TODO
	return -1;
}

/*0x07*/
int32 mANGLE(int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	if (!actor->staticFlags.bIsSpriteActor) {
		currentScriptValue = *((int16 *)scriptPtr);
		if (actor->move.numOfStep == 0) {
			moveActor(actor->angle, currentScriptValue, actor->speed, move);
		}
		if (actor->angle == currentScriptValue) {
			clearRealAngle(actor);
			return 0;
		}
		continueMove = 0;
		actor->positionInMoveScript -= 3;
	}
	return 0;
}

/*0x08*/
int32 mPOS_POINT(int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript++;
	currentScriptValue = *(scriptPtr);

	destX = sceneTracks[currentScriptValue].X;
	destY = sceneTracks[currentScriptValue].Y;
	destZ = sceneTracks[currentScriptValue].Z;

	if (actor->staticFlags.bIsSpriteActor) {
		actor->speed = 0;
	}

	actor->X = destX;
	actor->Y = destY;
	actor->Z = destZ;

	return 0;
}

/*0x09*/
int32 mLABEL(int32 actorIdx, ActorStruct *actor) {
	actor->labelIdx = *(scriptPtr);
	actor->positionInMoveScript++;
	actor->currentLabelPtr = actor->positionInMoveScript - 2;
	return 0;
}

/*0x0A*/
int32 mGOTO(int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript = *((int16 *)scriptPtr);
	return 0;
}

/*0x0B*/
int32 mSTOP(int32 actorIdx, ActorStruct *actor) {
	continueMove = 0;
	actor->positionInMoveScript = -1;
	return 0;
}

/*0x0C*/
int32 mGOTO_SYM_POINT(int32 actorIdx, ActorStruct *actor) {
	int32 newAngle;

	actor->positionInMoveScript++;
	currentScriptValue = *(scriptPtr);

	destX = sceneTracks[currentScriptValue].X;
	destY = sceneTracks[currentScriptValue].Y;
	destZ = sceneTracks[currentScriptValue].Z;

	newAngle = 0x200 + getAngleAndSetTargetActorDistance(actor->X, actor->Z, destX, destZ);

	if (actor->staticFlags.bIsSpriteActor) {
		actor->angle = newAngle;
	} else {
		moveActor(actor->angle, newAngle, actor->speed, &actor->move);
	}

	if (targetActorDistance > 500) {
		continueMove = 0;
		actor->positionInMoveScript -= 2;
	}

	return 0;
}

/*0x0D*/
int32 mWAIT_NUM_ANIM(int32 actorIdx, ActorStruct *actor) {
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
int32 mSAMPLE(int32 actorIdx, ActorStruct *actor) {
	int32 sampleIdx = *((int16 *)scriptPtr);
	playSample(sampleIdx, 0x1000, 1, actor->X, actor->Y, actor->Z, actorIdx);
	actor->positionInMoveScript += 2;
	return 0;
}

/*0x0F*/
int32 mGOTO_POINT_3D(int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript++;

	if (actor->staticFlags.bIsSpriteActor) {
		currentScriptValue = *(scriptPtr);

		destX = sceneTracks[currentScriptValue].X;
		destY = sceneTracks[currentScriptValue].Y;
		destZ = sceneTracks[currentScriptValue].Z;

		actor->angle = getAngleAndSetTargetActorDistance(actor->X, actor->Z, destX, destZ);
		actor->animType = getAngleAndSetTargetActorDistance(actor->Y, 0, destY, targetActorDistance);

		if (targetActorDistance > 100) {
			continueMove = 0;
			actor->positionInMoveScript -= 2;
		} else {
			actor->X = destX;
			actor->Y = destY;
			actor->Z = destZ;
		}
	}

	return 0;
}

/*0x10*/
int32 mSPEED(int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	actor->speed = *((int16 *)scriptPtr);

	if (actor->staticFlags.bIsSpriteActor) {
		setActorAngle(0, actor->speed, 50, move);
	}

	return 0;
}

/*0x11*/
int32 mBACKGROUND(int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript++;

	if (*(scriptPtr) != 0) {
		if (!actor->staticFlags.bIsBackgrounded) {
			actor->staticFlags.bIsBackgrounded = 1;
			if (actor->dynamicFlags.bIsVisible) {
				reqBgRedraw = 1;
			}
		}
	} else {
		if (actor->staticFlags.bIsBackgrounded) {
			actor->staticFlags.bIsBackgrounded = 0;
			if (actor->dynamicFlags.bIsVisible) {
				reqBgRedraw = 1;
			}
		}
	}

	return 0;
}

/*0x12*/
int32 mWAIT_NUM_SECOND(int32 actorIdx, ActorStruct *actor) {
	int32 numSeconds, currentTime;
	actor->positionInMoveScript += 5;

	numSeconds  = *(scriptPtr++);
	currentTime = *((int32 *)scriptPtr);

	if (currentTime == 0) {
		currentTime = lbaTime + numSeconds * 50;
		*((int32 *)scriptPtr) = currentTime;
	}

	if (lbaTime < currentTime) {
		continueMove = 0;
		actor->positionInMoveScript -= 6;
	} else {
		*((int32 *)scriptPtr) = 0;
	}

	return 0;
}

/*0x13*/
int32 mNO_BODY(int32 actorIdx, ActorStruct *actor) {
	initModelActor(-1, actorIdx);
	return 0;
}

/*0x14*/
int32 mBETA(int32 actorIdx, ActorStruct *actor) {
	int16 beta;

	beta = *((int16 *)scriptPtr);
	actor->positionInMoveScript += 2;

	actor->angle = beta;

	if (actor->staticFlags.bIsSpriteActor) {
		clearRealAngle(actor);
	}

	return 0;
}

/*0x15*/
int32 mOPEN_LEFT(int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	if (actor->staticFlags.bIsSpriteActor && actor->staticFlags.bUsesClipping) {
		actor->angle = 0x300;
		actor->doorStatus = *((int16 *)scriptPtr);
		actor->dynamicFlags.bIsSpriteMoving = 1;
		actor->speed = 1000;
		setActorAngle(0, 1000, 50, move);
	}
	return 0;
}

/*0x16*/
int32 mOPEN_RIGHT(int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	if (actor->staticFlags.bIsSpriteActor && actor->staticFlags.bUsesClipping) {
		actor->angle = 0x100;
		actor->doorStatus = *((int16 *)scriptPtr);
		actor->dynamicFlags.bIsSpriteMoving = 1;
		actor->speed = 1000;
		setActorAngle(0, 1000, 50, move);
	}
	return 0;
}

/*0x17*/
int32 mOPEN_UP(int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	if (actor->staticFlags.bIsSpriteActor && actor->staticFlags.bUsesClipping) {
		actor->angle = 0x200;
		actor->doorStatus = *((int16 *)scriptPtr);
		actor->dynamicFlags.bIsSpriteMoving = 1;
		actor->speed = 1000;
		setActorAngle(0, 1000, 50, move);
	}
	return 0;
}

/*0x18*/
int32 mOPEN_DOWN(int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	if (actor->staticFlags.bIsSpriteActor && actor->staticFlags.bUsesClipping) {
		actor->angle = 0;
		actor->doorStatus = *((int16 *)scriptPtr);
		actor->dynamicFlags.bIsSpriteMoving = 1;
		actor->speed = 1000;
		setActorAngle(0, 1000, 50, move);
	}
	return 0;
}

/*0x19*/
int32 mCLOSE(int32 actorIdx, ActorStruct *actor) {
	if (actor->staticFlags.bIsSpriteActor && actor->staticFlags.bUsesClipping) {
		actor->doorStatus = 0;
		actor->dynamicFlags.bIsSpriteMoving = 1;
		actor->speed = -1000;
		setActorAngle(0, -1000, 50, move);
	}
	return 0;
}

/*0x1A*/
int32 mWAIT_DOOR(int32 actorIdx, ActorStruct *actor) {
	if (actor->staticFlags.bIsSpriteActor && actor->staticFlags.bUsesClipping) {
		if (actor->speed) {
			continueMove = 0;
			actor->positionInMoveScript--;
		}
	}
	return 0;
}

/*0x1B*/
int32 mSAMPLE_RND(int32 actorIdx, ActorStruct *actor) {
	int32 freq = Rnd(2048) + 2048;
	int32 sampleIdx = *((int16 *)scriptPtr);
	playSample(sampleIdx, freq, 1, actor->X, actor->Y, actor->Z, actorIdx);
	actor->positionInMoveScript += 2;
	return 0;
}

/*0x1C*/
int32 mSAMPLE_ALWAYS(int32 actorIdx, ActorStruct *actor) {
	int32 sampleIdx = *((int16 *)scriptPtr);
	if (getSampleChannel(sampleIdx) == -1) { // if its not playing
		playSample(sampleIdx, 0x1000, -1, actor->X, actor->Y, actor->Z, actorIdx);
	}
	actor->positionInMoveScript += 2;
	return 0;
}

/*0x1D*/
int32 mSAMPLE_STOP(int32 actorIdx, ActorStruct *actor) {
	int32 sampleIdx = *((int16 *)scriptPtr);
	stopSample(sampleIdx);
	actor->positionInMoveScript += 2;
	return 0;
}

/*0x1E*/
int32 mPLAY_FLA(int32 actorIdx, ActorStruct *actor) {
	// TODO
	return -1;
}

/*0x1F*/
int32 mREPEAT_SAMPLE(int32 actorIdx, ActorStruct *actor) {
	numRepeatSample = *((int16 *)scriptPtr);
	actor->positionInMoveScript += 2;
	return 0;
}

/*0x20*/
int32 mSIMPLE_SAMPLE(int32 actorIdx, ActorStruct *actor) {
	int32 sampleIdx = *((int16 *)scriptPtr);
	playSample(sampleIdx, 0x1000, numRepeatSample, actor->X, actor->Y, actor->Z, actorIdx);
	numRepeatSample = 1;
	actor->positionInMoveScript += 2;
	return 0;
}

/*0x21*/
int32 mFACE_HERO(int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript += 2;
	if (!actor->staticFlags.bIsSpriteActor) {
		currentScriptValue = *((int16 *)scriptPtr);
		if (currentScriptValue == -1 && actor->move.numOfStep == 0) {
			currentScriptValue = getAngleAndSetTargetActorDistance(actor->X, actor->Z, sceneHero->X, sceneHero->Z);
			moveActor(actor->angle, currentScriptValue, actor->speed, &actor->move);
			*((int16 *)scriptPtr) = currentScriptValue;
		}

		if (actor->angle != currentScriptValue) {
			continueMove = 0;
			actor->positionInMoveScript -= 3;
		} else {
			clearRealAngle(actor);
			*((int16 *)scriptPtr) = -1;
		}
	}
	return 0;
}

/*0x22*/
int32 mANGLE_RND(int32 actorIdx, ActorStruct *actor) {
	int32 newAngle;

	actor->positionInMoveScript += 4;
	if (!actor->staticFlags.bIsSpriteActor) {
		currentScriptValue = *((int16 *)scriptPtr + 2);

		if (currentScriptValue == -1 && actor->move.numOfStep == 0) {
			if (rand() & 1) {
				currentScriptValue = *((int16 *)scriptPtr);
				newAngle = actor->angle + 0x100 + (Abs(currentScriptValue) >> 1);
				currentScriptValue = (newAngle - Rnd(currentScriptValue)) & 0x3FF;
			} else {
				currentScriptValue = *((int16 *)scriptPtr);
				newAngle = actor->angle - 0x100 + (Abs(currentScriptValue) >> 1);
				currentScriptValue = (newAngle - Rnd(currentScriptValue)) & 0x3FF;
			}

			moveActor(actor->angle, currentScriptValue, actor->speed, &actor->move);
			*((int16 *)scriptPtr + 2) = currentScriptValue;
		}

		if (actor->angle != currentScriptValue) {
			continueMove = 0;
			actor->positionInMoveScript -= 5;
		} else {
			clearRealAngle(actor);
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
	/*0x22*/ MAPFUNC("ANGLE_RND", mANGLE_RND)
};

/** Process actor move script
	@param actorIdx Current processed actor index */
void processMoveScript(int32 actorIdx) {
	int32 scriptOpcode;
	ActorStruct *actor;

	continueMove = 1;
	actor = &sceneActors[actorIdx];
	move  = &actor->move;

	do {
		scriptPosition = actor->positionInMoveScript;
		scriptPtr      = actor->moveScript + scriptPosition;
		scriptOpcode   = *(scriptPtr++);

		actor->positionInMoveScript++;

		function_map[scriptOpcode].function(actorIdx, actor);
	} while(continueMove);
}
