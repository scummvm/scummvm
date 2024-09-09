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

#include "twine/script/script_move.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "twine/scene/animations.h"
#include "twine/audio/sound.h"
#include "twine/movies.h"
#include "twine/scene/movements.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/scene/scene.h"
#include "twine/twine.h"

namespace TwinE {

/**
 * For unused opcodes
 */
int32 ScriptMove::mEMPTY(TwinEEngine *engine, MoveScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::EMPTY()");
	return 0;
}

/**
 * End of Actor Move Script
 * @note Opcode @c 0x00
 */
int32 ScriptMove::mEND(TwinEEngine *engine, MoveScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::END()");
	ctx.actor->_offsetTrack = -1;
	return 1;
}

/**
 * No Operation
 * @note Opcode @c 0x01
 */
int32 ScriptMove::mNOP(TwinEEngine *engine, MoveScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::NOP()");
	return 0;
}

/**
 * Choose new body for the current actor (Parameter = File3D Body Instance)
 * @note Opcode @c 0x02
 */
int32 ScriptMove::mBODY(TwinEEngine *engine, MoveScriptContext &ctx) {
	BodyType bodyIdx = (BodyType)ctx.stream.readByte();
	engine->_actor->initBody(bodyIdx, ctx.actorIdx);
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::BODY(%i)", (int)bodyIdx);
	return 0;
}

/**
 * Choose new animation for the current actor (Parameter = File3D Animation Instance)
 * @note Opcode @c 0x03
 */
int32 ScriptMove::mANIM(TwinEEngine *engine, MoveScriptContext &ctx) {
	AnimationTypes animIdx = (AnimationTypes)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::ANIM(%i)", (int)animIdx);
	if (engine->_animations->initAnim(animIdx, AnimType::kAnimationTypeRepeat, AnimationTypes::kStanding, ctx.actorIdx)) {
		return 0;
	}
	ctx.undo(1);
	return 1;
}

/**
 * Tell the actor to go to a new position (Parameter = Track Index)
 * @note Opcode @c 0x04
 */
int32 ScriptMove::mGOTO_POINT(TwinEEngine *engine, MoveScriptContext &ctx) {
	engine->_scene->_currentScriptValue = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::GOTO_POINT(%i)", (int)engine->_scene->_currentScriptValue);

	const IVec3 &sp = engine->_scene->_sceneTracks[engine->_scene->_currentScriptValue];
	const int32 newAngle = engine->_movements->getAngle(ctx.actor->_posObj.x, ctx.actor->_posObj.z, sp.x, sp.z);

	if (ctx.actor->_staticFlags.bSprite3D) {
		ctx.actor->_beta = newAngle;
	} else {
		engine->_movements->initRealAngleConst(ctx.actor->_beta, newAngle, ctx.actor->_speed, &ctx.actor->realAngle);
	}

	if (engine->_movements->_targetActorDistance > 500) {
		ctx.undo(1);
		return 1;
	}

	return 0;
}

/**
 * Wait the end of the current animation
 * @note Opcode @c 0x05
 */
int32 ScriptMove::mWAIT_ANIM(TwinEEngine *engine, MoveScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::WAIT_ANIM()");
	if (!ctx.actor->_workFlags.bAnimEnded) {
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
int32 ScriptMove::mLOOP(TwinEEngine *engine, MoveScriptContext &ctx) {
	ctx.actor->_offsetTrack = 0;
	ctx.stream.seek(0);
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::LOOP()");
	return 0;
}

/**
 * Make the actor turn around
 * @note Opcode @c 0x07
 */
int32 ScriptMove::mANGLE(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 angle = ToAngle(ctx.stream.readSint16LE());
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::ANGLE(%i)", (int)angle);
	if (ctx.actor->_staticFlags.bSprite3D) {
		return 0;
	}
	engine->_scene->_currentScriptValue = angle;
	if (ctx.actor->realAngle.timeValue == 0) {
		engine->_movements->initRealAngleConst(ctx.actor->_beta, angle, ctx.actor->_speed, &ctx.actor->realAngle);
	}
	if (ctx.actor->_beta == angle) {
		engine->_movements->clearRealAngle(ctx.actor);
		return 0;
	}
	ctx.undo(2);
	return 1;
}

/**
 * Set new position for the current actor (Parameter = Track Index)
 * @note Opcode @c 0x08
 */
int32 ScriptMove::mPOS_POINT(TwinEEngine *engine, MoveScriptContext &ctx) {
	engine->_scene->_currentScriptValue = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::POS_POINT(%i)", (int)engine->_scene->_currentScriptValue);

	const IVec3 &sp = engine->_scene->_sceneTracks[engine->_scene->_currentScriptValue];
	if (ctx.actor->_staticFlags.bSprite3D) {
		ctx.actor->_speed = 0;
	}

	ctx.actor->_posObj = sp;

	return 0;
}

/**
 * Specify a new label (Parameter = Label Number)
 * @note Opcode @c 0x09
 */
int32 ScriptMove::mLABEL(TwinEEngine *engine, MoveScriptContext &ctx) {
	ctx.actor->_labelTrack = ctx.stream.readByte();
	ctx.actor->_offsetLabelTrack = ctx.stream.pos() - 2;
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::LABEL(%i)", (int)ctx.actor->_labelTrack);
	if (engine->_scene->_currentSceneIdx == LBA1SceneId::Proxima_Island_Museum && ctx.actor->_actorIdx == 2 &&
		(ctx.actor->_labelTrack == 0 || ctx.actor->_labelTrack == 1)) {
		engine->unlockAchievement("LBA_ACH_004");
	}
	return 0;
}

/**
 * Go to a certain label (Parameter = Label Number)
 * @note Opcode @c 0x0A
 */
int32 ScriptMove::mGOTO(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 pos = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::GOTO(%i)", (int)pos);
	if (pos == -1) {
		ctx.actor->_offsetTrack = -1;
		return 1;
	}
	ctx.stream.seek(pos);
	return 0;
}

/**
 * Tell the actor to stop the current animation
 * @note Opcode @c 0x0B
 */
int32 ScriptMove::mSTOP(TwinEEngine *engine, MoveScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::STOP()");
	ctx.actor->_offsetTrack = -1;
	return 1;
}

/**
 * Tell the actor to go to a symbolic point
 * @note Opcode @c 0x0C
 */
int32 ScriptMove::mGOTO_SYM_POINT(TwinEEngine *engine, MoveScriptContext &ctx) {
	engine->_scene->_currentScriptValue = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::GOTO_SYM_POINT(%i)", (int)engine->_scene->_currentScriptValue);

	const IVec3 &sp = engine->_scene->_sceneTracks[engine->_scene->_currentScriptValue];
	const int32 newAngle = LBAAngles::ANGLE_180 + engine->_movements->getAngle(ctx.actor->_posObj, sp);

	if (ctx.actor->_staticFlags.bSprite3D) {
		ctx.actor->_beta = newAngle;
	} else {
		engine->_movements->initRealAngleConst(ctx.actor->_beta, newAngle, ctx.actor->_speed, &ctx.actor->realAngle);
	}

	if (engine->_movements->_targetActorDistance > 500) {
		ctx.undo(1);
		return 1;
	}

	return 0;
}

/**
 * Wait a certain number of frame update in the current animation
 * @note Opcode @c 0x0D
 */
int32 ScriptMove::mWAIT_NUM_ANIM(TwinEEngine *engine, MoveScriptContext &ctx) {
	bool abortMove = false;
	const int32 animRepeats = ctx.stream.readByte();
	int32 animPos = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::WAIT_NUM_ANIM(%i, %i)", (int)animRepeats, animPos);
	if (ctx.actor->_workFlags.bAnimEnded) {
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
int32 ScriptMove::mSAMPLE(TwinEEngine *engine, MoveScriptContext &ctx) {
	int32 sampleIdx = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::SAMPLE(%i)", (int)sampleIdx);
	engine->_sound->playSample(sampleIdx, 1, ctx.actor->posObj(), ctx.actorIdx);
	return 0;
}

/**
 * Tell the actor to go to a new position (Parameter = Track Index)
 * @note Opcode @c 0x0F
 */
int32 ScriptMove::mGOTO_POINT_3D(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int32 trackId = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::GOTO_POINT_3D(%i)", (int)trackId);
	if (!ctx.actor->_staticFlags.bSprite3D) {
		return 0;
	}

	engine->_scene->_currentScriptValue = trackId;

	const IVec3 &sp = engine->_scene->_sceneTracks[engine->_scene->_currentScriptValue];
	ctx.actor->_beta = engine->_movements->getAngle(ctx.actor->_posObj.x, ctx.actor->_posObj.z, sp.x, sp.z);
	ctx.actor->_spriteActorRotation = engine->_movements->getAngle(ctx.actor->_posObj.y, 0, sp.y, engine->_movements->_targetActorDistance);

	if (engine->_movements->_targetActorDistance > 100) {
		ctx.undo(1);
		return 1;
	}
	ctx.actor->_posObj = sp;

	return 0;
}

/**
 * Specify a new rotation speed for the current actor (Parameter = Rotation speed) [ 0 means fast, 32767 means slow ]
 * @note Opcode @c 0x10
 */
int32 ScriptMove::mSPEED(TwinEEngine *engine, MoveScriptContext &ctx) {
	ctx.actor->_speed = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::SPEED(%i)", (int)ctx.actor->_speed);

	if (ctx.actor->_staticFlags.bSprite3D) {
		engine->_movements->setActorAngle(LBAAngles::ANGLE_0, ctx.actor->_speed, LBAAngles::ANGLE_17, &ctx.actor->realAngle);
	}

	return 0;
}

/**
 * Set actor as background (Parameter = 1 (true); = 0 (false))
 * @note Opcode @c 0x11
 */
int32 ScriptMove::mBACKGROUND(TwinEEngine *engine, MoveScriptContext &ctx) {
	const uint8 val = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::BACKGROUND(%i)", (int)val);
	if (val != 0) {
		if (!ctx.actor->_staticFlags.bIsBackgrounded) {
			ctx.actor->_staticFlags.bIsBackgrounded = 1;
			if (ctx.actor->_workFlags.bWasDrawn) {
				engine->_redraw->_firstTime = true;
			}
		}
	} else {
		if (ctx.actor->_staticFlags.bIsBackgrounded) {
			ctx.actor->_staticFlags.bIsBackgrounded = 0;
			if (ctx.actor->_workFlags.bWasDrawn) {
				engine->_redraw->_firstTime = true;
			}
		}
	}

	return 0;
}

/**
 * Number os seconds to wait.
 * @note Opcode @c 0x12
 */
int32 ScriptMove::mWAIT_NUM_SECOND(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int32 numSeconds = ctx.stream.readByte();
	int32 currentTime = ctx.stream.readSint32LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::WAIT_NUM_SECOND(%i, %i)", (int)numSeconds, currentTime);

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

/**
 * To not use Bodies.
 * @note Opcode @c 0x13
 */
int32 ScriptMove::mNO_BODY(TwinEEngine *engine, MoveScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::NO_BODY()");
	engine->_actor->initBody(BodyType::btNone, ctx.actorIdx);
	return 0;
}

/**
 * Change actor orientation. (Parameter = New Angle)
 * @note Opcode @c 0x14
 */
int32 ScriptMove::mBETA(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 beta = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::BETA(%i)", (int)beta);

	ctx.actor->_beta = beta;

	if (!ctx.actor->_staticFlags.bSprite3D) {
		engine->_movements->clearRealAngle(ctx.actor);
	}

	return 0;
}

int32 ScriptMove::mOPEN_GENERIC(TwinEEngine *engine, MoveScriptContext &ctx, int32 angle) {
	const int16 doorStatus = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::OPEN(%i, %i)", (int)doorStatus, angle);
	if (ctx.actor->_staticFlags.bSprite3D && ctx.actor->_staticFlags.bSpriteClip) {
		ctx.actor->_beta = angle;
		ctx.actor->_doorWidth = doorStatus;
		ctx.actor->_workFlags.bIsSpriteMoving = 1;
		ctx.actor->_speed = 1000;
		engine->_movements->setActorAngle(LBAAngles::ANGLE_0, LBAAngles::ANGLE_351, LBAAngles::ANGLE_17, &ctx.actor->realAngle);
	}
	if (engine->_scene->_currentSceneIdx == LBA1SceneId::Proxima_Island_Museum && ctx.actor->_actorIdx == 16) {
		engine->unlockAchievement("LBA_ACH_009");
	}
	return 0;
}

/**
 * Open the door (left way) (Parameter = distance to open).
 * @note Opcode @c 0x15
 */
int32 ScriptMove::mOPEN_LEFT(TwinEEngine *engine, MoveScriptContext &ctx) {
	return mOPEN_GENERIC(engine, ctx, LBAAngles::ANGLE_270);
}

/**
 * Open the door (right way) (Parameter = distance to open).
 * @note Opcode @c 0x16
 */
int32 ScriptMove::mOPEN_RIGHT(TwinEEngine *engine, MoveScriptContext &ctx) {
	return mOPEN_GENERIC(engine, ctx, LBAAngles::ANGLE_90);

}

/**
 * Open the door (up way) (Parameter = distance to open).
 * @note Opcode @c 0x17
 */
int32 ScriptMove::mOPEN_UP(TwinEEngine *engine, MoveScriptContext &ctx) {
	return mOPEN_GENERIC(engine, ctx, LBAAngles::ANGLE_180);

}

/**
 * Open the door (down way) (Parameter = distance to open).
 * @note Opcode @c 0x18
 */
int32 ScriptMove::mOPEN_DOWN(TwinEEngine *engine, MoveScriptContext &ctx) {
	return mOPEN_GENERIC(engine, ctx, LBAAngles::ANGLE_0);
}

/**
 * Close the door.
 * @note Opcode @c 0x19
 */
int32 ScriptMove::mCLOSE(TwinEEngine *engine, MoveScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::CLOSE()");
	if (ctx.actor->_staticFlags.bSprite3D && ctx.actor->_staticFlags.bSpriteClip) {
		ctx.actor->_doorWidth = 0;
		ctx.actor->_workFlags.bIsSpriteMoving = 1;
		ctx.actor->_speed = -1000;
		engine->_movements->setActorAngle(LBAAngles::ANGLE_0, -LBAAngles::ANGLE_351, LBAAngles::ANGLE_17, &ctx.actor->realAngle);
	}
	return 0;
}

/**
 * Wait till door close.
 * @note Opcode @c 0x1A
 */
int32 ScriptMove::mWAIT_DOOR(TwinEEngine *engine, MoveScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::WAIT_DOOR()");
	if (ctx.actor->_staticFlags.bSprite3D && ctx.actor->_staticFlags.bSpriteClip) {
		if (ctx.actor->_speed) {
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
int32 ScriptMove::mSAMPLE_RND(TwinEEngine *engine, MoveScriptContext &ctx) {
	int32 sampleIdx = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::SAMPLE_RND(%i)", (int)sampleIdx);
	engine->_sound->playSample(sampleIdx, 1, ctx.actor->posObj(), ctx.actorIdx);
	return 0;
}

/**
 * Play always the sample (Parameter = Sample index)
 * @note Opcode @c 0x1C
 */
int32 ScriptMove::mSAMPLE_ALWAYS(TwinEEngine *engine, MoveScriptContext &ctx) {
	int32 sampleIdx = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::SAMPLE_ALWAYS(%i)", (int)sampleIdx);
	if (!engine->_sound->isSamplePlaying(sampleIdx)) { // if its not playing
		engine->_sound->playSample(sampleIdx, -1, ctx.actor->posObj(), ctx.actorIdx);
	}
	return 0;
}

/**
 * Stop playing the sample
 * @note Opcode @c 0x1D
 */
int32 ScriptMove::mSAMPLE_STOP(TwinEEngine *engine, MoveScriptContext &ctx) {
	int32 sampleIdx = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::SAMPLE_STOP(%i)", (int)sampleIdx);
	engine->_sound->stopSample(sampleIdx);
	return 0;
}

/**
 * Play FLA cutscenes (Parameter = Cutscene Name)
 * @note Opcode @c 0x1E
 */
int32 ScriptMove::mPLAY_FLA(TwinEEngine *engine, MoveScriptContext &ctx) {
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

	debugC(3, kDebugLevels::kDebugScripts, "MOVE::PLAY_FLA(%s)", movie);
	engine->_movie->playMovie(movie);
	engine->setPalette(engine->_screens->_paletteRGBA);
	engine->_screens->clearScreen();
	return 0;
}

/**
 * Repeat sample (Parameter = Sample index).
 * @note Opcode @c 0x1F
 */
int32 ScriptMove::mREPEAT_SAMPLE(TwinEEngine *engine, MoveScriptContext &ctx) {
	ctx.numRepeatSample = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::REPEAT_SAMPLE(%i)", (int)ctx.numRepeatSample);
	return 0;
}

/**
 * Play a sample (Parameter = Sample index)
 * @note Opcode @c 0x20
 */
int32 ScriptMove::mSIMPLE_SAMPLE(TwinEEngine *engine, MoveScriptContext &ctx) {
	int32 sampleIdx = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::SIMPLE_SAMPLE(%i)", (int)sampleIdx);
	engine->_sound->playSample(sampleIdx, ctx.numRepeatSample, ctx.actor->posObj(), ctx.actorIdx);
	ctx.numRepeatSample = 1;
	return 0;
}

/**
 * The actor rotate to Twinsen direction (Parameter = -1 (near); = 0 (far))
 * @note Opcode @c 0x21
 */
int32 ScriptMove::mFACE_HERO(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 angle = ToAngle(ctx.stream.readSint16LE());
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::FACE_HERO(%i)", (int)angle);
	if (ctx.actor->_staticFlags.bSprite3D) {
		return 0;
	}
	engine->_scene->_currentScriptValue = angle;
	if (engine->_scene->_currentScriptValue == -1 && ctx.actor->realAngle.timeValue == 0) {
		engine->_scene->_currentScriptValue = engine->_movements->getAngle(ctx.actor->posObj(), engine->_scene->_sceneHero->posObj());
		engine->_movements->initRealAngleConst(ctx.actor->_beta, engine->_scene->_currentScriptValue, ctx.actor->_speed, &ctx.actor->realAngle);
		ctx.stream.rewind(2);
		ctx.stream.writeSint16LE(engine->_scene->_currentScriptValue);
	}

	if (ctx.actor->_beta != engine->_scene->_currentScriptValue) {
		ctx.undo(2);
		return 1;
	}
	engine->_movements->clearRealAngle(ctx.actor);
	ctx.stream.rewind(2);
	ctx.stream.writeSint16LE(-1);
	return 0;
}

/**
 * Generate a random angle for the current actor
 * @note Opcode @c 0x22
 */
int32 ScriptMove::mANGLE_RND(TwinEEngine *engine, MoveScriptContext &ctx) {
	const int16 val1 = ctx.stream.readSint16LE();
	const int16 val2 = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::LBAAngles::ANGLE_RND(%i, %i)", (int)val1, (int)val2);
	if (ctx.actor->_staticFlags.bSprite3D) {
		return 0;
	}

	engine->_scene->_currentScriptValue = val2;

	if (engine->_scene->_currentScriptValue == -1 && ctx.actor->realAngle.timeValue == 0) {
		if (engine->getRandomNumber() & 1) {
			const int32 newAngle = ctx.actor->_beta + LBAAngles::ANGLE_90 + (ABS(val1) >> 1);
			engine->_scene->_currentScriptValue = ClampAngle(newAngle - engine->getRandomNumber(val1));
		} else {
			const int32 newAngle = ctx.actor->_beta - LBAAngles::ANGLE_90 + (ABS(val1) >> 1);
			engine->_scene->_currentScriptValue = ClampAngle(newAngle - engine->getRandomNumber(val1));
		}

		engine->_movements->initRealAngleConst(ctx.actor->_beta, engine->_scene->_currentScriptValue, ctx.actor->_speed, &ctx.actor->realAngle);
		ctx.stream.rewind(2);
		ctx.stream.writeSint16LE(engine->_scene->_currentScriptValue);
	}

	if (ctx.actor->_beta != engine->_scene->_currentScriptValue) {
		ctx.undo(4);
		return 1;
	}
	engine->_movements->clearRealAngle(ctx.actor);
	ctx.stream.rewind(2);
	ctx.stream.writeSint16LE(-1);
	return 0;
}

ScriptMove::ScriptMove(TwinEEngine *engine, const ScriptMoveFunction *functionMap, size_t entries) : _engine(engine), _functionMap(functionMap), _functionMapSize(entries) {
}

void ScriptMove::doTrack(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	int32 end = -2;

	MoveScriptContext ctx(actorIdx, actor);
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::BEGIN(%i)", actorIdx);
	do {
		const byte scriptOpcode = ctx.stream.readByte();
		if (scriptOpcode < _functionMapSize) {
			debugC(3, kDebugLevels::kDebugScripts, "MOVE::EXEC(%s, %i)", _functionMap[scriptOpcode].name, actorIdx);
			end = _functionMap[scriptOpcode].function(_engine, ctx);
		} else {
			error("Actor %d with wrong offset/opcode in move script - Offset: %d/%d (opcode: %u)", actorIdx, (int)ctx.stream.pos() - 1, (int)ctx.stream.size(), scriptOpcode);
		}

		if (end < 0) {
			warning("Actor %d Life script [%s] not implemented", actorIdx, _functionMap[scriptOpcode].name);
		} else if (end == 1) {
			debugC(3, kDebugLevels::kDebugScripts, "MOVE::BREAK(%i)", actorIdx);
		}

		if (ctx.actor->_offsetTrack != -1) {
			actor->_offsetTrack = ctx.stream.pos();
		}
	} while (end != 1);
	debugC(3, kDebugLevels::kDebugScripts, "MOVE::END(%i)", actorIdx);
}

} // namespace TwinE
