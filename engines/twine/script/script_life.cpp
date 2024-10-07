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

#include "twine/script/script_life.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "twine/debugger/debug_state.h"
#include "twine/scene/actor.h"
#include "twine/scene/animations.h"
#include "twine/audio/music.h"
#include "twine/audio/sound.h"
#include "twine/scene/collision.h"
#include "twine/movies.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/grid.h"
#include "twine/holomap.h"
#include "twine/input.h"
#include "twine/menu/interface.h"
#include "twine/menu/menu.h"
#include "twine/scene/movements.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/resources.h"
#include "twine/scene/extra.h"
#include "twine/scene/scene.h"
#include "twine/shared.h"
#include "twine/text.h"
#include "twine/twine.h"

// SCENE_SIZE_MAX
#define MAX_TARGET_ACTOR_DISTANCE 32000

namespace TwinE {

// the y position for lTEXT opcode - see lCLEAR (used in credits scene)
// TODO: move into scene class?
static int32 lTextYPos;

/** Script condition command opcodes */
enum LifeScriptConditions {
	// lba1 and lba2
	kcCOL = 0,              /*<! Current actor collision with another actor. (Parameter = Actor Index) */
	kcCOL_OBJ = 1,          /*<! Actor collision with the actor passed as parameter. (Parameter = Actor Index, Parameter = Actor Index) */
	kcDISTANCE = 2,         /*<! Distance between the current actor and the actor passed as parameter. (Parameter = Actor Index, Parameter = Distance between) */
	kcZONE = 3,             /*<! Current actor tread on zone passed as parameter. (Parameter = Zone Index) */
	kcZONE_OBJ = 4,         /*<! The actor passed as parameter will tread on zone passed as parameter. (Parameter = Actor Index, Parameter = Zone Index) */
	kcBODY = 5,             /*<! Body of the current actor. (Parameter = Body Index) */
	kcBODY_OBJ = 6,         /*<! Body of the actor passed as parameter. (Parameter = Body Index) */
	kcANIM = 7,             /*<! Body Animation of the current actor. (Parameter = Animation Index) */
	kcANIM_OBJ = 8,         /*<! Body Animation of the actor passed as parameter. (Parameter = Animation Index) */
	kcL_TRACK = 9,          /*<! Current actor track. (Parameter = Track Index) */
	kcL_TRACK_OBJ = 10,     /*<! Track of the actor passed as parameter. (Parameter = Track Index) */
	kcFLAG_CUBE = 11,       /*<! Game Cube Flags. (Parameter = Cube Flag Index, Parameter = 0 (not set), = 1 (set))k */
	kcCONE_VIEW = 12,       /*<! The actor passed as parameter have a "vision in circle". (Parameter = Actor Index, Parameter = Distance) */
	kcHIT_BY = 13,          /*<! Current actor hited by the actor passed as parameter. (Parameter = Actor Index) */
	kcACTION = 14,          /*<! Execute action (boolean value, e.g. when hiding in the waste of the 2nd scene to escape the prison) */
	kcFLAG_GAME = 15,       /*<! Game Flags (See further list). (Parameter = Flag Index, Parameter = 0 (not set), = 1 (set)) */
	kcLIFE_POINT = 16,      /*<! Current actor life points. (Parameter = Life points) */
	kcLIFE_POINT_OBJ = 17,  /*<! Life points of the current actor passed as parameter. (Parameter = Life points) */
	kcNUM_LITTLE_KEYS = 18, /*<! Number of keys. (Parameter = Number of keys) */
	kcNUM_GOLD_PIECES = 19, /*<! Coins/Gold Amount. (Parameter = Coins/Gold amount) */
	kcBEHAVIOUR = 20,       /*<! Hero behaviour. (Parameter = Behaviour Index) */
	kcCHAPTER = 21,         /*<! Story Chapters. (Parameter = Chapter Index) */
	kcDISTANCE_3D = 22,     /*<! Distance between the actor passed as parameter and the current actor. (Parameter = Actor Index, Parameter = Distance) */
	kcMAGIC_LEVEL = 23,
	kcMAGIC_POINTS = 24,
	kcUSE_INVENTORY = 25, /*<! Use inventory object. (Parameter = Object Index in the inventory, Paramenter = 0 (Not in Inventory), = 1 (In the Inventory)) */
	kcCHOICE = 26,        /*<! Menu choice. (Parameter = Text Index in the current Text Bank) */
	kcFUEL = 27,          /*<! Amount of fuel gas the Hero have in his inventory. (Parameter = Gas amount) */
	kcCARRIED_BY = 28,    /*<! The current is carried by the actor passed as paramenter. (Parameter = Actor Index) */
	kcCDROM = 29,         /*<! CDROM audio tracks. (Parameter = Audio Tracks Index) */
	// lba2 only
	kcLADDER = 30,
	kcRND = 31,
	kcRAIL = 32,
	kcBETA = 33,
	kcBETA_OBJ = 34,
	kcCARRY_OBJ_BY = 35,
	kcANGLE = 36,            /*<! meansure the angle between two actors */
	kcDISTANCE_MESSAGE = 37,
	kcHIT_OBJ_BY = 38,
	kcREAL_ANGLE = 39,       /*<! meansure the angle between two actors */
	kcDEMO = 40,
	kcCOL_DECORS = 41,
	kcCOL_DECORS_OBJ = 42,
	kcPROCESSOR = 43,
	kcOBJECT_DISPLAYED = 44,
	kcANGLE_OBJ = 45         /*<! meansure the angle between two actors */
};

enum class ReturnType {
	RET_S8 = 0,
	RET_S16 = 1,
	RET_STRING = 2,
	RET_U8 = 4
};

/**
 * Returns @c 1 Condition value size (1 byte), @c 2 Condition value size (2 bytes)
 */
static ReturnType processLifeConditions(TwinEEngine *engine, LifeScriptContext &ctx) { // DoFuncLife
	ReturnType conditionValueSize = engine->isLBA1() ? ReturnType::RET_U8 : ReturnType::RET_S8;
	int32 conditionOpcode = ctx.stream.readByte();
	switch (conditionOpcode) {
	case kcCOL:
		if (ctx.actor->_lifePoint <= 0) {
			engine->_scene->_currentScriptValue = -1;
		} else {
			engine->_scene->_currentScriptValue = ctx.actor->_objCol;
		}
		debugCN(3, kDebugLevels::kDebugScripts, "collision(");
		break;
	case kcCOL_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		if (engine->_scene->getActor(actorIdx)->_lifePoint <= 0) {
			engine->_scene->_currentScriptValue = -1;
		} else {
			engine->_scene->_currentScriptValue = engine->_scene->getActor(actorIdx)->_objCol;
		}
		debugCN(3, kDebugLevels::kDebugScripts, "col_obj(%i, ", actorIdx);
		break;
	}
	case kcDISTANCE: {
		int32 actorIdx = ctx.stream.readByte();
		debugCN(3, kDebugLevels::kDebugScripts, "distance(%i, ", actorIdx);
		conditionValueSize = ReturnType::RET_S16;
		ActorStruct *otherActor = engine->_scene->getActor(actorIdx);
		if (!otherActor->_workFlags.bIsDead) {
			if (ABS(ctx.actor->_posObj.y - otherActor->_posObj.y) >= 1500) {
				engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
			} else {
				// Returns int32, so we check for integer overflow
				int32 distance = getDistance2D(ctx.actor->posObj(), otherActor->posObj());
				if (ABS(distance) > MAX_TARGET_ACTOR_DISTANCE) {
					engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
				} else {
					engine->_scene->_currentScriptValue = distance;
				}
			}
		} else {
			engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
		}
		break;
	}
	case kcZONE:
		debugCN(3, kDebugLevels::kDebugScripts, "zone(");
		engine->_scene->_currentScriptValue = ctx.actor->_zoneSce;
		break;
	case kcZONE_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		debugCN(3, kDebugLevels::kDebugScripts, "zone_obj(%i, ", actorIdx);
		engine->_scene->_currentScriptValue = engine->_scene->getActor(actorIdx)->_zoneSce;
		break;
	}
	case kcBODY:
		debugCN(3, kDebugLevels::kDebugScripts, "body(");
		engine->_scene->_currentScriptValue = (int16)ctx.actor->_genBody;
		break;
	case kcBODY_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		debugCN(3, kDebugLevels::kDebugScripts, "body_obj(%i, ", actorIdx);
		engine->_scene->_currentScriptValue = (int16)engine->_scene->getActor(actorIdx)->_genBody;
		break;
	}
	case kcANIM:
		debugCN(3, kDebugLevels::kDebugScripts, "anim(");
		engine->_scene->_currentScriptValue = (int16)ctx.actor->_genAnim;
		break;
	case kcANIM_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		debugCN(3, kDebugLevels::kDebugScripts, "anim_obj(%i, ", actorIdx);
		engine->_scene->_currentScriptValue = (int16)engine->_scene->getActor(actorIdx)->_genAnim;
		break;
	}
	case kcL_TRACK:
		debugCN(3, kDebugLevels::kDebugScripts, "track(");
		conditionValueSize = ReturnType::RET_U8;
		engine->_scene->_currentScriptValue = ctx.actor->_labelTrack;
		break;
	case kcL_TRACK_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		debugCN(3, kDebugLevels::kDebugScripts, "track_obj(%i, ", actorIdx);
		engine->_scene->_currentScriptValue = engine->_scene->getActor(actorIdx)->_labelTrack;
		break;
	}
	case kcFLAG_CUBE: {
		int32 flagIdx = ctx.stream.readByte();
		debugCN(3, kDebugLevels::kDebugScripts, "flag_cube(%i, ", flagIdx);
		conditionValueSize = ReturnType::RET_U8;
		engine->_scene->_currentScriptValue = engine->_scene->_listFlagCube[flagIdx];
		break;
	}
	case kcCONE_VIEW: {
		int32 newAngle = 0;
		int32 targetActorIdx = ctx.stream.readByte();
		debugCN(3, kDebugLevels::kDebugScripts, "cone_view(%i, ", targetActorIdx);
		ActorStruct *targetActor = engine->_scene->getActor(targetActorIdx);

		conditionValueSize = ReturnType::RET_S16;

		if (targetActor->_workFlags.bIsDead) {
			engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
			break;
		}

		if (ABS(targetActor->_posObj.y - ctx.actor->_posObj.y) < 1500) {
			newAngle = engine->_movements->getAngle(ctx.actor->posObj(), targetActor->posObj());
			if (ABS(engine->_movements->_targetActorDistance) > MAX_TARGET_ACTOR_DISTANCE) {
				engine->_movements->_targetActorDistance = MAX_TARGET_ACTOR_DISTANCE;
			}
		} else {
			engine->_movements->_targetActorDistance = MAX_TARGET_ACTOR_DISTANCE;
		}

		if (IS_HERO(targetActorIdx)) {
			if (engine->_actor->_heroBehaviour == HeroBehaviourType::kDiscrete) {
				int32 heroAngle = ClampAngle(ctx.actor->_beta + LBAAngles::ANGLE_360 + LBAAngles::ANGLE_45 - newAngle + LBAAngles::ANGLE_360);

				if (ABS(heroAngle) <= LBAAngles::ANGLE_90) {
					engine->_scene->_currentScriptValue = engine->_movements->_targetActorDistance;
				} else {
					engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
				}
			} else {
				engine->_scene->_currentScriptValue = engine->_movements->_targetActorDistance;
			}
		} else {
			int32 heroAngle = ClampAngle(ctx.actor->_beta + LBAAngles::ANGLE_360 + LBAAngles::ANGLE_45 - newAngle + LBAAngles::ANGLE_360);

			if (ABS(heroAngle) <= LBAAngles::ANGLE_90) {
				engine->_scene->_currentScriptValue = engine->_movements->_targetActorDistance;
			} else {
				engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
			}
		}
		break;
	}
	case kcHIT_BY:
		debugCN(3, kDebugLevels::kDebugScripts, "hit_by(");
		engine->_scene->_currentScriptValue = ctx.actor->_hitBy;
		break;
	case kcHIT_OBJ_BY: {
		int32 actorIdx = ctx.stream.readByte();
		engine->_scene->_currentScriptValue = engine->_scene->getActor(actorIdx)->_hitBy;
		debugCN(3, kDebugLevels::kDebugScripts, "hit_by(%i, ", actorIdx);
		break;
	}
	case kcACTION:
		debugCN(3, kDebugLevels::kDebugScripts, "action(");
		engine->_scene->_currentScriptValue = engine->_movements->shouldExecuteAction() ? 1 : 0;
		break;
	case kcFLAG_GAME: {
		int32 flagIdx = ctx.stream.readByte();
		debugCN(3, kDebugLevels::kDebugScripts, "flag_game(%i, ", flagIdx);
		if (!engine->_gameState->inventoryDisabled() ||
		    (engine->_gameState->inventoryDisabled() && flagIdx >= MaxInventoryItems)) {
			engine->_scene->_currentScriptValue = engine->_gameState->hasGameFlag(flagIdx);
		} else {
			if (flagIdx == GAMEFLAG_INVENTORY_DISABLED) {
				// TODO: this case should already get handled in the above if branch as the flagIdx is bigger than MaxInventoryItems
				engine->_scene->_currentScriptValue = engine->_gameState->inventoryDisabled();
			} else {
				engine->_scene->_currentScriptValue = 0;
			}
		}
		break;
	}
	case kcLIFE_POINT:
		debugCN(3, kDebugLevels::kDebugScripts, "life_point(");
		if (engine->isLBA2()) {
			conditionValueSize = ReturnType::RET_S16;
		}
		engine->_scene->_currentScriptValue = ctx.actor->_lifePoint;
		break;
	case kcLIFE_POINT_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		debugCN(3, kDebugLevels::kDebugScripts, "life_point_obj(%i, ", actorIdx);
		engine->_scene->_currentScriptValue = engine->_scene->getActor(actorIdx)->_lifePoint;
		break;
	}
	case kcNUM_LITTLE_KEYS:
		debugCN(3, kDebugLevels::kDebugScripts, "num_little_keys(");
		engine->_scene->_currentScriptValue = engine->_gameState->_inventoryNumKeys;
		break;
	case kcNUM_GOLD_PIECES:
		debugCN(3, kDebugLevels::kDebugScripts, "num_gold_pieces(");
		conditionValueSize = ReturnType::RET_S16;
		if (engine->_scene->_planet > 2) {
			engine->_scene->_currentScriptValue = engine->_gameState->_zlitosPieces;
		} else {
			engine->_scene->_currentScriptValue = engine->_gameState->_goldPieces;
		}
		break;
	case kcBEHAVIOUR:
		debugCN(3, kDebugLevels::kDebugScripts, "behaviour(");
		engine->_scene->_currentScriptValue = (int16)engine->_actor->_heroBehaviour;
		break;
	case kcCHAPTER:
		debugCN(3, kDebugLevels::kDebugScripts, "chapter(");
		engine->_scene->_currentScriptValue = engine->_gameState->getChapter();
		break;
	case kcDISTANCE_3D: {
		int32 targetActorIdx;
		ActorStruct *targetActor;

		targetActorIdx = ctx.stream.readByte();
		debugCN(3, kDebugLevels::kDebugScripts, "distance_3d(%i, ", targetActorIdx);
		targetActor = engine->_scene->getActor(targetActorIdx);

		conditionValueSize = ReturnType::RET_S16;

		if (!targetActor->_workFlags.bIsDead) {
			// Returns int32, so we check for integer overflow
			int32 distance = getDistance3D(ctx.actor->posObj(), targetActor->posObj());
			if (ABS(distance) > MAX_TARGET_ACTOR_DISTANCE) {
				engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
			} else {
				engine->_scene->_currentScriptValue = distance;
			}
		} else {
			engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
		}
		break;
	}
	case kcMAGIC_LEVEL:
		debugCN(3, kDebugLevels::kDebugScripts, "magic_level(");
		engine->_scene->_currentScriptValue = engine->_gameState->_magicLevelIdx;
		break;
	case kcMAGIC_POINTS:
		debugCN(3, kDebugLevels::kDebugScripts, "magic_points(");
		engine->_scene->_currentScriptValue = engine->_gameState->_magicPoint;
		break;
	case kcUSE_INVENTORY: {
		int32 item = ctx.stream.readByte();
		debugCN(3, kDebugLevels::kDebugScripts, "use_inventory(%i, ", item);

		if (engine->_gameState->inventoryDisabled()) {
			engine->_scene->_currentScriptValue = 0;
		} else {
			if (item == engine->_loopInventoryItem) {
				engine->_scene->_currentScriptValue = 1;
			} else {
				if (engine->_gameState->_inventoryFlags[item] == 1 && engine->_gameState->hasItem((InventoryItems)item)) {
					engine->_scene->_currentScriptValue = 1;
				} else {
					engine->_scene->_currentScriptValue = 0;
				}
			}

			if (engine->_scene->_currentScriptValue == 1) {
				engine->_redraw->addOverlay(OverlayType::koInventoryItem, item, 0, 0, 0, OverlayPosType::koNormal, 3);
			}
		}
		break;
	}
	case kcCHOICE:
		debugCN(3, kDebugLevels::kDebugScripts, "choice(");
		conditionValueSize = ReturnType::RET_S16;
		engine->_scene->_currentScriptValue = (int16)engine->_gameState->_gameChoice;
		break;
	case kcFUEL:
		debugCN(3, kDebugLevels::kDebugScripts, "fuel(");
		if (engine->isLBA2()) {
			conditionValueSize = ReturnType::RET_S16;
		}
		engine->_scene->_currentScriptValue = engine->_gameState->_inventoryNumGas;
		break;
	case kcCARRIED_BY:
		debugCN(3, kDebugLevels::kDebugScripts, "carried_by(");
		engine->_scene->_currentScriptValue = ctx.actor->_carryBy;
		break;
	case kcCDROM:
		// used in lba1 scenes 80 and 117
		debugCN(3, kDebugLevels::kDebugScripts, "cdrom(");
		engine->_scene->_currentScriptValue = engine->isCDROM();
		break;
	case kcCARRY_OBJ_BY: {
		int32 actorIdx = ctx.stream.readByte();
		engine->_scene->_currentScriptValue = engine->_scene->getActor(actorIdx)->_carryBy;
		debugCN(3, kDebugLevels::kDebugScripts, "carried_by(%i, ", actorIdx);
		break;
	}
	case kcRND: {
		int32 val = ctx.stream.readByte();
		engine->_scene->_currentScriptValue = engine->getRandomNumber(val);
		conditionValueSize = ReturnType::RET_U8;
		break;
	}
	case kcBETA:
		engine->_scene->_currentScriptValue = ctx.actor->_beta;
		conditionValueSize = ReturnType::RET_S16;
		break;
	case kcBETA_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		engine->_scene->_currentScriptValue = engine->_scene->getActor(actorIdx)->_beta;
		conditionValueSize = ReturnType::RET_S16;
		break;
	}
	case kcDEMO:
		engine->_scene->_currentScriptValue = engine->isDemo() ? 1 : 0; // TODO: slide demo is 2
		break;
	case kcOBJECT_DISPLAYED: {
		int32 actorIdx = ctx.stream.readByte();
		engine->_scene->_currentScriptValue = engine->_scene->getActor(actorIdx)->_workFlags.bWasDrawn ? 1 : 0;
		break;
	}
	case kcPROCESSOR:
		// TODO psx = 2, pentium = 0, 486 = 1
		engine->_scene->_currentScriptValue = 0;
		break;
	case kcANGLE: {
		conditionValueSize = ReturnType::RET_S16;
		int32 actorIdx = ctx.stream.readByte();
		ActorStruct *otherActor = engine->_scene->getActor(actorIdx);
		if (otherActor->_workFlags.bIsDead) {
			engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
			break;
		}
		int32 angle = engine->_movements->getAngle(ctx.actor->posObj(), otherActor->posObj());
		engine->_scene->_currentScriptValue = ClampAngle(ctx.actor->_beta - angle);
		if (engine->_scene->_currentScriptValue > LBAAngles::ANGLE_180) {
			engine->_scene->_currentScriptValue = LBAAngles::ANGLE_360 - engine->_scene->_currentScriptValue;
		}
		break;
	}
	case kcANGLE_OBJ: {
		conditionValueSize = ReturnType::RET_S16;
		int32 actorIdx = ctx.stream.readByte();
		ActorStruct *otherActor = engine->_scene->getActor(actorIdx);
		if (otherActor->_workFlags.bIsDead) {
			engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
			break;
		}
		int32 angle = engine->_movements->getAngle(otherActor->posObj(), ctx.actor->posObj());
		engine->_scene->_currentScriptValue = ClampAngle(otherActor->_beta - angle);
		if (engine->_scene->_currentScriptValue > LBAAngles::ANGLE_180) {
			engine->_scene->_currentScriptValue = LBAAngles::ANGLE_360 - engine->_scene->_currentScriptValue;
		}
		break;
	}
	case kcREAL_ANGLE: {
		conditionValueSize = ReturnType::RET_S16;
		int32 actorIdx = ctx.stream.readByte();
		ActorStruct *otherActor = engine->_scene->getActor(actorIdx);
		if (otherActor->_workFlags.bIsDead) {
			engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
			break;
		}
		int32 angle = engine->_movements->getAngle(ctx.actor->posObj(), otherActor->posObj());
		engine->_scene->_currentScriptValue = ClampAngle(ctx.actor->_beta - angle);
		if (engine->_scene->_currentScriptValue > LBAAngles::ANGLE_180) {
			engine->_scene->_currentScriptValue = LBAAngles::ANGLE_360 - engine->_scene->_currentScriptValue;
		} else {
			engine->_scene->_currentScriptValue = -engine->_scene->_currentScriptValue;
		}
		break;
	}
	case kcCOL_DECORS:
		if (ctx.actor->_workFlags.bIsDead) {
			engine->_scene->_currentScriptValue = 255;
			break;
		}
		engine->_scene->_currentScriptValue = (uint8)ctx.actor->brickShape();
		conditionValueSize = ReturnType::RET_U8;
		break;
	case kcCOL_DECORS_OBJ: {
		int32 actorIdx = ctx.stream.readByte();
		ActorStruct *otherActor = engine->_scene->getActor(actorIdx);
		if (otherActor->_workFlags.bIsDead) {
			engine->_scene->_currentScriptValue = 255;
			break;
		}
		engine->_scene->_currentScriptValue = (uint8)otherActor->brickShape();
		conditionValueSize = ReturnType::RET_U8;
		break;
	}
	case kcLADDER: {
		int32 num = ctx.stream.readByte();
		int n = 0;
		engine->_scene->_currentScriptValue = 2;
		while (engine->_scene->_currentScriptValue == 2 && n < engine->_scene->_sceneNumZones) {
			const ZoneStruct &zone = engine->_scene->_sceneZones[n];
			if (zone.type == ZoneType::kLadder && zone.num == num) {
				engine->_scene->_currentScriptValue = zone.infoData.generic.info1;
			}
			++n;
		}
		break;
	}
	case kcRAIL: {
		int32 num = ctx.stream.readByte();
		int n = 0;
		engine->_scene->_currentScriptValue = 2;
		while (engine->_scene->_currentScriptValue == 2 && n < engine->_scene->_sceneNumZones) {
			const ZoneStruct &zone = engine->_scene->_sceneZones[n];
			if (zone.type == ZoneType::kRail && zone.num == num) {
				engine->_scene->_currentScriptValue = zone.infoData.generic.info1;
			}
			n++;
		}
		break;
	}
	case kcDISTANCE_MESSAGE: {
		int32 actorIdx = ctx.stream.readByte();
		conditionValueSize = ReturnType::RET_S16;
		ActorStruct *otherActor = engine->_scene->getActor(actorIdx);

		if (otherActor->_workFlags.bIsDead) {
			engine->_scene->_currentScriptValue = 32000;
			break;
		}

		if (ABS(otherActor->posObj().y - ctx.actor->posObj().y) < 1500) {
			int32 angle = engine->_movements->getAngle(ctx.actor->posObj(),
																				otherActor->posObj());
			angle = ClampAngle(ctx.actor->_beta - angle + LBAAngles::ANGLE_90);

			// 320: CONE_VIEW
			if (angle <= LBAAngles::ANGLE_157_5)  {
				int32 distance = getDistance2D(ctx.actor->posObj(),
											   otherActor->posObj());

				if (distance > MAX_TARGET_ACTOR_DISTANCE) {
					engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
				} else {
					engine->_scene->_currentScriptValue = distance;
				}
			} else {
				engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
			}
		} else {
			engine->_scene->_currentScriptValue = MAX_TARGET_ACTOR_DISTANCE;
		}
		break;
	}
	default:
		error("Actor condition opcode %d", conditionOpcode);
		break;
	}

	return conditionValueSize;
}

/**
 * Returns @c -1 Need implementation, @c 0 Condition false, @c 1 Condition true
 */
static int32 processLifeOperators(TwinEEngine *engine, LifeScriptContext &ctx, ReturnType valueType) { // DoTest
	const int32 operatorCode = ctx.stream.readByte();

	int32 conditionValue;
	if (valueType == ReturnType::RET_S8) {
		conditionValue = ctx.stream.readSByte();
	} else if (valueType == ReturnType::RET_S16) {
		conditionValue = ctx.stream.readSint16LE();
	} else if (valueType == ReturnType::RET_U8) {
		conditionValue = ctx.stream.readByte();
	} else if (valueType == ReturnType::RET_STRING) {
#if 0
		const Common::String &str = ctx.stream.readString();
		// TODO: this String is the inventory item description or the behaviour text - translated
		// not sure why this was useful... or whether it was ever used
		const int valueword = scummvm_stricmp(String, str.c_str());
		switch (operatorCode) {
		default:
			return 0;
		case kEqualTo:
			return (valueword == 0);
		case kNotEqualTo:
			return (valueword != 0);
		}
#else
		error("String return type is not yet supported");
#endif
	} else {
		error("Unknown operator value size %d", (int)valueType);
	}

	switch (operatorCode) {
	case kEqualTo:
		debugCN(3, kDebugLevels::kDebugScripts, "%i == %i)", engine->_scene->_currentScriptValue, conditionValue);
		if (engine->_scene->_currentScriptValue == conditionValue) {
			return 1;
		}
		break;
	case kGreaterThan:
		debugCN(3, kDebugLevels::kDebugScripts, "%i > %i)", engine->_scene->_currentScriptValue, conditionValue);
		if (engine->_scene->_currentScriptValue > conditionValue) {
			return 1;
		}
		break;
	case kLessThan:
		debugCN(3, kDebugLevels::kDebugScripts, "%i < %i)", engine->_scene->_currentScriptValue, conditionValue);
		if (engine->_scene->_currentScriptValue < conditionValue) {
			return 1;
		}
		break;
	case kGreaterThanOrEqualTo:
		debugCN(3, kDebugLevels::kDebugScripts, "%i >= %i)", engine->_scene->_currentScriptValue, conditionValue);
		if (engine->_scene->_currentScriptValue >= conditionValue) {
			return 1;
		}
		break;
	case kLessThanOrEqualTo:
		debugCN(3, kDebugLevels::kDebugScripts, "%i <= %i)", engine->_scene->_currentScriptValue, conditionValue);
		if (engine->_scene->_currentScriptValue <= conditionValue) {
			return 1;
		}
		break;
	case kNotEqualTo:
		debugCN(3, kDebugLevels::kDebugScripts, "%i != %i)", engine->_scene->_currentScriptValue, conditionValue);
		if (engine->_scene->_currentScriptValue != conditionValue) {
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

/**
 * For unused opcodes
 */
int32 ScriptLife::lEMPTY(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::EMPTY()");
	return 0;
}

/**
 * End of Actor Life Script
 * @note Opcode @c 0x00
 */
int32 ScriptLife::lEND(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::END()");
	ctx.actor->_offsetLife = -1;
	return 1; // break script
}

/**
 * No Operation
 * @note Opcode @c 0x01
 */
int32 ScriptLife::lNOP(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::NOP()");
	ctx.stream.skip(1);
	return 0;
}

/**
 * To execute a switch no if. It's used to toggle the switch.
 * @note Opcode @c 0x02
 */
int32 ScriptLife::lSNIF(TwinEEngine *engine, LifeScriptContext &ctx) {
	const ReturnType valueType = processLifeConditions(engine, ctx);
	debugCN(3, kDebugLevels::kDebugScripts, "LIFE::IF(");
	if (!processLifeOperators(engine, ctx, valueType)) {
		ctx.setOpcode(0x0D); // SWIF
	}
	const int16 offset = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, ", %i)", offset);
	ctx.stream.seek(offset); // condition offset
	return 0;
}

/**
 * To jump to another offset in the current script. (Parameter = Offset)
 * @note Opcode @c 0x03
 */
int32 ScriptLife::lOFFSET(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 offset = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::LABEL(%i)", (int)offset);
	ctx.stream.seek(offset); // offset
	return 0;
}

/**
 * Will never execute that condition.
 * @note Opcode @c 0x04
 */
int32 ScriptLife::lNEVERIF(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugCN(3, kDebugLevels::kDebugScripts, "LIFE::IF(");
	const ReturnType valueType = processLifeConditions(engine, ctx);
	processLifeOperators(engine, ctx, valueType);
	const int16 offset = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, ", %i)", offset);
	ctx.stream.seek(offset); // condition offset
	return 0;
}

/**
 * Will not execute the condition.
 * @note Opcode @c 0x06
 */
int32 ScriptLife::lNO_IF(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::NO_IF()");
	return 0;
}

/**
 * Specify a new label
 * @note Opcode @c 0x0A
 */
int32 ScriptLife::lLABEL(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::LABEL(x)");
	ctx.stream.skip(1); // label id - script offset
	return 0;
}

/**
 * To stop running the current script
 * @note Opcode @c 0x0B
 */
int32 ScriptLife::lRETURN(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::RETURN()");
	return 1; // break script
}

/**
 * Do a certain statement according the condition.
 * @note Opcode @c 0x0C
 */
int32 ScriptLife::lIF(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugCN(3, kDebugLevels::kDebugScripts, "LIFE::IF(");
	const ReturnType valueType = processLifeConditions(engine, ctx);
	if (!processLifeOperators(engine, ctx, valueType)) {
		const int16 offset = ctx.stream.readSint16LE();
		debugC(3, kDebugLevels::kDebugScripts, ", %i)", offset);
		ctx.stream.seek(offset); // condition offset
	} else {
		ctx.stream.skip(2);
		debugC(3, kDebugLevels::kDebugScripts, ")");
	}

	return 0;
}

/**
 * To execute a switch if.
 * @note Opcode @c 0x0D
 */
int32 ScriptLife::lSWIF(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugCN(3, kDebugLevels::kDebugScripts, "LIFE::SWIF(");
	const ReturnType valueType = processLifeConditions(engine, ctx);
	if (!processLifeOperators(engine, ctx, valueType)) {
		const int16 offset = ctx.stream.readSint16LE();
		debugC(3, kDebugLevels::kDebugScripts, ", %i)", offset);
		ctx.stream.seek(offset); // condition offset
	} else {
		ctx.stream.skip(2);
		ctx.setOpcode(0x02); // SNIF
		debugC(3, kDebugLevels::kDebugScripts, ")");
	}

	return 0;
}

/**
 * Will only execute that condition one time.
 * @note Opcode @c 0x0E
 */
int32 ScriptLife::lONEIF(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugCN(3, kDebugLevels::kDebugScripts, "LIFE::ONEIF(");
	const ReturnType valueType = processLifeConditions(engine, ctx);
	if (!processLifeOperators(engine, ctx, valueType)) {
		const int16 offset = ctx.stream.readSint16LE();
		debugC(3, kDebugLevels::kDebugScripts, ", %i)", offset);
		ctx.stream.seek(offset); // condition offset
	} else {
		ctx.stream.skip(2);
		ctx.setOpcode(0x04); // NEVERIF
		debugC(3, kDebugLevels::kDebugScripts, ")");
	}

	return 0;
}

/**
 * Else statement for an IF condition.
 * @note Opcode @c 0x0F
 */
int32 ScriptLife::lELSE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 offset = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::ELSE(%i)", (int)offset);
	ctx.stream.seek(offset); // offset
	return 0;
}

/**
 * Choose new body for the current actor (Parameter = File3D Body Instance)
 * @note Opcode @c 0x11
 */
int32 ScriptLife::lBODY(TwinEEngine *engine, LifeScriptContext &ctx) {
	const BodyType bodyIdx = (BodyType)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::BODY(%i)", (int)bodyIdx);
	engine->_actor->initBody(bodyIdx, ctx.actorIdx);
	return 0;
}

/**
 * Choose new body for the actor passed as parameter (Parameter = Actor Index, Parameter = File3D Body Instance)
 * @note Opcode @c 0x12
 */
int32 ScriptLife::lBODY_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const BodyType otherBodyIdx = (BodyType)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::BODY_OBJ(%i, %i)", (int)otherActorIdx, (int)otherBodyIdx);
	engine->_actor->initBody(otherBodyIdx, otherActorIdx);
	return 0;
}

/**
 * Choose new animation for the current actor (Parameter = File3D Animation Instance)
 * @note Opcode @c 0x13
 */
int32 ScriptLife::lANIM(TwinEEngine *engine, LifeScriptContext &ctx) {
	const AnimationTypes animIdx = (AnimationTypes)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::ANIM(%i)", (int)animIdx);
	engine->_animations->initAnim(animIdx, AnimType::kAnimationTypeRepeat, AnimationTypes::kStanding, ctx.actorIdx);
	return 0;
}

/**
 * Choose new animation for the actor passed as parameter (Parameter = Actor Index, Parameter = File3D Animation Instance)
 * @note Opcode @c 0x14
 */
int32 ScriptLife::lANIM_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const AnimationTypes otherAnimIdx = (AnimationTypes)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::ANIM_OBJ(%i, %i)", (int)otherActorIdx, (int)otherAnimIdx);
	engine->_animations->initAnim(otherAnimIdx, AnimType::kAnimationTypeRepeat, AnimationTypes::kStanding, otherActorIdx);
	return 0;
}

/**
 * Same as SET_COMPORTAMENT
 * @note Opcode @c 0x15
 */
int32 ScriptLife::lSET_LIFE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 offset = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_LIFE(%i)", (int)offset);
	ctx.actor->_offsetLife = offset;
	return 0;
}

/**
 * Same as SET_COMPORTAMENT_OBJ
 * @note Opcode @c 0x16
 */
int32 ScriptLife::lSET_LIFE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const int16 offset = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_LIFE_OBJ(%i, %i)", (int)otherActorIdx, (int)offset);
	engine->_scene->getActor(otherActorIdx)->_offsetLife = offset;
	return 0;
}

/**
 * Set a new track for the current actor. (Parameter = Track offset)
 * @note Opcode @c 0x17
 */
int32 ScriptLife::lSET_TRACK(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 offset = ctx.stream.readSint16LE();
	ctx.actor->_offsetTrack = offset;
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_TRACK(%i)", (int)offset);
	return 0;
}

/**
 * Set a new track for tha actor passed as parameter (Parameter = Actor Index, Parameter = Track offset)
 * @note Opcode @c 0x18
 */
int32 ScriptLife::lSET_TRACK_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const int16 offset = ctx.stream.readSint16LE();
	engine->_scene->getActor(otherActorIdx)->_offsetTrack = offset;
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_TRACK_OBJ(%i, %i)", (int)otherActorIdx, (int)offset);
	return 0;
}

/**
 * Choose a message to display. (Parameter = Text Index in the current Text Bank)
 * @note Opcode @c 0x19
 */
int32 ScriptLife::lMESSAGE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const TextId textIdx = (TextId)ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::MESSAGE(%i)", (int)textIdx);

	ScopedEngineFreeze scopedFreeze(engine);
	engine->testRestoreModeSVGA(true);
	if (engine->_text->_showDialogueBubble) {
		engine->_redraw->drawBubble(ctx.actorIdx);
	}
	engine->_text->setFontCrossColor(ctx.actor->_talkColor);
	engine->_scene->_talkingActor = ctx.actorIdx;

	// if we are in sporty mode, we might have triggered a jump with the special action binding
	// see https://bugs.scummvm.org/ticket/13676 for more details.
	if (ctx.actor->isJumpAnimationActive()) {
		engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeRepeat, AnimationTypes::kAnimInvalid, OWN_ACTOR_SCENE_INDEX);
	}

	engine->_text->drawTextProgressive(textIdx);
	if (engine->isLBA1()) {
		if (engine->_scene->_currentSceneIdx == LBA1SceneId::Principal_Island_Library && engine->_scene->_talkingActor == 8 && textIdx == TextId::kStarWarsFanBoy) {
			engine->unlockAchievement("LBA_ACH_008");
		}
	}
	engine->_redraw->redrawEngineActions(true);

	return 0;
}

/**
 * To set the current actor static flag fallable. (Parameter = value & 1)
 * @note Opcode @c 0x1A
 */
int32 ScriptLife::lFALLABLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 flag = ctx.stream.readByte();
	ctx.actor->_staticFlags.bCanFall = flag & 1;
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::FALLABLE(%i)", (int)flag);
	return 0;
}

/**
 * To set direction for current actor.
 * @note Opcode @c 0x1B
 */
int32 ScriptLife::lSET_DIRMODE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 controlMode = ctx.stream.readByte();

	ctx.actor->_controlMode = (ControlMode)controlMode;
	if (ctx.actor->_controlMode == ControlMode::kFollow) {
		ctx.actor->_followedActor = ctx.stream.readByte();
		debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_DIRMODE(%i, %i)", (int)controlMode, (int)ctx.actor->_followedActor);
	} else {
		debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_DIRMODE(%i)", (int)controlMode);
	}

	return 0;
}

/**
 * To set direction
 * @note Opcode @c 0x1C
 */
int32 ScriptLife::lSET_DIRMODE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const int32 controlMode = ctx.stream.readByte();

	ActorStruct *otherActor = engine->_scene->getActor(otherActorIdx);
	otherActor->_controlMode = (ControlMode)controlMode;
	// TODO: should ControlMode::kSameXZ be taken into account, too - see processSameXZAction
	if (otherActor->_controlMode == ControlMode::kFollow || ctx.actor->_controlMode == ControlMode::kFollow2) {
		otherActor->_followedActor = ctx.stream.readByte();
		debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_DIRMODE_OBJ(%i, %i, %i)", (int)otherActorIdx, (int)controlMode, (int)otherActor->_followedActor);
	} else {
		debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_DIRMODE_OBJ(%i, %i)", (int)otherActorIdx, (int)controlMode);
	}

	return 0;
}

/**
 * Camara follow the actor (Parameter = Actor to Follow)
 * @note Opcode @c 0x1D
 */
int32 ScriptLife::lCAM_FOLLOW(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 followedActorIdx = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::CAM_FOLLOW(%i)", (int)followedActorIdx);
	if (engine->_scene->_currentlyFollowedActor != followedActorIdx) {
		const ActorStruct *followedActor = engine->_scene->getActor(followedActorIdx);
		engine->_grid->centerOnActor(followedActor);
		engine->_scene->_currentlyFollowedActor = followedActorIdx;
	}

	return 0;
}

/**
 * Set a new behavior for Twinsen (Paramenter = Behavior Index)
 * @note Opcode @c 0x1E
 */
int32 ScriptLife::lSET_BEHAVIOUR(TwinEEngine *engine, LifeScriptContext &ctx) {
	const HeroBehaviourType behavior = (HeroBehaviourType)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_BEHAVIOUR(%i)", (int)behavior);

	engine->_animations->initAnim(AnimationTypes::kStanding, AnimType::kAnimationTypeRepeat, AnimationTypes::kAnimInvalid, OWN_ACTOR_SCENE_INDEX);
	engine->_actor->setBehaviour(behavior);

	return 0;
}

/**
 * Set a new value for the cube flag (Paramter = Cube Flag Index, Parameter = Value)
 * @note Opcode @c 0x1F
 */
int32 ScriptLife::lSET_FLAG_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 flagIdx = ctx.stream.readByte();
	const int32 flagValue = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_FLAG_CUBE(%i, %i)", (int)flagIdx, (int)flagValue);

	engine->_scene->_listFlagCube[flagIdx] = flagValue;

	return 0;
}

/**
 * Set a new behaviour for the current actor. (Parameter = Comportament number)
 * @note Opcode @c 0x20
 * @note Was only used in the lba editor
 */
int32 ScriptLife::lCOMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.stream.skip(1);
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::COMPORTEMENT()");
	return 0;
}

/**
 * Set a new comportament for the current actor. (Parameter = Comportament Offset)
 * @note Opcode @c 0x21
 */
int32 ScriptLife::lSET_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.actor->_offsetLife = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_COMPORTEMENT(%i)", (int)ctx.actor->_offsetLife);
	return 0;
}

/**
 * Set a new comportament for the actor passed as parameter. (Paramter = Actor Index, Parameter = Comportament Offset)
 * @note Opcode @c 0x22
 */
int32 ScriptLife::lSET_COMPORTEMENT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const int16 pos = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_COMPORTEMENT_OBJ(%i, %i)", (int)otherActorIdx, (int)pos);
	engine->_scene->getActor(otherActorIdx)->_offsetLife = pos;
	return 0;
}

/**
 * End of comportament.
 * @note Opcode @c 0x23
 */
int32 ScriptLife::lEND_COMPORTEMENT(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::END_COMPORTEMENT()");
	return 1; // break
}

/**
 * Kill the actor passed as paramenter (Parameter = Actor Index)
 * @note Opcode @c 0x25
 */
int32 ScriptLife::lKILL_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::lKILL_OBJ(%i)", (int)otherActorIdx);

	engine->_actor->processActorCarrier(otherActorIdx);
	ActorStruct *otherActor = engine->_scene->getActor(otherActorIdx);
	otherActor->_workFlags.bIsDead = 1;
	otherActor->_body = -1;
	otherActor->_zoneSce = -1;
	otherActor->setLife(0);

	return 0;
}

/**
 * Kill the current actor
 * @note Opcode @c 0x26
 */
int32 ScriptLife::lSUICIDE(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SUICIDE()");
	engine->_actor->processActorCarrier(ctx.actorIdx);
	ctx.actor->_workFlags.bIsDead = 1;
	ctx.actor->_body = -1;
	ctx.actor->_zoneSce = -1;
	ctx.actor->setLife(0);

	return 0;
}

/**
 * Use one key collected in the behaviors menu.
 * @note Opcode @c 0x27
 */
int32 ScriptLife::lUSE_ONE_LITTLE_KEY(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::USE_ONE_LITTLE_KEY()");
	engine->_gameState->addKeys(-1);
	engine->_redraw->addOverlay(OverlayType::koSprite, SPRITEHQR_KEY, 0, 0, 0, OverlayPosType::koFollowActor, 1);

	return 0;
}

/**
 * To give money. (Paramenter = Amount)
 * @note Opcode @c 0x28
 */
int32 ScriptLife::lGIVE_GOLD_PIECES(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 oldNumKashes = engine->_gameState->_goldPieces;
	bool hideRange = false;
	const int16 kashes = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::GIVE_GOLD_PIECES(%i)", (int)kashes);

	engine->_gameState->addKashes(-kashes);

	engine->_redraw->addOverlay(OverlayType::koSprite, SPRITEHQR_KASHES, 10, 15, 0, OverlayPosType::koNormal, 3);

	for (int16 i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		OverlayListStruct *overlay = &engine->_redraw->overlayList[i];
		if (overlay->num != -1 && overlay->type == OverlayType::koNumberRange) {
			overlay->num = engine->_collision->boundRuleThree(overlay->info, overlay->num, engine->toSeconds(2), overlay->timerEnd - engine->timerRef - engine->toSeconds(1));
			overlay->info = engine->_gameState->_goldPieces;
			overlay->timerEnd = engine->timerRef + engine->toSeconds(3);
			hideRange = true;
			break;
		}
	}

	if (!hideRange) {
		engine->_redraw->addOverlay(OverlayType::koNumberRange, oldNumKashes, 50, 20, engine->_gameState->_goldPieces, OverlayPosType::koNormal, 3);
	}

	return 0;
}

/**
 * The game will not play the current actor script anymore
 * @note Opcode @c 0x29
 */
int32 ScriptLife::lEND_LIFE(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::END_LIFE()");
	ctx.actor->_offsetLife = -1;
	return 1; // break;
}

/**
 * The current actor will stop doing the track.
 * @note Opcode @c 0x2A
 */
int32 ScriptLife::lSTOP_L_TRACK(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::STOP_L_TRACK(%i)", (int)ctx.actor->_offsetLabelTrack);
	ctx.actor->_memoLabelTrack = ctx.actor->_offsetLabelTrack;
	ctx.actor->_offsetTrack = -1;
	return 0;
}

/**
 * The current actor will resume the tracked started before.
 * @note Opcode @c 0x2B
 */
int32 ScriptLife::lRESTORE_L_TRACK(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::RESTORE_L_TRACK(%i)", (int)ctx.actor->_memoLabelTrack);
	ctx.actor->_offsetTrack = ctx.actor->_memoLabelTrack;
	return 0;
}

/**
 * The actor passed as parameter will say that massage (Parameter = Actor Index, Parameter = Text Index in the current Text Bank)
 * @note Opcode @c 0x2C
 */
int32 ScriptLife::lMESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const TextId textIdx = (TextId)ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::MESSAGE_OBJ(%i, %i)", (int)otherActorIdx, (int)textIdx);

	ScopedEngineFreeze scopedFreeze(engine);
	engine->testRestoreModeSVGA(true);
	if (engine->_text->_showDialogueBubble) {
		engine->_redraw->drawBubble(otherActorIdx);
	}
	engine->_text->setFontCrossColor(engine->_scene->getActor(otherActorIdx)->_talkColor);
	engine->_scene->_talkingActor = otherActorIdx;
	engine->_text->drawTextProgressive(textIdx);
	engine->_redraw->redrawEngineActions(true);

	return 0;
}

/**
 * To increment the current chapter value
 * @note Opcode @c 0x2D
 */
int32 ScriptLife::lINC_CHAPTER(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::INC_CHAPTER()");
	engine->_gameState->setChapter(engine->_gameState->getChapter() + 1);
	debug("Switched chapter to %i", engine->_gameState->getChapter());
	return 0;
}

/**
 * Found an object. (Parameter = Object Index)
 * @note Opcode @c 0x2E
 */
int32 ScriptLife::lFOUND_OBJECT(TwinEEngine *engine, LifeScriptContext &ctx) {
	const InventoryItems item = (InventoryItems)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::FOUND_OBJECT(%i)", (int)item);

	ScopedEngineFreeze scopedFreeze(engine);
	engine->testRestoreModeSVGA(true);
	engine->_gameState->doFoundObj(item);
	engine->_redraw->redrawEngineActions(true);

	return 0;
}

/**
 * Set a new value to open the door (left way) (Parameter = distance to open).
 * @note Opcode @c 0x2F
 */
int32 ScriptLife::lSET_DOOR_LEFT(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 distance = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_DOOR_LEFT(%i)", (int)distance);

	ctx.actor->_beta = LBAAngles::ANGLE_270;
	ctx.actor->_posObj.x = ctx.actor->_animStep.x - distance;
	ctx.actor->_workFlags.bIsSpriteMoving = 0;
	ctx.actor->_speed = 0;

	return 0;
}

/**
 * Set a new value to open the door (right way) (Parameter = distance to open).
 * @note Opcode @c 0x30
 */
int32 ScriptLife::lSET_DOOR_RIGHT(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 distance = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_DOOR_RIGHT(%i)", (int)distance);

	ctx.actor->_beta = LBAAngles::ANGLE_90;
	ctx.actor->_posObj.x = ctx.actor->_animStep.x + distance;
	ctx.actor->_workFlags.bIsSpriteMoving = 0;
	ctx.actor->_speed = 0;

	return 0;
}

/**
 * Set a new value to open the door (up way) (Parameter = distance to open).
 * @note Opcode @c 0x31
 */
int32 ScriptLife::lSET_DOOR_UP(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 distance = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_DOOR_UP(%i)", (int)distance);

	ctx.actor->_beta = LBAAngles::ANGLE_180;
	ctx.actor->_posObj.z = ctx.actor->_animStep.z - distance;
	ctx.actor->_workFlags.bIsSpriteMoving = 0;
	ctx.actor->_speed = 0;

	return 0;
}

/**
 * Set a new value to open the door (down way) (Parameter = distance to open).
 * @note Opcode @c 0x32
 */
int32 ScriptLife::lSET_DOOR_DOWN(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 distance = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_DOOR_DOWN(%i)", (int)distance);

	ctx.actor->_beta = LBAAngles::ANGLE_0;
	ctx.actor->_posObj.z = ctx.actor->_animStep.z + distance;
	ctx.actor->_workFlags.bIsSpriteMoving = 0;
	ctx.actor->_speed = 0;

	return 0;
}

/**
 * Give actor bonus. (Parameter = 0 (Don't change the actor bonus), > 0 (Change to another bonus))
 * @note Opcode @c 0x33
 */
int32 ScriptLife::lGIVE_BONUS(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 flag = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::GIVE_BONUS(%i)", (int)flag);

	if (ctx.actor->_bonusParameter.cloverleaf || ctx.actor->_bonusParameter.kashes || ctx.actor->_bonusParameter.key || ctx.actor->_bonusParameter.lifepoints || ctx.actor->_bonusParameter.magicpoints) {
		engine->_actor->giveExtraBonus(ctx.actorIdx);
	}

	if (flag != 0) {
		ctx.actor->_bonusParameter.givenNothing = 1;
	}

	return 0;
}

/**
 * Change to another room. (Parameter = Scene Index)
 * @note Opcode @c 0x34
 */
int32 ScriptLife::lCHANGE_CUBE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 sceneIdx = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::CHANGE_CUBE(%i)", (int)sceneIdx);
	engine->_scene->_needChangeScene = sceneIdx;
	engine->_scene->_heroPositionType = ScenePositionType::kScene;
	return 0;
}

/**
 * To set the current actor to collid with objects. (Parameter = 1(True) = other values(False))
 * @note Opcode @c 0x35
 */
int32 ScriptLife::lOBJ_COL(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 collision = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::OBJ_COL(%i)", (int)collision);
	if (collision != 0) {
		ctx.actor->_staticFlags.bComputeCollisionWithObj = 1;
	} else {
		ctx.actor->_staticFlags.bComputeCollisionWithObj = 0;
	}
	return 0;
}

/**
 * To set the current actor to collid with bricks. (Parameter = 1(True), = 2(True and the actor is dead), = other values(False))
 * @note Opcode @c 0x36
 */
int32 ScriptLife::lBRICK_COL(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 collision = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::BRICK_COL(%i)", (int)collision);

	ctx.actor->_staticFlags.bComputeCollisionWithBricks = 0;
	ctx.actor->_staticFlags.bComputeLowCollision = 0;

	if (collision == 1) {
		ctx.actor->_staticFlags.bComputeCollisionWithBricks = 1;
	} else if (collision == 2) {
		ctx.actor->_staticFlags.bComputeCollisionWithBricks = 1;
		ctx.actor->_staticFlags.bComputeLowCollision = 1;
	}
	return 0;
}

/**
 * To use various conditions for the same IF statement. (Use above an IF condition)
 * @note Opcode @c 0x37
 */
int32 ScriptLife::lOR_IF(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugCN(3, kDebugLevels::kDebugScripts, "LIFE::OR_IF(");
	const ReturnType valueType = processLifeConditions(engine, ctx);
	if (processLifeOperators(engine, ctx, valueType)) {
		const int16 offset = ctx.stream.readSint16LE();
		ctx.stream.seek(offset); // condition offset
		debugC(3, kDebugLevels::kDebugScripts, ", %i)", offset);
	} else {
		ctx.stream.skip(2);
		debugC(3, kDebugLevels::kDebugScripts, ")");
	}

	return 0;
}

/**
 * Put an actor invisible (Parameter = 1(True), = 0(False))
 * @note Opcode @c 0x38
 */
int32 ScriptLife::lINVISIBLE(TwinEEngine *engine, LifeScriptContext &ctx) {
	ctx.actor->_staticFlags.bIsInvisible = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::INVISIBLE(%i)", (int)ctx.actor->_staticFlags.bIsInvisible);
	return 0;
}

/**
 * Camara zoom in and zoom out. (Parameter = 1(in) = 0(out))
 * @note Opcode @c 0x39
 */
int32 ScriptLife::lZOOM(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int zoomScreen = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::ZOOM(%i)", zoomScreen);

	if (zoomScreen && !engine->_redraw->_flagMCGA && engine->_cfgfile.SceZoom) {
		engine->_screens->fadeToBlack(engine->_screens->_mainPaletteRGBA);
		engine->extInitMcga();
		engine->_screens->setBackPal();
		engine->_screens->_fadePalette = true;
	} else if (!zoomScreen && engine->_redraw->_flagMCGA) {
		engine->_screens->fadeToBlack(engine->_screens->_mainPaletteRGBA);
		engine->extInitSvga();
		engine->_screens->setBackPal();
		engine->_screens->_fadePalette = true;
		engine->_redraw->_firstTime = true;
	}

	return 0;
}

/**
 * Set new position for the current actor (Parameter = Track Index)
 * @note Opcode @c 0x3A
 */
int32 ScriptLife::lPOS_POINT(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 trackIdx = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::POS_POINT(%i)", (int)trackIdx);
	if (engine->_scene->_enableEnhancements) {
		if (IS_HERO(ctx.actorIdx) && engine->_scene->_currentSceneIdx == LBA1SceneId::Citadel_Island_Harbor && trackIdx == 8) {
			ctx.stream.rewind(2);
			ctx.stream.writeByte(0x34); // CHANGE_CUBE
			ctx.stream.writeByte(LBA1SceneId::Principal_Island_Harbor);
			ctx.stream.rewind(2);
			return 0;
		}
	}
	ctx.actor->_posObj = engine->_scene->_sceneTracks[trackIdx];
	return 0;
}

/**
 * To set the magic level. (Paramater = Magic Level)
 * @note Opcode @c 0x3B
 */
int32 ScriptLife::lSET_MAGIC_LEVEL(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_gameState->_magicLevelIdx = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_MAGIC_LEVEL(%i)", (int)engine->_gameState->_magicLevelIdx);
	engine->_gameState->setMaxMagicPoints();
	return 0;
}

/**
 * Substract the magic points. (Parameter = Points Value)
 * @note Opcode @c 0x3C
 */
int32 ScriptLife::lSUB_MAGIC_POINT(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 magicPoints = (int16)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_MAGIC_POINT(%i)", (int)magicPoints);
	engine->_gameState->addMagicPoints(-magicPoints);
	return 0;
}

/**
 * Set new a life point. (Parameter = Actor Index, Parameter = Points Value)
 * @note Opcode @c 0x3D
 */
int32 ScriptLife::lSET_LIFE_POINT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const int32 lifeValue = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_LIFE_POINT_OBJ(%i, %i)", (int)otherActorIdx, (int)lifeValue);

	engine->_scene->getActor(otherActorIdx)->setLife(lifeValue);

	return 0;
}

/**
 * Substract the life points. (Parameter = Actor Index, Parameter = Points Value)
 * @note Opcode @c 0x3E
 */
int32 ScriptLife::lSUB_LIFE_POINT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const int32 lifeValue = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SUB_LIFE_POINT_OBJ(%i, %i)", (int)otherActorIdx, (int)lifeValue);

	ActorStruct *otherActor = engine->_scene->getActor(otherActorIdx);
	otherActor->addLife(-lifeValue);
	if (otherActor->_lifePoint < 0) {
		otherActor->setLife(0);
	}

	return 0;
}

/**
 * Hit an actor. (Parameter = Actor Index)
 * @note Opcode @c 0x3F
 */
int32 ScriptLife::lHIT_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const int32 strengthOfHit = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::HIT_OBJ(%i, %i)", (int)otherActorIdx, (int)strengthOfHit);
	engine->_actor->hitObj(ctx.actorIdx, otherActorIdx, strengthOfHit, engine->_scene->getActor(otherActorIdx)->_beta);
	return 0;
}

/**
 * Play FLA cutscenes (Parameter = Cutscene Name)
 * @note Opcode @c 0x40
 */
int32 ScriptLife::lPLAY_FLA(TwinEEngine *engine, LifeScriptContext &ctx) {
	ScopedEngineFreeze timer(engine);
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
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::PLAY_FLA(%s)", movie);

	engine->_movie->playMovie(movie);
	engine->setPalette(engine->_screens->_paletteRGBA);
	engine->_redraw->_firstTime = true;

	return 0;
}

/**
 * To increment the clover box current value.
 * @note Opcode @c 0x42
 */
int32 ScriptLife::lINC_CLOVER_BOX(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::INC_CLOVER_BOX()");
	engine->_gameState->addLeafBoxes(1);
	return 0;
}

/**
 * To set an inventory object as used (Parameter = Object Index)
 * @note Opcode @c 0x43
 */
int32 ScriptLife::lSET_USED_INVENTORY(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 item = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_USED_INVENTORY(%i)", (int)item);
	// Only up to keypad. lbawin and dotemu are doing this, too
	if (item < InventoryItems::kKeypad) {
		engine->_gameState->_inventoryFlags[item] = 1;
	}
	return 0;
}

/**
 * Add an option for the asked choice . (Parameter = Text Index in the current Text Bank)
 * @note Opcode @c 0x44
 */
int32 ScriptLife::lADD_CHOICE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const TextId choiceIdx = (TextId)ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::ADD_CHOICE(%i)", (int)choiceIdx);
	engine->_gameState->_gameListChoice[engine->_gameState->_gameNbChoices++] = choiceIdx;
	return 0;
}

/**
 * The current actor will ask something (parameter) with choices to choose. (Parameter = Text Index in the current Text Bank)
 * @note Opcode @c 0x45
 */
int32 ScriptLife::lASK_CHOICE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const TextId choiceIdx = (TextId)ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::ASK_CHOICE(%i)", (int)choiceIdx);

	ScopedEngineFreeze scopedFreeze(engine);
	engine->testRestoreModeSVGA(true);
	if (engine->_text->_showDialogueBubble) {
		engine->_redraw->drawBubble(ctx.actorIdx);
	}
	engine->_text->setFontCrossColor(ctx.actor->_talkColor);
	engine->_gameState->gameAskChoice(choiceIdx);
	engine->_gameState->_gameNbChoices = 0;
	engine->_redraw->redrawEngineActions(true);

	return 0;
}

/**
 * Show text in full screen. (Parameter = Text Index in the current Text Bank)
 * @note Opcode @c 0x46
 */
int32 ScriptLife::lBIG_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const TextId textIdx = (TextId)ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::BIG_MESSAGE(%i)", (int)textIdx);

	ScopedEngineFreeze scopedFreeze(engine);
	engine->testRestoreModeSVGA(true);
	engine->_text->bigWinDial();
	if (engine->_text->_showDialogueBubble) {
		engine->_redraw->drawBubble(ctx.actorIdx);
	}
	engine->_text->setFontCrossColor(ctx.actor->_talkColor);
	engine->_scene->_talkingActor = ctx.actorIdx;
	engine->_text->drawTextProgressive(textIdx);
	engine->_text->normalWinDial();
	engine->_redraw->redrawEngineActions(true);

	return 0;
}

/**
 * To initiate the hidden meca-pingouin in the current scene. (Parameter = Actor Index)
 * @note Opcode @c 0x47
 */
int32 ScriptLife::lINIT_PINGOUIN(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 penguinActor = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::INIT_PINGOUIN(%i)", (int)penguinActor);
	engine->_scene->_mecaPenguinIdx = penguinActor;
	ActorStruct *penguin = engine->_scene->getActor(penguinActor);
	penguin->_workFlags.bIsDead = 1;
	penguin->_body = -1;
	penguin->_zoneSce = -1;
	return 0;
}

/**
 * To set an holomap position. (Parameter = Holomap/Scene Index)
 * @note Opcode @c 0x48
 */
int32 ScriptLife::lSET_HOLO_POS(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 location = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_HOLO_POS(%i)", (int)location);
	if (engine->_holomap->setHoloPos(location)) {
		if (engine->_gameState->hasItem(InventoryItems::kiHolomap)) {
			engine->_redraw->addOverlay(OverlayType::koInventoryItem, InventoryItems::kiHolomap, 0, 0, 0, OverlayPosType::koNormal, 3);
		}
	}
	return 0;
}

/**
 * To clear an holomap position. (Parameter = Holomap/Scene Index)
 * @note Opcode @c 0x49
 */
int32 ScriptLife::lCLR_HOLO_POS(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 location = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::CLR_HOLO_POS(%i)", (int)location);
	engine->_holomap->clrHoloPos(location);
	return 0;
}

/**
 * Add to the current fuel value the passed parameter. (Parameter = Fuel Amount)
 * @note Opcode @c 0x4A
 */
int32 ScriptLife::lADD_FUEL(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 value = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::ADD_FUEL(%i)", (int)value);
	if (engine->isLBA2()) {
		return 0;
	}
	engine->_gameState->addGas(value);
	return 0;
}

/**
 * Substract the to fuel value the value passed as parameter. (Parameter = Fuel Amount)
 * @note Opcode @c 0x4B
 */
int32 ScriptLife::lSUB_FUEL(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int16 value = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SUB_FUEL(%i)", (int)value);
	if (engine->isLBA2()) {
		return 0;
	}
	engine->_gameState->addGas(-value);
	return 0;
}

/**
 * To set a GRID disappearing ceiling piece (Parameter = Disappearing ceiling piece Index)
 * @note Opcode @c 0x4C
 */
int32 ScriptLife::lSET_GRM(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_grid->_cellingGridIdx = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_GRM(%i)", (int)engine->_grid->_cellingGridIdx);
	engine->_grid->initCellingGrid(engine->_grid->_cellingGridIdx);
	return 0;
}

/**
 * The current actor will say the message passed as paramenter. (Parameter = Actor Index)
 * @note Opcode @c 0x4D
 */
int32 ScriptLife::lSAY_MESSAGE(TwinEEngine *engine, LifeScriptContext &ctx) {
	const TextId textEntry = (TextId)ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SAY_MESSAGE(%i)", (int)textEntry);

	engine->_redraw->addOverlay(OverlayType::koText, (int16)textEntry, 0, 0, ctx.actorIdx, OverlayPosType::koFollowActor, 2);

	ScopedEngineFreeze scoped(engine);
	engine->_text->initVoxToPlayTextId(textEntry);

	return 0;
}

/**
 * The actor passed as parameter will say the message passed as paramenter. (Parameter = Actor Index, Parameter = Text Index in the current Text Bank)
 * @note Opcode @c 0x4E
 */
int32 ScriptLife::lSAY_MESSAGE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const TextId textEntry = (TextId)ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SAY_MESSAGE_OBJ(%i, %i)", (int)otherActorIdx, (int)textEntry);

	engine->_redraw->addOverlay(OverlayType::koText, (int16)textEntry, 0, 0, otherActorIdx, OverlayPosType::koFollowActor, 2);

	ScopedEngineFreeze scoped(engine);
	engine->_text->initVoxToPlayTextId(textEntry);

	return 0;
}

/**
 * Set Twinsen life point as full
 * @note Opcode @c 0x4F
 */
int32 ScriptLife::lFULL_POINT(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::FULL_POINT()");
	engine->_scene->_sceneHero->setLife(engine->getMaxLife());
	engine->_gameState->setMaxMagicPoints();
	return 0;
}

/**
 * Change actor orientation. (Parameter = New Angle)
 * @note Opcode @c 0x50
 */
int32 ScriptLife::lBETA(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 newAngle = ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::BETA(%i)", (int)newAngle);
	ctx.actor->_beta = ToAngle(newAngle);
	engine->_movements->clearRealAngle(ctx.actor);
	return 0;
}

/**
 * To unset the GRID disappearing ceiling piece.
 * @note Opcode @c 0x51
 */
int32 ScriptLife::lGRM_OFF(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::GRM_OFF()");
	if (engine->_grid->_cellingGridIdx != -1) {
		engine->_grid->_useCellingGrid = -1;
		engine->_grid->_cellingGridIdx = -1;
		engine->_grid->copyMapToCube();
		engine->_redraw->redrawEngineActions(true);
	}

	return 0;
}

/**
 * Fade palette to red
 * @note Opcode @c 0x52
 */
int32 ScriptLife::lFADE_PAL_RED(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::FADE_PAL_RED()");
	ScopedEngineFreeze scoped(engine);
	engine->_screens->fadePalRed(engine->_screens->_mainPaletteRGBA);
	engine->_screens->_useAlternatePalette = false;
	return 0;
}

/**
 * Fade alarm to red
 * @note Opcode @c 0x53
 */
int32 ScriptLife::lFADE_ALARM_RED(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::FADE_ALARM_RED()");
	ScopedEngineFreeze scoped(engine);
	HQR::getEntry(engine->_screens->_palette, Resources::HQR_RESS_FILE, RESSHQR_ALARMREDPAL);
	engine->_screens->convertPalToRGBA(engine->_screens->_palette, engine->_screens->_paletteRGBA);
	engine->_screens->fadePalRed(engine->_screens->_paletteRGBA);
	engine->_screens->_useAlternatePalette = true;
	return 0;
}

/**
 * Fade alarm to palette
 * @note Opcode @c 0x54
 */
int32 ScriptLife::lFADE_ALARM_PAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::FADE_ALARM_PAL()");
	ScopedEngineFreeze scoped(engine);
	HQR::getEntry(engine->_screens->_palette, Resources::HQR_RESS_FILE, RESSHQR_ALARMREDPAL);
	engine->_screens->convertPalToRGBA(engine->_screens->_palette, engine->_screens->_paletteRGBA);
	engine->_screens->adjustCrossPalette(engine->_screens->_paletteRGBA, engine->_screens->_mainPaletteRGBA);
	engine->_screens->_useAlternatePalette = false;
	return 0;
}

/**
 * Fade red to palette
 * @note Opcode @c 0x55
 */
int32 ScriptLife::lFADE_RED_PAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::FADE_RED_PAL()");
	ScopedEngineFreeze scoped(engine);
	engine->_screens->fadeRedPal(engine->_screens->_mainPaletteRGBA);
	engine->_screens->_useAlternatePalette = false;
	return 0;
}

/**
 * Fade red to alarm
 * @note Opcode @c 0x56
 */
int32 ScriptLife::lFADE_RED_ALARM(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::FADE_RED_ALARM()");
	ScopedEngineFreeze scoped(engine);
	HQR::getEntry(engine->_screens->_palette, Resources::HQR_RESS_FILE, RESSHQR_ALARMREDPAL);
	engine->_screens->convertPalToRGBA(engine->_screens->_palette, engine->_screens->_paletteRGBA);
	engine->_screens->fadeRedPal(engine->_screens->_paletteRGBA);
	engine->_screens->_useAlternatePalette = true;
	return 0;
}

/**
 * Fade palette to alarm
 * @note Opcode @c 0x57
 */
int32 ScriptLife::lFADE_PAL_ALARM(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::FADE_PAL_ALARM()");
	ScopedEngineFreeze scoped(engine);
	HQR::getEntry(engine->_screens->_palette, Resources::HQR_RESS_FILE, RESSHQR_ALARMREDPAL);
	engine->_screens->convertPalToRGBA(engine->_screens->_palette, engine->_screens->_paletteRGBA);
	engine->_screens->adjustCrossPalette(engine->_screens->_mainPaletteRGBA, engine->_screens->_paletteRGBA);
	engine->_screens->_useAlternatePalette = true;
	return 0;
}

/**
 * Explode an object. (Parameter = Object Index)
 * @note Opcode @c 0x58
 */
int32 ScriptLife::lEXPLODE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::EXPLODE_OBJ(%i)", (int)otherActorIdx);
	const ActorStruct *otherActor = engine->_scene->getActor(otherActorIdx);

	IVec3 pos = otherActor->posObj();
	pos.x += engine->getRandomNumber(512) - 256;
	pos.y += engine->getRandomNumber(256) - 128;
	pos.z += engine->getRandomNumber(512) - 256;
	engine->_extra->extraExplo(pos);
	return 0;
}

/**
 * The actor will ask something with choices to choose. (Parameter = Actor Index, Parameter = Text Index in the current Text Bank)
 * @note Opcode @c 0x5B
 */
int32 ScriptLife::lASK_CHOICE_OBJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 otherActorIdx = ctx.stream.readByte();
	const TextId choiceIdx = (TextId)ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::ASK_CHOICE_OBJ(%i, %i)", (int)otherActorIdx, (int)choiceIdx);

	ScopedEngineFreeze freeze(engine);
	engine->testRestoreModeSVGA(true);
	if (engine->_text->_showDialogueBubble) {
		engine->_redraw->drawBubble(otherActorIdx);
	}
	engine->_text->setFontCrossColor(engine->_scene->getActor(otherActorIdx)->_talkColor);
	engine->_gameState->gameAskChoice(choiceIdx);
	engine->_gameState->_gameNbChoices = 0;
	engine->_redraw->redrawEngineActions(true);

	return 0;
}

/**
 * Set a dark palette (in the museum).
 * @note Opcode @c 0x5C
 */
int32 ScriptLife::lSET_DARK_PAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_DARK_PAL()");
	engine->_screens->setDarkPal();
	return 0;
}

/**
 * Set main palette.
 * @note Opcode @c 0x5D
 */
int32 ScriptLife::lSET_NORMAL_PAL(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::SET_NORMAL_PAL()");
	engine->_screens->setNormalPal();
	return 0;
}

/**
 * Show Sendell message.
 * @note Opcode @c 0x5E
 */
int32 ScriptLife::lMESSAGE_SENDELL(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::MESSAGE_SENDELL()");
	ScopedEngineFreeze scoped(engine);
	engine->testRestoreModeSVGA(true);
	engine->_screens->fadeToBlack(engine->_screens->_paletteRGBA);
	engine->_screens->loadImage(TwineImage(Resources::HQR_RESS_FILE, 25, 26));
	engine->_text->bigWinDial();
	engine->_text->setFontCrossColor(COLOR_WHITE);
	engine->_text->_flagMessageShade = false;
	const bool tmpFlagDisplayText = engine->_cfgfile.FlagDisplayText;
	engine->_cfgfile.FlagDisplayText = true;
	engine->_text->drawTextProgressive(TextId::kSendell);
	engine->_cfgfile.FlagDisplayText = tmpFlagDisplayText;
	engine->_text->_flagMessageShade = true;
	engine->_text->normalWinDial();
	engine->_screens->fadeToBlack(engine->_screens->_paletteRGBACustom);
	engine->_screens->clearScreen();
	engine->setPalette(engine->_screens->_paletteRGBA);
	return 0;
}

/**
 * Set new animation for the current actor (Parameter = Animation Index)
 * @note Opcode @c 0x5F
 */
int32 ScriptLife::lANIM_SET(TwinEEngine *engine, LifeScriptContext &ctx) {
	const AnimationTypes animIdx = (AnimationTypes)ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::ANIM_SET(%i)", (int)animIdx);

	ctx.actor->_genAnim = AnimationTypes::kAnimNone;
	ctx.actor->_anim = -1;
	engine->_animations->initAnim(animIdx, AnimType::kAnimationTypeRepeat, AnimationTypes::kStanding, ctx.actorIdx);

	return 0;
}

/**
 * Displays holomap travel animation. (Parameter = Trajectory)
 * @note Opcode @c 0x60
 */
int32 ScriptLife::lHOLOMAP_TRAJ(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_scene->_holomapTrajectory = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::HOLOMAP_TRAJ(%i)", (int)engine->_scene->_holomapTrajectory);
	return 0;
}

/**
 * Game over.
 * @note Opcode @c 0x61
 */
int32 ScriptLife::lGAME_OVER(TwinEEngine *engine, LifeScriptContext &ctx) {
	engine->_scene->_sceneHero->_workFlags.bAnimEnded = 1;
	engine->_scene->_sceneHero->setLife(0);
	engine->_gameState->setLeafs(0);
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::GAME_OVER()");
	return 1; // break
}

/**
 * End of the game.
 * @note Opcode @c 0x62
 */
int32 ScriptLife::lTHE_END(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::THE_END()");
	engine->_sceneLoopState = SceneLoopState::Finished;
	engine->_gameState->setLeafs(0);
	engine->_scene->_sceneHero->setLife(engine->getMaxLife());
	engine->_gameState->setMagicPoints(80);
	// TODO: lba2 has a different ending
	engine->_scene->_currentSceneIdx = LBA1SceneId::Polar_Island_Final_Battle;
	engine->_actor->_heroBehaviour = engine->_actor->_previousHeroBehaviour;
	engine->_scene->_newHeroPos.x = -1;
	engine->_scene->_sceneHero->_beta = engine->_actor->_previousHeroAngle;
	engine->autoSave();
	return 1; // break;
}

/**
 * Play a CD Track (Paramenter = CD Track).
 * @note Opcode @c 0x64
 */
int32 ScriptLife::lPLAY_CD_TRACK(TwinEEngine *engine, LifeScriptContext &ctx) {
	const int32 track = ctx.stream.readByte();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::PLAY_CD_TRACK(%i)", (int)track);
	engine->_music->playCdTrack(track);
	return 0;
}

/**
 * Set isometric projections
 * @note Opcode @c 0x65
 */
int32 ScriptLife::lPROJ_ISO(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::PROJ_ISO()");
	engine->_gameState->init3DGame();
	return 0;
}

/**
 * Set 3D projections
 * @note Opcode @c 0x66
 */
int32 ScriptLife::lPROJ_3D(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::PROJ_3D()");
	// TODO: only used for credits scene? If not, then move the credits related code into the menu->showCredits method
	engine->_screens->copyScreen(engine->_frontVideoBuffer, engine->_workVideoBuffer);
	engine->_scene->_enableGridTileRendering = false;

	engine->_renderer->setProjection(engine->width() / 2, engine->height() / 2, 128, 1024, 1024);
	engine->_renderer->setFollowCamera(0, 1500, 0, 25, -128, 0, 13000);
	engine->_renderer->setLightVector(LBAAngles::ANGLE_315, LBAAngles::ANGLE_334, LBAAngles::ANGLE_0);

	engine->_text->initDial(TextBankId::Credits);

	return 0;
}

/**
 * Only display the text. (e.g. like in the credit list) (Parameter = Text Index in the current Text Bank)
 * @note Opcode @c 0x67
 */
int32 ScriptLife::lTEXT(TwinEEngine *engine, LifeScriptContext &ctx) {
	TextId textIdx = (TextId)ctx.stream.readSint16LE();
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::TEXT(%i)", (int)textIdx);

	const int32 textHeight = 40;
	if (lTextYPos < engine->height() - textHeight) {
		if (engine->_cfgfile.Version == USA_VERSION) {
			// TODO: these are most likely not the menu text ids - but from a different text bank
			if (textIdx == TextId::kBehaviourNormal) {
				textIdx = TextId::kSaveSettings;
			}
		}

		char textStr[256];
		engine->_text->getMenuText(textIdx, textStr, sizeof(textStr));
		const int32 textSize = engine->_text->sizeFont(textStr);
		int32 textBoxRight = textSize;
		const int32 textBoxBottom = lTextYPos + textHeight;
		engine->_text->setFontColor(COLOR_WHITE);
		engine->_text->drawText(0, lTextYPos, textStr);
		if (textSize > engine->width() - 1) {
			textBoxRight = engine->width() - 1;
		}

		engine->copyBlockPhys(0, lTextYPos, textBoxRight, textBoxBottom);
		lTextYPos += textHeight;
	}

	return 0;
}

/**
 * Clear displayed text in the screen.
 * @note Opcode @c 0x68
 */
int32 ScriptLife::lCLEAR_TEXT(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::CLEAR_TEXT()");
	lTextYPos = 0;
	const Common::Rect rect(0, 0, engine->width() - 1, engine->height() / 2);
	engine->_interface->box(rect, COLOR_BLACK);
	return 0;
}

/**
 * Exit the script execution.
 * @note Opcode @c 0x69
 */
int32 ScriptLife::lBRUTAL_EXIT(TwinEEngine *engine, LifeScriptContext &ctx) {
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::BRUTAL_EXIT()");
	engine->_sceneLoopState = SceneLoopState::ReturnToMenu;
	return 1; // break
}

ScriptLife::ScriptLife(TwinEEngine *engine, const ScriptLifeFunction *functionMap, size_t entries) : _engine(engine), _functionMap(functionMap), _functionMapSize(entries) {
	lTextYPos = 0;
}

void ScriptLife::doLife(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	int32 end = -2;

	LifeScriptContext ctx(actorIdx, actor);
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::BEGIN(%i)", actorIdx);
	do {
		const byte scriptOpcode = ctx.stream.readByte();
		if (scriptOpcode < _functionMapSize) {
			debugC(3, kDebugLevels::kDebugScripts, "LIFE::EXEC(%s, %i)", _functionMap[scriptOpcode].name, actorIdx);
			end = _functionMap[scriptOpcode].function(_engine, ctx);
		} else {
			error("Actor %d with wrong offset/opcode in life script - Offset: %d/%d (opcode: %i)", actorIdx, (int)ctx.stream.pos() - 1, (int)ctx.stream.size(), scriptOpcode);
		}

		if (end < 0) {
			warning("Actor %d Life script [%s] not implemented", actorIdx, _functionMap[scriptOpcode].name);
		} else if (end == 1) {
			debugC(3, kDebugLevels::kDebugScripts, "LIFE::BREAK(%i)", actorIdx);
		}
		ctx.updateOpcodePos();
	} while (end != 1);
	debugC(3, kDebugLevels::kDebugScripts, "LIFE::END(%i)", actorIdx);
}

} // namespace TwinE
