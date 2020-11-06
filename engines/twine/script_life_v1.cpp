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

#include "twine/script_life_v1.h"
#include "common/stream.h"
#include "twine/actor.h"
#include "twine/animations.h"
#include "twine/collision.h"
#include "twine/flamovies.h"
#include "twine/gamestate.h"
#include "twine/grid.h"
#include "twine/holomap.h"
#include "twine/input.h"
#include "twine/interface.h"
#include "twine/menu.h"
#include "twine/movements.h"
#include "twine/music.h"
#include "twine/redraw.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "common/memstream.h"
#include "twine/sound.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

static int32 drawVar1;
static char textStr[256]; // string

struct LifeScriptContext {
	int32 actorIdx;
	ActorStruct *actor;
	Common::MemorySeekableReadWriteStream stream;
	uint8 *opcodePtr; // local opcode script pointer

	LifeScriptContext(int32 _actorIdx, ActorStruct *_actor) : actorIdx(_actorIdx), actor(_actor), stream(_actor->lifeScript, _actor->lifeScriptSize) {
		assert(actor->positionInLifeScript >= 0);
		stream.skip(_actor->positionInLifeScript);
		updateOpcodePos();
	}

	void setOpcode(uint8 opcode) {
		*opcodePtr = opcode;
	}

	void updateOpcodePos() {
		opcodePtr = actor->lifeScript + stream.pos();
	}
};

/** Returns:
	   -1 - Need implementation
		0 - Completed
		1 - Break script */
typedef int32 ScriptLifeFunc(TwinEEngine *engine, LifeScriptContext &ctx);

struct ScriptLifeFunction {
	const char *name;
	ScriptLifeFunc *function;
};

#define MAPFUNC(name, func) \
	{ name, func }

/** Script condition operators */
enum LifeScriptOperators {
	/*==*/kEqualTo = 0,
	/*> */ kGreaterThan = 1,
	/*< */ kLessThan = 2,
	/*>=*/kGreaterThanOrEqualTo = 3,
	/*<=*/kLessThanOrEqualTo = 4,
	/*!=*/kNotEqualTo = 5
};

/** Script condition command opcodes */
enum LifeScriptConditions {
	/*0x00*/ kcCOL = 0,
	/*0x01*/ kcCOL_OBJ = 1,
	/*0x02*/ kcDISTANCE = 2,
	/*0x03*/ kcZONE = 3,
	/*0x04*/ kcZONE_OBJ = 4,
	/*0x05*/ kcBODY = 5,
	/*0x06*/ kcBODY_OBJ = 6,
	/*0x07*/ kcANIM = 7,
	/*0x08*/ kcANIM_OBJ = 8,
	/*0x09*/ kcL_TRACK = 9,
	/*0x0A*/ kcL_TRACK_OBJ = 10,
	/*0x0B*/ kcFLAG_CUBE = 11,
	/*0x0C*/ kcCONE_VIEW = 12,
	/*0x0D*/ kcHIT_BY = 13,
	/*0x0E*/ kcACTION = 14,
	/*0x0F*/ kcFLAG_GAME = 15,
	/*0x10*/ kcLIFE_POINT = 16,
	/*0x11*/ kcLIFE_POINT_OBJ = 17,
	/*0x12*/ kcNUM_LITTLE_KEYS = 18,
	/*0x13*/ kcNUM_GOLD_PIECES = 19,
	/*0x14*/ kcBEHAVIOUR = 20,
	/*0x15*/ kcCHAPTER = 21,
	/*0x16*/ kcDISTANCE_3D = 22,
	/*0x17 - 23 unused */
	/*0x18 - 24 unused */
	/*0x19*/ kcUSE_INVENTORY = 25,
	/*0x1A*/ kcCHOICE = 26,
	/*0x1B*/ kcFUEL = 27,
	/*0x1C*/ kcCARRIED_BY = 28,
	/*0x1D*/ kcCDROM = 29
};

/** Returns:
	   -1 - Need implementation
		1 - Condition value size (1 byte)
		2 - Condition value size (2 byes) */
static int32 processLifeConditions(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 conditionValueSize = 1;
	int32 conditionOpcode = ctx.stream.readByte();

	switch (conditionOpcode) {
	case kcCOL:
		if (ctx.actor->life <= 0) {
			engine->_scene->currentScriptValue = -1;
		} else {
			engine->_scene->currentScriptValue = ctx.actor->collision;
		}
		break;
	case kcCOL_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		if (engine->_scene->getActor(actorIdx)->life <= 0) {
			engine->_scene->currentScriptValue = -1;
		} else {
			engine->_scene->currentScriptValue = engine->_scene->getActor(actorIdx)->collision;
		}
		break;
	}
	case kcDISTANCE: {
		int32 actorIdx = ctx.stream.readByte();
		conditionValueSize = 2;
		ActorStruct *otherActor = engine->_scene->getActor(actorIdx);
		if (!otherActor->dynamicFlags.bIsDead) {
			if (ABS(ctx.actor->y - otherActor->y) >= 1500) {
				engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
			} else {
				// Returns int32, so we check for integer overflow
				int32 distance = engine->_movements->getDistance2D(ctx.actor->x, ctx.actor->z, otherActor->x, otherActor->z);
				if (ABS(distance) > MAX_TARGET_ACTOR_DISTANCE) {
					engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
				} else {
					engine->_scene->currentScriptValue = distance;
				}
			}
		} else {
			engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
		}
		break;
	}
	case kcZONE:
		engine->_scene->currentScriptValue = ctx.actor->zone;
		break;
	case kcZONE_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		engine->_scene->currentScriptValue = engine->_scene->getActor(actorIdx)->zone;
		break;
	}
	case kcBODY:
		engine->_scene->currentScriptValue = ctx.actor->body;
		break;
	case kcBODY_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		engine->_scene->currentScriptValue = engine->_scene->getActor(actorIdx)->body;
		break;
	}
	case kcANIM:
		engine->_scene->currentScriptValue = ctx.actor->anim;
		break;
	case kcANIM_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		engine->_scene->currentScriptValue = engine->_scene->getActor(actorIdx)->anim;
		break;
	}
	case kcL_TRACK:
		engine->_scene->currentScriptValue = ctx.actor->labelIdx;
		break;
	case kcL_TRACK_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		engine->_scene->currentScriptValue = engine->_scene->getActor(actorIdx)->labelIdx;
		break;
	}
	case kcFLAG_CUBE: {
		int32 flagIdx = ctx.stream.readByte();
		engine->_scene->currentScriptValue = engine->_scene->sceneFlags[flagIdx];
		break;
	}
	case kcCONE_VIEW: {
		int32 newAngle = 0;
		int32 targetActorIdx = ctx.stream.readByte();
		ActorStruct *targetActor = engine->_scene->getActor(targetActorIdx);

		conditionValueSize = 2;

		if (!targetActor->dynamicFlags.bIsDead) {
			if (ABS(targetActor->y - ctx.actor->y) < 1500) {
				newAngle = engine->_movements->getAngleAndSetTargetActorDistance(ctx.actor->x, ctx.actor->z, targetActor->x, targetActor->z);
				if (ABS(engine->_movements->targetActorDistance) > MAX_TARGET_ACTOR_DISTANCE) {
					engine->_movements->targetActorDistance = MAX_TARGET_ACTOR_DISTANCE;
				}
			} else {
				engine->_movements->targetActorDistance = MAX_TARGET_ACTOR_DISTANCE;
			}

			if (!targetActorIdx) {
				int32 heroAngle;

				heroAngle = ctx.actor->angle + 0x480 - newAngle + 0x400;
				heroAngle &= 0x3FF;

				if (ABS(heroAngle) > 0x100) {
					engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
				} else {
					engine->_scene->currentScriptValue = engine->_movements->targetActorDistance;
				}
			} else {
				if (engine->_actor->heroBehaviour == kDiscrete) {
					int32 heroAngle;

					heroAngle = ctx.actor->angle + 0x480 - newAngle + 0x400;
					heroAngle &= 0x3FF;

					if (ABS(heroAngle) > 0x100) {
						engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
					} else {
						engine->_scene->currentScriptValue = engine->_movements->targetActorDistance;
					}
				} else {
					engine->_scene->currentScriptValue = engine->_movements->targetActorDistance;
				}
			}
		} else {
			engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
		}
		break;
	}
	case kcHIT_BY:
		engine->_scene->currentScriptValue = ctx.actor->hitBy;
		break;
	case kcACTION:
		engine->_scene->currentScriptValue = engine->_movements->heroAction ? 1 : 0;
		break;
	case kcFLAG_GAME: {
		int32 flagIdx = ctx.stream.readByte();
		if (!engine->_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED] ||
		    (engine->_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED] && flagIdx >= MaxInventoryItems)) {
			engine->_scene->currentScriptValue = engine->_gameState->gameFlags[flagIdx];
		} else {
			if (flagIdx == GAMEFLAG_INVENTORY_DISABLED) {
				engine->_scene->currentScriptValue = engine->_gameState->gameFlags[flagIdx];
			} else {
				engine->_scene->currentScriptValue = 0;
			}
		}
		break;
	}
	case kcLIFE_POINT:
		engine->_scene->currentScriptValue = ctx.actor->life;
		break;
	case kcLIFE_POINT_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		engine->_scene->currentScriptValue = engine->_scene->getActor(actorIdx)->life;
		break;
	}
	case kcNUM_LITTLE_KEYS:
		engine->_scene->currentScriptValue = engine->_gameState->inventoryNumKeys;
		break;
	case kcNUM_GOLD_PIECES:
		conditionValueSize = 2;
		engine->_scene->currentScriptValue = engine->_gameState->inventoryNumKashes;
		break;
	case kcBEHAVIOUR:
		engine->_scene->currentScriptValue = engine->_actor->heroBehaviour;
		break;
	case kcCHAPTER:
		engine->_scene->currentScriptValue = engine->_gameState->gameChapter;
		break;
	case kcDISTANCE_3D: {
		int32 targetActorIdx;
		ActorStruct *targetActor;

		targetActorIdx = ctx.stream.readByte();
		targetActor = engine->_scene->getActor(targetActorIdx);

		conditionValueSize = 2;

		if (!targetActor->dynamicFlags.bIsDead) {
			// Returns int32, so we check for integer overflow
			int32 distance = engine->_movements->getDistance3D(ctx.actor->x, ctx.actor->y, ctx.actor->z, targetActor->x, targetActor->y, targetActor->z);
			if (ABS(distance) > MAX_TARGET_ACTOR_DISTANCE) {
				engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
			} else {
				engine->_scene->currentScriptValue = distance;
			}
		} else {
			engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
		}
		break;
	}
	case 23: // unused
	case 24:
		break;
	case kcUSE_INVENTORY: {
		int32 item = ctx.stream.readByte();

		if (!engine->_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED]) {
			if (item == engine->loopInventoryItem) {
				engine->_scene->currentScriptValue = 1;
			} else {
				if (engine->_gameState->inventoryFlags[item] == 1 && engine->_gameState->gameFlags[item] == 1) {
					engine->_scene->currentScriptValue = 1;
				} else {
					engine->_scene->currentScriptValue = 0;
				}
			}

			if (engine->_scene->currentScriptValue == 1) {
				engine->_redraw->addOverlay(koInventoryItem, item, 0, 0, 0, koNormal, 3);
			}
		} else {
			engine->_scene->currentScriptValue = 0;
		}
		break;
	}
	case kcCHOICE:
		conditionValueSize = 2;
		engine->_scene->currentScriptValue = engine->_gameState->choiceAnswer;
		break;
	case kcFUEL:
		engine->_scene->currentScriptValue = engine->_gameState->inventoryNumGas;
		break;
	case kcCARRIED_BY:
		engine->_scene->currentScriptValue = ctx.actor->standOn;
		break;
	case kcCDROM:
		engine->_scene->currentScriptValue = 1;
		break;
	default:
		error("Actor condition opcode %d", conditionOpcode);
		break;
	}

	return conditionValueSize;
}

/** Returns:
	   -1 - Need implementation
		0 - Condition false
		1 - Condition true */
static int32 processLifeOperators(TwinEEngine *engine, LifeScriptContext &ctx, int32 valueSize) {
	const int32 operatorCode = ctx.stream.readByte();

	int32 conditionValue;
	if (valueSize == 1) {
		conditionValue = ctx.stream.readByte();
	} else if (valueSize == 2) {
		conditionValue = ctx.stream.readSint16LE();
	} else {
		error("Unknown operator value size %d", valueSize);
	}

	switch (operatorCode) {
	case kEqualTo:
		if (engine->_scene->currentScriptValue == conditionValue) {
			return 1;
		}
		break;
	case kGreaterThan:
		if (engine->_scene->currentScriptValue > conditionValue) {
			return 1;
		}
		break;
	case kLessThan:
		if (engine->_scene->currentScriptValue < conditionValue) {
			return 1;
		}
		break;
	case kGreaterThanOrEqualTo:
		if (engine->_scene->currentScriptValue >= conditionValue) {
			return 1;
		}
		break;
	case kLessThanOrEqualTo:
		if (engine->_scene->currentScriptValue <= conditionValue) {
			return 1;
		}
		break;
	case kNotEqualTo:
		if (engine->_scene->currentScriptValue != conditionValue) {
			return 1;
		}
		break;
	default:
		warning("Unknown life script operator opcode %d", operatorCode);
		break;
	}

	return 0;
}

/** Life script command definitions */

/* For unused opcodes */
static int32 lEMPTY(TwinEEngine *engine, LifeScriptContext &ctx) {
	return 0;
}

/*0x00*/
static int32 lEND(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.actor->positionInLifeScript = -1;
	return 1; // break script
}

/*0x01*/
static int32 lNOP(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.stream.skip(1);
	return 0;
}

/*0x02*/
static int32 lSNIF(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 valueSize = processLifeConditions(engine, ctx);
	if (!processLifeOperators(engine, ctx, valueSize)) {
		ctx.setOpcode(0x0D); // SWIF
	}
	ctx.stream.seek(ctx.stream.readSint16LE()); // condition offset
	return 0;
}

/*0x03*/
static int32 lOFFSET(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.stream.seek(ctx.stream.readSint16LE()); // offset
	return 0;
}

/*0x04*/
static int32 lNEVERIF(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 valueSize = processLifeConditions(engine, ctx);
	processLifeOperators(engine, ctx, valueSize);
	ctx.stream.seek(ctx.stream.readSint16LE()); // condition offset
	return 0;
}

/*0x06*/
static int32 lNO_IF(TwinEEngine *engine, LifeScriptContext &ctx) {
	return 0;
}

/*0x0A*/
static int32 lLABEL(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.stream.skip(1);
	return 0;
}

/*0x0B*/
static int32 lRETURN(TwinEEngine *engine, LifeScriptContext &ctx) {
	return 1; // break script
}

/*0x0C*/
static int32 lIF(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 valueSize = processLifeConditions(engine, ctx);
	if (!processLifeOperators(engine, ctx, valueSize)) {
		ctx.stream.seek(ctx.stream.readSint16LE()); // condition offset
	} else {
		ctx.stream.skip(2);
	}

	return 0;
}

/*0x0D*/
static int32 lSWIF(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 valueSize = processLifeConditions(engine, ctx);
	if (!processLifeOperators(engine, ctx, valueSize)) {
		ctx.stream.seek(ctx.stream.readSint16LE()); // condition offset
	} else {
		ctx.stream.skip(2);
		ctx.setOpcode(0x02); // SNIF
	}

	return 0;
}

/*0x0E*/
static int32 lONEIF(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 valueSize = processLifeConditions(engine, ctx);
	if (!processLifeOperators(engine, ctx, valueSize)) {
		ctx.stream.seek(ctx.stream.readSint16LE()); // condition offset
	} else {
		ctx.stream.skip(2);
		ctx.setOpcode(0x04); // NEVERIF
	}

	return 0;
}

/*0x0F*/
static int32 lELSE(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.stream.seek(ctx.stream.readSint16LE()); // offset
	return 0;
}

/*0x11*/
static int32 lBODY(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 bodyIdx = ctx.stream.readByte();
	engine->_actor->initModelActor(bodyIdx, ctx.actorIdx);
	return 0;
}

/*0x12*/
static int32 lBODY_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 otherActorIdx = ctx.stream.readByte();
	int32 otherBodyIdx = ctx.stream.readByte();
	engine->_actor->initModelActor(otherBodyIdx, otherActorIdx);
	return 0;
}

/*0x13*/
static int32 lANIM(TwinEEngine *engine, LifeScriptContext &ctx) {
	AnimationTypes animIdx = (AnimationTypes)ctx.stream.readByte();
	engine->_animations->initAnim(animIdx, 0, 0, ctx.actorIdx);
	return 0;
}

/*0x14*/
static int32 lANIM_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 otherActorIdx = ctx.stream.readByte();
	AnimationTypes otherAnimIdx = (AnimationTypes)ctx.stream.readByte();
	engine->_animations->initAnim(otherAnimIdx, 0, 0, otherActorIdx);
	return 0;
}

/*0x15*/
static int32 lSET_LIFE(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.actor->positionInLifeScript = ctx.stream.readSint16LE(); // offset
	return 0;
}

/*0x16*/
static int32 lSET_LIFE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 otherActorIdx = ctx.stream.readByte();
	engine->_scene->getActor(otherActorIdx)->positionInLifeScript = ctx.stream.readSint16LE(); // offset
	return 0;
}

/*0x17*/
static int32 lSET_TRACK(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.actor->positionInMoveScript = ctx.stream.readSint16LE(); // offset
	return 0;
}

/*0x18*/
static int32 lSET_TRACK_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 otherActorIdx = ctx.stream.readByte();
	engine->_scene->getActor(otherActorIdx)->positionInMoveScript = ctx.stream.readSint16LE(); // offset
	return 0;
}

/*0x19*/
static int32 lMESSAGE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 textIdx = ctx.stream.readSint16LE();

	engine->freezeTime();
	if (engine->_text->showDialogueBubble) {
		engine->_redraw->drawBubble(ctx.actorIdx);
	}
	engine->_text->setFontCrossColor(ctx.actor->talkColor);
	engine->_scene->talkingActor = ctx.actorIdx;
	engine->_text->drawTextFullscreen(textIdx);
	engine->unfreezeTime();
	engine->_redraw->redrawEngineActions(1);

	return 0;
}

/*0x1A*/
static int32 lFALLABLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 flag = ctx.stream.readByte();
	ctx.actor->staticFlags.bCanFall = flag & 1;
	return 0;
}

/*0x1B*/
static int32 lSET_DIRMODE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 controlMode = ctx.stream.readByte();

	ctx.actor->controlMode = (ControlMode)controlMode;
	if (ctx.actor->controlMode == ControlMode::kFollow) {
		ctx.actor->followedActor = ctx.stream.readByte();
	}

	return 0;
}

/*0x1C*/
static int32 lSET_DIRMODE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const int32 controlMode = ctx.stream.readByte();

	ActorStruct *otherActor = engine->_scene->getActor(otherActorIdx);
	otherActor->controlMode = (ControlMode)controlMode;
	if (otherActor->controlMode == ControlMode::kFollow) {
		otherActor->followedActor = ctx.stream.readByte();
	}

	return 0;
}

/*0x1D*/
static int32 lCAM_FOLLOW(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 followedActorIdx = ctx.stream.readByte();

	if (engine->_scene->currentlyFollowedActor != followedActorIdx) {
		const ActorStruct *followedActor = engine->_scene->getActor(followedActorIdx);
		engine->_grid->newCameraX = followedActor->x >> 9;
		engine->_grid->newCameraY = followedActor->y >> 8;
		engine->_grid->newCameraZ = followedActor->z >> 9;

		engine->_scene->currentlyFollowedActor = followedActorIdx;
		engine->_redraw->reqBgRedraw = true;
	}

	return 0;
}

/*0x1E*/
static int32 lSET_BEHAVIOUR(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 behavior = ctx.stream.readByte();

	engine->_animations->initAnim(kStanding, 0, 255, 0);
	engine->_actor->setBehaviour(behavior);

	return 0;
}

/*0x1F*/
static int32 lSET_FLAG_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 flagIdx = ctx.stream.readByte();
	const int32 flagValue = ctx.stream.readByte();

	engine->_scene->sceneFlags[flagIdx] = flagValue;

	return 0;
}

/*0x20*/
static int32 lCOMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.stream.skip(1);
	return 0;
}

/*0x21*/
static int32 lSET_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.actor->positionInLifeScript = ctx.stream.readSint16LE();
	return 0;
}

/*0x22*/
static int32 lSET_COMPORTEMENT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	engine->_scene->getActor(otherActorIdx)->positionInLifeScript = ctx.stream.readSint16LE();
	return 0;
}

/*0x23*/
static int32 lEND_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx) {
	return 1; // break
}

/*0x24*/
static int32 lSET_FLAG_GAME(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 flagIdx = ctx.stream.readByte();
	const int32 flagValue = ctx.stream.readByte();

	engine->_gameState->gameFlags[flagIdx] = flagValue;

	return 0;
}

/*0x25*/
static int32 lKILL_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();

	engine->_actor->processActorCarrier(otherActorIdx);
	ActorStruct *otherActor = engine->_scene->getActor(otherActorIdx);
	otherActor->dynamicFlags.bIsDead = 1;
	otherActor->entity = -1;
	otherActor->zone = -1;
	otherActor->life = 0;

	return 0;
}

/*0x26*/
static int32 lSUICIDE(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_actor->processActorCarrier(ctx.actorIdx);
	ctx.actor->dynamicFlags.bIsDead = 1;
	ctx.actor->entity = -1;
	ctx.actor->zone = -1;
	ctx.actor->life = 0;

	return 0;
}

/*0x27*/
static int32 lUSE_ONE_LITTLE_KEY(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_gameState->inventoryNumKeys--;

	if (engine->_gameState->inventoryNumKeys < 0) {
		engine->_gameState->inventoryNumKeys = 0;
	}

	engine->_redraw->addOverlay(koSprite, SPRITEHQR_KEY, 0, 0, 0, koFollowActor, 1);

	return 0;
}

/*0x28*/
static int32 lGIVE_GOLD_PIECES(TwinEEngine *engine, LifeScriptContext &ctx) {
	int16 oldNumKashes = engine->_gameState->inventoryNumKashes;
	bool hideRange = false;
	int16 kashes = ctx.stream.readSint16LE();

	engine->_gameState->inventoryNumKashes -= kashes;
	if (engine->_gameState->inventoryNumKashes < 0) {
		engine->_gameState->inventoryNumKashes = 0;
	}

	engine->_redraw->addOverlay(koSprite, SPRITEHQR_KASHES, 10, 15, 0, koNormal, 3);

	for (int16 i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		OverlayListStruct *overlay = &engine->_redraw->overlayList[i];
		if (overlay->info0 != -1 && overlay->type == koNumberRange) {
			overlay->info0 = engine->_collision->getAverageValue(overlay->info1, overlay->info0, 100, overlay->lifeTime - engine->lbaTime - 50);
			overlay->info1 = engine->_gameState->inventoryNumKashes;
			overlay->lifeTime = engine->lbaTime + 150;
			hideRange = true;
			break;
		}
	}

	if (!hideRange) {
		engine->_redraw->addOverlay(koNumberRange, oldNumKashes, 50, 20, engine->_gameState->inventoryNumKashes, koNormal, 3);
	}

	return 0;
}

/*0x29*/
static int32 lEND_LIFE(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.actor->positionInLifeScript = -1;
	return 1; // break;
}

/*0x2A*/
static int32 lSTOP_L_TRACK(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.actor->pausedTrackPtr = ctx.actor->currentLabelPtr;
	ctx.actor->positionInMoveScript = -1;
	return 0;
}

/*0x2B*/
static int32 lRESTORE_L_TRACK(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.actor->positionInMoveScript = ctx.actor->pausedTrackPtr;
	return 0;
}

/*0x2C*/
static int32 lMESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const int32 textIdx = ctx.stream.readSint16LE();

	engine->freezeTime();
	if (engine->_text->showDialogueBubble) {
		engine->_redraw->drawBubble(otherActorIdx);
	}
	engine->_text->setFontCrossColor(engine->_scene->getActor(otherActorIdx)->talkColor);
	engine->_scene->talkingActor = otherActorIdx;
	engine->_text->drawTextFullscreen(textIdx);
	engine->unfreezeTime();
	engine->_redraw->redrawEngineActions(1);

	return 0;
}

/*0x2D*/
static int32 lINC_CHAPTER(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_gameState->gameChapter++;
	return 0;
}

/*0x2E*/
static int32 lFOUND_OBJECT(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 item = ctx.stream.readByte();

	engine->freezeTime();
	engine->_gameState->processFoundItem(item);
	engine->unfreezeTime();
	engine->_redraw->redrawEngineActions(1);

	return 0;
}

/*0x2F*/
static int32 lSET_DOOR_LEFT(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 distance = ctx.stream.readSint16LE();

	ctx.actor->angle = 0x300;
	ctx.actor->x = ctx.actor->lastX - distance;
	ctx.actor->dynamicFlags.bIsSpriteMoving = 0;
	ctx.actor->speed = 0;

	return 0;
}

/*0x30*/
static int32 lSET_DOOR_RIGHT(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 distance = ctx.stream.readSint16LE();

	ctx.actor->angle = 0x100;
	ctx.actor->x = ctx.actor->lastX + distance;
	ctx.actor->dynamicFlags.bIsSpriteMoving = 0;
	ctx.actor->speed = 0;

	return 0;
}

/*0x31*/
static int32 lSET_DOOR_UP(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 distance = ctx.stream.readSint16LE();

	ctx.actor->angle = 0x200;
	ctx.actor->z = ctx.actor->lastZ - distance;
	ctx.actor->dynamicFlags.bIsSpriteMoving = 0;
	ctx.actor->speed = 0;

	return 0;
}

/*0x32*/
static int32 lSET_DOOR_DOWN(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 distance = ctx.stream.readSint16LE();

	ctx.actor->angle = 0;
	ctx.actor->z = ctx.actor->lastZ + distance;
	ctx.actor->dynamicFlags.bIsSpriteMoving = 0;
	ctx.actor->speed = 0;

	return 0;
}

/*0x33*/
static int32 lGIVE_BONUS(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 flag = ctx.stream.readByte();

	if (ctx.actor->bonusParameter & 0x1F0) {
		engine->_actor->processActorExtraBonus(ctx.actorIdx);
	}

	if (flag != 0) {
		ctx.actor->bonusParameter |= 1;
	}

	return 0;
}

/*0x34*/
static int32 lCHANGE_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 sceneIdx = ctx.stream.readByte();
	engine->_scene->needChangeScene = sceneIdx;
	engine->_scene->heroPositionType = ScenePositionType::kScene;
	return 0;
}

/*0x35*/
static int32 lOBJ_COL(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 collision = ctx.stream.readByte();
	if (collision != 0) {
		ctx.actor->staticFlags.bComputeCollisionWithObj = 1;
	} else {
		ctx.actor->staticFlags.bComputeCollisionWithObj = 0;
	}
	return 0;
}

/*0x36*/
static int32 lBRICK_COL(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 collision = ctx.stream.readByte();

	ctx.actor->staticFlags.bComputeCollisionWithBricks = 0;
	ctx.actor->staticFlags.bComputeLowCollision = 0;

	if (collision == 1) {
		ctx.actor->staticFlags.bComputeCollisionWithBricks = 1;
	} else if (collision == 2) {
		ctx.actor->staticFlags.bComputeCollisionWithBricks = 1;
		ctx.actor->staticFlags.bComputeLowCollision = 1;
	}
	return 0;
}

/*0x37*/
static int32 lOR_IF(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 valueSize = processLifeConditions(engine, ctx);
	if (processLifeOperators(engine, ctx, valueSize)) {
		ctx.stream.seek(ctx.stream.readSint16LE()); // condition offset
	} else {
		ctx.stream.skip(2);
	}

	return 0;
}

/*0x38*/
static int32 lINVISIBLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.actor->staticFlags.bIsHidden = ctx.stream.readByte();
	return 0;
}

/*0x39*/
static int32 lZOOM(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->zoomScreen = ctx.stream.readByte();

	if (engine->zoomScreen && !engine->_redraw->drawInGameTransBox && engine->cfgfile.SceZoom) {
		engine->_screens->fadeToBlack(engine->_screens->mainPaletteRGBA);
		engine->initMCGA();
		engine->_screens->setBackPal();
		engine->_screens->lockPalette = true;
	} else if (!engine->zoomScreen && engine->_redraw->drawInGameTransBox) {
		engine->_screens->fadeToBlack(engine->_screens->mainPaletteRGBA);
		engine->initSVGA();
		engine->_screens->setBackPal();
		engine->_screens->lockPalette = true;
		engine->_redraw->reqBgRedraw = true;
	}

	return 0;
}

/*0x3A*/
static int32 lPOS_POINT(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 trackIdx = ctx.stream.readByte();

	const ScenePoint &sp = engine->_scene->sceneTracks[trackIdx];
	engine->_renderer->destX = sp.x;
	engine->_renderer->destY = sp.y;
	engine->_renderer->destZ = sp.z;

	ctx.actor->x = sp.x;
	ctx.actor->y = sp.y;
	ctx.actor->z = sp.z;

	return 0;
}

/*0x3B*/
static int32 lSET_MAGIC_LEVEL(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_gameState->magicLevelIdx = ctx.stream.readByte();
	engine->_gameState->inventoryMagicPoints = engine->_gameState->magicLevelIdx * 20;
	return 0;
}

/*0x3C*/
static int32 lSUB_MAGIC_POINT(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_gameState->inventoryMagicPoints = ctx.stream.readByte();
	if (engine->_gameState->inventoryMagicPoints < 0) {
		engine->_gameState->inventoryMagicPoints = 0;
	}
	return 0;
}

/*0x3D*/
static int32 lSET_LIFE_POINT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 otherActorIdx = ctx.stream.readByte();
	static int32 lifeValue = ctx.stream.readByte();

	engine->_scene->getActor(otherActorIdx)->life = lifeValue;

	return 0;
}

/*0x3E*/
static int32 lSUB_LIFE_POINT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 otherActorIdx = ctx.stream.readByte();
	static int32 lifeValue = ctx.stream.readByte();

	engine->_scene->getActor(otherActorIdx)->life -= lifeValue;

	if (engine->_scene->getActor(otherActorIdx)->life < 0) {
		engine->_scene->getActor(otherActorIdx)->life = 0;
	}

	return 0;
}

/*0x3F*/
static int32 lHIT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 otherActorIdx = ctx.stream.readByte();
	int32 strengthOfHit = ctx.stream.readByte();
	engine->_actor->hitActor(ctx.actorIdx, otherActorIdx, strengthOfHit, engine->_scene->getActor(otherActorIdx)->angle);
	return 0;
}

/*0x40*/
static int32 lPLAY_FLA(TwinEEngine *engine, LifeScriptContext &ctx) {
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

/*0x41*/
static int32 lPLAY_MIDI(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 midiIdx = ctx.stream.readByte();
	engine->_music->playMidiMusic(midiIdx); // TODO: improve this
	return 0;
}

/*0x42*/
static int32 lINC_CLOVER_BOX(TwinEEngine *engine, LifeScriptContext &ctx) {
	if (engine->_gameState->inventoryNumLeafsBox < 10) {
		engine->_gameState->inventoryNumLeafsBox++;
	}
	return 0;
}

/*0x43*/
static int32 lSET_USED_INVENTORY(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 item = ctx.stream.readByte();
	if (item < InventoryItems::kKeypad) { // TODO: this looks wrong - why only up to keypad?
		engine->_gameState->inventoryFlags[item] = 1;
	}
	return 0;
}

/*0x44*/
static int32 lADD_CHOICE(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 choiceIdx = ctx.stream.readSint16LE();
	engine->_gameState->gameChoices[engine->_gameState->numChoices++] = choiceIdx;
	return 0;
}

/*0x45*/
static int32 lASK_CHOICE(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 choiceIdx = ctx.stream.readSint16LE();

	engine->freezeTime();
	if (engine->_text->showDialogueBubble) {
		engine->_redraw->drawBubble(ctx.actorIdx);
	}
	engine->_text->setFontCrossColor(ctx.actor->talkColor);
	engine->_gameState->processGameChoices(choiceIdx);
	engine->_gameState->numChoices = 0;
	engine->unfreezeTime();
	engine->_redraw->redrawEngineActions(1);

	return 0;
}

/*0x46*/
static int32 lBIG_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 textIdx = ctx.stream.readSint16LE();

	engine->freezeTime();
	engine->_text->textClipFull();
	if (engine->_text->showDialogueBubble) {
		engine->_redraw->drawBubble(ctx.actorIdx);
	}
	engine->_text->setFontCrossColor(ctx.actor->talkColor);
	engine->_scene->talkingActor = ctx.actorIdx;
	engine->_text->drawTextFullscreen(textIdx);
	engine->_text->textClipSmall();
	engine->unfreezeTime();
	engine->_redraw->redrawEngineActions(1);

	return 0;
}

/*0x47*/
static int32 lINIT_PINGOUIN(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 pingouinActor = ctx.stream.readByte();
	engine->_scene->mecaPinguinIdx = pingouinActor;
	ActorStruct *mecaPinguin = engine->_scene->getActor(pingouinActor);
	mecaPinguin->dynamicFlags.bIsDead = 1;
	mecaPinguin->entity = -1;
	mecaPinguin->zone = -1;
	return 0;
}

/*0x48*/
static int32 lSET_HOLO_POS(TwinEEngine *engine, LifeScriptContext &ctx) {
	static int32 location = ctx.stream.readByte();
	engine->_holomap->setHolomapPosition(location);
	if (engine->_gameState->gameFlags[InventoryItems::kiHolomap]) {
		engine->_redraw->addOverlay(koInventoryItem, 0, 0, 0, 0, koNormal, 3);
	}

	return 0;
}

/*0x49*/
static int32 lCLR_HOLO_POS(TwinEEngine *engine, LifeScriptContext &ctx) {
	static int32 location = ctx.stream.readByte();
	engine->_holomap->clearHolomapPosition(location);
	return 0;
}

/*0x4A*/
static int32 lADD_FUEL(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_gameState->inventoryNumGas += ctx.stream.readByte();
	if (engine->_gameState->inventoryNumGas > 100) {
		engine->_gameState->inventoryNumGas = 100;
	}
	return 0;
}

/*0x4B*/
static int32 lSUB_FUEL(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_gameState->inventoryNumGas -= ctx.stream.readByte();
	if (engine->_gameState->inventoryNumGas < 0) {
		engine->_gameState->inventoryNumGas = 0;
	}
	return 0;
}

/*0x4C*/
static int32 lSET_GRM(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_grid->cellingGridIdx = ctx.stream.readByte();
	engine->_grid->initCellingGrid(engine->_grid->cellingGridIdx);
	return 0;
}

/*0x4D*/
static int32 lSAY_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx) {
	int16 textEntry = ctx.stream.readSint16LE();

	engine->_redraw->addOverlay(koText, textEntry, 0, 0, ctx.actorIdx, koFollowActor, 2);

	ScopedEngineFreeze scoped(engine);
	engine->_text->initVoxToPlay(textEntry);

	return 0;
}

/*04E*/
static int32 lSAY_MESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 otherActorIdx = ctx.stream.readByte();
	int16 textEntry = ctx.stream.readSint16LE();

	engine->_redraw->addOverlay(koText, textEntry, 0, 0, otherActorIdx, koFollowActor, 2);

	ScopedEngineFreeze scoped(engine);
	engine->_text->initVoxToPlay(textEntry);

	return 0;
}

/*0x4F*/
static int32 lFULL_POINT(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_scene->sceneHero->life = 50;
	engine->_gameState->inventoryMagicPoints = engine->_gameState->magicLevelIdx * 20;
	return 0;
}

/*0x50*/
static int32 lBETA(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 newAngle = ctx.stream.readSint16LE();
	ctx.actor->angle = newAngle;
	engine->_movements->clearRealAngle(ctx.actor);
	return 0;
}

/*0x51*/
static int32 lGRM_OFF(TwinEEngine *engine, LifeScriptContext &ctx) {
	if (engine->_grid->cellingGridIdx != -1) {
		engine->_grid->useCellingGrid = -1;
		engine->_grid->cellingGridIdx = -1;
		engine->_grid->createGridMap();
		engine->_redraw->redrawEngineActions(1);
	}

	return 0;
}

/*0x52*/
static int32 lFADE_PAL_RED(TwinEEngine *engine, LifeScriptContext &ctx) {
	ScopedEngineFreeze scoped(engine);
	engine->_screens->fadePalRed(engine->_screens->mainPaletteRGBA);
	engine->_screens->useAlternatePalette = false;
	return 0;
}

/*0x53*/
static int32 lFADE_ALARM_RED(TwinEEngine *engine, LifeScriptContext &ctx) {
	ScopedEngineFreeze scoped(engine);
	HQR::getEntry(engine->_screens->palette, Resources::HQR_RESS_FILE, RESSHQR_ALARMREDPAL);
	engine->_screens->convertPalToRGBA(engine->_screens->palette, engine->_screens->paletteRGBA);
	engine->_screens->fadePalRed(engine->_screens->paletteRGBA);
	engine->_screens->useAlternatePalette = true;
	return 0;
}

/*0x54*/
static int32 lFADE_ALARM_PAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	ScopedEngineFreeze scoped(engine);
	HQR::getEntry(engine->_screens->palette, Resources::HQR_RESS_FILE, RESSHQR_ALARMREDPAL);
	engine->_screens->convertPalToRGBA(engine->_screens->palette, engine->_screens->paletteRGBA);
	engine->_screens->adjustCrossPalette(engine->_screens->paletteRGBA, engine->_screens->mainPaletteRGBA);
	engine->_screens->useAlternatePalette = false;
	return 0;
}

/*0x55*/
static int32 lFADE_RED_PAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	ScopedEngineFreeze scoped(engine);
	engine->_screens->fadeRedPal(engine->_screens->mainPaletteRGBA);
	engine->_screens->useAlternatePalette = false;
	return 0;
}

/*0x56*/
static int32 lFADE_RED_ALARM(TwinEEngine *engine, LifeScriptContext &ctx) {
	ScopedEngineFreeze scoped(engine);
	HQR::getEntry(engine->_screens->palette, Resources::HQR_RESS_FILE, RESSHQR_ALARMREDPAL);
	engine->_screens->convertPalToRGBA(engine->_screens->palette, engine->_screens->paletteRGBA);
	engine->_screens->fadeRedPal(engine->_screens->paletteRGBA);
	engine->_screens->useAlternatePalette = true;
	return 0;
}

/*0x57*/
static int32 lFADE_PAL_ALARM(TwinEEngine *engine, LifeScriptContext &ctx) {
	ScopedEngineFreeze scoped(engine);
	HQR::getEntry(engine->_screens->palette, Resources::HQR_RESS_FILE, RESSHQR_ALARMREDPAL);
	engine->_screens->convertPalToRGBA(engine->_screens->palette, engine->_screens->paletteRGBA);
	engine->_screens->adjustCrossPalette(engine->_screens->mainPaletteRGBA, engine->_screens->paletteRGBA);
	engine->_screens->useAlternatePalette = true;
	return 0;
}

/*0x58*/
static int32 lEXPLODE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 otherActorIdx = ctx.stream.readByte();
	ActorStruct *otherActor = engine->_scene->getActor(otherActorIdx);

	engine->_extra->addExtraExplode(otherActor->x, otherActor->y, otherActor->z); // RECHECK this

	return 0;
}

/*0x59*/
static int32 lBUBBLE_ON(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_text->showDialogueBubble = true;
	return 0;
}

/*0x5A*/
static int32 lBUBBLE_OFF(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_text->showDialogueBubble = true; // TODO: this looks wrong - why true and not false?
	return 0;
}

/*0x5B*/
static int32 lASK_CHOICE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 otherActorIdx = ctx.stream.readByte();
	int32 choiceIdx = ctx.stream.readSint16LE();

	engine->freezeTime();
	if (engine->_text->showDialogueBubble) {
		engine->_redraw->drawBubble(otherActorIdx);
	}
	engine->_text->setFontCrossColor(engine->_scene->getActor(otherActorIdx)->talkColor);
	engine->_gameState->processGameChoices(choiceIdx);
	engine->_gameState->numChoices = 0;
	engine->unfreezeTime();
	engine->_redraw->redrawEngineActions(1);

	return 0;
}

/*0x5C*/
static int32 lSET_DARK_PAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	ScopedEngineFreeze scoped(engine);
	HQR::getEntry(engine->_screens->palette, Resources::HQR_RESS_FILE, RESSHQR_DARKPAL);
	if (!engine->_screens->lockPalette) {
		engine->_screens->convertPalToRGBA(engine->_screens->palette, engine->_screens->paletteRGBA);
		engine->setPalette(engine->_screens->paletteRGBA);
	}
	engine->_screens->useAlternatePalette = true;
	return 0;
}

/*0x5D*/
static int32 lSET_NORMAL_PAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_screens->useAlternatePalette = false;
	if (!engine->_screens->lockPalette) {
		engine->setPalette(engine->_screens->mainPaletteRGBA);
	}
	return 0;
}

/*0x5E*/
static int32 lMESSAGE_SENDELL(TwinEEngine *engine, LifeScriptContext &ctx) {
	ScopedEngineFreeze scoped(engine);
	engine->_screens->fadeToBlack(engine->_screens->paletteRGBA);
	engine->_screens->loadImage(25);
	engine->_text->textClipFull();
	engine->_text->setFontCrossColor(15);
	engine->_text->newGameVar4 = 0;
	const bool tmpFlagDisplayText = engine->cfgfile.FlagDisplayText;
	engine->cfgfile.FlagDisplayText = true;
	engine->_text->drawTextFullscreen(6);
	engine->_text->newGameVar4 = 1;
	engine->_text->textClipSmall();
	engine->_screens->fadeToBlack(engine->_screens->paletteRGBACustom);
	engine->_screens->clearScreen();
	engine->setPalette(engine->_screens->paletteRGBA);
	engine->cfgfile.FlagDisplayText = tmpFlagDisplayText;

	do {
		engine->readKeys();
		engine->_system->delayMillis(1);
	} while (engine->_input->toggleAbortAction());

	return 0;
}

/*0x5F*/
static int32 lANIM_SET(TwinEEngine *engine, LifeScriptContext &ctx) {
	AnimationTypes animIdx = (AnimationTypes)ctx.stream.readByte();

	ctx.actor->anim = kAnimNone;
	ctx.actor->previousAnimIdx = -1;
	engine->_animations->initAnim(animIdx, 0, 0, ctx.actorIdx);

	return 0;
}

/** Displays holomap travel animation */
/*0x60*/
static int32 lHOLOMAP_TRAJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.stream.skip(1); // index of the holomap trajectory
	return -1;
}

/*0x61*/
static int32 lGAME_OVER(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_scene->sceneHero->dynamicFlags.bAnimEnded = 1;
	engine->_scene->sceneHero->life = 0;
	engine->_gameState->inventoryNumLeafs = 0;
	return 1; // break
}

/*0x62*/
static int32 lTHE_END(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->quitGame = 1;
	engine->_gameState->inventoryNumLeafs = 0;
	engine->_scene->sceneHero->life = 50;
	engine->_gameState->inventoryMagicPoints = 80;
	engine->_scene->currentSceneIdx = LBA1SceneId::Polar_Island_Final_Battle;
	engine->_actor->heroBehaviour = engine->_actor->previousHeroBehaviour;
	engine->_scene->newHeroX = -1;
	engine->_scene->sceneHero->angle = engine->_actor->previousHeroAngle;
	engine->autoSave();
	return 1; // break;
}

/*0x63*/
static int32 lMIDI_OFF(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_music->stopMidiMusic();
	return 0;
}

/*0x64*/
static int32 lPLAY_CD_TRACK(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 track = ctx.stream.readByte();
	engine->_music->playTrackMusic(track);
	return 0;
}

/*0x65*/
static int32 lPROJ_ISO(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_renderer->setOrthoProjection(311, 240, 512);
	engine->_renderer->setBaseTranslation(0, 0, 0);
	engine->_renderer->setBaseRotation(0, 0, 0);
	engine->_renderer->setLightVector(engine->_scene->alphaLight, engine->_scene->betaLight, 0);
	return 0;
}

/*0x66*/
static int32 lPROJ_3D(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_screens->copyScreen(engine->frontVideoBuffer, engine->workVideoBuffer);
	engine->flip();
	engine->_scene->changeRoomVar10 = 0;

	engine->_renderer->setCameraPosition(320, 240, 128, 1024, 1024);
	engine->_renderer->setCameraAngle(0, 1500, 0, 25, -128, 0, 13000);
	engine->_renderer->setLightVector(896, 950, 0);

	engine->_text->initTextBank(TextBankId::Credits);

	return 0;
}

/*0x67*/
static int32 lTEXT(TwinEEngine *engine, LifeScriptContext &ctx) {
	int32 textIdx = ctx.stream.readSint16LE();

	if (drawVar1 < 440) {
		if (engine->cfgfile.Version == USA_VERSION) {
			if (!textIdx) {
				textIdx = TextId::kSaveSettings;
			}
		}

		engine->_text->getMenuText(textIdx, textStr, sizeof(textStr));
		int32 textSize = engine->_text->getTextSize(textStr);
		int32 textBoxRight = textSize;
		engine->_text->setFontColor(15);
		engine->_text->drawText(0, drawVar1, textStr);
		if (textSize > DEFAULT_SCREEN_WIDTH - 1) {
			textBoxRight = DEFAULT_SCREEN_WIDTH - 1;
		}

		drawVar1 += 40;
		engine->copyBlockPhys(0, drawVar1, textBoxRight, drawVar1);
	}

	return 0;
}

/*0x68*/
static int32 lCLEAR_TEXT(TwinEEngine *engine, LifeScriptContext &ctx) {
	drawVar1 = 0;
	engine->_interface->drawSplittedBox(0, 0, 639, 240, 0);
	engine->copyBlockPhys(0, 0, 639, 240);
	return 0;
}

/*0x69*/
static int32 lBRUTAL_EXIT(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->quitGame = 0;
	return 1; // break
}

static const ScriptLifeFunction function_map[] = {
    /*0x00*/ MAPFUNC("END", lEND),
    /*0x01*/ MAPFUNC("NOP", lNOP),
    /*0x02*/ MAPFUNC("SNIF", lSNIF),
    /*0x03*/ MAPFUNC("OFFSET", lOFFSET),
    /*0x04*/ MAPFUNC("NEVERIF", lNEVERIF),
    /*0x05*/ MAPFUNC("", lEMPTY), // unused
    /*0x06*/ MAPFUNC("NO_IF", lNO_IF),
    /*0x07*/ MAPFUNC("", lEMPTY), // unused
    /*0x08*/ MAPFUNC("", lEMPTY), // unused
    /*0x09*/ MAPFUNC("", lEMPTY), // unused
    /*0x0A*/ MAPFUNC("LABEL", lLABEL),
    /*0x0B*/ MAPFUNC("RETURN", lRETURN),
    /*0x0C*/ MAPFUNC("IF", lIF),
    /*0x0D*/ MAPFUNC("SWIF", lSWIF),
    /*0x0E*/ MAPFUNC("ONEIF", lONEIF),
    /*0x0F*/ MAPFUNC("ELSE", lELSE),
    /*0x10*/ MAPFUNC("ENDIF", lEMPTY), // unused
    /*0x11*/ MAPFUNC("BODY", lBODY),
    /*0x12*/ MAPFUNC("BODY_OBJ", lBODY_OBJ),
    /*0x13*/ MAPFUNC("ANIM", lANIM),
    /*0x14*/ MAPFUNC("ANIM_OBJ", lANIM_OBJ),
    /*0x15*/ MAPFUNC("SET_LIFE", lSET_LIFE),
    /*0x16*/ MAPFUNC("SET_LIFE_OBJ", lSET_LIFE_OBJ),
    /*0x17*/ MAPFUNC("SET_TRACK", lSET_TRACK),
    /*0x18*/ MAPFUNC("SET_TRACK_OBJ", lSET_TRACK_OBJ),
    /*0x19*/ MAPFUNC("MESSAGE", lMESSAGE),
    /*0x1A*/ MAPFUNC("FALLABLE", lFALLABLE),
    /*0x1B*/ MAPFUNC("SET_DIRMODE", lSET_DIRMODE),
    /*0x1C*/ MAPFUNC("SET_DIRMODE_OBJ", lSET_DIRMODE_OBJ),
    /*0x1D*/ MAPFUNC("CAM_FOLLOW", lCAM_FOLLOW),
    /*0x1E*/ MAPFUNC("SET_BEHAVIOUR", lSET_BEHAVIOUR),
    /*0x1F*/ MAPFUNC("SET_FLAG_CUBE", lSET_FLAG_CUBE),
    /*0x20*/ MAPFUNC("COMPORTEMENT", lCOMPORTEMENT),
    /*0x21*/ MAPFUNC("SET_COMPORTEMENT", lSET_COMPORTEMENT),
    /*0x22*/ MAPFUNC("SET_COMPORTEMENT_OBJ", lSET_COMPORTEMENT_OBJ),
    /*0x23*/ MAPFUNC("END_COMPORTEMENT", lEND_COMPORTEMENT),
    /*0x24*/ MAPFUNC("SET_FLAG_GAME", lSET_FLAG_GAME),
    /*0x25*/ MAPFUNC("KILL_OBJ", lKILL_OBJ),
    /*0x26*/ MAPFUNC("SUICIDE", lSUICIDE),
    /*0x27*/ MAPFUNC("USE_ONE_LITTLE_KEY", lUSE_ONE_LITTLE_KEY),
    /*0x28*/ MAPFUNC("GIVE_GOLD_PIECES", lGIVE_GOLD_PIECES),
    /*0x29*/ MAPFUNC("END_LIFE", lEND_LIFE),
    /*0x2A*/ MAPFUNC("STOP_L_TRACK", lSTOP_L_TRACK),
    /*0x2B*/ MAPFUNC("RESTORE_L_TRACK", lRESTORE_L_TRACK),
    /*0x2C*/ MAPFUNC("MESSAGE_OBJ", lMESSAGE_OBJ),
    /*0x2D*/ MAPFUNC("INC_CHAPTER", lINC_CHAPTER),
    /*0x2E*/ MAPFUNC("FOUND_OBJECT", lFOUND_OBJECT),
    /*0x2F*/ MAPFUNC("SET_DOOR_LEFT", lSET_DOOR_LEFT),
    /*0x30*/ MAPFUNC("SET_DOOR_RIGHT", lSET_DOOR_RIGHT),
    /*0x31*/ MAPFUNC("SET_DOOR_UP", lSET_DOOR_UP),
    /*0x32*/ MAPFUNC("SET_DOOR_DOWN", lSET_DOOR_DOWN),
    /*0x33*/ MAPFUNC("GIVE_BONUS", lGIVE_BONUS),
    /*0x34*/ MAPFUNC("CHANGE_CUBE", lCHANGE_CUBE),
    /*0x35*/ MAPFUNC("OBJ_COL", lOBJ_COL),
    /*0x36*/ MAPFUNC("BRICK_COL", lBRICK_COL),
    /*0x37*/ MAPFUNC("OR_IF", lOR_IF),
    /*0x38*/ MAPFUNC("INVISIBLE", lINVISIBLE),
    /*0x39*/ MAPFUNC("ZOOM", lZOOM),
    /*0x3A*/ MAPFUNC("POS_POINT", lPOS_POINT),
    /*0x3B*/ MAPFUNC("SET_MAGIC_LEVEL", lSET_MAGIC_LEVEL),
    /*0x3C*/ MAPFUNC("SUB_MAGIC_POINT", lSUB_MAGIC_POINT),
    /*0x3D*/ MAPFUNC("SET_LIFE_POINT_OBJ", lSET_LIFE_POINT_OBJ),
    /*0x3E*/ MAPFUNC("SUB_LIFE_POINT_OBJ", lSUB_LIFE_POINT_OBJ),
    /*0x3F*/ MAPFUNC("HIT_OBJ", lHIT_OBJ),
    /*0x40*/ MAPFUNC("PLAY_FLA", lPLAY_FLA),
    /*0x41*/ MAPFUNC("PLAY_MIDI", lPLAY_MIDI),
    /*0x42*/ MAPFUNC("INC_CLOVER_BOX", lINC_CLOVER_BOX),
    /*0x43*/ MAPFUNC("SET_USED_INVENTORY", lSET_USED_INVENTORY),
    /*0x44*/ MAPFUNC("ADD_CHOICE", lADD_CHOICE),
    /*0x45*/ MAPFUNC("ASK_CHOICE", lASK_CHOICE),
    /*0x46*/ MAPFUNC("BIG_MESSAGE", lBIG_MESSAGE),
    /*0x47*/ MAPFUNC("INIT_PINGOUIN", lINIT_PINGOUIN),
    /*0x48*/ MAPFUNC("SET_HOLO_POS", lSET_HOLO_POS),
    /*0x49*/ MAPFUNC("CLR_HOLO_POS", lCLR_HOLO_POS),
    /*0x4A*/ MAPFUNC("ADD_FUEL", lADD_FUEL),
    /*0x4B*/ MAPFUNC("SUB_FUEL", lSUB_FUEL),
    /*0x4C*/ MAPFUNC("SET_GRM", lSET_GRM),
    /*0x4D*/ MAPFUNC("SAY_MESSAGE", lSAY_MESSAGE),
    /*0x4E*/ MAPFUNC("SAY_MESSAGE_OBJ", lSAY_MESSAGE_OBJ),
    /*0x4F*/ MAPFUNC("FULL_POINT", lFULL_POINT),
    /*0x50*/ MAPFUNC("BETA", lBETA),
    /*0x51*/ MAPFUNC("GRM_OFF", lGRM_OFF),
    /*0x52*/ MAPFUNC("FADE_PAL_RED", lFADE_PAL_RED),
    /*0x53*/ MAPFUNC("FADE_ALARM_RED", lFADE_ALARM_RED),
    /*0x54*/ MAPFUNC("FADE_ALARM_PAL", lFADE_ALARM_PAL),
    /*0x55*/ MAPFUNC("FADE_RED_PAL", lFADE_RED_PAL),
    /*0x56*/ MAPFUNC("FADE_RED_ALARM", lFADE_RED_ALARM),
    /*0x57*/ MAPFUNC("FADE_PAL_ALARM", lFADE_PAL_ALARM),
    /*0x58*/ MAPFUNC("EXPLODE_OBJ", lEXPLODE_OBJ),
    /*0x59*/ MAPFUNC("BUBBLE_ON", lBUBBLE_ON),
    /*0x5A*/ MAPFUNC("BUBBLE_OFF", lBUBBLE_OFF),
    /*0x5B*/ MAPFUNC("ASK_CHOICE_OBJ", lASK_CHOICE_OBJ),
    /*0x5C*/ MAPFUNC("SET_DARK_PAL", lSET_DARK_PAL),
    /*0x5D*/ MAPFUNC("SET_NORMAL_PAL", lSET_NORMAL_PAL),
    /*0x5E*/ MAPFUNC("MESSAGE_SENDELL", lMESSAGE_SENDELL),
    /*0x5F*/ MAPFUNC("ANIM_SET", lANIM_SET),
    /*0x60*/ MAPFUNC("HOLOMAP_TRAJ", lHOLOMAP_TRAJ),
    /*0x61*/ MAPFUNC("GAME_OVER", lGAME_OVER),
    /*0x62*/ MAPFUNC("THE_END", lTHE_END),
    /*0x63*/ MAPFUNC("MIDI_OFF", lMIDI_OFF),
    /*0x64*/ MAPFUNC("PLAY_CD_TRACK", lPLAY_CD_TRACK),
    /*0x65*/ MAPFUNC("PROJ_ISO", lPROJ_ISO),
    /*0x66*/ MAPFUNC("PROJ_3D", lPROJ_3D),
    /*0x67*/ MAPFUNC("TEXT", lTEXT),
    /*0x68*/ MAPFUNC("CLEAR_TEXT", lCLEAR_TEXT),
    /*0x69*/ MAPFUNC("BRUTAL_EXIT", lBRUTAL_EXIT)};

ScriptLife::ScriptLife(TwinEEngine *engine) : _engine(engine) {
	drawVar1 = 0;
	textStr[0] = '\0';
}

void ScriptLife::processLifeScript(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	int32 end = -2;

	LifeScriptContext ctx(actorIdx, actor);
	do {
		const byte scriptOpcode = ctx.stream.readByte();
		if (scriptOpcode < ARRAYSIZE(function_map)) {
			end = function_map[scriptOpcode].function(_engine, ctx);
		} else {
			error("Actor %d with wrong offset/opcode - Offset: %d (opcode: %i)", actorIdx, ctx.stream.pos() - 1, scriptOpcode);
		}

		if (end < 0) {
			warning("Actor %d Life script [%s] not implemented", actorIdx, function_map[scriptOpcode].name);
		}
		ctx.updateOpcodePos();
	} while (end != 1);
}

} // namespace TwinE
