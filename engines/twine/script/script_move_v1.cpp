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

#include "twine/script/script_move_v1.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "twine/scene/actor.h"
#include "twine/scene/animations.h"
#include "twine/audio/sound.h"
#include "twine/flamovies.h"
#include "twine/scene/movements.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/scene/scene.h"
#include "twine/twine.h"

namespace TwinE {

struct MoveScriptContext {
	int32 actorIdx;
	ActorStruct *actor;
	int32 numRepeatSample = 1;

	Common::MemorySeekableReadWriteStream stream;

	MoveScriptContext(int32 _actorIdx, ActorStruct *_actor) : actorIdx(_actorIdx), actor(_actor), stream(actor->moveScript, actor->moveScriptSize) {
		assert(actor->positionInMoveScript >= 0);
		stream.skip(actor->positionInMoveScript);
	}

	void undo(int32 bytes) {
		assert(bytes >= 0);
		// the additional 1 byte is for the opcode
		stream.rewind(bytes + 1);
	}
};

/**
 * Returns @c -1 Need implementation, @c 0 Condition false, @c 1 - Condition true
 */
typedef int32 ScriptMoveFunc(TwinEEngine *engine, MoveScriptContext &ctx);

struct ScriptMoveFunction {
	const char *name;
	ScriptMoveFunc *function;
};

#define MAPFUNC(name, func) \
	{ name, func }

/**
 * End of Actor Move Script
 * @note Opcode @c 0x00
 */
static int32 mEND(TwinEEngine *engine, MoveScriptContext &ctx) {
	ctx.actor->positionInMoveScript = -1;
	return 1;
}

/**
 * No Operation
 * @note Opcode @c 0x01
 */
static int32 mNOP(TwinEEngine *engine, MoveScriptContext &ctx) {
	return 0;
}

/**
 * Choose new body for the current actor (Parameter = File3D Body Instance)
 * @note Opcode @c 0x02
 */
static int32 mBODY(TwinEEngine *engine, MoveScriptContext &ctx) {
	int32 bodyIdx = ctx.stream.readByte();
	engine->_actor->initModelActor(bodyIdx, ctx.actorIdx);
	return 0;
}

/**
 * Choose new animation for the current actor (Parameter = File3D Animation Instance)
 * @note Opcode @c 0x03
 */
static int32 mANIM(TwinEEngine *engine, MoveScriptContext &ctx) {
	AnimationTypes animIdx = (AnimationTypes)ctx.stream.readByte();
	if (engine->_animations->initAnim(animIdx, 0, AnimationTypes::kStanding, ctx.actorIdx)) {
		return 0;
	}
	ctx.undo(1);
	return 1;
}

/**
 * Tell the actor to go to a new position (Parameter = Track Index)
 * @note Opcode @c 0x04
 */
static int32 mGOTO_POINT(TwinEEngine *engine, MoveScriptContext &ctx) {
	engine->_scene->currentScriptValue = ctx.stream.readByte();

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
		ctx.undo(1);
		return 1;
	}

	return 0;
}

/**
 * Wait the end of the current animation
 * @note Opcode @c 0x05
 */
static int32 mWAIT_ANIM(TwinEEngine *engine, MoveScriptContext &ctx) {
	if (!ctx.actor->dynamicFlags.bAnimEnded) {
		ctx.undo(0);
	} else {
		engine->_movements->clearRealAngle(ctx.actor);
	}
	return 1;
}

/**
 * Loop a certain label (Parameter = Label Number)
 * @note Opcode @c 0x06
 */
static int32 mLOOP(TwinEEngine *engine, MoveScriptContext &ctx) {
	// TODO no params
	return -1;
}

/**
 * Make the actor turn around
 * @note Opcode @c 0x07
 */
static int32 mANGLE(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 angle = ToAngle(ctx.stream.readSint16LE());
	if (ctx.actor->staticFlags.bIsSpriteActor) {
		return 0;
	}
	engine->_scene->currentScriptValue = angle;
	if (ctx.actor->move.numOfStep == 0) {
		engine->_movements->moveActor(ctx.actor->angle, angle, ctx.actor->speed, &ctx.actor->move);
	}
	if (ctx.actor->angle == angle) {
		engine->_movements->clearRealAngle(ctx.actor);
		return 0;
	}
	ctx.undo(2);
	return 1;
}

/**
 * Set new postion for the current actor (Parameter = Track Index)
 * @note Opcode @c 0x08
 */
static int32 mPOS_POINT(TwinEEngine *engine, MoveScriptContext &ctx) {
	engine->_scene->currentScriptValue = ctx.stream.readByte();

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

/**
 * Specify a new label (Parameter = Label Number)
 * @note Opcode @c 0x09
 */
static int32 mLABEL(TwinEEngine *engine, MoveScriptContext &ctx) {
	ctx.actor->labelIdx = ctx.stream.readByte();
	ctx.actor->currentLabelPtr = ctx.stream.pos() - 2;
	return 0;
}

/**
 * Go to a certain label (Parameter = Label Number)
 * @note Opcode @c 0x0A
 */
static int32 mGOTO(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 pos = ctx.stream.readSint16LE();
	if (pos == -1) {
		ctx.actor->positionInMoveScript = -1;
		return 1;
	}
	ctx.stream.seek(pos);
	return 0;
}

/**
 * Tell the actor to stop the current animation
 * @note Opcode @c 0x0B
 */
static int32 mSTOP(TwinEEngine *engine, MoveScriptContext &ctx) {
	ctx.actor->positionInMoveScript = -1;
	return 1;
}

/**
 * Tell the actor to go to a symbolic point
 * @note Opcode @c 0x0C
 */
static int32 mGOTO_SYM_POINT(TwinEEngine *engine, MoveScriptContext &ctx) {
	engine->_scene->currentScriptValue = ctx.stream.readByte();

	const ScenePoint &sp = engine->_scene->sceneTracks[engine->_scene->currentScriptValue];
	engine->_renderer->destX = sp.x;
	engine->_renderer->destY = sp.y;
	engine->_renderer->destZ = sp.z;

	const int32 newAngle = ANGLE_180 + engine->_movements->getAngleAndSetTargetActorDistance(ctx.actor->x, ctx.actor->z, sp.x, sp.z);

	if (ctx.actor->staticFlags.bIsSpriteActor) {
		ctx.actor->angle = newAngle;
	} else {
		engine->_movements->moveActor(ctx.actor->angle, newAngle, ctx.actor->speed, &ctx.actor->move);
	}

	if (engine->_movements->targetActorDistance > 500) {
		ctx.undo(1);
		return 1;
	}

	return 0;
}

/**
 * Wait a certain number of frame update in the current animation
 * @note Opcode @c 0x0D
 */
static int32 mWAIT_NUM_ANIM(TwinEEngine *engine, MoveScriptContext &ctx) {
	bool abortMove = false;
	const int32 animRepeats = ctx.stream.readByte();
	int32 animPos = ctx.stream.readByte();
	if (ctx.actor->dynamicFlags.bAnimEnded) {
		animPos++;

		if (animPos == animRepeats) {
			animPos = 0;
		} else {
			abortMove = true;
		}

		ctx.stream.rewind(1);
		ctx.stream.writeByte(animPos);
	} else {
		abortMove = true;
	}

	if (abortMove) {
		ctx.undo(2);
	}

	return abortMove ? 1 : 0;
}

/**
 * Play a sample (Parameter = Sample index)
 * @note Opcode @c 0x0E
 */
static int32 mSAMPLE(TwinEEngine *engine, MoveScriptContext &ctx) {
	int32 sampleIdx = ctx.stream.readSint16LE();
	engine->_sound->playSample(sampleIdx, 1, ctx.actor->x, ctx.actor->y, ctx.actor->z, ctx.actorIdx);
	return 0;
}

/**
 * Tell the actor to go to a new position (Parameter = Track Index)
 * @note Opcode @c 0x0F
 */
static int32 mGOTO_POINT_3D(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int32 trackId = ctx.stream.readByte();
	if (!ctx.actor->staticFlags.bIsSpriteActor) {
		return 0;
	}

	engine->_scene->currentScriptValue = trackId;

	const ScenePoint &sp = engine->_scene->sceneTracks[engine->_scene->currentScriptValue];
	engine->_renderer->destX = sp.x;
	engine->_renderer->destY = sp.y;
	engine->_renderer->destZ = sp.z;

	ctx.actor->angle = engine->_movements->getAngleAndSetTargetActorDistance(ctx.actor->x, ctx.actor->z, sp.x, sp.z);
	// TODO: this adds an angle to the animType value
	ctx.actor->animType = engine->_movements->getAngleAndSetTargetActorDistance(ctx.actor->y, 0, sp.y, engine->_movements->targetActorDistance);

	if (engine->_movements->targetActorDistance > 100) {
		ctx.undo(1);
		return 1;
	}
	ctx.actor->x = sp.x;
	ctx.actor->y = sp.y;
	ctx.actor->z = sp.z;

	return 0;
}

/**
 * Specify a new rotation speed for the current actor (Parameter = Rotation speed) [ 0 means fast, 32767 means slow ]
 * @note Opcode @c 0x10
 */
static int32 mSPEED(TwinEEngine *engine, MoveScriptContext &ctx) {
	ctx.actor->speed = ctx.stream.readSint16LE();

	if (ctx.actor->staticFlags.bIsSpriteActor) {
		engine->_movements->setActorAngle(ANGLE_0, ctx.actor->speed, ANGLE_17, &ctx.actor->move);
	}

	return 0;
}

/**
 * Set actor as background (Parameter = 1 (true); = 0 (false))
 * @note Opcode @c 0x11
 */
static int32 mBACKGROUND(TwinEEngine *engine, MoveScriptContext &ctx) {
	if (ctx.stream.readByte() != 0) {
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

/**
 * Number os seconds to wait.
 * @note Opcode @c 0x12
 */
static int32 mWAIT_NUM_SECOND(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int32 numSeconds = ctx.stream.readByte();
	int32 currentTime = ctx.stream.readSint32LE();

	if (currentTime == 0) {
		currentTime = engine->lbaTime + numSeconds * 50;
		ctx.stream.rewind(4);
		ctx.stream.writeSint32LE(currentTime);
	}

	if (engine->lbaTime < currentTime) {
		ctx.undo(5);
		return 1;
	}

	ctx.stream.rewind(4);
	ctx.stream.writeSint32LE(0);

	return 0;
}

/**
 * To not use Bodies.
 * @note Opcode @c 0x13
 */
static int32 mNO_BODY(TwinEEngine *engine, MoveScriptContext &ctx) {
	engine->_actor->initModelActor(-1, ctx.actorIdx);
	return 0;
}

/**
 * Change actor orientation. (Parameter = New Angle)
 * @note Opcode @c 0x14
 */
static int32 mBETA(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 beta = ctx.stream.readSint16LE();

	ctx.actor->angle = beta;

	if (ctx.actor->staticFlags.bIsSpriteActor) {
		engine->_movements->clearRealAngle(ctx.actor);
	}

	return 0;
}

/**
 * Open the door (left way) (Parameter = distance to open).
 * @note Opcode @c 0x15
 */
static int32 mOPEN_LEFT(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 doorStatus = ctx.stream.readSint16LE();
	if (ctx.actor->staticFlags.bIsSpriteActor && ctx.actor->staticFlags.bUsesClipping) {
		ctx.actor->angle = ANGLE_270;
		ctx.actor->doorStatus = doorStatus;
		ctx.actor->dynamicFlags.bIsSpriteMoving = 1;
		ctx.actor->speed = 1000;
		engine->_movements->setActorAngle(ANGLE_0, ANGLE_351, ANGLE_17, &ctx.actor->move);
	}
	return 0;
}

/**
 * Open the door (right way) (Parameter = distance to open).
 * @note Opcode @c 0x16
 */
static int32 mOPEN_RIGHT(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 doorStatus = ctx.stream.readSint16LE();
	if (ctx.actor->staticFlags.bIsSpriteActor && ctx.actor->staticFlags.bUsesClipping) {
		ctx.actor->angle = ANGLE_90;
		ctx.actor->doorStatus = doorStatus;
		ctx.actor->dynamicFlags.bIsSpriteMoving = 1;
		ctx.actor->speed = 1000;
		engine->_movements->setActorAngle(ANGLE_0, ANGLE_351, ANGLE_17, &ctx.actor->move);
	}
	return 0;
}

/**
 * Open the door (up way) (Parameter = distance to open).
 * @note Opcode @c 0x17
 */
static int32 mOPEN_UP(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 doorStatus = ctx.stream.readSint16LE();
	if (ctx.actor->staticFlags.bIsSpriteActor && ctx.actor->staticFlags.bUsesClipping) {
		ctx.actor->angle = ANGLE_180;
		ctx.actor->doorStatus = doorStatus;
		ctx.actor->dynamicFlags.bIsSpriteMoving = 1;
		ctx.actor->speed = 1000;
		engine->_movements->setActorAngle(ANGLE_0, ANGLE_351, ANGLE_17, &ctx.actor->move);
	}
	return 0;
}

/**
 * Open the door (down way) (Parameter = distance to open).
 * @note Opcode @c 0x18
 */
static int32 mOPEN_DOWN(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 doorStatus = ctx.stream.readSint16LE();
	if (ctx.actor->staticFlags.bIsSpriteActor && ctx.actor->staticFlags.bUsesClipping) {
		ctx.actor->angle = ANGLE_0;
		ctx.actor->doorStatus = doorStatus;
		ctx.actor->dynamicFlags.bIsSpriteMoving = 1;
		ctx.actor->speed = 1000;
		engine->_movements->setActorAngle(ANGLE_0, ANGLE_351, ANGLE_17, &ctx.actor->move);
	}
	return 0;
}

/**
 * Close the door.
 * @note Opcode @c 0x19
 */
static int32 mCLOSE(TwinEEngine *engine, MoveScriptContext &ctx) {
	if (ctx.actor->staticFlags.bIsSpriteActor && ctx.actor->staticFlags.bUsesClipping) {
		ctx.actor->doorStatus = 0;
		ctx.actor->dynamicFlags.bIsSpriteMoving = 1;
		ctx.actor->speed = -1000;
		engine->_movements->setActorAngle(ANGLE_0, -ANGLE_351, ANGLE_17, &ctx.actor->move);
	}
	return 0;
}

/**
 * Wait till door close.
 * @note Opcode @c 0x1A
 */
static int32 mWAIT_DOOR(TwinEEngine *engine, MoveScriptContext &ctx) {
	if (ctx.actor->staticFlags.bIsSpriteActor && ctx.actor->staticFlags.bUsesClipping) {
		if (ctx.actor->speed) {
			ctx.undo(0);
			return 1;
		}
	}
	return 0;
}

/**
 * Generate a random sample.
 * @note Opcode @c 0x1B
 */
static int32 mSAMPLE_RND(TwinEEngine *engine, MoveScriptContext &ctx) {
	int32 sampleIdx = ctx.stream.readSint16LE();
	engine->_sound->playSample(sampleIdx, 1, ctx.actor->x, ctx.actor->y, ctx.actor->z, ctx.actorIdx);
	return 0;
}

/**
 * Play always the sample (Parameter = Sample index)
 * @note Opcode @c 0x1C
 */
static int32 mSAMPLE_ALWAYS(TwinEEngine *engine, MoveScriptContext &ctx) {
	int32 sampleIdx = ctx.stream.readSint16LE();
	if (!engine->_sound->isSamplePlaying(sampleIdx)) { // if its not playing
		engine->_sound->playSample(sampleIdx, -1, ctx.actor->x, ctx.actor->y, ctx.actor->z, ctx.actorIdx);
	}
	return 0;
}

/**
 * Stop playing the sample
 * @note Opcode @c 0x1D
 */
static int32 mSAMPLE_STOP(TwinEEngine *engine, MoveScriptContext &ctx) {
	int32 sampleIdx = ctx.stream.readSint16LE();
	engine->_sound->stopSample(sampleIdx);
	return 0;
}

/**
 * Play FLA cutscenes (Parameter = Cutscene Name)
 * @note Opcode @c 0x1E
 */
static int32 mPLAY_FLA(TwinEEngine *engine, MoveScriptContext &ctx) {
	int strIdx = 0;
	char movie[64];
	do {
		const byte c = ctx.stream.readByte();
		movie[strIdx++] = c;
		if (c == '\0') {
			break;
		}
		if (strIdx >= ARRAYSIZE(movie)) {
			error("Max string size exceeded for fla name");
		}
	} while (true);

	engine->_flaMovies->playFlaMovie(movie);
	engine->setPalette(engine->_screens->paletteRGBA);
	engine->_screens->clearScreen();
	engine->flip();
	return 0;
}

/**
 * Repeat sample (Parameter = Sample index).
 * @note Opcode @c 0x1F
 */
static int32 mREPEAT_SAMPLE(TwinEEngine *engine, MoveScriptContext &ctx) {
	ctx.numRepeatSample = ctx.stream.readSint16LE();
	return 0;
}

/**
 * Play a sample (Parameter = Sample index)
 * @note Opcode @c 0x20
 */
static int32 mSIMPLE_SAMPLE(TwinEEngine *engine, MoveScriptContext &ctx) {
	int32 sampleIdx = ctx.stream.readSint16LE();
	engine->_sound->playSample(sampleIdx, ctx.numRepeatSample, ctx.actor->x, ctx.actor->y, ctx.actor->z, ctx.actorIdx);
	ctx.numRepeatSample = 1;
	return 0;
}

/**
 * The actor rotate to Twinsen direction (Parameter = -1 (near); = 0 (far))
 * @note Opcode @c 0x21
 */
static int32 mFACE_HERO(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 angle = ToAngle(ctx.stream.readSint16LE());
	if (ctx.actor->staticFlags.bIsSpriteActor) {
		return 0;
	}
	engine->_scene->currentScriptValue = angle;
	if (engine->_scene->currentScriptValue == -1 && ctx.actor->move.numOfStep == 0) {
		engine->_scene->currentScriptValue = engine->_movements->getAngleAndSetTargetActorDistance(ctx.actor->x, ctx.actor->z, engine->_scene->sceneHero->x, engine->_scene->sceneHero->z);
		engine->_movements->moveActor(ctx.actor->angle, engine->_scene->currentScriptValue, ctx.actor->speed, &ctx.actor->move);
		ctx.stream.rewind(2);
		ctx.stream.writeSint16LE(engine->_scene->currentScriptValue);
	}

	if (ctx.actor->angle != engine->_scene->currentScriptValue) {
		ctx.undo(2);
		return 1;
	}
	engine->_movements->clearRealAngle(ctx.actor);
	ctx.stream.rewind(2);
	ctx.stream.writeSint16LE(-1);
	return 0;
}

/**
 * Generate an random angle for the current actor
 * @note Opcode @c 0x22
 */
static int32 mANGLE_RND(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 val1 = ctx.stream.readSint16LE();
	const int16 val2 = ctx.stream.readSint16LE();
	if (ctx.actor->staticFlags.bIsSpriteActor) {
		return 0;
	}

	engine->_scene->currentScriptValue = val2;

	if (engine->_scene->currentScriptValue == -1 && ctx.actor->move.numOfStep == 0) {
		if (engine->getRandomNumber() & 1) {
			const int32 newAngle = ctx.actor->angle + ANGLE_90 + (ABS(val1) >> 1);
			engine->_scene->currentScriptValue = ClampAngle(newAngle - engine->getRandomNumber(val1));
		} else {
			const int32 newAngle = ctx.actor->angle - ANGLE_90 + (ABS(val1) >> 1);
			engine->_scene->currentScriptValue = ClampAngle(newAngle - engine->getRandomNumber(val1));
		}

		engine->_movements->moveActor(ctx.actor->angle, engine->_scene->currentScriptValue, ctx.actor->speed, &ctx.actor->move);
		ctx.stream.rewind(2);
		ctx.stream.writeSint16LE(engine->_scene->currentScriptValue);
	}

	if (ctx.actor->angle != engine->_scene->currentScriptValue) {
		ctx.undo(4);
		return 1;
	}
	engine->_movements->clearRealAngle(ctx.actor);
	ctx.stream.rewind(2);
	ctx.stream.writeSint16LE(-1);
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
}

void ScriptMove::processMoveScript(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	int32 end = -2;

	MoveScriptContext ctx(actorIdx, actor);
	do {
		const byte scriptOpcode = ctx.stream.readByte();
		if (scriptOpcode < ARRAYSIZE(function_map)) {
			end = function_map[scriptOpcode].function(_engine, ctx);
		} else {
			error("Actor %d with wrong offset/opcode - Offset: %d (opcode: %u)", actorIdx, ctx.stream.pos() - 1, scriptOpcode);
		}

		if (end < 0) {
			warning("Actor %d Life script [%s] not implemented", actorIdx, function_map[scriptOpcode].name);
		}
		if (ctx.actor->positionInMoveScript != -1) {
			actor->positionInMoveScript = ctx.stream.pos();
		}
	} while (end != 1);
}

} // namespace TwinE
