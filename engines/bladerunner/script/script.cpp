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

#include "bladerunner/script/script.h"

#include "bladerunner/actor.h"
#include "bladerunner/actor_combat.h"
#include "bladerunner/actor_dialogue_queue.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/bladerunner.h"
#include "bladerunner/crimes_database.h"
#include "bladerunner/combat.h"
#include "bladerunner/dialogue_menu.h"
#include "bladerunner/game_flags.h"
#include "bladerunner/game_info.h"
#include "bladerunner/items.h"
#include "bladerunner/item_pickup.h"
#include "bladerunner/movement_track.h"
#include "bladerunner/music.h"
#include "bladerunner/overlays.h"
#include "bladerunner/regions.h"
#include "bladerunner/set.h"
#include "bladerunner/settings.h"
#include "bladerunner/set_effects.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/script/police_maze.h"
#include "bladerunner/slice_animations.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/suspects_database.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/ui/elevator.h"
#include "bladerunner/ui/esper.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/scores.h"
#include "bladerunner/ui/spinner.h"
#include "bladerunner/ui/vk.h"
#include "bladerunner/vector.h"
#include "bladerunner/waypoints.h"

namespace BladeRunner {

ScriptBase::ScriptBase(BladeRunnerEngine *vm) {
	_vm = vm;
}

void ScriptBase::Preload(int animationId) {
	_vm->_sliceRenderer->preload(animationId);
}

void ScriptBase::Actor_Put_In_Set(int actorId, int setId) {
	_vm->_actors[actorId]->setSetId(setId);
}

void ScriptBase::Actor_Set_At_XYZ(int actorId, float x, float y, float z, int direction) {
	_vm->_actors[actorId]->setAtXYZ(Vector3(x, y, z), direction);
}

void ScriptBase::Actor_Set_At_Waypoint(int actorId, int waypointId, int angle) {
	_vm->_actors[actorId]->setAtWaypoint(waypointId, angle, 0, false);
}

bool ScriptBase::Region_Check(int left, int top, int right, int down) {
	//TODO: return _vm->_mouse.x >= left && _vm->_mouse.y >= top && _vm->_mouse.x <= right && _vm->_mouse.y <= down;
	warning("Region_Check(%d, %d, %d, %d)", left, top, right, down);
	return false;
}

bool ScriptBase::Object_Query_Click(const char *objectName1, const char *objectName2) {
	return strcmp(objectName1, objectName2) == 0;
}

void ScriptBase::Object_Do_Ground_Click() {
	//This is not implemented in game
	return;
}

bool ScriptBase::Object_Mark_For_Hot_Mouse(const char *objectName) {
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return false;
	return _vm->_scene->objectSetHotMouse(objectId);
}

void ScriptBase::Actor_Face_Actor(int actorId, int otherActorId, bool animate) {
	_vm->_actors[actorId]->faceActor(otherActorId, animate);
}

void ScriptBase::Actor_Face_Object(int actorId, const char *objectName, bool animate) {
	_vm->_actors[actorId]->faceObject(objectName, animate);
}

void ScriptBase::Actor_Face_Item(int actorId, int itemId, bool animate) {
	_vm->_actors[actorId]->faceItem(itemId, animate);
}

void ScriptBase::Actor_Face_Waypoint(int actorId, int waypointId, bool animate) {
	_vm->_actors[actorId]->faceWaypoint(waypointId, animate);
}

void ScriptBase::Actor_Face_XYZ(int actorId, float x, float y, float z, bool animate) {
	_vm->_actors[actorId]->faceXYZ(x, y, z, animate);
}

void ScriptBase::Actor_Face_Current_Camera(int actorId, bool animate) {
	_vm->_actors[actorId]->faceCurrentCamera(animate);
}

void ScriptBase::Actor_Face_Heading(int actorId, int heading, bool animate) {
	_vm->_actors[actorId]->faceHeading(heading, true);
}

int ScriptBase::Actor_Query_Friendliness_To_Other(int actorId, int otherActorId) {
	return _vm->_actors[actorId]->getFriendlinessToOther(otherActorId);
}

void ScriptBase::Actor_Modify_Friendliness_To_Other(int actorId, int otherActorId, signed int change) {
	_vm->_actors[actorId]->modifyFriendlinessToOther(otherActorId, change);
}

void ScriptBase::Actor_Set_Friendliness_To_Other(int actorId, int otherActorId, int friendliness) {
	_vm->_actors[actorId]->setFriendlinessToOther(otherActorId, friendliness);
}

void ScriptBase::Actor_Set_Honesty(int actorId, int honesty) {
	_vm->_actors[actorId]->setHonesty(honesty);
}

void ScriptBase::Actor_Set_Intelligence(int actorId, int intelligence) {
	_vm->_actors[actorId]->setIntelligence(intelligence);
}

void ScriptBase::Actor_Set_Stability(int actorId, int stability) {
	_vm->_actors[actorId]->setStability(stability);
}

void ScriptBase::Actor_Set_Combat_Aggressiveness(int actorId, int combatAggressiveness) {
	_vm->_actors[actorId]->setCombatAggressiveness(combatAggressiveness);
}

int ScriptBase::Actor_Query_Current_HP(int actorId) {
	return _vm->_actors[actorId]->getCurrentHP();
}

int ScriptBase::Actor_Query_Max_HP(int actorId) {
	return _vm->_actors[actorId]->getMaxHP();
}

int ScriptBase::Actor_Query_Combat_Aggressiveness(int actorId) {
	return _vm->_actors[actorId]->getCombatAggressiveness();
}

int ScriptBase::Actor_Query_Honesty(int actorId) {
	return _vm->_actors[actorId]->getHonesty();
}

int ScriptBase::Actor_Query_Intelligence(int actorId) {
	return _vm->_actors[actorId]->getIntelligence();
}

int ScriptBase::Actor_Query_Stability(int actorId) {
	return _vm->_actors[actorId]->getStability();
}

void ScriptBase::Actor_Modify_Current_HP(int actorId, signed int change) {
	_vm->_actors[actorId]->modifyCurrentHP(change);
}

void ScriptBase::Actor_Modify_Max_HP(int actorId, signed int change) {
	_vm->_actors[actorId]->modifyMaxHP(change);
}

void ScriptBase::Actor_Modify_Combat_Aggressiveness(int actorId, signed int change) {
	_vm->_actors[actorId]->modifyCombatAggressiveness(change);
}

void ScriptBase::Actor_Modify_Honesty(int actorId, signed int change) {
	_vm->_actors[actorId]->modifyHonesty(change);
}

void ScriptBase::Actor_Modify_Intelligence(int actorId, signed int change) {
	_vm->_actors[actorId]->modifyIntelligence(change);
}

void ScriptBase::Actor_Modify_Stability(int actorId, signed int change) {
	_vm->_actors[actorId]->modifyStability(change);
}

void ScriptBase::Actor_Set_Flag_Damage_Anim_If_Moving(int actorId, bool value) {
	_vm->_actors[actorId]->setFlagDamageAnimIfMoving(value);
}

bool ScriptBase::Actor_Query_Flag_Damage_Anim_If_Moving(int actorId) {
	return _vm->_actors[actorId]->getFlagDamageAnimIfMoving();
}

void ScriptBase::Actor_Combat_AI_Hit_Attempt(int actorId) {
	if (_vm->_actors[actorId]->inCombat())
		_vm->_actors[actorId]->_combatInfo->hitAttempt();
}

void ScriptBase::Non_Player_Actor_Combat_Mode_On(int actorId, int initialState, bool rangedAttack, int enemyId, int waypointType, int animationModeCombatIdle, int animationModeCombatWalk, int animationModeCombatRun, int fleeRatio, int coverRatio, int actionRatio, int damage, int range, bool unstoppable) {
	_vm->_actors[actorId]->combatModeOn(initialState, rangedAttack, enemyId, waypointType, animationModeCombatIdle, animationModeCombatWalk, animationModeCombatRun, fleeRatio, coverRatio, actionRatio, damage, range, unstoppable);
}

void ScriptBase::Non_Player_Actor_Combat_Mode_Off(int actorId) {
	_vm->_actors[actorId]->combatModeOff();
}

void ScriptBase::Actor_Set_Health(int actorId, int hp, int maxHp) {
	_vm->_actors[actorId]->setHealth(hp, maxHp);
}

void ScriptBase::Actor_Set_Targetable(int actorId, bool targetable) {
	_vm->_actors[actorId]->setTarget(targetable);

}

void ScriptBase::Actor_Says(int actorId, int sentenceId, int animationMode){
	_vm->loopActorSpeaking();
	_vm->_actorDialogueQueue->flush(1, true);
	Actor_Says_With_Pause(actorId, sentenceId, 0.5f, animationMode);
}

void ScriptBase::Actor_Says_With_Pause(int actorId, int sentenceId, float pause, int animationMode) {
	_vm->gameWaitForActive();
	_vm->loopActorSpeaking();
	_vm->_actorDialogueQueue->flush(1, true);

	Actor *actor = _vm->_actors[actorId];

	if (animationMode != -1) {
		actor->stopWalking(false);
	}

	actor->speechPlay(sentenceId, false);
	bool animationModeChanged = false;
	if (animationMode >= 0) {
		if (actorId != kActorMcCoy) {
			actor->changeAnimationMode(animationMode, false);
			animationModeChanged = true;
		} else if (!_vm->_combat->isActive()) {
			actor->changeAnimationMode(animationMode, false);
			animationModeChanged = true;
		}
	}
	Player_Loses_Control();
	while (_vm->_gameIsRunning) {
		_vm->_speechSkipped = false;
		_vm->gameTick();
		if (_vm->_speechSkipped || !actor->isSpeeching()) {
			actor->speechStop();
			break;
		}
	}
	if (animationModeChanged) {
		actor->changeAnimationMode(kAnimationModeIdle, false);
	}

	//TODO: sitcom
	//if (_vm->isSitcom)
	//{
	//	int rnd = _vm->random(1, 100);
	//	if (rnd <= actor::get_unknown3(actor))
	//	{
	//		int soundId = _vm->random(319, 327);
	//		_vm->_audioPlayer->play(soundId, 40, 0, 0, 50);
	//	}
	//}
	if(pause > 0.0f && !_vm->_speechSkipped) {
		Delay(pause * 1000);
	}
	Player_Gains_Control();
}

void ScriptBase::Actor_Voice_Over(int sentenceId, int actorId) {
	assert(actorId < BladeRunnerEngine::kActorCount);

	_vm->gameWaitForActive();
	_vm->loopActorSpeaking();
	_vm->_actorDialogueQueue->flush(1, true);

	Actor *actor = _vm->_actors[actorId];

	actor->speechPlay(sentenceId, true);
	Player_Loses_Control();
	while(_vm->_gameIsRunning) {
		_vm->_speechSkipped = false;
		_vm->gameTick();
		if(_vm->_speechSkipped || !actor->isSpeeching()) {
			actor->speechStop();
			break;
		}
	}
	Player_Gains_Control();
}

void ScriptBase::Actor_Start_Speech_Sample(int actorId, int sentenceId) {
	_vm->loopActorSpeaking();
	_vm->_actors[actorId]->speechPlay(sentenceId, false);
}

void ScriptBase::Actor_Start_Voice_Over_Sample(int sentenceId) {
	_vm->loopActorSpeaking();
	_vm->_actors[kActorVoiceOver]->speechPlay(sentenceId, true);
}

int ScriptBase::Actor_Query_Which_Set_In(int actorId) {
	return _vm->_actors[actorId]->getSetId();
}

bool ScriptBase::Actor_Query_Is_In_Current_Set(int actorId) {
	int actorSetId = _vm->_actors[actorId]->getSetId();
	return actorSetId >= 0 && actorSetId == _vm->_scene->getSetId();
}

bool ScriptBase::Actor_Query_In_Set(int actorId, int setId) {
	return _vm->_actors[actorId]->getSetId() == setId;
}

int ScriptBase::Actor_Query_Inch_Distance_From_Actor(int actorId, int otherActorId) {
	if (_vm->_actors[actorId]->getSetId() != _vm->_actors[otherActorId]->getSetId()) {
		return 0.0f;
	}
	return _vm->_actors[actorId]->distanceFromActor(otherActorId);
}

int ScriptBase::Actor_Query_Inch_Distance_From_Waypoint(int actorId, int waypointId) {
	if (_vm->_actors[actorId]->getSetId() != _vm->_waypoints->getSetId(waypointId))
		return 0;

	float actorX = _vm->_actors[actorId]->getX();
	float actorZ = _vm->_actors[actorId]->getZ();
	float waypointX = _vm->_waypoints->getX(waypointId);
	float waypointZ = _vm->_waypoints->getZ(waypointId);

	float distX = actorX - waypointX;
	float distZ = actorZ - waypointZ;

	return sqrt(distX * distX + distZ * distZ);
}

bool ScriptBase::Actor_Query_In_Between_Two_Actors(int actorId, int otherActor1Id, int otherActor2Id) {
	float x1 = _vm->_actors[otherActor1Id]->getX();
	float z1 = _vm->_actors[otherActor1Id]->getZ();
	float x2 = _vm->_actors[otherActor2Id]->getX();
	float z2 = _vm->_actors[otherActor2Id]->getZ();
	return _vm->_sceneObjects->isBetween(x1,         z1,         x2,         z1,         actorId + kSceneObjectOffsetActors)
		|| _vm->_sceneObjects->isBetween(x1 - 12.0f, z1 - 12.0f, x2 - 12.0f, z2 - 12.0f, actorId + kSceneObjectOffsetActors)
		|| _vm->_sceneObjects->isBetween(x1 + 12.0f, z1 - 12.0f, x2 + 12.0f, z2 - 12.0f, actorId + kSceneObjectOffsetActors)
		|| _vm->_sceneObjects->isBetween(x1 + 12.0f, z1 + 12.0f, x2 + 12.0f, z2 + 12.0f, actorId + kSceneObjectOffsetActors)
		|| _vm->_sceneObjects->isBetween(x1 - 12.0f, z1 + 12.0f, x2 - 12.0f, z2 + 12.0f, actorId + kSceneObjectOffsetActors);
}

void ScriptBase::Actor_Set_Goal_Number(int actorId, int goalNumber) {
	_vm->_actors[actorId]->setGoal(goalNumber);
}

int ScriptBase::Actor_Query_Goal_Number(int actorId) {
	return _vm->_actors[actorId]->getGoal();
}

void ScriptBase::Actor_Query_XYZ(int actorId, float *x, float *y, float *z) {
	*x = _vm->_actors[actorId]->getX();
	*y = _vm->_actors[actorId]->getY();
	*z = _vm->_actors[actorId]->getZ();
}

int ScriptBase::Actor_Query_Facing_1024(int actorId) {
	return _vm->_actors[actorId]->getFacing();
}

void ScriptBase::Actor_Set_Frame_Rate_FPS(int actorId, int fps) {
	_vm->_actors[actorId]->setFPS(fps);
}

int ScriptBase::Slice_Animation_Query_Number_Of_Frames(int animation) {
	return _vm->_sliceAnimations->getFrameCount(animation);
}

void ScriptBase::Actor_Change_Animation_Mode(int actorId, int animationMode) {
	_vm->_actors[actorId]->changeAnimationMode(animationMode, false);
}

int ScriptBase::Actor_Query_Animation_Mode(int actorId) {
	return _vm->_actors[actorId]->getAnimationMode();
}

bool ScriptBase::Loop_Actor_Walk_To_Actor(int actorId, int otherActorId, int distance, bool interruptible, bool run) {
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		run = true;
	}

	_vm->_playerActorIdle = false;

	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToActor(otherActorId, distance, interruptible, run, true, &isRunning);

	if (_vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
	if (isRunning) {
		_vm->_runningActorId = actorId;
	}
	Global_Variable_Set(kVariableWalkLoopActor, actorId);
	Global_Variable_Set(kVariableWalkLoopRun, isRunning);
	return result;
}

bool ScriptBase::Loop_Actor_Walk_To_Item(int actorId, int itemId, int destinationOffset, bool interruptible, bool run) {
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		run = true;
	}

	_vm->_playerActorIdle = false;

	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToItem(itemId, destinationOffset, interruptible, run, true, &isRunning);

	if (_vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
	if (isRunning) {
		_vm->_runningActorId = actorId;
	}
	Global_Variable_Set(kVariableWalkLoopActor, actorId);
	Global_Variable_Set(kVariableWalkLoopRun, isRunning);
	return result;
}

bool ScriptBase::Loop_Actor_Walk_To_Scene_Object(int actorId, const char *objectName, int destinationOffset, bool interruptible, bool run) {
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		run = true;
	}

	_vm->_playerActorIdle = false;

	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToSceneObject(objectName, destinationOffset, interruptible, run, true, &isRunning);

	if (_vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
	if (isRunning) {
		_vm->_runningActorId = actorId;
	}
	Global_Variable_Set(kVariableWalkLoopActor, actorId);
	Global_Variable_Set(kVariableWalkLoopRun, isRunning);
	return result;
}

bool ScriptBase::Loop_Actor_Walk_To_Waypoint(int actorId, int waypointId, int destinationOffset, bool interruptible, bool run) {
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		run = true;
	}

	_vm->_playerActorIdle = false;

	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToWaypoint(waypointId, destinationOffset, interruptible, run, true, &isRunning);

	if (_vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
	if (isRunning) {
		_vm->_runningActorId = actorId;
	}
	Global_Variable_Set(kVariableWalkLoopActor, actorId);
	Global_Variable_Set(kVariableWalkLoopRun, isRunning);
	return result;
}

bool ScriptBase::Loop_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int destinationOffset, bool interruptible, bool run, int a7) {
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		if (a7) {
			_vm->_runningActorId = -1;
		} else {
			run = true;
		}
	}
	_vm->_playerActorIdle = false;

	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToXYZ(Vector3(x, y, z), destinationOffset, interruptible, run, true, &isRunning);

	if (_vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
	if (isRunning) {
		_vm->_runningActorId = actorId;
	}
	Global_Variable_Set(kVariableWalkLoopActor, actorId);
	Global_Variable_Set(kVariableWalkLoopRun, isRunning);
	return result;
}

void ScriptBase::Async_Actor_Walk_To_Waypoint(int actorId, int waypointId, int destinationOffset, bool run) {
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		run = true;
	}

	_vm->_actors[actorId]->asyncWalkToWaypoint(waypointId, destinationOffset, run, true);
}

void ScriptBase::Async_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int destinationOffset, bool run) {
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		run = true;
	}

	_vm->_actors[actorId]->asyncWalkToXYZ(Vector3(x, y, z), destinationOffset, run, true);
}

void ScriptBase::Actor_Force_Stop_Walking(int actorId) {
	//TODO
	warning("Loop_Actor_Travel_Stairs(%d)", actorId);
}

void ScriptBase::Loop_Actor_Travel_Stairs(int actorId, int stepCount, bool up, int animationModeEnd) {
	_vm->gameWaitForActive();

	Player_Loses_Control();

	Actor *actor = _vm->_actors[actorId];

	int animationModeWalk = 0;
	if (actor->inCombat()) {
		animationModeWalk = up ? kAnimationModeCombatWalkUp : kAnimationModeCombatWalkDown;
		if (animationModeEnd == kAnimationModeIdle) {
			animationModeEnd = kAnimationModeCombatIdle;
		}
	} else {
		animationModeWalk = up ? kAnimationModeWalkUp : kAnimationModeWalkDown;
	}
	actor->changeAnimationMode(animationModeWalk, false);

	int stairsHeight = stepCount * 9 * (up ? 1 : -1);
	float targetY = actor->getY() + stairsHeight;

	bool immunityToObstacles = actor->isImmuneToObstacles();
	actor->setImmunityToObstacles(true);
	do {
		_vm->gameTick();
		if (up) {
			if (targetY <= actor->getY()) {
				break;
			}
		} else {
			if (targetY >= actor->getY()) {
				break;
			}
		}
	} while (_vm->_gameIsRunning);
	actor->setImmunityToObstacles(immunityToObstacles);

	actor->setAtXYZ(Vector3(actor->getX(), targetY, actor->getZ()), actor->getFacing(), true, false, false);
	actor->changeAnimationMode(animationModeEnd, false);

	Player_Gains_Control();
}

void ScriptBase::Loop_Actor_Travel_Ladder(int actorId, int stepCount, bool up, int animationModeEnd) {
	_vm->gameWaitForActive();

	Player_Loses_Control();

	Actor *actor = _vm->_actors[actorId];

	int animationModeWalk = 0;
	if (actor->inCombat()) {
		animationModeWalk = up ? kAnimationModeCombatClimbUp : kAnimationModeCombatClimbDown;
		if (animationModeEnd == kAnimationModeIdle) {
			animationModeEnd = kAnimationModeCombatIdle;
		}
	} else {
		animationModeWalk = up ? kAnimationModeClimbUp : kAnimationModeClimbDown;
	}
	actor->changeAnimationMode(animationModeWalk, false);

	int ladderHeight = stepCount * 12 * (up ? 1 : -1);
	float targetY = actor->getY() + ladderHeight;

	bool immunityToObstacles = actor->isImmuneToObstacles();
	actor->setImmunityToObstacles(true);
	do {
		_vm->gameTick();
		if (up) {
			if (targetY <= actor->getY()) {
				break;
			}
		} else {
			if (targetY >= actor->getY()) {
				break;
			}
		}
	} while (_vm->_gameIsRunning);
	actor->setImmunityToObstacles(immunityToObstacles);

	actor->setAtXYZ(Vector3(actor->getX(), targetY, actor->getZ()), actor->getFacing(), true, false, false);
	actor->changeAnimationMode(animationModeEnd, false);

	Player_Gains_Control();
}

void ScriptBase::Actor_Clue_Add_To_Database(int actorId, int clueId, int weight, bool clueAcquired, bool unknownFlag, int fromActorId) {
	_vm->_actors[actorId]->addClueToDatabase(clueId, weight, clueAcquired, unknownFlag, fromActorId);
}

void ScriptBase::Actor_Clue_Acquire(int actorId, int clueId, bool unknownFlag, int fromActorId) {
	_vm->_actors[actorId]->acquireClue(clueId, unknownFlag, fromActorId);
}

void ScriptBase::Actor_Clue_Lose(int actorId, int clueId) {
	_vm->_actors[actorId]->loseClue(clueId);
}

bool ScriptBase::Actor_Clue_Query(int actorId, int clueId) {
	return _vm->_actors[actorId]->hasClue(clueId);
}

void ScriptBase::Actor_Clues_Transfer_New_To_Mainframe(int actorId) {
	_vm->_actors[actorId]->copyClues(kActorVoiceOver);
}

void ScriptBase::Actor_Clues_Transfer_New_From_Mainframe(int actorId) {
	_vm->_actors[kActorVoiceOver]->copyClues(actorId);
}

void ScriptBase::Actor_Set_Invisible(int actorId, bool isInvisible) {
	_vm->_actors[actorId]->setInvisible(isInvisible);
}

void ScriptBase::Actor_Set_Immunity_To_Obstacles(int actorId, bool isImmune) {
	_vm->_actors[actorId]->setImmunityToObstacles(isImmune);
}

void ScriptBase::Item_Add_To_World(int itemId, int animationId, int setId, float x, float y, float z, signed int facing, int height, int width, bool isTargetable, bool isObstacle, bool isPoliceMazeEnemy, bool updateOnly) {
	_vm->_items->addToWorld(itemId, animationId, setId, Vector3(x, y, z), facing, height, width, isTargetable, isObstacle, isPoliceMazeEnemy, updateOnly == 0);
}

void ScriptBase::Item_Remove_From_World(int itemId) {
	_vm->_items->remove(itemId);
}

void ScriptBase::Item_Spin_In_World(int itemId) {
	_vm->_items->spinInWorld(itemId);
	if (_vm->_items->isPoliceMazeEnemy(itemId)) {
		Police_Maze_Increment_Score(1);
	} else {
		Police_Maze_Decrement_Score(1);
	}
}

void ScriptBase::Item_Flag_As_Target(int itemId) {
	_vm->_items->setIsTarget(itemId, true);
}

void ScriptBase::Item_Flag_As_Non_Target(int itemId) {
	_vm->_items->setIsTarget(itemId, false);
}

void ScriptBase::Item_Pickup_Spin_Effect(int animationId, int x, int y) {
	_vm->_itemPickup->setup(animationId, x, y);
}

int ScriptBase::Animation_Open() {
	//This is not implemented in game
	return -1;
}

int ScriptBase::Animation_Close() {
	//This is not implemented in game
	return 0;
}

int ScriptBase::Animation_Start() {
	//This is not implemented in game
	return 0;
}

int ScriptBase::Animation_Stop() {
	//This is not implemented in game
	return 0;
}

int ScriptBase::Animation_Skip_To_Frame() {
	//This is not implemented in game
	return 0;
}

void ScriptBase::Delay(int miliseconds) {
	Player_Loses_Control();
	int endTime = _vm->_time->current() + miliseconds;
	while (_vm->_gameIsRunning && (_vm->_time->current() < endTime)) {
		_vm->gameTick();
	}
	Player_Gains_Control();
}

void ScriptBase::Player_Loses_Control() {
	_vm->playerLosesControl();
}

void ScriptBase::Player_Gains_Control() {
	_vm->playerGainsControl();
}

void ScriptBase::Player_Set_Combat_Mode(bool activate) {
	if (!_vm->_combat->isActive() || activate) {
		if (_vm->_combat->isActive() && activate) {
			_vm->_combat->activate();
		}
	} else {
		_vm->_combat->deactivate();
	}
}

bool ScriptBase::Player_Query_Combat_Mode() {
	return _vm->_combat->isActive();
}

void ScriptBase::Player_Set_Combat_Mode_Access(bool enable) {
	if (enable) {
		_vm->_combat->enable();
	} else {
		_vm->_combat->disable();
	}
}

int ScriptBase::Player_Query_Current_Set() {
	return _vm->_scene->getSetId();
}

int ScriptBase::Player_Query_Current_Scene() {
	return _vm->_scene->getSceneId();
}

int ScriptBase::Player_Query_Agenda() {
	return _vm->_settings->getPlayerAgenda();
}

void ScriptBase::Player_Set_Agenda(int agenda) {
	_vm->_settings->setPlayerAgenda(agenda);
}

int ScriptBase::Query_Difficulty_Level() {
	return _vm->_settings->getDifficulty();
}


void ScriptBase::Game_Flag_Set(int flag) {
	_vm->_gameFlags->set(flag);
}

void ScriptBase::Game_Flag_Reset(int flag) {
	_vm->_gameFlags->reset(flag);
}

bool ScriptBase::Game_Flag_Query(int flag) {
	return _vm->_gameFlags->query(flag);
}

void ScriptBase::Set_Enter(int setId, int sceneId) {
	_vm->_settings->setNewSetAndScene(setId, sceneId);
}

void ScriptBase::Chapter_Enter(int chapter, int setId, int sceneId) {
	_vm->_settings->setChapter(chapter);
	Set_Enter(setId, sceneId);
}

int ScriptBase::Global_Variable_Set(int var, int value) {
	return _vm->_gameVars[var] = value;
}

int ScriptBase::Global_Variable_Reset(int var) {
	return _vm->_gameVars[var] = 0;
}

int ScriptBase::Global_Variable_Query(int var) {
	return _vm->_gameVars[var];
}

int ScriptBase::Global_Variable_Increment(int var, int inc) {
	return _vm->_gameVars[var] += inc;
}

int ScriptBase::Global_Variable_Decrement(int var, int dec) {
	return _vm->_gameVars[var] -= dec;
}

int ScriptBase::Random_Query(int min, int max) {
	return _vm->_rnd.getRandomNumberRng(min, max);
}

void ScriptBase::Sound_Play(int id, int volume, int panFrom, int panTo, int priority) {
	_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(id), volume, panFrom, panTo, priority);
}

void ScriptBase::Sound_Play_Speech_Line(int actorId, int sentenceId, int volume, int a4, int priority) {
	_vm->_audioSpeech->playSpeechLine(actorId, sentenceId, volume, a4, priority);
}

void ScriptBase::Sound_Left_Footstep_Walk(int actorId) {
	int walkboxId = _vm->_actors[actorId]->getWalkbox();
	if (walkboxId < 0) {
		walkboxId = 0;
	}

	_vm->_walkSoundId = _vm->_scene->_set->getWalkboxSoundWalkLeft(walkboxId);
	_vm->_walkSoundVolume = _vm->_actors[actorId]->soundVolume();
	_vm->_walkSoundBalance = _vm->_actors[actorId]->soundBalance();
}

void ScriptBase::Sound_Right_Footstep_Walk(int actorId) {
	int walkboxId = _vm->_actors[actorId]->getWalkbox();
	if (walkboxId < 0) {
		walkboxId = 0;
	}

	_vm->_walkSoundId = _vm->_scene->_set->getWalkboxSoundWalkRight(walkboxId);
	_vm->_walkSoundVolume = _vm->_actors[actorId]->soundVolume();
	_vm->_walkSoundBalance = _vm->_actors[actorId]->soundBalance();
}

void ScriptBase::Sound_Left_Footstep_Run(int actorId) {
	int walkboxId = _vm->_actors[actorId]->getWalkbox();
	if (walkboxId < 0) {
		walkboxId = 0;
	}

	_vm->_walkSoundId = _vm->_scene->_set->getWalkboxSoundRunLeft(walkboxId);
	_vm->_walkSoundVolume = _vm->_actors[actorId]->soundVolume();
	_vm->_walkSoundBalance = _vm->_actors[actorId]->soundBalance();
}

void ScriptBase::Sound_Right_Footstep_Run(int actorId) {
	int walkboxId = _vm->_actors[actorId]->getWalkbox();
	if (walkboxId < 0) {
		walkboxId = 0;
	}

	_vm->_walkSoundId = _vm->_scene->_set->getWalkboxSoundRunRight(walkboxId);
	_vm->_walkSoundVolume = _vm->_actors[actorId]->soundVolume();
	_vm->_walkSoundBalance = _vm->_actors[actorId]->soundBalance();
}

// ScriptBase::Sound_Walk_Shuffle_Stop

void ScriptBase::Footstep_Sounds_Set(int walkboxId, int stepSound) {
	_vm->_scene->_set->setWalkboxStepSound(walkboxId, stepSound);
}

void ScriptBase::Footstep_Sound_Override_On(int footstepSoundOverride) {
	_vm->_scene->_set->setFoodstepSoundOverride(footstepSoundOverride);
}

void ScriptBase::Footstep_Sound_Override_Off() {
	_vm->_scene->_set->resetFoodstepSoundOverride();
}

bool ScriptBase::Music_Play(int musicId, int volume, int pan, int timeFadeIn, int timePlay, int loop, int timeFadeOut) {
	return _vm->_music->play(_vm->_gameInfo->getMusicTrack(musicId), volume, pan, timeFadeIn, timePlay, loop, timeFadeOut);
}

void ScriptBase::Music_Adjust(int volume, int pan, int delay) {
	_vm->_music->adjust(volume, pan, delay);
}

void ScriptBase::Music_Stop(int delay) {
	_vm->_music->stop(delay);
}

bool ScriptBase::Music_Is_Playing() {
	return _vm->_music->isPlaying();
}

void ScriptBase::Overlay_Play(const char *overlay, int loopId, bool loopForever, bool startNow, int a5) {
	_vm->_overlays->play(overlay, loopId, loopForever, startNow, a5);
}

void ScriptBase::Overlay_Remove(const char *overlay) {
	_vm->_overlays->remove(overlay);
}

void ScriptBase::Scene_Loop_Set_Default(int loopId) {
	_vm->_scene->loopSetDefault(loopId);
}

void ScriptBase::Scene_Loop_Start_Special(int sceneLoopMode, int loopId, bool immediately) {
	if (sceneLoopMode == kSceneLoopModeOnce) {
		immediately = true;
	}
	_vm->_scene->loopStartSpecial(sceneLoopMode, loopId, immediately);
	if (sceneLoopMode == kSceneLoopModeChangeSet) {
		_vm->_settings->clearNewSetAndScene();
	}
}

void ScriptBase::Outtake_Play(int id, int noLocalization, int container) {
	_vm->outtakePlay(id, noLocalization, container);
}

void ScriptBase::Ambient_Sounds_Add_Sound(int sfxId, int timeMin, int timeMax, int volumeMin, int volumeMax, int panStartMin, int panStartMax, int panEndMin, int panEndMax, int priority, int unk) {
	_vm->_ambientSounds->addSound(sfxId, timeMin, timeMax, volumeMin, volumeMax, panStartMin, panStartMax, panEndMin, panEndMax, priority, unk);
}

void  ScriptBase::Ambient_Sounds_Remove_Sound(int sfxId, bool stopPlaying) {
	_vm->_ambientSounds->removeNonLoopingSound(sfxId,  stopPlaying);
}

void ScriptBase::Ambient_Sounds_Add_Speech_Sound(int actorId, int sentenceId, int timeMin, int timeMax, int volumeMin, int volumeMax, int panStartMin, int panStartMax, int panEndMin, int panEndMax, int priority, int unk){
	_vm->_ambientSounds->addSpeech(actorId, sentenceId, timeMin, timeMax, volumeMin, volumeMax, panStartMin, panStartMax, panEndMin, panEndMax, priority, unk);
}

// ScriptBase::Ambient_Sounds_Remove_Speech_Sound

void ScriptBase::Ambient_Sounds_Play_Sound(int sfxId, int volume, int panStart, int panEnd, int priority) {
	_vm->_ambientSounds->playSound(sfxId, volume, panStart, panEnd, priority);
}

void ScriptBase::Ambient_Sounds_Play_Speech_Sound(int actorId, int sfxId, int volume, int panStart, int panEnd, int priority) {
	warning("STIB: Ambient_Sounds_Play_Speech_Sound()");
}

void ScriptBase::Ambient_Sounds_Remove_All_Non_Looping_Sounds(bool stopPlaying) {
	_vm->_ambientSounds->removeAllNonLoopingSounds(stopPlaying);
}

void ScriptBase::Ambient_Sounds_Add_Looping_Sound(int sfxId, int volume, int pan, int delay) {
	_vm->_ambientSounds->addLoopingSound(sfxId, volume, pan, delay);
}

void ScriptBase::Ambient_Sounds_Adjust_Looping_Sound(int sfxId, int volume, int pan, int delay) {
	_vm->_ambientSounds->adjustLoopingSound(sfxId, volume, pan, delay);
}

void ScriptBase::Ambient_Sounds_Remove_Looping_Sound(int sfxId, int delay){
	_vm->_ambientSounds->removeLoopingSound(sfxId, delay);
}

void ScriptBase::Ambient_Sounds_Remove_All_Looping_Sounds(int delay) {
	_vm->_ambientSounds->removeAllLoopingSounds(delay);
}

void ScriptBase::Setup_Scene_Information(float actorX, float actorY, float actorZ, int actorFacing) {
	_vm->_scene->setActorStart(Vector3(actorX, actorY, actorZ), actorFacing);
}

bool ScriptBase::Dialogue_Menu_Appear(int x, int y) {
	if (!_vm->_dialogueMenu->isVisible()) {
		return _vm->_dialogueMenu->show();
	}
	return false;
}

bool ScriptBase::Dialogue_Menu_Disappear() {
	if (_vm->_dialogueMenu->isVisible()) {
		return _vm->_dialogueMenu->hide();
	}
	return false;
}

bool ScriptBase::Dialogue_Menu_Clear_List() {
	_vm->_dialogueMenu->clearList();
	return false;
}

bool ScriptBase::Dialogue_Menu_Add_To_List(int answer) {
	_vm->_dialogueMenu->addToList(answer, false, 5, 5, 5);
	return false;
}

bool ScriptBase::Dialogue_Menu_Add_DONE_To_List(int answerValue) {
	_vm->_dialogueMenu->addToList(answerValue, 1, 0, 0, 0);
	return false;
}

bool ScriptBase::Dialogue_Menu_Add_To_List_Never_Repeat_Once_Selected(int answer) {
	return _vm->_dialogueMenu->addToListNeverRepeatOnceSelected(answer, 5, 5, 5);
}

bool ScriptBase::DM_Add_To_List(int answer, int priorityPolite, int priorityNormal, int prioritySurly) {
	return _vm->_dialogueMenu->addToList(answer, false, priorityPolite, priorityNormal, prioritySurly);
}

bool ScriptBase::DM_Add_To_List_Never_Repeat_Once_Selected(int answer, int priorityPolite, int priorityNormal, int prioritySurly) {
	return _vm->_dialogueMenu->addToListNeverRepeatOnceSelected(answer, priorityPolite, priorityNormal, prioritySurly);
}

void ScriptBase::Dialogue_Menu_Remove_From_List(int answer) {
	//TODO
	warning("Dialogue_Menu_Remove_From_List(%d)", answer);
}

int ScriptBase::Dialogue_Menu_Query_Input() {
	//TODO
	return _vm->_dialogueMenu->queryInput();
}

int ScriptBase::Dialogue_Menu_Query_List_Size() {
	return _vm->_dialogueMenu->listSize();
}

void ScriptBase::Scene_Exit_Add_2D_Exit(int index, int left, int top, int right, int down, int type) {
	_vm->_scene->_exits->add(index, Common::Rect(left, top, right, down), type);
}

void ScriptBase::Scene_Exit_Remove(int index) {
	_vm->_scene->_exits->remove(index);
}

void ScriptBase::Scene_Exits_Disable() {
	_vm->_scene->_exits->setEnabled(false);
}
void ScriptBase::Scene_Exits_Enable() {
	_vm->_scene->_exits->setEnabled(true);
}

void ScriptBase::Scene_2D_Region_Add(int index, int left, int top, int right, int down) {
	_vm->_scene->_regions->add(index, Common::Rect(left, top, right, down), 0);
}

void ScriptBase::Scene_2D_Region_Remove(int index) {
	_vm->_scene->_regions->remove(index);
}

void ScriptBase::World_Waypoint_Set(int waypointId, int setId, float x, float y, float z) {
	_vm->_waypoints->set(waypointId, setId, Vector3(x, y, z));
}
// ScriptBase::World_Waypoint_Reset

float ScriptBase::World_Waypoint_Query_X(int waypointId) {
	return _vm->_waypoints->getX(waypointId);
}

float ScriptBase::World_Waypoint_Query_Y(int waypointId) {
	return _vm->_waypoints->getY(waypointId);
}

float ScriptBase::World_Waypoint_Query_Z(int waypointId) {
	return _vm->_waypoints->getZ(waypointId);
}

void ScriptBase::Combat_Cover_Waypoint_Set_Data(int coverWaypointId, int type, int setId, int sceneId, float x, float y, float z) {
	assert(coverWaypointId < (int)_vm->_combat->_coverWaypoints.size());

	_vm->_combat->_coverWaypoints[coverWaypointId].type = type;
	_vm->_combat->_coverWaypoints[coverWaypointId].setId = setId;
	_vm->_combat->_coverWaypoints[coverWaypointId].sceneId = sceneId;
	_vm->_combat->_coverWaypoints[coverWaypointId].position.x = x;
	_vm->_combat->_coverWaypoints[coverWaypointId].position.y = y;
	_vm->_combat->_coverWaypoints[coverWaypointId].position.z = z;
}

void ScriptBase::Combat_Flee_Waypoint_Set_Data(int fleeWaypointId, int type, int setId, int sceneId, float x, float y, float z, int a8) {
	assert(fleeWaypointId < (int)_vm->_combat->_fleeWaypoints.size());

	_vm->_combat->_fleeWaypoints[fleeWaypointId].type = type;
	_vm->_combat->_fleeWaypoints[fleeWaypointId].setId = setId;
	_vm->_combat->_fleeWaypoints[fleeWaypointId].sceneId = sceneId;
	_vm->_combat->_fleeWaypoints[fleeWaypointId].position.x = x;
	_vm->_combat->_fleeWaypoints[fleeWaypointId].position.y = y;
	_vm->_combat->_fleeWaypoints[fleeWaypointId].position.z = z;
	_vm->_combat->_fleeWaypoints[fleeWaypointId].field7 = a8;
}

void ScriptBase::Police_Maze_Target_Track_Add(int itemId, float startX, float startY, float startZ, float endX, float endY, float endZ, int steps, const int* instructions, bool isActive) {
	_vm->_policeMaze->_tracks[itemId]->add(itemId, startX, startY, startZ, endX, endY, endZ, steps, instructions, isActive);
	_vm->_policeMaze->activate();
}

int ScriptBase::Police_Maze_Query_Score() {
	return Global_Variable_Query(kVariablePoliceMazeScore);
}

void ScriptBase::Police_Maze_Zero_Score() {
	Global_Variable_Reset(kVariablePoliceMazeScore);
}

void ScriptBase::Police_Maze_Increment_Score(int delta) {
	Global_Variable_Set(kVariablePoliceMazeScore, Global_Variable_Query(kVariablePoliceMazeScore) + delta);
}

void ScriptBase::Police_Maze_Decrement_Score(int delta) {
	Global_Variable_Set(kVariablePoliceMazeScore, Global_Variable_Query(kVariablePoliceMazeScore) - delta);
}

void ScriptBase::Police_Maze_Set_Score(int value) {
	Global_Variable_Set(kVariablePoliceMazeScore, value);
}

void ScriptBase::Police_Maze_Set_Pause_State(bool state) {
	_vm->_policeMaze->setPauseState(state);
}

void ScriptBase::CDB_Set_Crime(int clueId, int crimeId) {
	_vm->_crimesDatabase->setCrime(clueId, crimeId);
}

void ScriptBase::CDB_Set_Clue_Asset_Type(int clueId, int assetType) {
	_vm->_crimesDatabase->setAssetType(clueId, assetType);
}

void ScriptBase::SDB_Set_Actor(int suspectId, int actorId) {
	_vm->_suspectsDatabase->get(suspectId)->setActor(actorId);
}

bool ScriptBase::SDB_Add_Photo_Clue(int suspectId, int clueId, int shapeId) {
	return _vm->_suspectsDatabase->get(suspectId)->addPhotoClue(shapeId, clueId);
}

void ScriptBase::SDB_Set_Name(int actorId) {
	// not implemented in game
}

void ScriptBase::SDB_Set_Sex(int suspectId, int sex) {
	_vm->_suspectsDatabase->get(suspectId)->setSex(sex);
}

bool ScriptBase::SDB_Add_Identity_Clue(int suspectId, int clueId) {
	return _vm->_suspectsDatabase->get(suspectId)->addIdentityClue(clueId);
}

bool ScriptBase::SDB_Add_MO_Clue(int suspectId, int clueId) {
	return _vm->_suspectsDatabase->get(suspectId)->addMOClue(clueId);
}

bool ScriptBase::SDB_Add_Whereabouts_Clue(int suspectId, int clueId) {
	return _vm->_suspectsDatabase->get(suspectId)->addWhereaboutsClue(clueId);
}

bool ScriptBase::SDB_Add_Replicant_Clue(int suspectId, int clueId) {
	return _vm->_suspectsDatabase->get(suspectId)->addReplicantClue(clueId);
}

bool ScriptBase::SDB_Add_Non_Replicant_Clue(int suspectId, int clueId) {
	return _vm->_suspectsDatabase->get(suspectId)->addNonReplicantClue(clueId);
}

bool ScriptBase::SDB_Add_Other_Clue(int suspectId, int clueId) {
	return _vm->_suspectsDatabase->get(suspectId)->addOtherClue(clueId);
}

void ScriptBase::Spinner_Set_Selectable_Destination_Flag(int destination, bool selectable) {
	_vm->_spinner->setSelectableDestinationFlag(destination, selectable);
}

// ScriptBase::Spinner_Query_Selectable_Destination_Flag

int ScriptBase::Spinner_Interface_Choose_Dest(int loopId, bool immediately) {
	return _vm->_spinner->chooseDestination(loopId, immediately);
}

void ScriptBase::ESPER_Flag_To_Activate() {
	if (!_vm->_esper->isOpen()) {
		_vm->_esper->open(&_vm->_surfaceBack);
		while (_vm->_esper->isOpen() && _vm->_gameIsRunning) {
			_vm->gameTick();
		}
	}
}

void ScriptBase::Voight_Kampff_Activate(int actorId, int calibrationRatio){
	_vm->_vk->open(actorId, calibrationRatio);
	while (_vm->_vk->isOpen() && _vm->_gameIsRunning) {
		_vm->gameTick();
	}
}

int ScriptBase::Elevator_Activate(int elevatorId) {
	return _vm->_elevator->activate(elevatorId);
}

void ScriptBase::View_Score_Board() {
	_vm->_scores->open();
}

int ScriptBase::Query_Score(int index) {
	return _vm->_scores->query(index);
}

void ScriptBase::Set_Score(int index, int value) {
	_vm->_scores->set(index, value);
}

void ScriptBase::Give_McCoy_Ammo(int ammoType, int ammo) {
	_vm->_settings->addAmmo(ammoType, ammo);
}

void ScriptBase::Assign_Player_Gun_Hit_Sounds(int ammoType, int soundId1, int soundId2, int soundId3) {
	_vm->_combat->setHitSound(ammoType, 0, soundId1);
	_vm->_combat->setHitSound(ammoType, 1, soundId2);
	_vm->_combat->setHitSound(ammoType, 2, soundId3);
}

void ScriptBase::Assign_Player_Gun_Miss_Sounds(int ammoType, int soundId1, int soundId2, int soundId3) {
	_vm->_combat->setMissSound(ammoType, 0, soundId1);
	_vm->_combat->setMissSound(ammoType, 1, soundId2);
	_vm->_combat->setMissSound(ammoType, 2, soundId3);
}

void ScriptBase::Disable_Shadows(int animationsIdsList[], int listSize) {
	_vm->_sliceRenderer->disableShadows(animationsIdsList, listSize);
}

bool ScriptBase::Query_System_Currently_Loading_Game() {
	return _vm->_gameIsLoading;
}

void ScriptBase::Actor_Retired_Here(int actorId, int width, int height, int retired, int retiredByActorId) {
	Actor *actor = _vm->_actors[actorId];
	actor->retire(retired, width, height, retiredByActorId);
	actor->setAtXYZ(actor->getXYZ(), actor->getFacing(), true, false, true);
	_vm->_sceneObjects->setRetired(actorId + kSceneObjectOffsetActors, true);
}

void ScriptBase::Clickable_Object(const char *objectName) {
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return;
	_vm->_scene->objectSetIsClickable(objectId, true, !_vm->_sceneIsLoading);
}

void ScriptBase::Unclickable_Object(const char *objectName) {
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return;
	_vm->_scene->objectSetIsClickable(objectId, false, !_vm->_sceneIsLoading);
}

void ScriptBase::Obstacle_Object(const char *objectName, bool updateWalkpath) {
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return;
	_vm->_scene->objectSetIsObstacle(objectId, true, !_vm->_sceneIsLoading, !_vm->_sceneIsLoading && updateWalkpath);
}

void ScriptBase::Unobstacle_Object(const char *objectName, bool updateWalkpath) {
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return;
	_vm->_scene->objectSetIsObstacle(objectId, false, !_vm->_sceneIsLoading, !_vm->_sceneIsLoading && updateWalkpath);
}

void ScriptBase::Obstacle_Flag_All_Objects(bool isObstacle) {
	_vm->_scene->objectSetIsObstacleAll(isObstacle, !_vm->_sceneIsLoading);
}

void ScriptBase::Combat_Target_Object(const char *objectName) {
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return;
	_vm->_scene->objectSetIsTarget(objectId, true, !_vm->_sceneIsLoading);
}

void ScriptBase::Un_Combat_Target_Object(const char *objectName) {
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return;
	_vm->_scene->objectSetIsTarget(objectId, true, !_vm->_sceneIsLoading);
}

void ScriptBase::Set_Fade_Color(float r, float g, float b) {
	_vm->_scene->_set->_effects->setFadeColor(r, g, b);
}

void ScriptBase::Set_Fade_Density(float density) {
	_vm->_scene->_set->_effects->setFadeDensity(density);
}

void ScriptBase::Set_Fog_Color(const char *fogName, float r, float g, float b) {
	_vm->_scene->_set->_effects->setFogColor(fogName, r, g, b);
}

void ScriptBase::Set_Fog_Density(const char *fogName, float density) {
	_vm->_scene->_set->_effects->setFogDensity(fogName, density);
}

void ScriptBase::ADQ_Flush() {
	_vm->_actorDialogueQueue->flush(0, true);
}

void ScriptBase::ADQ_Add(int actorId, int sentenceId, int animationMode) {
	_vm->_actorDialogueQueue->add(actorId, sentenceId, animationMode);
}

void ScriptBase::ADQ_Add_Pause(int delay) {
	_vm->_actorDialogueQueue->addPause(delay);
}

bool ScriptBase::Game_Over() {
	_vm->_gameIsRunning = false;
	_vm->_gameOver = true;
	return true;
}

void ScriptBase::Autosave_Game(int textId) {
	_vm->_gameAutoSave = textId;
}

void ScriptBase::I_Sez(const char *str) {
	_vm->ISez(str);
}

void ScriptBase::AI_Countdown_Timer_Start(int actorId, signed int timer, int seconds) {
	if (timer >= 0 && timer <= 2) {
		_vm->_actors[actorId]->timerStart(timer, 1000 * seconds);
	}
}

void ScriptBase::AI_Countdown_Timer_Reset(int actorId, int timer) {
	if (timer >= 0 && timer <= 2) {
		_vm->_actors[actorId]->timerReset(timer);
	}
}

void ScriptBase::AI_Movement_Track_Unpause(int actorId) {
	_vm->_actors[actorId]->movementTrackUnpause();
}

void ScriptBase::AI_Movement_Track_Pause(int actorId) {
	_vm->_actors[actorId]->movementTrackPause();
}

void ScriptBase::AI_Movement_Track_Repeat(int actorId) {
	_vm->_actors[actorId]->_movementTrack->repeat();
	_vm->_actors[actorId]->movementTrackNext(true);
}

void ScriptBase::AI_Movement_Track_Append_Run_With_Facing(int actorId, int waypointId, int delay, int angle) {
	_vm->_actors[actorId]->_movementTrack->append(waypointId, delay * 1000, angle, true);
}

void ScriptBase::AI_Movement_Track_Append_With_Facing(int actorId, int waypointId, int delay, int angle) {
	_vm->_actors[actorId]->_movementTrack->append(waypointId, delay * 1000, angle, false);
}

void ScriptBase::AI_Movement_Track_Append_Run(int actorId, int waypointId, int delay) {
	_vm->_actors[actorId]->_movementTrack->append(waypointId, delay * 1000, true);
}

void ScriptBase::AI_Movement_Track_Append(int actorId, int waypointId, int delay) {
	_vm->_actors[actorId]->_movementTrack->append(waypointId, delay * 1000, false);
}

void ScriptBase::AI_Movement_Track_Flush(int actorId) {
	_vm->_actors[actorId]->_movementTrack->flush();
	_vm->_actors[actorId]->stopWalking(false);
}

void ScriptBase::KIA_Play_Actor_Dialogue(int actorId, int sentenceId) {
	_vm->gameWaitForActive();
	_vm->_kia->playActorDialogue(actorId, sentenceId);
}

void ScriptBase::KIA_Play_Slice_Model(int sliceModelId) {
	_vm->gameWaitForActive();
	_vm->_kia->playSliceModel(sliceModelId);
}

void ScriptBase::KIA_Play_Photograph(int photographId) {
	_vm->gameWaitForActive();
	_vm->_kia->playPhotograph(photographId);
}

void ScriptBase::ESPER_Add_Photo(const char *name, int photoId, int shapeId) {
	_vm->_esper->addPhoto(name, photoId, shapeId);
}

void ScriptBase::ESPER_Define_Special_Region(int regionId, int innerLeft, int innerTop, int innerRight, int innerBottom, int outerLeft, int outerTop, int outerRight, int outerBottom, int selectionLeft, int selectionTop, int selectionRight, int selectionBottom, const char *name) {
	_vm->_esper->defineRegion(regionId, Common::Rect(innerLeft, innerTop, innerRight, innerBottom), Common::Rect(outerLeft, outerTop, outerRight, outerBottom), Common::Rect(selectionLeft, selectionTop, selectionRight, selectionBottom), name);
}

void ScriptBase::VK_Play_Speech_Line(int actorId, int sentenceId, float duration) {
	_vm->_vk->playSpeechLine(actorId, sentenceId, duration);
}

void ScriptBase::VK_Add_Question(int intensity, int sentenceId, int relatedSentenceId) {
	_vm->_vk->addQuestion(intensity, sentenceId, relatedSentenceId);
}

void ScriptBase::VK_Subject_Reacts(int intensity, int humanResponse, int replicantResponse, int anxiety) {
	_vm->gameWaitForActive();
	_vm->_vk->subjectReacts(intensity, humanResponse, replicantResponse, anxiety);
}

void ScriptBase::VK_Eye_Animates(int loopId) {
	_vm->_vk->eyeAnimates(loopId);
}

} // End of namespace BladeRunner
