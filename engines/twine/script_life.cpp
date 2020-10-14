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

#include "twine/actor.h"
#include "twine/animations.h"
#include "twine/collision.h"
#include "twine/flamovies.h"
#include "twine/gamestate.h"
#include "twine/grid.h"
#include "twine/holomap.h"
#include "twine/interface.h"
#include "twine/keyboard.h"
#include "twine/movements.h"
#include "twine/music.h"
#include "twine/redraw.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/script_life.h"
#include "twine/sound.h"
#include "twine/text.h"

namespace TwinE {

static uint8 *scriptPtr; // local script pointer
static uint8 *opcodePtr; // local opcode script pointer

static int32 drawVar1;
static char textStr[256]; // string

/** Returns:
	   -1 - Need implementation
		0 - Completed
		1 - Break script */
typedef int32 ScriptLifeFunc(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor);

typedef struct ScriptLifeFunction {
	const char *name;
	ScriptLifeFunc *function;
} ScriptLifeFunction;

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
static int32 processLifeConditions(TwinEEngine *engine, ActorStruct *actor) {
	int32 conditionOpcode, conditionValueSize;

	conditionValueSize = 1;
	conditionOpcode = *(scriptPtr++);

	switch (conditionOpcode) {
	case kcCOL:
		if (actor->life <= 0) {
			engine->_scene->currentScriptValue = -1;
		} else {
			engine->_scene->currentScriptValue = actor->collision;
		}
		break;
	case kcCOL_OBJ: {
		int32 actorIdx = *(scriptPtr++);
		if (engine->_scene->sceneActors[actorIdx].life <= 0) {
			engine->_scene->currentScriptValue = -1;
		} else {
			engine->_scene->currentScriptValue = engine->_scene->sceneActors[actorIdx].collision;
		}
	} break;
	case kcDISTANCE: {
		ActorStruct *otherActor;
		int32 actorIdx = *(scriptPtr++);
		conditionValueSize = 2;
		otherActor = &engine->_scene->sceneActors[actorIdx];
		if (!otherActor->dynamicFlags.bIsDead) {
			if (ABS(actor->y - otherActor->y) >= 1500) {
				engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
			} else {
				// Returns int32, so we check for integer overflow
				int32 distance = engine->_movements->getDistance2D(actor->x, actor->z, otherActor->x, otherActor->z);
				if (ABS(distance) > MAX_TARGET_ACTOR_DISTANCE) {
					engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
				} else {
					engine->_scene->currentScriptValue = distance;
				}
			}
		} else {
			engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
		}
	} break;
	case kcZONE:
		engine->_scene->currentScriptValue = actor->zone;
		break;
	case kcZONE_OBJ: {
		int32 actorIdx = *(scriptPtr++);
		engine->_scene->currentScriptValue = engine->_scene->sceneActors[actorIdx].zone;
	} break;
	case kcBODY:
		engine->_scene->currentScriptValue = actor->body;
		break;
	case kcBODY_OBJ: {
		int32 actorIdx = *(scriptPtr++);
		engine->_scene->currentScriptValue = engine->_scene->sceneActors[actorIdx].body;
	} break;
	case kcANIM:
		engine->_scene->currentScriptValue = actor->anim;
		break;
	case kcANIM_OBJ: {
		int32 actorIdx = *(scriptPtr++);
		engine->_scene->currentScriptValue = engine->_scene->sceneActors[actorIdx].anim;
	} break;
	case kcL_TRACK:
		engine->_scene->currentScriptValue = actor->labelIdx;
		break;
	case kcL_TRACK_OBJ: {
		int32 actorIdx = *(scriptPtr++);
		engine->_scene->currentScriptValue = engine->_scene->sceneActors[actorIdx].labelIdx;
	} break;
	case kcFLAG_CUBE: {
		int32 flagIdx = *(scriptPtr++);
		engine->_scene->currentScriptValue = engine->_scene->sceneFlags[flagIdx];
	} break;
	case kcCONE_VIEW: {
		int32 newAngle;
		int32 targetActorIdx;
		ActorStruct *targetActor;

		newAngle = 0;
		targetActorIdx = *(scriptPtr++);
		targetActor = &engine->_scene->sceneActors[targetActorIdx];

		conditionValueSize = 2;

		if (!targetActor->dynamicFlags.bIsDead) {
			if (ABS(targetActor->y - actor->y) < 1500) {
				newAngle = engine->_movements->getAngleAndSetTargetActorDistance(actor->x, actor->z, targetActor->x, targetActor->z);
				if (ABS(engine->_movements->targetActorDistance) > MAX_TARGET_ACTOR_DISTANCE) {
					engine->_movements->targetActorDistance = MAX_TARGET_ACTOR_DISTANCE;
				}
			} else {
				engine->_movements->targetActorDistance = MAX_TARGET_ACTOR_DISTANCE;
			}

			if (!targetActorIdx) {
				int32 heroAngle;

				heroAngle = actor->angle + 0x480 - newAngle + 0x400;
				heroAngle &= 0x3FF;

				if (ABS(heroAngle) > 0x100) {
					engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
				} else {
					engine->_scene->currentScriptValue = engine->_movements->targetActorDistance;
				}
			} else {
				if (engine->_actor->heroBehaviour == kDiscrete) {
					int32 heroAngle;

					heroAngle = actor->angle + 0x480 - newAngle + 0x400;
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
	} break;
	case kcHIT_BY:
		engine->_scene->currentScriptValue = actor->hitBy;
		break;
	case kcACTION:
		engine->_scene->currentScriptValue = engine->_movements->heroAction;
		break;
	case kcFLAG_GAME: {
		int32 flagIdx = *(scriptPtr++);
		if (!engine->_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED] ||
		    (engine->_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED] && flagIdx >= 28)) {
			engine->_scene->currentScriptValue = engine->_gameState->gameFlags[flagIdx];
		} else {
			if (flagIdx == GAMEFLAG_INVENTORY_DISABLED) {
				engine->_scene->currentScriptValue = engine->_gameState->gameFlags[flagIdx];
			} else {
				engine->_scene->currentScriptValue = 0;
			}
		}
	} break;
	case kcLIFE_POINT:
		engine->_scene->currentScriptValue = actor->life;
		break;
	case kcLIFE_POINT_OBJ: {
		int32 actorIdx = *(scriptPtr++);
		engine->_scene->currentScriptValue = engine->_scene->sceneActors[actorIdx].life;
	} break;
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

		targetActorIdx = *(scriptPtr++);
		targetActor = &engine->_scene->sceneActors[targetActorIdx];

		conditionValueSize = 2;

		if (!targetActor->dynamicFlags.bIsDead) {
			// Returns int32, so we check for integer overflow
			int32 distance = engine->_movements->getDistance3D(actor->x, actor->y, actor->z, targetActor->x, targetActor->y, targetActor->z);
			if (ABS(distance) > MAX_TARGET_ACTOR_DISTANCE) {
				engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
			} else {
				engine->_scene->currentScriptValue = distance;
			}
		} else {
			engine->_scene->currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
		}
	} break;
	case 23: // unused
	case 24:
		break;
	case kcUSE_INVENTORY: {
		int32 item = *(scriptPtr++);

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
	} break;
	case kcCHOICE:
		conditionValueSize = 2;
		engine->_scene->currentScriptValue = engine->_gameState->choiceAnswer;
		break;
	case kcFUEL:
		engine->_scene->currentScriptValue = engine->_gameState->inventoryNumGas;
		break;
	case kcCARRIED_BY:
		engine->_scene->currentScriptValue = actor->standOn;
		break;
	case kcCDROM:
		engine->_scene->currentScriptValue = 1;
		break;
	default:
		error("Actor condition opcode %d\n", conditionOpcode);
		break;
	}

	return conditionValueSize;
}

/** Returns:
	   -1 - Need implementation
		0 - Condition false
		1 - Condition true */
static int32 processLifeOperators(TwinEEngine *engine, int32 valueSize) {
	int32 operatorCode, conditionValue;

	operatorCode = *(scriptPtr++);

	if (valueSize == 1) {
		conditionValue = *(scriptPtr++);
	} else if (valueSize == 2) {
		conditionValue = *((int16 *)scriptPtr);
		scriptPtr += 2;
	} else {
		error("Unknown operator value size %d\n", valueSize);
		return 0;
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
		error("Actor operator opcode %d\n", operatorCode);
		break;
	}

	return 0;
}

/** Life script command definitions */

/* For unused opcodes */
static int32 lEMPTY(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	return 0;
}

/*0x00*/
static int32 lEND(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInLifeScript = -1;
	return 1; // break script
}

/*0x01*/
static int32 lNOP(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	scriptPtr++;
	return 0;
}

/*0x02*/
static int32 lSNIF(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 valueSize = processLifeConditions(engine, actor);
	if (!processLifeOperators(engine, valueSize)) {
		*opcodePtr = 13; // SWIF
	}
	scriptPtr = actor->lifeScript + *((int16 *)scriptPtr); // condition offset
	return 0;
}

/*0x03*/
static int32 lOFFSET(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	scriptPtr = actor->lifeScript + *((int16 *)scriptPtr); // offset
	return 0;
}

/*0x04*/
static int32 lNEVERIF(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 valueSize = processLifeConditions(engine, actor);
	processLifeOperators(engine, valueSize);
	scriptPtr = actor->lifeScript + *((int16 *)scriptPtr); // condition offset
	return 0;
}

/*0x06*/
static int32 lNO_IF(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	return 0;
}

/*0x0A*/
static int32 lLABEL(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	scriptPtr++;
	return 0;
}

/*0x0B*/
static int32 lRETURN(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	return 1; // break script
}

/*0x0C*/
static int32 lIF(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 valueSize = processLifeConditions(engine, actor);
	if (!processLifeOperators(engine, valueSize)) {
		scriptPtr = actor->lifeScript + *((int16 *)scriptPtr); // condition offset
	} else {
		scriptPtr += 2;
	}

	return 0;
}

/*0x0D*/
static int32 lSWIF(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 valueSize = processLifeConditions(engine, actor);
	if (!processLifeOperators(engine, valueSize)) {
		scriptPtr = actor->lifeScript + *((int16 *)scriptPtr); // condition offset
	} else {
		scriptPtr += 2;
		*opcodePtr = 2; // SNIF
	}

	return 0;
}

/*0x0E*/
static int32 lONEIF(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 valueSize = processLifeConditions(engine, actor);
	if (!processLifeOperators(engine, valueSize)) {
		scriptPtr = actor->lifeScript + *((int16 *)scriptPtr); // condition offset
	} else {
		scriptPtr += 2;
		*opcodePtr = 4; // NEVERIF
	}

	return 0;
}

/*0x0F*/
static int32 lELSE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	scriptPtr = actor->lifeScript + *((int16 *)scriptPtr); // offset
	return 0;
}

/*0x11*/
static int32 lBODY(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 bodyIdx = *(scriptPtr);
	engine->_actor->initModelActor(bodyIdx, actorIdx);
	scriptPtr++;
	return 0;
}

/*0x12*/
static int32 lBODY_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);
	int32 otherBodyIdx = *(scriptPtr++);
	engine->_actor->initModelActor(otherBodyIdx, otherActorIdx);
	return 0;
}

/*0x13*/
static int32 lANIM(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	AnimationTypes animIdx = (AnimationTypes)*(scriptPtr++);
	engine->_animations->initAnim(animIdx, 0, 0, actorIdx);
	return 0;
}

/*0x14*/
static int32 lANIM_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);
	AnimationTypes otherAnimIdx = (AnimationTypes)*(scriptPtr++);
	engine->_animations->initAnim(otherAnimIdx, 0, 0, otherActorIdx);
	return 0;
}

/*0x15*/
static int32 lSET_LIFE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInLifeScript = *((int16 *)scriptPtr); // offset
	scriptPtr += 2;
	return 0;
}

/*0x16*/
static int32 lSET_LIFE_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);
	engine->_scene->sceneActors[otherActorIdx].positionInLifeScript = *((int16 *)scriptPtr); // offset
	scriptPtr += 2;
	return 0;
}

/*0x17*/
static int32 lSET_TRACK(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript = *((int16 *)scriptPtr); // offset
	scriptPtr += 2;
	return 0;
}

/*0x18*/
static int32 lSET_TRACK_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);
	engine->_scene->sceneActors[otherActorIdx].positionInMoveScript = *((int16 *)scriptPtr); // offset
	scriptPtr += 2;
	return 0;
}

/*0x19*/
static int32 lMESSAGE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 textIdx = *((int16 *)scriptPtr);
	scriptPtr += 2;

	engine->freezeTime();
	if (engine->_text->showDialogueBubble) {
		engine->_redraw->drawBubble(actorIdx);
	}
	engine->_text->setFontCrossColor(actor->talkColor);
	engine->_scene->talkingActor = actorIdx;
	engine->_text->drawTextFullscreen(textIdx);
	engine->unfreezeTime();
	engine->_redraw->redrawEngineActions(1);

	return 0;
}

/*0x1A*/
static int32 lFALLABLE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 flag = *(scriptPtr++);
	actor->staticFlags.bCanFall = flag & 1;
	return 0;
}

/*0x1B*/
static int32 lSET_DIRMODE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 controlMode = *(scriptPtr++);

	actor->controlMode = controlMode;
	if (controlMode == kFollow) {
		actor->followedActor = *(scriptPtr++);
	}

	return 0;
}

/*0x1C*/
static int32 lSET_DIRMODE_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);
	int32 controlMode = *(scriptPtr++);

	engine->_scene->sceneActors[otherActorIdx].controlMode = controlMode;
	if (controlMode == kFollow) {
		engine->_scene->sceneActors[otherActorIdx].followedActor = *(scriptPtr++);
	}

	return 0;
}

/*0x1D*/
static int32 lCAM_FOLLOW(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 followedActorIdx;
	followedActorIdx = *(scriptPtr++);

	if (engine->_scene->currentlyFollowedActor != followedActorIdx) {
		engine->_grid->newCameraX = engine->_scene->sceneActors[followedActorIdx].x >> 9;
		engine->_grid->newCameraY = engine->_scene->sceneActors[followedActorIdx].y >> 8;
		engine->_grid->newCameraZ = engine->_scene->sceneActors[followedActorIdx].z >> 9;

		engine->_scene->currentlyFollowedActor = followedActorIdx;
		engine->_redraw->reqBgRedraw = 1;
	}

	return 0;
}

/*0x1E*/
static int32 lSET_BEHAVIOUR(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 behavior = *(scriptPtr++);

	engine->_animations->initAnim(kStanding, 0, 255, 0);
	engine->_actor->setBehaviour(behavior);

	return 0;
}

/*0x1F*/
static int32 lSET_FLAG_CUBE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 flagIdx = *(scriptPtr++);
	int32 flagValue = *(scriptPtr++);

	engine->_scene->sceneFlags[flagIdx] = flagValue;

	return 0;
}

/*0x20*/
static int32 lCOMPORTEMENT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	scriptPtr++;
	return 0;
}

/*0x21*/
static int32 lSET_COMPORTEMENT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInLifeScript = *((int16 *)scriptPtr);
	scriptPtr += 2;
	return 0;
}

/*0x22*/
static int32 lSET_COMPORTEMENT_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);

	engine->_scene->sceneActors[otherActorIdx].positionInLifeScript = *((int16 *)scriptPtr);
	scriptPtr += 2;

	return 0;
}

/*0x23*/
static int32 lEND_COMPORTEMENT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	return 1; // break
}

/*0x24*/
static int32 lSET_FLAG_GAME(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 flagIdx = *(scriptPtr++);
	int32 flagValue = *(scriptPtr++);

	engine->_gameState->gameFlags[flagIdx] = flagValue;

	return 0;
}

/*0x25*/
static int32 lKILL_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);

	engine->_actor->processActorCarrier(otherActorIdx);
	engine->_scene->sceneActors[otherActorIdx].dynamicFlags.bIsDead = 1;
	engine->_scene->sceneActors[otherActorIdx].entity = -1;
	engine->_scene->sceneActors[otherActorIdx].zone = -1;
	engine->_scene->sceneActors[otherActorIdx].life = 0;

	return 0;
}

/*0x26*/
static int32 lSUICIDE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_actor->processActorCarrier(actorIdx);
	actor->dynamicFlags.bIsDead = 1;
	actor->entity = -1;
	actor->zone = -1;
	actor->life = 0;

	return 0;
}

/*0x27*/
static int32 lUSE_ONE_LITTLE_KEY(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_gameState->inventoryNumKeys--;

	if (engine->_gameState->inventoryNumKeys < 0) {
		engine->_gameState->inventoryNumKeys = 0;
	}

	engine->_redraw->addOverlay(koSprite, SPRITEHQR_KEY, 0, 0, 0, koFollowActor, 1);

	return 0;
}

/*0x28*/
static int32 lGIVE_GOLD_PIECES(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int16 kashes, i, hideRange;
	int16 oldNumKashes = engine->_gameState->inventoryNumKashes;

	hideRange = 0;

	kashes = *((int16 *)scriptPtr);
	scriptPtr += 2;

	engine->_gameState->inventoryNumKashes -= kashes;
	if (engine->_gameState->inventoryNumKashes < 0) {
		engine->_gameState->inventoryNumKashes = 0;
	}

	engine->_redraw->addOverlay(koSprite, SPRITEHQR_KASHES, 10, 15, 0, koNormal, 3);

	for (i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		OverlayListStruct *overlay = &engine->_redraw->overlayList[i];
		if (overlay->info0 != -1 && overlay->type == koNumberRange) {
			overlay->info0 = engine->_collision->getAverageValue(overlay->info1, overlay->info0, 100, overlay->lifeTime - engine->lbaTime - 50);
			overlay->info1 = engine->_gameState->inventoryNumKashes;
			overlay->lifeTime = engine->lbaTime + 150;
			hideRange = 1;
			break;
		}
	}

	if (!hideRange) {
		engine->_redraw->addOverlay(koNumberRange, oldNumKashes, 50, 20, engine->_gameState->inventoryNumKashes, koNormal, 3);
	}

	return 0;
}

/*0x29*/
static int32 lEND_LIFE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInLifeScript = -1;
	return 1; // break;
}

/*0x2A*/
static int32 lSTOP_L_TRACK(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->pausedTrackPtr = actor->currentLabelPtr;
	actor->positionInMoveScript = -1;
	return 0;
}

/*0x2B*/
static int32 lRESTORE_L_TRACK(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->positionInMoveScript = actor->pausedTrackPtr;
	return 0;
}

/*0x2C*/
static int32 lMESSAGE_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);
	int32 textIdx = *((int16 *)scriptPtr);
	scriptPtr += 2;

	engine->freezeTime();
	if (engine->_text->showDialogueBubble) {
		engine->_redraw->drawBubble(otherActorIdx);
	}
	engine->_text->setFontCrossColor(engine->_scene->sceneActors[otherActorIdx].talkColor);
	engine->_scene->talkingActor = otherActorIdx;
	engine->_text->drawTextFullscreen(textIdx);
	engine->unfreezeTime();
	engine->_redraw->redrawEngineActions(1);

	return 0;
}

/*0x2D*/
static int32 lINC_CHAPTER(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_gameState->gameChapter++;
	return 0;
}

/*0x2E*/
static int32 lFOUND_OBJECT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 item = *(scriptPtr++);

	engine->freezeTime();
	engine->_gameState->processFoundItem(item);
	engine->unfreezeTime();
	engine->_redraw->redrawEngineActions(1);

	return 0;
}

/*0x2F*/
static int32 lSET_DOOR_LEFT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 distance = *((int16 *)scriptPtr);
	scriptPtr += 2;

	actor->angle = 0x300;
	actor->x = actor->lastX - distance;
	actor->dynamicFlags.bIsSpriteMoving = 0;
	actor->speed = 0;

	return 0;
}

/*0x30*/
static int32 lSET_DOOR_RIGHT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 distance = *((int16 *)scriptPtr);
	scriptPtr += 2;

	actor->angle = 0x100;
	actor->x = actor->lastX + distance;
	actor->dynamicFlags.bIsSpriteMoving = 0;
	actor->speed = 0;

	return 0;
}

/*0x31*/
static int32 lSET_DOOR_UP(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 distance = *((int16 *)scriptPtr);
	scriptPtr += 2;

	actor->angle = 0x200;
	actor->z = actor->lastZ - distance;
	actor->dynamicFlags.bIsSpriteMoving = 0;
	actor->speed = 0;

	return 0;
}

/*0x32*/
static int32 lSET_DOOR_DOWN(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 distance = *((int16 *)scriptPtr);
	scriptPtr += 2;

	actor->angle = 0;
	actor->z = actor->lastZ + distance;
	actor->dynamicFlags.bIsSpriteMoving = 0;
	actor->speed = 0;

	return 0;
}

/*0x33*/
static int32 lGIVE_BONUS(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 flag = *(scriptPtr++);

	if (actor->bonusParameter & 0x1F0) {
		engine->_actor->processActorExtraBonus(actorIdx);
	}

	if (flag != 0) {
		actor->bonusParameter |= 1;
	}

	return 0;
}

/*0x34*/
static int32 lCHANGE_CUBE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 sceneIdx = *(scriptPtr++);
	engine->_scene->needChangeScene = sceneIdx;
	engine->_scene->heroPositionType = kScene;
	return 0;
}

/*0x35*/
static int32 lOBJ_COL(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 collision = *(scriptPtr++);
	if (collision != 0) {
		actor->staticFlags.bComputeCollisionWithObj = 1;
	} else {
		actor->staticFlags.bComputeCollisionWithObj = 0;
	}
	return 0;
}

/*0x36*/
static int32 lBRICK_COL(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 collision = *(scriptPtr++);

	actor->staticFlags.bComputeCollisionWithBricks = 0;
	actor->staticFlags.bComputeLowCollision = 0;

	if (collision == 1) {
		actor->staticFlags.bComputeCollisionWithBricks = 1;
	} else if (collision == 2) {
		actor->staticFlags.bComputeCollisionWithBricks = 1;
		actor->staticFlags.bComputeLowCollision = 1;
	}
	return 0;
}

/*0x37*/
static int32 lOR_IF(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 valueSize = processLifeConditions(engine, actor);
	if (processLifeOperators(engine, valueSize)) {
		scriptPtr = actor->lifeScript + *((int16 *)scriptPtr); // condition offset
	} else {
		scriptPtr += 2;
	}

	return 0;
}

/*0x38*/
static int32 lINVISIBLE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	actor->staticFlags.bIsHidden = *(scriptPtr++);
	return 0;
}

/*0x39*/
static int32 lZOOM(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->zoomScreen = *(scriptPtr++);

	if (engine->zoomScreen && !engine->_redraw->drawInGameTransBox && engine->cfgfile.SceZoom) {
		engine->_screens->fadeToBlack(engine->_screens->mainPaletteRGB);
		engine->initMCGA();
		engine->_screens->setBackPal();
		engine->_screens->lockPalette = 1;
	} else if (!engine->zoomScreen && engine->_redraw->drawInGameTransBox) {
		engine->_screens->fadeToBlack(engine->_screens->mainPaletteRGB);
		engine->initSVGA();
		engine->_screens->setBackPal();
		engine->_screens->lockPalette = 1;
		engine->_redraw->reqBgRedraw = 1;
	}

	return 0;
}

/*0x3A*/
static int32 lPOS_POINT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 trackIdx = *(scriptPtr++);

	engine->_renderer->destX = engine->_scene->sceneTracks[trackIdx].x;
	engine->_renderer->destY = engine->_scene->sceneTracks[trackIdx].y;
	engine->_renderer->destZ = engine->_scene->sceneTracks[trackIdx].z;

	actor->x = engine->_renderer->destX;
	actor->y = engine->_renderer->destY;
	actor->z = engine->_renderer->destZ;

	return 0;
}

/*0x3B*/
static int32 lSET_MAGIC_LEVEL(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_gameState->magicLevelIdx = *(scriptPtr++);
	engine->_gameState->inventoryMagicPoints = engine->_gameState->magicLevelIdx * 20;
	return 0;
}

/*0x3C*/
static int32 lSUB_MAGIC_POINT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_gameState->inventoryMagicPoints = *(scriptPtr++);
	if (engine->_gameState->inventoryMagicPoints < 0) {
		engine->_gameState->inventoryMagicPoints = 0;
	}
	return 0;
}

/*0x3D*/
static int32 lSET_LIFE_POINT_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);
	static int32 lifeValue = *(scriptPtr++);

	engine->_scene->sceneActors[otherActorIdx].life = lifeValue;

	return 0;
}

/*0x3E*/
static int32 lSUB_LIFE_POINT_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);
	static int32 lifeValue = *(scriptPtr++);

	engine->_scene->sceneActors[otherActorIdx].life -= lifeValue;

	if (engine->_scene->sceneActors[otherActorIdx].life < 0) {
		engine->_scene->sceneActors[otherActorIdx].life = 0;
	}

	return 0;
}

/*0x3F*/
static int32 lHIT_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);
	int32 strengthOfHit = *(scriptPtr++);
	engine->_actor->hitActor(actorIdx, otherActorIdx, strengthOfHit, engine->_scene->sceneActors[otherActorIdx].angle);
	return 0;
}

/*0x40*/
static int32 lPLAY_FLA(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	const char *movie = (const char *)scriptPtr;
	int32 nameSize = strlen(movie);
	scriptPtr += nameSize + 1;

	engine->_flaMovies->playFlaMovie(movie);
	engine->setPalette(engine->_screens->paletteRGB);
	engine->_screens->clearScreen();
	engine->flip();

	return 0;
}

/*0x41*/
static int32 lPLAY_MIDI(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 midiIdx = *(scriptPtr++);
	engine->_music->playMidiMusic(midiIdx, 0); // TODO: improve this
	return 0;
}

/*0x42*/
static int32 lINC_CLOVER_BOX(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	if (engine->_gameState->inventoryNumLeafsBox < 10) {
		engine->_gameState->inventoryNumLeafsBox++;
	}
	return 0;
}

/*0x43*/
static int32 lSET_USED_INVENTORY(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 item = *(scriptPtr++);
	if (item < 24) {
		engine->_gameState->inventoryFlags[item] = 1;
	}
	return 0;
}

/*0x44*/
static int32 lADD_CHOICE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 choiceIdx = *((int16 *)scriptPtr);
	scriptPtr += 2;
	engine->_gameState->gameChoices[engine->_gameState->numChoices++] = choiceIdx;
	return 0;
}

/*0x45*/
static int32 lASK_CHOICE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 choiceIdx = *((int16 *)scriptPtr);
	scriptPtr += 2;

	engine->freezeTime();
	if (engine->_text->showDialogueBubble) {
		engine->_redraw->drawBubble(actorIdx);
	}
	engine->_text->setFontCrossColor(actor->talkColor);
	engine->_gameState->processGameChoices(choiceIdx);
	engine->_gameState->numChoices = 0;
	engine->unfreezeTime();
	engine->_redraw->redrawEngineActions(1);

	return 0;
}

/*0x46*/
static int32 lBIG_MESSAGE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 textIdx = *((int16 *)scriptPtr);
	scriptPtr += 2;

	engine->freezeTime();
	engine->_text->textClipFull();
	if (engine->_text->showDialogueBubble) {
		engine->_redraw->drawBubble(actorIdx);
	}
	engine->_text->setFontCrossColor(actor->talkColor);
	engine->_scene->talkingActor = actorIdx;
	engine->_text->drawTextFullscreen(textIdx);
	engine->_text->textClipSmall();
	engine->unfreezeTime();
	engine->_redraw->redrawEngineActions(1);

	return 0;
}

/*0x47*/
static int32 lINIT_PINGOUIN(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 pingouinActor = *(scriptPtr++);
	engine->_scene->sceneActors[pingouinActor].dynamicFlags.bIsDead = 1;
	engine->_scene->mecaPinguinIdx = pingouinActor;
	engine->_scene->sceneActors[pingouinActor].entity = -1;
	engine->_scene->sceneActors[pingouinActor].zone = -1;
	return 0;
}

/*0x48*/
static int32 lSET_HOLO_POS(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	static int32 location = *(scriptPtr++);

	engine->_holomap->setHolomapPosition(location);
	if (engine->_gameState->gameFlags[InventoryItems::kiHolomap]) {
		engine->_redraw->addOverlay(koInventoryItem, 0, 0, 0, 0, koNormal, 3);
	}

	return 0;
}

/*0x49*/
static int32 lCLR_HOLO_POS(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	static int32 location = *(scriptPtr++);
	engine->_holomap->clearHolomapPosition(location);
	return 0;
}

/*0x4A*/
static int32 lADD_FUEL(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_gameState->inventoryNumGas += *(scriptPtr++);
	if (engine->_gameState->inventoryNumGas > 100) {
		engine->_gameState->inventoryNumGas = 100;
	}
	return 0;
}

/*0x4B*/
static int32 lSUB_FUEL(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_gameState->inventoryNumGas -= *(scriptPtr++);
	if (engine->_gameState->inventoryNumGas < 0) {
		engine->_gameState->inventoryNumGas = 0;
	}
	return 0;
}

/*0x4C*/
static int32 lSET_GRM(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_grid->cellingGridIdx = *(scriptPtr++);
	engine->_grid->initCellingGrid(engine->_grid->cellingGridIdx);
	return 0;
}

/*0x4D*/
static int32 lSAY_MESSAGE(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int16 textEntry = *((int16 *)scriptPtr);
	scriptPtr += 2;

	engine->_redraw->addOverlay(koText, textEntry, 0, 0, actorIdx, koFollowActor, 2);

	engine->freezeTime();
	engine->_text->initVoxToPlay(textEntry);
	engine->unfreezeTime();

	return 0;
}

/*04E*/
static int32 lSAY_MESSAGE_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);
	int16 textEntry = *((int16 *)scriptPtr);
	scriptPtr += 2;

	engine->_redraw->addOverlay(koText, textEntry, 0, 0, otherActorIdx, koFollowActor, 2);

	engine->freezeTime();
	engine->_text->initVoxToPlay(textEntry);
	engine->unfreezeTime();

	return 0;
}

/*0x4F*/
static int32 lFULL_POINT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_scene->sceneHero->life = 50;
	engine->_gameState->inventoryMagicPoints = engine->_gameState->magicLevelIdx * 20;
	return 0;
}

/*0x50*/
static int32 lBETA(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 newAngle = *((int16 *)scriptPtr);
	scriptPtr += 2;
	actor->angle = newAngle;
	engine->_movements->clearRealAngle(actor);
	return 0;
}

/*0x51*/
static int32 lGRM_OFF(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	if (engine->_grid->cellingGridIdx != -1) {
		engine->_grid->useCellingGrid = -1;
		engine->_grid->cellingGridIdx = -1;
		engine->_grid->createGridMap();
		engine->_redraw->redrawEngineActions(1);
	}

	return 0;
}

/*0x52*/
static int32 lFADE_PAL_RED(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->freezeTime();
	engine->_screens->fadePalRed(engine->_screens->mainPaletteRGB);
	engine->_screens->useAlternatePalette = 0;
	engine->unfreezeTime();
	return 0;
}

/*0x53*/
static int32 lFADE_ALARM_RED(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->freezeTime();
	engine->_hqrdepack->hqrGetEntry(engine->_screens->palette, Resources::HQR_RESS_FILE, RESSHQR_ALARMREDPAL);
	engine->_screens->copyPal(engine->_screens->palette, engine->_screens->paletteRGB);
	engine->_screens->fadePalRed(engine->_screens->paletteRGB);
	engine->_screens->useAlternatePalette = 1;
	engine->unfreezeTime();
	return 0;
}

/*0x54*/
static int32 lFADE_ALARM_PAL(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->freezeTime();
	engine->_hqrdepack->hqrGetEntry(engine->_screens->palette, Resources::HQR_RESS_FILE, RESSHQR_ALARMREDPAL);
	engine->_screens->copyPal(engine->_screens->palette, engine->_screens->paletteRGB);
	engine->_screens->adjustCrossPalette(engine->_screens->paletteRGB, engine->_screens->mainPaletteRGB);
	engine->_screens->useAlternatePalette = 0;
	engine->unfreezeTime();
	return 0;
}

/*0x55*/
static int32 lFADE_RED_PAL(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->freezeTime();
	engine->_screens->fadeRedPal(engine->_screens->mainPaletteRGB);
	engine->_screens->useAlternatePalette = 0;
	engine->unfreezeTime();
	return 0;
}

/*0x56*/
static int32 lFADE_RED_ALARM(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->freezeTime();
	engine->_hqrdepack->hqrGetEntry(engine->_screens->palette, Resources::HQR_RESS_FILE, RESSHQR_ALARMREDPAL);
	engine->_screens->copyPal(engine->_screens->palette, engine->_screens->paletteRGB);
	engine->_screens->fadeRedPal(engine->_screens->paletteRGB);
	engine->_screens->useAlternatePalette = 1;
	engine->unfreezeTime();
	return 0;
}

/*0x57*/
static int32 lFADE_PAL_ALARM(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->freezeTime();
	engine->_hqrdepack->hqrGetEntry(engine->_screens->palette, Resources::HQR_RESS_FILE, RESSHQR_ALARMREDPAL);
	engine->_screens->copyPal(engine->_screens->palette, engine->_screens->paletteRGB);
	engine->_screens->adjustCrossPalette(engine->_screens->mainPaletteRGB, engine->_screens->paletteRGB);
	engine->_screens->useAlternatePalette = 1;
	engine->unfreezeTime();
	return 0;
}

/*0x58*/
static int32 lEXPLODE_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);
	ActorStruct *otherActor = &engine->_scene->sceneActors[otherActorIdx];

	engine->_extra->addExtraExplode(otherActor->x, otherActor->y, otherActor->z); // RECHECK this

	return 0;
}

/*0x59*/
static int32 lBUBBLE_ON(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_text->showDialogueBubble = 1;
	return 0;
}

/*0x5A*/
static int32 lBUBBLE_OFF(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_text->showDialogueBubble = 1;
	return 0;
}

/*0x5B*/
static int32 lASK_CHOICE_OBJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 otherActorIdx = *(scriptPtr++);
	int32 choiceIdx = *((int16 *)scriptPtr);
	scriptPtr += 2;

	engine->freezeTime();
	if (engine->_text->showDialogueBubble) {
		engine->_redraw->drawBubble(otherActorIdx);
	}
	engine->_text->setFontCrossColor(engine->_scene->sceneActors[otherActorIdx].talkColor);
	engine->_gameState->processGameChoices(choiceIdx);
	engine->_gameState->numChoices = 0;
	engine->unfreezeTime();
	engine->_redraw->redrawEngineActions(1);

	return 0;
}

/*0x5C*/
static int32 lSET_DARK_PAL(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->freezeTime();
	engine->_hqrdepack->hqrGetEntry(engine->_screens->palette, Resources::HQR_RESS_FILE, RESSHQR_DARKPAL);
	if (!engine->_screens->lockPalette) {
		engine->_screens->copyPal(engine->_screens->palette, engine->_screens->paletteRGB);
		engine->setPalette(engine->_screens->paletteRGB);
	}
	engine->_screens->useAlternatePalette = 1;
	engine->unfreezeTime();
	return 0;
}

/*0x5D*/
static int32 lSET_NORMAL_PAL(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_screens->useAlternatePalette = 0;
	if (!engine->_screens->lockPalette) {
		engine->setPalette(engine->_screens->mainPaletteRGB);
	}
	return 0;
}

/*0x5E*/
static int32 lMESSAGE_SENDELL(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 tmpFlagDisplayText;

	engine->freezeTime();
	engine->_screens->fadeToBlack(engine->_screens->paletteRGB);
	engine->_screens->loadImage(25);
	engine->_text->textClipFull();
	engine->_text->setFontCrossColor(15);
	engine->_text->newGameVar4 = 0;
	tmpFlagDisplayText = engine->cfgfile.FlagDisplayText;
	engine->cfgfile.FlagDisplayText = 1;
	engine->_text->drawTextFullscreen(6);
	engine->_text->newGameVar4 = 1;
	engine->_text->textClipSmall();
	engine->_screens->fadeToBlack(engine->_screens->paletteRGBCustom);
	engine->_screens->clearScreen();
	engine->setPalette(engine->_screens->paletteRGB);
	engine->cfgfile.FlagDisplayText = tmpFlagDisplayText;

	do {
		engine->readKeys();
	} while (engine->_keyboard.skipIntro || engine->_keyboard.skippedKey);

	engine->unfreezeTime();

	return 0;
}

/*0x5F*/
static int32 lANIM_SET(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	AnimationTypes animIdx = (AnimationTypes)*(scriptPtr++);

	actor->anim = kAnimNone;
	actor->previousAnimIdx = -1;
	engine->_animations->initAnim(animIdx, 0, 0, actorIdx);

	return 0;
}

/*0x60*/
static int32 lHOLOMAP_TRAJ(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	scriptPtr++; // TODO
	return -1;
}

/*0x61*/
static int32 lGAME_OVER(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_scene->sceneHero->dynamicFlags.bAnimEnded = 1;
	engine->_scene->sceneHero->life = 0;
	engine->_gameState->inventoryNumLeafs = 0;
	return 1; // break
}

/*0x62*/
static int32 lTHE_END(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->quitGame = 1;
	engine->_gameState->inventoryNumLeafs = 0;
	engine->_scene->sceneHero->life = 50;
	engine->_gameState->inventoryMagicPoints = 80;
	engine->_scene->currentSceneIdx = 113;
	engine->_actor->heroBehaviour = engine->_actor->previousHeroBehaviour;
	engine->_scene->newHeroX = -1;
	engine->_scene->sceneHero->angle = engine->_actor->previousHeroAngle;
	engine->_gameState->saveGame();
	return 1; // break;
}

/*0x63*/
static int32 lMIDI_OFF(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_music->stopMidiMusic();
	return 0;
}

/*0x64*/
static int32 lPLAY_CD_TRACK(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 track = *(scriptPtr++);
	engine->_music->playTrackMusic(track);
	return 0;
}

/*0x65*/
static int32 lPROJ_ISO(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_renderer->setOrthoProjection(311, 240, 512);
	engine->_renderer->setBaseTranslation(0, 0, 0);
	engine->_renderer->setBaseRotation(0, 0, 0);
	engine->_renderer->setLightVector(engine->_scene->alphaLight, engine->_scene->betaLight, 0);
	return 0;
}

/*0x66*/
static int32 lPROJ_3D(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	engine->_screens->copyScreen(engine->frontVideoBuffer, engine->workVideoBuffer);
	engine->flip();
	engine->_scene->changeRoomVar10 = 0;

	engine->_renderer->setCameraPosition(320, 240, 128, 1024, 1024);
	engine->_renderer->setCameraAngle(0, 1500, 0, 25, -128, 0, 13000);
	engine->_renderer->setLightVector(896, 950, 0);

	engine->_text->initTextBank(1);

	return 0;
}

/*0x67*/
static int32 lTEXT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	int32 textSize, textBoxRight;
	int32 textIdx = *((int16 *)scriptPtr);
	scriptPtr += 2;

	if (drawVar1 < 440) {
		if (engine->cfgfile.Version == USA_VERSION) {
			if (!textIdx) {
				textIdx = 16;
			}
		}

		engine->_text->getMenuText(textIdx, textStr, sizeof(textStr));
		textSize = textBoxRight = engine->_text->getTextSize(textStr);
		engine->_text->setFontColor(15);
		engine->_text->drawText(0, drawVar1, textStr);
		if (textSize > 639) {
			textBoxRight = 639;
		}

		drawVar1 += 40;
		engine->copyBlockPhys(0, drawVar1, textBoxRight, drawVar1);
	}

	return 0;
}

/*0x68*/
static int32 lCLEAR_TEXT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
	drawVar1 = 0;
	engine->_interface->drawSplittedBox(0, 0, 639, 240, 0);
	engine->copyBlockPhys(0, 0, 639, 240);
	return 0;
}

/*0x69*/
static int32 lBRUTAL_EXIT(TwinEEngine *engine, int32 actorIdx, ActorStruct *actor) {
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
	scriptPtr = nullptr;
	opcodePtr = nullptr;
	drawVar1 = 0;
	textStr[0] = '\0';
}

void ScriptLife::processLifeScript(int32 actorIdx) {
	int32 end, scriptOpcode;
	ActorStruct *actor;

	actor = &_engine->_scene->sceneActors[actorIdx];
	scriptPtr = actor->lifeScript + actor->positionInLifeScript;

	end = -2;

	do {
		opcodePtr = scriptPtr;
		scriptOpcode = *(scriptPtr++);

		if (scriptOpcode >= 0 && scriptOpcode < ARRAYSIZE(function_map)) {
			end = function_map[scriptOpcode].function(_engine, actorIdx, actor);
		} else {
			error("Actor %d with wrong offset/opcode - Offset: %d\n", actorIdx, actor->positionInLifeScript);
		}

		if (end < 0) { // show error message
			warning("Actor %d Life script [%s] not implemented\n", actorIdx, function_map[scriptOpcode].name);
		}

	} while (end != 1);
}

} // namespace TwinE
