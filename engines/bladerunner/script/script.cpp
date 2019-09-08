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
#if BLADERUNNER_ORIGINAL_BUGS
#else
#include "bladerunner/screen_effects.h"
#endif // BLADERUNNER_ORIGINAL_BUGS
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/script/police_maze.h"
#include "bladerunner/script/scene_script.h"
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
#include "bladerunner/subtitles.h"

#include "common/debug-channels.h"

namespace BladeRunner {

ScriptBase::ScriptBase(BladeRunnerEngine *vm) {
	_vm = vm;
}

void ScriptBase::Preload(int animationId) {
	debugC(8, kDebugScript, "Preload(%d)", animationId);
	_vm->_sliceRenderer->preload(animationId);
}

void ScriptBase::Actor_Put_In_Set(int actorId, int setId) {
	debugC(kDebugScript, "Actor_Put_In_Set(%d, %d)", actorId, setId);
	_vm->_actors[actorId]->setSetId(setId);
}

void ScriptBase::Actor_Set_At_XYZ(int actorId, float x, float y, float z, int direction) {
	debugC(kDebugScript, "Actor_Set_At_XYZ(%d, %f, %f, %f, %d)", actorId, x, y, z, direction);
	_vm->_actors[actorId]->setAtXYZ(Vector3(x, y, z), direction);
}

void ScriptBase::Actor_Set_At_Waypoint(int actorId, int waypointId, int angle) {
	debugC(kDebugScript, "Actor_Set_At_Waypoint(%d, %d, %d)", actorId, waypointId, angle);
	_vm->_actors[actorId]->setAtWaypoint(waypointId, angle, 0, false);
}

bool ScriptBase::Region_Check(int left, int top, int right, int down) {
	debugC(kDebugScript, "Region_Check(%d, %d, %d, %d)", left, top, right, down);

	return _vm->_sceneScript->_mouseX >= left
		&& _vm->_sceneScript->_mouseY >= top
		&& _vm->_sceneScript->_mouseX <= right
		&& _vm->_sceneScript->_mouseY <= down;
}

bool ScriptBase::Object_Query_Click(const char *objectName1, const char *objectName2) {
	debugC(8, kDebugScript, "Object_Query_Click(%s, %s)", objectName1, objectName2);
	return strcmp(objectName1, objectName2) == 0;
}

void ScriptBase::Object_Do_Ground_Click() {
	debugC(kDebugScript, "Object_Do_Ground_Click()");
	//This is not implemented in game
	return;
}

bool ScriptBase::Object_Mark_For_Hot_Mouse(const char *objectName) {
	debugC(kDebugScript, "Object_Mark_For_Hot_Mouse(%s)", objectName);
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return false;
	return _vm->_scene->objectSetHotMouse(objectId);
}

void ScriptBase::Actor_Face_Actor(int actorId, int otherActorId, bool animate) {
	debugC(kDebugScript, "Actor_Face_Actor(%d, %d, %d)", actorId, otherActorId, animate);
	_vm->_actors[actorId]->faceActor(otherActorId, animate);
}

void ScriptBase::Actor_Face_Object(int actorId, const char *objectName, bool animate) {
	debugC(kDebugScript, "Actor_Face_Object(%d, %s, %d)", actorId, objectName, animate);
	_vm->_actors[actorId]->faceObject(objectName, animate);
}

void ScriptBase::Actor_Face_Item(int actorId, int itemId, bool animate) {
	debugC(kDebugScript, "Actor_Face_Item(%d, %d, %d)", actorId, itemId, animate);
	_vm->_actors[actorId]->faceItem(itemId, animate);
}

void ScriptBase::Actor_Face_Waypoint(int actorId, int waypointId, bool animate) {
	debugC(kDebugScript, "Actor_Face_Waypoint(%d, %d, %d)", actorId, waypointId, animate);
	_vm->_actors[actorId]->faceWaypoint(waypointId, animate);
}

void ScriptBase::Actor_Face_XYZ(int actorId, float x, float y, float z, bool animate) {
	debugC(kDebugScript, "Actor_Face_XYZ(%d, %f, %f, %f, %d)", actorId, x, y, z, animate);
	_vm->_actors[actorId]->faceXYZ(x, y, z, animate);
}

void ScriptBase::Actor_Face_Current_Camera(int actorId, bool animate) {
	debugC(kDebugScript, "Actor_Face_Current_Camera(%d, %d)", actorId, animate);
	_vm->_actors[actorId]->faceCurrentCamera(animate);
}

void ScriptBase::Actor_Face_Heading(int actorId, int heading, bool animate) {
	debugC(kDebugScript, "Actor_Face_Heading(%d, %d, %d)", actorId, heading, animate);
	_vm->_actors[actorId]->faceHeading(heading, true);
}

int ScriptBase::Actor_Query_Friendliness_To_Other(int actorId, int otherActorId) {
	debugC(8, kDebugScript, "Actor_Query_Friendliness_To_Other(%d, %d)", actorId, otherActorId);
	return _vm->_actors[actorId]->getFriendlinessToOther(otherActorId);
}

void ScriptBase::Actor_Modify_Friendliness_To_Other(int actorId, int otherActorId, signed int change) {
	debugC(kDebugScript, "Actor_Modify_Friendliness_To_Other(%d, %d, %d)", actorId, otherActorId, change);
	_vm->_actors[actorId]->modifyFriendlinessToOther(otherActorId, change);
}

void ScriptBase::Actor_Set_Friendliness_To_Other(int actorId, int otherActorId, int friendliness) {
	debugC(kDebugScript, "Actor_Set_Friendliness_To_Other(%d, %d, %d)", actorId, otherActorId, friendliness);
	_vm->_actors[actorId]->setFriendlinessToOther(otherActorId, friendliness);
}

void ScriptBase::Actor_Set_Honesty(int actorId, int honesty) {
	debugC(kDebugScript, "Actor_Set_Honesty(%d, %d)", actorId, honesty);
	_vm->_actors[actorId]->setHonesty(honesty);
}

void ScriptBase::Actor_Set_Intelligence(int actorId, int intelligence) {
	debugC(kDebugScript, "Actor_Set_Intelligence(%d, %d)", actorId, intelligence);
	_vm->_actors[actorId]->setIntelligence(intelligence);
}

void ScriptBase::Actor_Set_Stability(int actorId, int stability) {
	debugC(kDebugScript, "Actor_Set_Stability(%d, %d)", actorId, stability);
	_vm->_actors[actorId]->setStability(stability);
}

void ScriptBase::Actor_Set_Combat_Aggressiveness(int actorId, int combatAggressiveness) {
	debugC(kDebugScript, "Actor_Set_Combat_Aggressiveness(%d, %d)", actorId, combatAggressiveness);
	_vm->_actors[actorId]->setCombatAggressiveness(combatAggressiveness);
}

int ScriptBase::Actor_Query_Current_HP(int actorId) {
	debugC(8, kDebugScript, "Actor_Query_Current_HP(%d)", actorId);
	return _vm->_actors[actorId]->getCurrentHP();
}

int ScriptBase::Actor_Query_Max_HP(int actorId) {
	debugC(8, kDebugScript, "Actor_Query_Max_HP(%d)", actorId);
	return _vm->_actors[actorId]->getMaxHP();
}

int ScriptBase::Actor_Query_Combat_Aggressiveness(int actorId) {
	debugC(8, kDebugScript, "Actor_Query_Combat_Aggressiveness(%d)", actorId);
	return _vm->_actors[actorId]->getCombatAggressiveness();
}

int ScriptBase::Actor_Query_Honesty(int actorId) {
	debugC(8, kDebugScript, "Actor_Query_Honesty(%d)", actorId);
	return _vm->_actors[actorId]->getHonesty();
}

int ScriptBase::Actor_Query_Intelligence(int actorId) {
	debugC(8, kDebugScript, "Actor_Query_Intelligence(%d)", actorId);
	return _vm->_actors[actorId]->getIntelligence();
}

int ScriptBase::Actor_Query_Stability(int actorId) {
	debugC(8, kDebugScript, "Actor_Query_Stability(%d)", actorId);
	return _vm->_actors[actorId]->getStability();
}

void ScriptBase::Actor_Modify_Current_HP(int actorId, signed int change) {
	debugC(kDebugScript, "Actor_Modify_Current_HP(%d, %d)", actorId, change);
	_vm->_actors[actorId]->modifyCurrentHP(change);
}

void ScriptBase::Actor_Modify_Max_HP(int actorId, signed int change) {
	debugC(kDebugScript, "Actor_Modify_Max_HP(%d, %d)", actorId, change);
	_vm->_actors[actorId]->modifyMaxHP(change);
}

void ScriptBase::Actor_Modify_Combat_Aggressiveness(int actorId, signed int change) {
	debugC(kDebugScript, "Actor_Modify_Combat_Aggressiveness(%d, %d)", actorId, change);
	_vm->_actors[actorId]->modifyCombatAggressiveness(change);
}

void ScriptBase::Actor_Modify_Honesty(int actorId, signed int change) {
	debugC(kDebugScript, "Actor_Modify_Honesty(%d, %d)", actorId, change);
	_vm->_actors[actorId]->modifyHonesty(change);
}

void ScriptBase::Actor_Modify_Intelligence(int actorId, signed int change) {
	debugC(kDebugScript, "Actor_Modify_Intelligence(%d, %d)", actorId, change);
	_vm->_actors[actorId]->modifyIntelligence(change);
}

void ScriptBase::Actor_Modify_Stability(int actorId, signed int change) {
	debugC(kDebugScript, "Actor_Modify_Stability(%d, %d)", actorId, change);
	_vm->_actors[actorId]->modifyStability(change);
}

void ScriptBase::Actor_Set_Flag_Damage_Anim_If_Moving(int actorId, bool value) {
	debugC(kDebugScript, "Actor_Set_Flag_Damage_Anim_If_Moving(%d, %d)", actorId, value);
	_vm->_actors[actorId]->setFlagDamageAnimIfMoving(value);
}

bool ScriptBase::Actor_Query_Flag_Damage_Anim_If_Moving(int actorId) {
	debugC(8, kDebugScript, "Actor_Query_Flag_Damage_Anim_If_Moving(%d)", actorId);
	return _vm->_actors[actorId]->getFlagDamageAnimIfMoving();
}

void ScriptBase::Actor_Combat_AI_Hit_Attempt(int actorId) {
	debugC(kDebugScript, "Actor_Combat_AI_Hit_Attempt(%d)", actorId);
	if (_vm->_actors[actorId]->inCombat()) {
		_vm->_actors[actorId]->_combatInfo->hitAttempt();
	}
}

void ScriptBase::Non_Player_Actor_Combat_Mode_On(int actorId, int initialState, bool rangedAttack, int enemyId, int waypointType, int animationModeCombatIdle, int animationModeCombatWalk, int animationModeCombatRun, int fleeRatio, int coverRatio, int attackRatio, int damage, int range, bool unstoppable) {
	debugC(kDebugScript, "Non_Player_Actor_Combat_Mode_On(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", actorId, initialState, rangedAttack, enemyId, waypointType, animationModeCombatIdle, animationModeCombatWalk, animationModeCombatRun, fleeRatio, coverRatio, attackRatio, damage, range, unstoppable);
	_vm->_actors[actorId]->combatModeOn(initialState, rangedAttack, enemyId, waypointType, animationModeCombatIdle, animationModeCombatWalk, animationModeCombatRun, fleeRatio, coverRatio, attackRatio, damage, range, unstoppable);
}

void ScriptBase::Non_Player_Actor_Combat_Mode_Off(int actorId) {
	debugC(kDebugScript, "Non_Player_Actor_Combat_Mode_Off(%d)", actorId);
	_vm->_actors[actorId]->combatModeOff();
}

void ScriptBase::Actor_Set_Health(int actorId, int hp, int maxHp) {
	debugC(kDebugScript, "Actor_Set_Health(%d, %d, %d)", actorId, hp, maxHp);
	_vm->_actors[actorId]->setHealth(hp, maxHp);
}

void ScriptBase::Actor_Set_Targetable(int actorId, bool targetable) {
	debugC(kDebugScript, "Actor_Set_Targetable(%d, %d)", actorId, targetable);
	_vm->_actors[actorId]->setTarget(targetable);
}

void ScriptBase::Actor_Says(int actorId, int sentenceId, int animationMode) {
	debugC(kDebugScript, "Actor_Says(%d, %d, %d)", actorId, sentenceId, animationMode);
	_vm->loopActorSpeaking();
	_vm->_actorDialogueQueue->flush(1, true);
	Actor_Says_With_Pause(actorId, sentenceId, 0.5f, animationMode);
}

void ScriptBase::Actor_Says_With_Pause(int actorId, int sentenceId, float pause, int animationMode) {
	debugC(kDebugScript, "Actor_Says_With_Pause(%d, %d, %f, %d)", actorId, sentenceId, pause, animationMode);
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
		_vm->_actorIsSpeaking = true;
		_vm->_actorSpeakStopIsRequested = false;
		_vm->gameTick();
		_vm->_actorIsSpeaking = false;
		if (_vm->_actorSpeakStopIsRequested || !actor->isSpeeching()) {
			actor->speechStop();
			break;
		}
	}
	if (animationModeChanged) {
		actor->changeAnimationMode(kAnimationModeIdle, false);
	}

	if (_vm->_sitcomMode) {
		int rnd = Random_Query(1, 100);
		if (rnd <= actor->getSitcomRatio()) {
			// Choose one of the nine (9) tracks between kSfxAUDLAFF1 and kSfxAUDLAFF9
			Sound_Play(Random_Query(kSfxAUDLAFF1, kSfxAUDLAFF9), 40, 0, 0, 50);
		}
	}

	if (pause > 0.0f && !_vm->_actorSpeakStopIsRequested) {
		Delay(pause * 1000u);
	}

	Player_Gains_Control();
}

void ScriptBase::Actor_Voice_Over(int sentenceId, int actorId) {
	debugC(kDebugScript, "Actor_Voice_Over(%d, %d)", sentenceId, actorId);
	assert(actorId < BladeRunnerEngine::kActorCount);

	_vm->gameWaitForActive();
	_vm->loopActorSpeaking();
	_vm->_actorDialogueQueue->flush(1, true);

	Actor *actor = _vm->_actors[actorId];

	actor->speechPlay(sentenceId, true);
	Player_Loses_Control();
	while (_vm->_gameIsRunning) {
		_vm->_actorIsSpeaking = true;
		_vm->_actorSpeakStopIsRequested = false;
		_vm->gameTick();
		_vm->_actorIsSpeaking = false;
		if (_vm->_actorSpeakStopIsRequested || !actor->isSpeeching()) {
			actor->speechStop();
			break;
		}
	}
	Player_Gains_Control();
}

void ScriptBase::Actor_Start_Speech_Sample(int actorId, int sentenceId) {
	debugC(kDebugScript, "Actor_Start_Speech_Sample(%d, %d)", actorId, sentenceId);
	_vm->loopActorSpeaking();
	_vm->_actors[actorId]->speechPlay(sentenceId, false);
}

void ScriptBase::Actor_Start_Voice_Over_Sample(int sentenceId) {
	debugC(kDebugScript, "Actor_Start_Voice_Over_Sample(%d)", sentenceId);
	_vm->loopActorSpeaking();
	_vm->_actors[kActorVoiceOver]->speechPlay(sentenceId, true);
}

int ScriptBase::Actor_Query_Which_Set_In(int actorId) {
	debugC(8, kDebugScript, "Actor_Query_Which_Set_In(%d)", actorId);
	return _vm->_actors[actorId]->getSetId();
}

bool ScriptBase::Actor_Query_Is_In_Current_Set(int actorId) {
	debugC(8, kDebugScript, "Actor_Query_Is_In_Current_Set(%d)", actorId);
	int actorSetId = _vm->_actors[actorId]->getSetId();
	return actorSetId >= 0 && actorSetId == _vm->_scene->getSetId();
}

bool ScriptBase::Actor_Query_In_Set(int actorId, int setId) {
	debugC(8, kDebugScript, "Actor_Query_In_Set(%d, %d)", actorId, setId);
	return _vm->_actors[actorId]->getSetId() == setId;
}

int ScriptBase::Actor_Query_Inch_Distance_From_Actor(int actorId, int otherActorId) {
	debugC(8, kDebugScript, "Actor_Query_Inch_Distance_From_Actor(%d, %d)", actorId, otherActorId);
	if (_vm->_actors[actorId]->getSetId() != _vm->_actors[otherActorId]->getSetId()) {
		return 0.0f;
	}
	return _vm->_actors[actorId]->distanceFromActor(otherActorId);
}

int ScriptBase::Actor_Query_Inch_Distance_From_Waypoint(int actorId, int waypointId) {
	debugC(8, kDebugScript, "Actor_Query_Inch_Distance_From_Waypoint(%d, %d)", actorId, waypointId);
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
	debugC(8, kDebugScript, "Actor_Query_In_Between_Two_Actors(%d, %d, %d)", actorId, otherActor1Id, otherActor2Id);
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
	debugC(kDebugScript, "Actor_Set_Goal_Number(%d, %d)", actorId, goalNumber);
	_vm->_actors[actorId]->setGoal(goalNumber);
}

int ScriptBase::Actor_Query_Goal_Number(int actorId) {
	debugC(8, kDebugScript, "Actor_Query_Goal_Number(%d)", actorId);
	return _vm->_actors[actorId]->getGoal();
}

void ScriptBase::Actor_Query_XYZ(int actorId, float *x, float *y, float *z) {
	debugC(8, kDebugScript, "Actor_Query_XYZ(%d, ptr, ptr, ptr)", actorId);
	*x = _vm->_actors[actorId]->getX();
	*y = _vm->_actors[actorId]->getY();
	*z = _vm->_actors[actorId]->getZ();
}

int ScriptBase::Actor_Query_Facing_1024(int actorId) {
	debugC(8, kDebugScript, "Actor_Query_Facing_1024(%d)", actorId);
	return _vm->_actors[actorId]->getFacing();
}

void ScriptBase::Actor_Set_Frame_Rate_FPS(int actorId, int fps) {
	debugC(kDebugScript, "Actor_Set_Frame_Rate_FPS(%d, %d)", actorId, fps);
	_vm->_actors[actorId]->setFPS(fps);
}

int ScriptBase::Slice_Animation_Query_Number_Of_Frames(int animation) {
	debugC(8, kDebugScript, "Slice_Animation_Query_Number_Of_Frames(%d)", animation);
	return _vm->_sliceAnimations->getFrameCount(animation);
}

void ScriptBase::Actor_Change_Animation_Mode(int actorId, int animationMode) {
	debugC(kDebugScript, "Actor_Change_Animation_Mode(%d, %d)", actorId, animationMode);
	_vm->_actors[actorId]->changeAnimationMode(animationMode, false);
}

int ScriptBase::Actor_Query_Animation_Mode(int actorId) {
	debugC(8, kDebugScript, "Actor_Query_Animation_Mode(%d)", actorId);
	return _vm->_actors[actorId]->getAnimationMode();
}

bool ScriptBase::Loop_Actor_Walk_To_Actor(int actorId, int otherActorId, int proximity, bool interruptible, bool run) {
	debugC(kDebugScript, "Loop_Actor_Walk_To_Actor(%d, %d, %d, %d, %d)", actorId, otherActorId, proximity, interruptible, run);
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		run = true;
	}

#if BLADERUNNER_ORIGINAL_BUGS
	_vm->_playerActorIdle = false;
#else
	if (actorId == kActorMcCoy) {
		_vm->_playerActorIdle = false;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToActor(otherActorId, proximity, interruptible, run, true, &isRunning);

#if BLADERUNNER_ORIGINAL_BUGS
	if (_vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
#else
	if (actorId == kActorMcCoy && _vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	if (isRunning) {
		_vm->_runningActorId = actorId;
	}
	Global_Variable_Set(kVariableWalkLoopActor, actorId);
	Global_Variable_Set(kVariableWalkLoopRun, isRunning);
	return result;
}

bool ScriptBase::Loop_Actor_Walk_To_Item(int actorId, int itemId, int proximity, bool interruptible, bool run) {
	debugC(kDebugScript, "Loop_Actor_Walk_To_Item(%d, %d, %d, %d, %d)", actorId, itemId, proximity, interruptible, run);
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		run = true;
	}

#if BLADERUNNER_ORIGINAL_BUGS
	_vm->_playerActorIdle = false;
#else
	if (actorId == kActorMcCoy) {
		_vm->_playerActorIdle = false;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToItem(itemId, proximity, interruptible, run, true, &isRunning);

#if BLADERUNNER_ORIGINAL_BUGS
	if (_vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
#else
	if (actorId == kActorMcCoy && _vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	if (isRunning) {
		_vm->_runningActorId = actorId;
	}
	Global_Variable_Set(kVariableWalkLoopActor, actorId);
	Global_Variable_Set(kVariableWalkLoopRun, isRunning);
	return result;
}

bool ScriptBase::Loop_Actor_Walk_To_Scene_Object(int actorId, const char *objectName, int proximity, bool interruptible, bool run) {
	debugC(kDebugScript, "Loop_Actor_Walk_To_Scene_Object(%d, %s, %d, %d, %d)", actorId, objectName, proximity, interruptible, run);
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		run = true;
	}

#if BLADERUNNER_ORIGINAL_BUGS
	_vm->_playerActorIdle = false;
#else
	if (actorId == kActorMcCoy) {
		_vm->_playerActorIdle = false;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToSceneObject(objectName, proximity, interruptible, run, true, &isRunning);

#if BLADERUNNER_ORIGINAL_BUGS
	if (_vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
#else
	if (actorId == kActorMcCoy && _vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	if (isRunning) {
		_vm->_runningActorId = actorId;
	}
	Global_Variable_Set(kVariableWalkLoopActor, actorId);
	Global_Variable_Set(kVariableWalkLoopRun, isRunning);
	return result;
}

bool ScriptBase::Loop_Actor_Walk_To_Waypoint(int actorId, int waypointId, int proximity, bool interruptible, bool run) {
	debugC(kDebugScript, "Loop_Actor_Walk_To_Waypoint(%d, %d, %d, %d, %d)", actorId, waypointId, proximity, interruptible, run);
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		run = true;
	}

#if BLADERUNNER_ORIGINAL_BUGS
	_vm->_playerActorIdle = false;
#else
	if (actorId == kActorMcCoy) {
		_vm->_playerActorIdle = false;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToWaypoint(waypointId, proximity, interruptible, run, true, &isRunning);

#if BLADERUNNER_ORIGINAL_BUGS
	if (_vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
#else
	if (actorId == kActorMcCoy && _vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	if (isRunning) {
		_vm->_runningActorId = actorId;
	}
	Global_Variable_Set(kVariableWalkLoopActor, actorId);
	Global_Variable_Set(kVariableWalkLoopRun, isRunning);
	return result;
}

bool ScriptBase::Loop_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int proximity, bool interruptible, bool run, bool force) {
	debugC(kDebugScript, "Loop_Actor_Walk_To_XYZ(%d, %f, %f, %f, %d, %d, %d, %d)", actorId, x, y, z, proximity, interruptible, run, force);
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		if (force) {
			_vm->_runningActorId = -1;
		} else {
			run = true;
		}
	}

#if BLADERUNNER_ORIGINAL_BUGS
	_vm->_playerActorIdle = false;
#else
	if (actorId == kActorMcCoy) {
		_vm->_playerActorIdle = false;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToXYZ(Vector3(x, y, z), proximity, interruptible, run, true, &isRunning);

#if BLADERUNNER_ORIGINAL_BUGS
	if (_vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
#else
	if (actorId == kActorMcCoy && _vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	if (isRunning) {
		_vm->_runningActorId = actorId;
	}
	Global_Variable_Set(kVariableWalkLoopActor, actorId);
	Global_Variable_Set(kVariableWalkLoopRun, isRunning);
	return result;
}

void ScriptBase::Async_Actor_Walk_To_Waypoint(int actorId, int waypointId, int proximity, bool run) {
	debugC(kDebugScript, "Async_Actor_Walk_To_Waypoint(%d, %d, %d, %d)", actorId, waypointId, proximity, run);
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		run = true;
	}

	_vm->_actors[actorId]->asyncWalkToWaypoint(waypointId, proximity, run, true);
}

void ScriptBase::Async_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int proximity, bool run) {
	debugC(kDebugScript, "Async_Actor_Walk_To_XYZ(%d, %f, %f, %f, %d, %d)", actorId, x, y, z, proximity, run);
	_vm->gameWaitForActive();

	if (_vm->_runningActorId == actorId) {
		run = true;
	}

	_vm->_actors[actorId]->asyncWalkToXYZ(Vector3(x, y, z), proximity, run, true);
}

void ScriptBase::Actor_Force_Stop_Walking(int actorId) {
	debugC(kDebugScript, "Actor_Force_Stop_Walking(%d)", actorId);
	_vm->_actors[actorId]->stopWalking(true);
}

void ScriptBase::Loop_Actor_Travel_Stairs(int actorId, int stepCount, bool up, int animationModeEnd) {
	debugC(kDebugScript, "Loop_Actor_Travel_Stairs(%d, %d, %d, %d)", actorId, stepCount, up, animationModeEnd);
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
	debugC(kDebugScript, "Loop_Actor_Travel_Ladder(%d, %d, %d, %d)", actorId, stepCount, up, animationModeEnd);
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
	debugC(kDebugScript, "Actor_Clue_Add_To_Database(%d, %d, %d, %d, %d, %d)", actorId, clueId, weight, clueAcquired, unknownFlag, fromActorId);
	_vm->_actors[actorId]->addClueToDatabase(clueId, weight, clueAcquired, unknownFlag, fromActorId);
}

void ScriptBase::Actor_Clue_Acquire(int actorId, int clueId, bool unknownFlag, int fromActorId) {
	debugC(kDebugScript, "Actor_Clue_Acquire(%d, %d, %d, %d)", actorId, clueId, unknownFlag, fromActorId);
	_vm->_actors[actorId]->acquireClue(clueId, unknownFlag, fromActorId);
}

void ScriptBase::Actor_Clue_Lose(int actorId, int clueId) {
	debugC(kDebugScript, "Actor_Clue_Lose(%d, %d)", actorId, clueId);
	_vm->_actors[actorId]->loseClue(clueId);
}

bool ScriptBase::Actor_Clue_Query(int actorId, int clueId) {
	debugC(8, kDebugScript, "Actor_Clue_Query(%d, %d)", actorId, clueId);
	return _vm->_actors[actorId]->hasClue(clueId);
}

bool ScriptBase::Actor_Clues_Transfer_New_To_Mainframe(int actorId) {
	debugC(kDebugScript, "Actor_Clues_Transfer_New_To_Mainframe(%d)", actorId);
	return _vm->_actors[actorId]->copyClues(kActorVoiceOver);
}

bool ScriptBase::Actor_Clues_Transfer_New_From_Mainframe(int actorId) {
	debugC(kDebugScript, "Actor_Clues_Transfer_New_From_Mainframe(%d)", actorId);
	return _vm->_actors[kActorVoiceOver]->copyClues(actorId);
}

void ScriptBase::Actor_Set_Invisible(int actorId, bool isInvisible) {
	debugC(kDebugScript, "Actor_Set_Invisible(%d, %d)", actorId, isInvisible);
	_vm->_actors[actorId]->setInvisible(isInvisible);
}

void ScriptBase::Actor_Set_Immunity_To_Obstacles(int actorId, bool isImmune) {
	debugC(kDebugScript, "Actor_Set_Immunity_To_Obstacles(%d, %d)", actorId, isImmune);
	_vm->_actors[actorId]->setImmunityToObstacles(isImmune);
}

void ScriptBase::Item_Add_To_World(int itemId, int animationId, int setId, float x, float y, float z, signed int facing, int height, int width, bool isTargetable, bool isObstacle, bool isPoliceMazeEnemy, bool updateOnly) {
	debugC(kDebugScript, "Item_Add_To_World(%d, %d, %d, %f, %f, %f, %d, %d, %d, %d, %d, %d, %d)", itemId, animationId, setId, x, y, z, facing, height, width, isTargetable, isObstacle, isPoliceMazeEnemy, updateOnly);
	_vm->_items->addToWorld(itemId, animationId, setId, Vector3(x, y, z), facing, height, width, isTargetable, isObstacle, isPoliceMazeEnemy, updateOnly == 0);
}

void ScriptBase::Item_Remove_From_World(int itemId) {
	debugC(kDebugScript, "Item_Remove_From_World(%d)", itemId);
	_vm->_items->remove(itemId);
}

void ScriptBase::Item_Spin_In_World(int itemId) {
	debugC(kDebugScript, "Item_Spin_In_World(%d)", itemId);
	_vm->_items->spinInWorld(itemId);
	if (_vm->_items->isPoliceMazeEnemy(itemId)) {
		Police_Maze_Increment_Score(1);
	} else {
		Police_Maze_Decrement_Score(1);
	}
}

void ScriptBase::Item_Flag_As_Target(int itemId) {
	debugC(kDebugScript, "Item_Flag_As_Target(%d)", itemId);
	_vm->_items->setIsTarget(itemId, true);
}

void ScriptBase::Item_Flag_As_Non_Target(int itemId) {
	debugC(kDebugScript, "Item_Flag_As_Non_Target(%d)", itemId);
	_vm->_items->setIsTarget(itemId, false);
}

void ScriptBase::Item_Pickup_Spin_Effect(int animationId, int x, int y) {
	debugC(kDebugScript, "Item_Pickup_Spin_Effect(%d, %d, %d)", animationId, x, y);
	_vm->_itemPickup->setup(animationId, x, y);
}

void ScriptBase::Item_Pickup_Spin_Effect_From_Actor(int animationId, int actorId, int xOffset, int yOffset) {
	debugC(kDebugScript, "Item_Pickup_Spin_Effect_From_Actor(%d, %d, %d, %d)", animationId, actorId, xOffset, yOffset);

	const Common::Rect &rect = _vm->_actors[actorId]->getScreenRectangle();
	_vm->_itemPickup->setup(animationId, CLIP((rect.right + rect.left) / 2 + xOffset, 0, 639), CLIP((rect.bottom + rect.top) / 2 + yOffset, 0, 479));
}

bool ScriptBase::Item_Query_Visible(int itemId) {
	debugC(kDebugScript, "Item_Query_Visible(%d)", itemId);
	return _vm->_items->isVisible(itemId);
}

// Show text as subtitles mainly for debugging purposes
// eg. display debug data on screen as subtitles
void ScriptBase::Set_Subtitle_Text_On_Screen(Common::String displayText) {
	debugC(kDebugScript, "Set_Subtitle_Text_On_Screen(%s)", displayText.c_str());
	_vm->_subtitles->setGameSubsText(displayText, true);
	_vm->_subtitles->show();
}

#if BLADERUNNER_ORIGINAL_BUGS
#else
void ScriptBase::Screen_Effect_Skip(int effectInc) {
	debugC(kDebugScript, "Screen_Effect_Skip(%d)", effectInc);
	_vm->_screenEffects->toggleEntry(effectInc, true);
	_vm->_scene->advanceFrame(false);
}

void ScriptBase::Screen_Effect_Restore(int effectInc) {
	debugC(kDebugScript, "Screen_Effect_Restore(%d)", effectInc);
	_vm->_screenEffects->toggleEntry(effectInc, false);
	_vm->_scene->advanceFrame(false);
}

void ScriptBase::Screen_Effect_Restore_All() {
	debugC(kDebugScript, "Screen_Effect_Restore_All()");
	_vm->_screenEffects->toggleEntry(-1, false);
	_vm->_scene->advanceFrame(false);
}
#endif // BLADERUNNER_ORIGINAL_BUGS

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

void ScriptBase::Delay(uint32 miliseconds) {
	debugC(kDebugScript, "Delay(%u)", miliseconds);
	Player_Loses_Control();
	uint32 startTime = _vm->_time->current();
	// unsigned difference is intentional
	while (_vm->_gameIsRunning && (_vm->_time->current() - startTime < miliseconds)) {
		_vm->gameTick();
	}
	Player_Gains_Control();
}

void ScriptBase::Player_Loses_Control() {
	debugC(kDebugScript, "Player_Loses_Control()");
	_vm->playerLosesControl();
}

void ScriptBase::Player_Gains_Control() {
	debugC(kDebugScript, "Player_Gains_Control()");
	_vm->playerGainsControl();
}

void ScriptBase::Player_Set_Combat_Mode(bool activate) {
	debugC(kDebugScript, "Player_Set_Combat_Mode(%d)", activate);
	if (!_vm->_combat->isActive() || activate) {
		if (!_vm->_combat->isActive() && activate) {
			_vm->_combat->activate();
		}
	} else {
		_vm->_combat->deactivate();
	}
}

bool ScriptBase::Player_Query_Combat_Mode() {
	debugC(8, kDebugScript, "Player_Query_Combat_Mode()");
	return _vm->_combat->isActive();
}

void ScriptBase::Player_Set_Combat_Mode_Access(bool enable) {
	debugC(kDebugScript, "Player_Set_Combat_Mode_Access(%d)", enable);
	if (enable) {
		_vm->_combat->enable();
	} else {
		_vm->_combat->disable();
	}
}

int ScriptBase::Player_Query_Current_Set() {
	debugC(8, kDebugScript, "Player_Query_Current_Set()");
	return _vm->_scene->getSetId();
}

int ScriptBase::Player_Query_Current_Scene() {
	debugC(8, kDebugScript, "Player_Query_Current_Scene()");
	return _vm->_scene->getSceneId();
}

int ScriptBase::Player_Query_Agenda() {
	debugC(8, kDebugScript, "Player_Query_Agenda()");
	return _vm->_settings->getPlayerAgenda();
}

void ScriptBase::Player_Set_Agenda(int agenda) {
	debugC(kDebugScript, "Player_Set_Agenda(%d)", agenda);
	_vm->_settings->setPlayerAgenda(agenda);
}

int ScriptBase::Query_Difficulty_Level() {
	debugC(8, kDebugScript, "Query_Difficulty_Level()");
	return _vm->_settings->getDifficulty();
}

void ScriptBase::Game_Flag_Set(int flag) {
	debugC(kDebugScript, "Game_Flag_Set(%d)", flag);
	_vm->_gameFlags->set(flag);
}

void ScriptBase::Game_Flag_Reset(int flag) {
	debugC(kDebugScript, "Game_Flag_Reset(%d)", flag);
	_vm->_gameFlags->reset(flag);
}

bool ScriptBase::Game_Flag_Query(int flag) {
	debugC(8, kDebugScript, "Game_Flag_Query(%d)", flag);
	return _vm->_gameFlags->query(flag);
}

void ScriptBase::Set_Enter(int setId, int sceneId) {
	debugC(kDebugScript, "Set_Enter(%d, %d)", setId, sceneId);
	_vm->_settings->setNewSetAndScene(setId, sceneId);
}

void ScriptBase::Chapter_Enter(int chapter, int setId, int sceneId) {
	debugC(kDebugScript, "Chapter_Enter(%d, %d, %d)", chapter, setId, sceneId);
	_vm->_settings->setChapter(chapter);
	Set_Enter(setId, sceneId);
}

int ScriptBase::Global_Variable_Set(int var, int value) {
	debugC(kDebugScript, "Global_Variable_Set(%d, %d)", var, value);
	return _vm->_gameVars[var] = value;
}

int ScriptBase::Global_Variable_Reset(int var) {
	debugC(kDebugScript, "Global_Variable_Reset(%d)", var);
	return _vm->_gameVars[var] = 0;
}

int ScriptBase::Global_Variable_Query(int var) {
	debugC(8, kDebugScript, "Global_Variable_Query(%d)", var);
	return _vm->_gameVars[var];
}

int ScriptBase::Global_Variable_Increment(int var, int inc) {
	debugC(kDebugScript, "Global_Variable_Increment(%d, %d)", var, inc);
	return _vm->_gameVars[var] += inc;
}

int ScriptBase::Global_Variable_Decrement(int var, int dec) {
	debugC(kDebugScript, "Global_Variable_Decrement(%d, %d)", var, dec);
	return _vm->_gameVars[var] -= dec;
}

int ScriptBase::Random_Query(int min, int max) {
	debugC(9, kDebugScript, "Random_Query(%d, %d)", min, max);
	if ( min == max )
	{
		return min;
	}
	if ( min > max ) // there is at least one such case
	{
		return _vm->_rnd.getRandomNumberRng(max, min); // swap the arguments
	}
	return _vm->_rnd.getRandomNumberRng(min, max);
}

void ScriptBase::Sound_Play(int id, int volume, int panFrom, int panTo, int priority) {
	debugC(6, kDebugScript, "Sound_Play(%d, %d, %d, %d, %d)", id, volume, panFrom, panTo, priority);
	_vm->_audioPlayer->playAud(_vm->_gameInfo->getSfxTrack(id), volume, panFrom, panTo, priority);
}

void ScriptBase::Sound_Play_Speech_Line(int actorId, int sentenceId, int volume, int a4, int priority) {
	debugC(kDebugScript, "Sound_Play_Speech_Line(%d, %d, %d, %d, %d)", actorId, sentenceId, volume, a4, priority);
	_vm->_audioSpeech->playSpeechLine(actorId, sentenceId, volume, a4, priority);
}

void ScriptBase::Sound_Left_Footstep_Walk(int actorId) {
	debugC(6, kDebugScript, "Sound_Left_Footstep_Walk(%d)", actorId);
	int walkboxId = _vm->_actors[actorId]->getWalkbox();
	if (walkboxId < 0) {
		walkboxId = 0;
	}

	_vm->_walkSoundId = _vm->_scene->_set->getWalkboxSoundWalkLeft(walkboxId);
	_vm->_walkSoundVolume = _vm->_actors[actorId]->soundVolume();
	_vm->_walkSoundPan = _vm->_actors[actorId]->soundPan();
}

void ScriptBase::Sound_Right_Footstep_Walk(int actorId) {
	debugC(6, kDebugScript, "Sound_Right_Footstep_Walk(%d)", actorId);
	int walkboxId = _vm->_actors[actorId]->getWalkbox();
	if (walkboxId < 0) {
		walkboxId = 0;
	}

	_vm->_walkSoundId = _vm->_scene->_set->getWalkboxSoundWalkRight(walkboxId);
	_vm->_walkSoundVolume = _vm->_actors[actorId]->soundVolume();
	_vm->_walkSoundPan = _vm->_actors[actorId]->soundPan();
}

void ScriptBase::Sound_Left_Footstep_Run(int actorId) {
	debugC(6, kDebugScript, "Sound_Left_Footstep_Run(%d)", actorId);
	int walkboxId = _vm->_actors[actorId]->getWalkbox();
	if (walkboxId < 0) {
		walkboxId = 0;
	}

	_vm->_walkSoundId = _vm->_scene->_set->getWalkboxSoundRunLeft(walkboxId);
	_vm->_walkSoundVolume = _vm->_actors[actorId]->soundVolume();
	_vm->_walkSoundPan = _vm->_actors[actorId]->soundPan();
}

void ScriptBase::Sound_Right_Footstep_Run(int actorId) {
	debugC(6, kDebugScript, "Sound_Right_Footstep_Run(%d)", actorId);
	int walkboxId = _vm->_actors[actorId]->getWalkbox();
	if (walkboxId < 0) {
		walkboxId = 0;
	}

	_vm->_walkSoundId = _vm->_scene->_set->getWalkboxSoundRunRight(walkboxId);
	_vm->_walkSoundVolume = _vm->_actors[actorId]->soundVolume();
	_vm->_walkSoundPan = _vm->_actors[actorId]->soundPan();
}

// ScriptBase::Sound_Walk_Shuffle_Stop

void ScriptBase::Footstep_Sounds_Set(int walkboxId, int stepSound) {
	debugC(kDebugScript, "Footstep_Sounds_Set(%d, %d)", walkboxId, stepSound);
	_vm->_scene->_set->setWalkboxStepSound(walkboxId, stepSound);
}

void ScriptBase::Footstep_Sound_Override_On(int footstepSoundOverride) {
	debugC(kDebugScript, "Footstep_Sound_Override_On(%d)", footstepSoundOverride);
	_vm->_scene->_set->setFoodstepSoundOverride(footstepSoundOverride);
}

void ScriptBase::Footstep_Sound_Override_Off() {
	debugC(kDebugScript, "Footstep_Sound_Override_Off()");
	_vm->_scene->_set->resetFoodstepSoundOverride();
}

bool ScriptBase::Music_Play(int musicId, int volume, int pan, int32 timeFadeIn, int32 timePlay, int loop, int32 timeFadeOut) {
	debugC(kDebugScript, "Music_Play(%d, %d, %d, %d, %d, %d, %d)", musicId, volume, pan, timeFadeIn, timePlay, loop, timeFadeOut);
	return _vm->_music->play(_vm->_gameInfo->getMusicTrack(musicId), volume, pan, timeFadeIn, timePlay, loop, timeFadeOut);
}

void ScriptBase::Music_Adjust(int volume, int pan, uint32 delay) {
	debugC(kDebugScript, "Music_Adjust(%d, %d, %u)", volume, pan, delay);
	_vm->_music->adjust(volume, pan, delay);
}

void ScriptBase::Music_Stop(uint32 delay) {
	debugC(kDebugScript, "Music_Stop(%u)", delay);
	_vm->_music->stop(delay);
}

bool ScriptBase::Music_Is_Playing() {
	debugC(8, kDebugScript, "Music_Is_Playing()");
	return _vm->_music->isPlaying();
}

void ScriptBase::Overlay_Play(const char *overlay, int loopId, bool loopForever, bool startNow, int a5) {
	debugC(kDebugScript, "Overlay_Play(%s, %d, %d, %d, %d)", overlay, loopId, loopForever, startNow, a5);
	_vm->_overlays->play(overlay, loopId, loopForever, startNow, a5);
}

void ScriptBase::Overlay_Remove(const char *overlay) {
	debugC(kDebugScript, "Overlay_Remove(%s)", overlay);
	_vm->_overlays->remove(overlay);
}

void ScriptBase::Scene_Loop_Set_Default(int loopId) {
	debugC(kDebugScript, "Scene_Loop_Set_Default(%d)", loopId);
	_vm->_scene->loopSetDefault(loopId);
}

void ScriptBase::Scene_Loop_Start_Special(int sceneLoopMode, int loopId, bool immediately) {
	debugC(kDebugScript, "Scene_Loop_Start_Special(%d, %d, %d)", sceneLoopMode, loopId, immediately);
	if (sceneLoopMode == kSceneLoopModeOnce) {
		immediately = true;
	}
	_vm->_scene->loopStartSpecial(sceneLoopMode, loopId, immediately);
	if (sceneLoopMode == kSceneLoopModeChangeSet) {
		_vm->_settings->clearNewSetAndScene();
	}
}

void ScriptBase::Outtake_Play(int id, int noLocalization, int container) {
	debugC(kDebugScript, "Outtake_Play(%d, %d, %d)", id, noLocalization, container);
	_vm->outtakePlay(id, noLocalization, container);
}

void ScriptBase::Ambient_Sounds_Add_Sound(int sfxId, uint32 timeMin, uint32 timeMax, int volumeMin, int volumeMax, int panStartMin, int panStartMax, int panEndMin, int panEndMax, int priority, int unk) {
	debugC(kDebugScript, "Ambient_Sounds_Add_Sound(%d, %u, %u, %d, %d, %d, %d, %d, %d, %d, %d)", sfxId, timeMin, timeMax, volumeMin, volumeMax, panStartMin, panStartMax, panEndMin, panEndMax, priority, unk);
	_vm->_ambientSounds->addSound(sfxId, timeMin, timeMax, volumeMin, volumeMax, panStartMin, panStartMax, panEndMin, panEndMax, priority, unk);
}

void  ScriptBase::Ambient_Sounds_Remove_Sound(int sfxId, bool stopPlaying) {
	debugC(kDebugScript, "Ambient_Sounds_Remove_Sound(%d, %d)", sfxId, stopPlaying);
	_vm->_ambientSounds->removeNonLoopingSound(sfxId,  stopPlaying);
}

void ScriptBase::Ambient_Sounds_Add_Speech_Sound(int actorId, int sentenceId, uint32 timeMin, uint32 timeMax, int volumeMin, int volumeMax, int panStartMin, int panStartMax, int panEndMin, int panEndMax, int priority, int unk) {
	debugC(kDebugScript, "Ambient_Sounds_Add_Speech_Sound(%d, %d, %u, %u, %d, %d, %d, %d, %d, %d, %d, %d)", actorId, sentenceId, timeMin, timeMax, volumeMin, volumeMax, panStartMin, panStartMax, panEndMin, panEndMax, priority, unk);
	_vm->_ambientSounds->addSpeech(actorId, sentenceId, timeMin, timeMax, volumeMin, volumeMax, panStartMin, panStartMax, panEndMin, panEndMax, priority, unk);
}

// ScriptBase::Ambient_Sounds_Remove_Speech_Sound

void ScriptBase::Ambient_Sounds_Play_Sound(int sfxId, int volume, int panStart, int panEnd, int priority) {
	debugC(kDebugScript, "Ambient_Sounds_Play_Sound(%d, %d, %d, %d, %d)", sfxId, volume, panStart, panEnd, priority);
	_vm->_ambientSounds->playSound(sfxId, volume, panStart, panEnd, priority);
}

void ScriptBase::Ambient_Sounds_Play_Speech_Sound(int actorId, int sentenceId, int volume, int panStart, int panEnd, int priority) {
	debugC(kDebugScript, "Ambient_Sounds_Play_Speech_Sound(%d, %d, %d, %d, %d, %d)", actorId, sentenceId, volume, panStart, panEnd, priority);
	_vm->_ambientSounds->playSpeech(actorId, sentenceId, volume, panStart, panEnd, priority);
}

void ScriptBase::Ambient_Sounds_Remove_All_Non_Looping_Sounds(bool stopPlaying) {
	debugC(kDebugScript, "Ambient_Sounds_Remove_All_Non_Looping_Sounds(%d)", stopPlaying);
	_vm->_ambientSounds->removeAllNonLoopingSounds(stopPlaying);
}

void ScriptBase::Ambient_Sounds_Add_Looping_Sound(int sfxId, int volume, int pan, uint32 delay) {
	debugC(kDebugScript, "Ambient_Sounds_Add_Looping_Sound(%d, %d, %d, %u)", sfxId, volume, pan, delay);
	_vm->_ambientSounds->addLoopingSound(sfxId, volume, pan, delay);
}

void ScriptBase::Ambient_Sounds_Adjust_Looping_Sound(int sfxId, int volume, int pan, uint32 delay) {
	debugC(kDebugScript, "Ambient_Sounds_Adjust_Looping_Sound(%d, %d, %d, %u)", sfxId, volume, pan, delay);
	_vm->_ambientSounds->adjustLoopingSound(sfxId, volume, pan, delay);
}

void ScriptBase::Ambient_Sounds_Remove_Looping_Sound(int sfxId, uint32 delay) {
	debugC(kDebugScript, "Ambient_Sounds_Remove_Looping_Sound(%d, %u)", sfxId, delay);
	_vm->_ambientSounds->removeLoopingSound(sfxId, delay);
}

void ScriptBase::Ambient_Sounds_Remove_All_Looping_Sounds(uint32 delay) {
	debugC(kDebugScript, "Ambient_Sounds_Remove_All_Looping_Sounds(%u)", delay);
	_vm->_ambientSounds->removeAllLoopingSounds(delay);
}

void ScriptBase::Setup_Scene_Information(float actorX, float actorY, float actorZ, int actorFacing) {
	debugC(kDebugScript, "Setup_Scene_Information(%f, %f, %f, %d)", actorX, actorY, actorZ, actorFacing);
	_vm->_scene->setActorStart(Vector3(actorX, actorY, actorZ), actorFacing);
}

bool ScriptBase::Dialogue_Menu_Appear(int x, int y) {
	debugC(kDebugScript, "Dialogue_Menu_Appear(%d, %d)", x, y);
	if (!_vm->_dialogueMenu->isVisible()) {
		return _vm->_dialogueMenu->show();
	}
	return false;
}

bool ScriptBase::Dialogue_Menu_Disappear() {
	debugC(kDebugScript, "Dialogue_Menu_Disappear()");
	if (_vm->_dialogueMenu->isVisible()) {
		return _vm->_dialogueMenu->hide();
	}
	return false;
}

bool ScriptBase::Dialogue_Menu_Clear_List() {
	debugC(kDebugScript, "Dialogue_Menu_Clear_List()");
	_vm->_dialogueMenu->clearList();
	return false;
}

bool ScriptBase::Dialogue_Menu_Add_To_List(int answer) {
	debugC(kDebugScript, "Dialogue_Menu_Add_To_List(%d)", answer);
	_vm->_dialogueMenu->addToList(answer, false, 5, 5, 5);
	return false;
}

bool ScriptBase::Dialogue_Menu_Add_DONE_To_List(int answer) {
	debugC(kDebugScript, "Dialogue_Menu_Add_DONE_To_List(%d)", answer);
	_vm->_dialogueMenu->addToList(answer, true, 0, 0, 0);
	return false;
}

bool ScriptBase::Dialogue_Menu_Add_To_List_Never_Repeat_Once_Selected(int answer) {
	debugC(kDebugScript, "Dialogue_Menu_Add_To_List_Never_Repeat_Once_Selected(%d)", answer);
	return _vm->_dialogueMenu->addToListNeverRepeatOnceSelected(answer, 5, 5, 5);
}

bool ScriptBase::DM_Add_To_List(int answer, int priorityPolite, int priorityNormal, int prioritySurly) {
	debugC(kDebugScript, "DM_Add_To_List(%d, %d, %d, %d)", answer, priorityPolite, priorityNormal, prioritySurly);
	return _vm->_dialogueMenu->addToList(answer, false, priorityPolite, priorityNormal, prioritySurly);
}

bool ScriptBase::DM_Add_To_List_Never_Repeat_Once_Selected(int answer, int priorityPolite, int priorityNormal, int prioritySurly) {
	debugC(kDebugScript, "DM_Add_To_List_Never_Repeat_Once_Selected(%d, %d, %d, %d)", answer, priorityPolite, priorityNormal, prioritySurly);
	return _vm->_dialogueMenu->addToListNeverRepeatOnceSelected(answer, priorityPolite, priorityNormal, prioritySurly);
}

bool ScriptBase::Dialogue_Menu_Clear_Never_Repeat_Was_Selected_Flag(int answer) {
	debugC(kDebugScript, "Dialogue_Menu_Clear_Never_Repeat_Was_Selected_Flag(%d)", answer);
	return _vm->_dialogueMenu->clearNeverRepeatWasSelectedFlag(answer);
}

bool ScriptBase::Dialogue_Menu_Remove_From_List(int answer) {
	debugC(kDebugScript, "Dialogue_Menu_Remove_From_List(%d)", answer);
	return _vm->_dialogueMenu->removeFromList(answer);
}

int ScriptBase::Dialogue_Menu_Query_Input() {
	debugC(kDebugScript, "Dialogue_Menu_Query_Input()");
	return _vm->_dialogueMenu->queryInput();
}

int ScriptBase::Dialogue_Menu_Query_List_Size() {
	debugC(8, kDebugScript, "Dialogue_Menu_Query_List_Size()");
	return _vm->_dialogueMenu->listSize();
}

void ScriptBase::Scene_Exit_Add_2D_Exit(int index, int left, int top, int right, int down, int type) {
	debugC(kDebugScript, "Scene_Exit_Add_2D_Exit(%d, %d, %d, %d, %d, %d)", index, left, top, right, down, type);
	_vm->_scene->_exits->add(index, Common::Rect(left, top, right, down), type);
}

void ScriptBase::Scene_Exit_Remove(int index) {
	debugC(kDebugScript, "Scene_Exit_Remove(%d)", index);
	_vm->_scene->_exits->remove(index);
}

void ScriptBase::Scene_Exits_Disable() {
	debugC(kDebugScript, "Scene_Exits_Disable()");
	_vm->_scene->_exits->setEnabled(false);
}
void ScriptBase::Scene_Exits_Enable() {
	debugC(kDebugScript, "Scene_Exits_Enable()");
	_vm->_scene->_exits->setEnabled(true);
}

void ScriptBase::Scene_2D_Region_Add(int index, int left, int top, int right, int down) {
	debugC(kDebugScript, "Scene_2D_Region_Add(%d, %d, %d, %d, %d)", index, left, top, right, down);
	_vm->_scene->_regions->add(index, Common::Rect(left, top, right, down), 0);
}

void ScriptBase::Scene_2D_Region_Remove(int index) {
	debugC(kDebugScript, "Scene_2D_Region_Remove(%d)", index);
	_vm->_scene->_regions->remove(index);
}

void ScriptBase::World_Waypoint_Set(int waypointId, int setId, float x, float y, float z) {
	debugC(kDebugScript, "World_Waypoint_Set(%d, %d, %f, %f, %f)", waypointId, setId, x, y, z);
	_vm->_waypoints->set(waypointId, setId, Vector3(x, y, z));
}
// ScriptBase::World_Waypoint_Reset

float ScriptBase::World_Waypoint_Query_X(int waypointId) {
	debugC(8, kDebugScript, "World_Waypoint_Query_X(%d)", waypointId);
	return _vm->_waypoints->getX(waypointId);
}

float ScriptBase::World_Waypoint_Query_Y(int waypointId) {
	debugC(8, kDebugScript, "World_Waypoint_Query_Y(%d)", waypointId);
	return _vm->_waypoints->getY(waypointId);
}

float ScriptBase::World_Waypoint_Query_Z(int waypointId) {
	debugC(8, kDebugScript, "World_Waypoint_Query_Z(%d)", waypointId);
	return _vm->_waypoints->getZ(waypointId);
}

void ScriptBase::Combat_Cover_Waypoint_Set_Data(int coverWaypointId, int type, int setId, int sceneId, float x, float y, float z) {
	debugC(kDebugScript, "Combat_Cover_Waypoint_Set_Data(%d, %d, %d, %d, %f, %f, %f)", coverWaypointId, type, setId, sceneId, x, y, z);
	assert(coverWaypointId < (int)_vm->_combat->_coverWaypoints.size());

	_vm->_combat->_coverWaypoints[coverWaypointId].type = type;
	_vm->_combat->_coverWaypoints[coverWaypointId].setId = setId;
	_vm->_combat->_coverWaypoints[coverWaypointId].sceneId = sceneId;
	_vm->_combat->_coverWaypoints[coverWaypointId].position.x = x;
	_vm->_combat->_coverWaypoints[coverWaypointId].position.y = y;
	_vm->_combat->_coverWaypoints[coverWaypointId].position.z = z;
}

void ScriptBase::Combat_Flee_Waypoint_Set_Data(int fleeWaypointId, int type, int setId, int sceneId, float x, float y, float z, int a8) {
	debugC(kDebugScript, "Combat_Flee_Waypoint_Set_Data(%d, %d, %d, %d, %f, %f, %f)", fleeWaypointId, type, setId, sceneId, x, y, z);
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
	debugC(kDebugScript, "Police_Maze_Target_Track_Add(%d, %f, %f, %f, %f, %f, %f, %d, ptr, %d)", itemId, startX, startY, startZ, endX, endY, endZ, steps, isActive);
	_vm->_policeMaze->_tracks[itemId]->add(itemId, startX, startY, startZ, endX, endY, endZ, steps, instructions, isActive);
	_vm->_policeMaze->activate();
}

int ScriptBase::Police_Maze_Query_Score() {
	debugC(kDebugScript, "Police_Maze_Query_Score()");
	return Global_Variable_Query(kVariablePoliceMazeScore);
}

void ScriptBase::Police_Maze_Zero_Score() {
	debugC(kDebugScript, "Police_Maze_Zero_Score()");
	Global_Variable_Reset(kVariablePoliceMazeScore);
}

void ScriptBase::Police_Maze_Increment_Score(int delta) {
	debugC(kDebugScript, "Police_Maze_Increment_Score(%d)", delta);
	Global_Variable_Set(kVariablePoliceMazeScore, Global_Variable_Query(kVariablePoliceMazeScore) + delta);
}

void ScriptBase::Police_Maze_Decrement_Score(int delta) {
	debugC(kDebugScript, "Police_Maze_Decrement_Score(%d)", delta);
	Global_Variable_Set(kVariablePoliceMazeScore, Global_Variable_Query(kVariablePoliceMazeScore) - delta);
}

void ScriptBase::Police_Maze_Set_Score(int value) {
	debugC(kDebugScript, "Police_Maze_Set_Score(%d)", value);
	Global_Variable_Set(kVariablePoliceMazeScore, value);
}

void ScriptBase::Police_Maze_Set_Pause_State(bool state) {
	debugC(kDebugScript, "Police_Maze_Set_Pause_State(%d)", state);
	_vm->_policeMaze->setPauseState(state);
}

void ScriptBase::CDB_Set_Crime(int clueId, int crimeId) {
	debugC(kDebugScript, "CDB_Set_Crime(%d, %d)", clueId, crimeId);
	_vm->_crimesDatabase->setCrime(clueId, crimeId);
}

void ScriptBase::CDB_Set_Clue_Asset_Type(int clueId, int assetType) {
	debugC(kDebugScript, "CDB_Set_Clue_Asset_Type(%d, %d)", clueId, assetType);
	_vm->_crimesDatabase->setAssetType(clueId, assetType);
}

void ScriptBase::SDB_Set_Actor(int suspectId, int actorId) {
	debugC(kDebugScript, "SDB_Set_Actor(%d, %d)", suspectId, actorId);
	_vm->_suspectsDatabase->get(suspectId)->setActor(actorId);
}

bool ScriptBase::SDB_Add_Photo_Clue(int suspectId, int clueId, int shapeId) {
	debugC(kDebugScript, "SDB_Add_Photo_Clue(%d, %d, %d)", suspectId, clueId, shapeId);
	return _vm->_suspectsDatabase->get(suspectId)->addPhotoClue(shapeId, clueId);
}

void ScriptBase::SDB_Set_Name(int actorId) {
	// not implemented in game
}

void ScriptBase::SDB_Set_Sex(int suspectId, int sex) {
	debugC(kDebugScript, "SDB_Set_Sex(%d, %d)", suspectId, sex);
	_vm->_suspectsDatabase->get(suspectId)->setSex(sex);
}

bool ScriptBase::SDB_Add_Identity_Clue(int suspectId, int clueId) {
	debugC(kDebugScript, "SDB_Add_Identity_Clue(%d, %d)", suspectId, clueId);
	return _vm->_suspectsDatabase->get(suspectId)->addIdentityClue(clueId);
}

bool ScriptBase::SDB_Add_MO_Clue(int suspectId, int clueId) {
	debugC(kDebugScript, "SDB_Add_MO_Clue(%d, %d)", suspectId, clueId);
	return _vm->_suspectsDatabase->get(suspectId)->addMOClue(clueId);
}

bool ScriptBase::SDB_Add_Whereabouts_Clue(int suspectId, int clueId) {
	debugC(kDebugScript, "SDB_Add_Whereabouts_Clue(%d, %d)", suspectId, clueId);
	return _vm->_suspectsDatabase->get(suspectId)->addWhereaboutsClue(clueId);
}

bool ScriptBase::SDB_Add_Replicant_Clue(int suspectId, int clueId) {
	debugC(kDebugScript, "SDB_Add_Replicant_Clue(%d, %d)", suspectId, clueId);
	return _vm->_suspectsDatabase->get(suspectId)->addReplicantClue(clueId);
}

bool ScriptBase::SDB_Add_Non_Replicant_Clue(int suspectId, int clueId) {
	debugC(kDebugScript, "SDB_Add_Non_Replicant_Clue(%d, %d)", suspectId, clueId);
	return _vm->_suspectsDatabase->get(suspectId)->addNonReplicantClue(clueId);
}

bool ScriptBase::SDB_Add_Other_Clue(int suspectId, int clueId) {
	debugC(kDebugScript, "SDB_Add_Other_Clue(%d, %d)", suspectId, clueId);
	return _vm->_suspectsDatabase->get(suspectId)->addOtherClue(clueId);
}

void ScriptBase::Spinner_Set_Selectable_Destination_Flag(int destination, bool selectable) {
	debugC(kDebugScript, "Spinner_Set_Selectable_Destination_Flag(%d, %d)", destination, selectable);
	_vm->_spinner->setSelectableDestinationFlag(destination, selectable);
}

// ScriptBase::Spinner_Query_Selectable_Destination_Flag

int ScriptBase::Spinner_Interface_Choose_Dest(int loopId, bool immediately) {
	debugC(kDebugScript, "Spinner_Interface_Choose_Dest(%d, %d)", loopId, immediately);
	return _vm->_spinner->chooseDestination(loopId, immediately);
}

void ScriptBase::ESPER_Flag_To_Activate() {
	debugC(kDebugScript, "ESPER_Flag_To_Activate()");
	if (!_vm->_esper->isOpen()) {
		_vm->_esper->open(&_vm->_surfaceBack);
		while (_vm->_esper->isOpen() && _vm->_gameIsRunning) {
			_vm->gameTick();
		}
	}
}

void ScriptBase::Voight_Kampff_Activate(int actorId, int calibrationRatio) {
	debugC(kDebugScript, "Voight_Kampff_Activate(%d, %d)", actorId, calibrationRatio);
	_vm->_vk->open(actorId, calibrationRatio);
	while (_vm->_vk->isOpen() && _vm->_gameIsRunning) {
		_vm->gameTick();
	}
}

int ScriptBase::Elevator_Activate(int elevatorId) {
	debugC(kDebugScript, "Elevator_Activate(%d)", elevatorId);
	return _vm->_elevator->activate(elevatorId);
}

void ScriptBase::View_Score_Board() {
	debugC(kDebugScript, "View_Score_Board()");
	_vm->_scores->open();
}

int ScriptBase::Query_Score(int index) {
	debugC(8, kDebugScript, "Query_score(%d)", index);
	return _vm->_scores->query(index);
}

void ScriptBase::Set_Score(int index, int value) {
	debugC(kDebugScript, "Set_Score(%d, %d)", index, value);
	_vm->_scores->set(index, value);
}

void ScriptBase::Give_McCoy_Ammo(int ammoType, int ammo) {
	debugC(kDebugScript, "Give_McCoy_Ammo(%d, %d)", ammoType, ammo);
	_vm->_settings->addAmmo(ammoType, ammo);
}

void ScriptBase::Assign_Player_Gun_Hit_Sounds(int ammoType, int soundId1, int soundId2, int soundId3) {
	debugC(kDebugScript, "Assign_Player_Gun_Hit_Sounds(%d, %d, %d, %d)", ammoType, soundId1, soundId2, soundId3);
	_vm->_combat->setHitSound(ammoType, 0, soundId1);
	_vm->_combat->setHitSound(ammoType, 1, soundId2);
	_vm->_combat->setHitSound(ammoType, 2, soundId3);
}

void ScriptBase::Assign_Player_Gun_Miss_Sounds(int ammoType, int soundId1, int soundId2, int soundId3) {
	debugC(kDebugScript, "Assign_Player_Gun_Miss_Sounds(%d, %d, %d, %d)", ammoType, soundId1, soundId2, soundId3);
	_vm->_combat->setMissSound(ammoType, 0, soundId1);
	_vm->_combat->setMissSound(ammoType, 1, soundId2);
	_vm->_combat->setMissSound(ammoType, 2, soundId3);
}

void ScriptBase::Disable_Shadows(int animationsIdsList[], int listSize) {
	debugC(kDebugScript, "Disable_Shadows(list, %d)", listSize);
	_vm->_sliceRenderer->disableShadows(animationsIdsList, listSize);
}

bool ScriptBase::Query_System_Currently_Loading_Game() {
	debugC(8, kDebugScript, "Query_System_Currently_Loading_Game()");
	return _vm->_gameIsLoading;
}

void ScriptBase::Actor_Retired_Here(int actorId, int width, int height, bool retired, int retiredByActorId) {
	debugC(kDebugScript, "Actor_Retired_Here(%d, %d, %d, %d, %d)", actorId, width, height, retired, retiredByActorId);
	Actor *actor = _vm->_actors[actorId];
	actor->retire(retired, width, height, retiredByActorId);
	actor->setAtXYZ(actor->getXYZ(), actor->getFacing(), true, false, true);
	_vm->_sceneObjects->setRetired(actorId + kSceneObjectOffsetActors, true);
}

void ScriptBase::Clickable_Object(const char *objectName) {
	debugC(kDebugScript, "Clickable_Object(%s)", objectName);
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return;
	_vm->_scene->objectSetIsClickable(objectId, true, !_vm->_sceneIsLoading);
}

void ScriptBase::Unclickable_Object(const char *objectName) {
	debugC(kDebugScript, "Unclickable_Object(%s)", objectName);
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1) {
		return;
	}
	_vm->_scene->objectSetIsClickable(objectId, false, !_vm->_sceneIsLoading);
}

void ScriptBase::Obstacle_Object(const char *objectName, bool updateWalkpath) {
	debugC(kDebugScript, "Obstacle_Object(%s, %d)", objectName, updateWalkpath);
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return;
	_vm->_scene->objectSetIsObstacle(objectId, true, !_vm->_sceneIsLoading, !_vm->_sceneIsLoading && updateWalkpath);
}

void ScriptBase::Unobstacle_Object(const char *objectName, bool updateWalkpath) {
	debugC(kDebugScript, "Unobstacle_Object(%s, %d)", objectName, updateWalkpath);
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return;
	_vm->_scene->objectSetIsObstacle(objectId, false, !_vm->_sceneIsLoading, !_vm->_sceneIsLoading && updateWalkpath);
}

void ScriptBase::Obstacle_Flag_All_Objects(bool isObstacle) {
	debugC(kDebugScript, "Obstacle_Flag_All_Objects(%d)", isObstacle);
	_vm->_scene->objectSetIsObstacleAll(isObstacle, !_vm->_sceneIsLoading);
}

void ScriptBase::Combat_Target_Object(const char *objectName) {
	debugC(kDebugScript, "Combat_Target_Object(%s)", objectName);
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return;
	_vm->_scene->objectSetIsTarget(objectId, true, !_vm->_sceneIsLoading);
}

void ScriptBase::Un_Combat_Target_Object(const char *objectName) {
	debugC(kDebugScript, "Un_Combat_Target_Object(%s)", objectName);
	int objectId = _vm->_scene->findObject(objectName);
	if (objectId == -1)
		return;
	_vm->_scene->objectSetIsTarget(objectId, false, !_vm->_sceneIsLoading);
}

void ScriptBase::Set_Fade_Color(float r, float g, float b) {
	debugC(kDebugScript, "Set_Fade_Color(%f, %f, %f)", r, g, b);
	_vm->_scene->_set->_effects->setFadeColor(r, g, b);
}

void ScriptBase::Set_Fade_Density(float density) {
	debugC(kDebugScript, "Set_Fade_Density(%f)", density);
	_vm->_scene->_set->_effects->setFadeDensity(density);
}

void ScriptBase::Set_Fog_Color(const char *fogName, float r, float g, float b) {
	debugC(kDebugScript, "Set_Fog_Color(%s, %f, %f, %f)", fogName, r, g, b);
	_vm->_scene->_set->_effects->setFogColor(fogName, r, g, b);
}

void ScriptBase::Set_Fog_Density(const char *fogName, float density) {
	debugC(kDebugScript, "Set_Fog_Density(%s, %f)", fogName, density);
	_vm->_scene->_set->_effects->setFogDensity(fogName, density);
}

void ScriptBase::ADQ_Flush() {
	debugC(kDebugScript, "ADQ_Flush()");
	_vm->_actorDialogueQueue->flush(0, true);
}

void ScriptBase::ADQ_Add(int actorId, int sentenceId, int animationMode) {
	debugC(kDebugScript, "ADQ_Add(%d, %d, %d)", actorId, sentenceId, animationMode);
	_vm->_actorDialogueQueue->add(actorId, sentenceId, animationMode);
}

void ScriptBase::ADQ_Add_Pause(int32 delay) {
	debugC(kDebugScript, "ADQ_Add_Pause(%d)", delay);
	_vm->_actorDialogueQueue->addPause(delay);
}

void ScriptBase::ADQ_Wait_For_All_Queued_Dialogue() {
	debugC(kDebugScript, "ADQ_Wait_For_All_Queued_Dialogue()");
	_vm->loopQueuedDialogueStillPlaying();
}

bool ScriptBase::Game_Over() {
	debugC(kDebugScript, "Game_Over()");
	_vm->_gameIsRunning = false;
	_vm->_gameOver = true;
	return true;
}

void ScriptBase::Autosave_Game(int textId) {
	debugC(kDebugScript, "Autosave_Game(%d)", textId);
	_vm->_gameAutoSaveTextId = textId;
}

void ScriptBase::I_Sez(const char *str) {
	debugC(kDebugScript, "I_Sez(%s)", str);
	_vm->ISez(str);
}

void ScriptBase::AI_Countdown_Timer_Start(int actorId, signed int timer, int32 seconds) {
	debugC(kDebugScript, "AI_Countdown_Timer_Start(%d, %d, %d)", actorId, timer, seconds);
	if (timer >= 0 && timer <= 2) {
		_vm->_actors[actorId]->timerStart(timer, 1000 * seconds);
	}
}

void ScriptBase::AI_Countdown_Timer_Reset(int actorId, int timer) {
	debugC(kDebugScript, "AI_Countdown_Timer_Reset(%d, %d)", actorId, timer);
	if (timer >= 0 && timer <= 2) {
		_vm->_actors[actorId]->timerReset(timer);
	}
}

void ScriptBase::AI_Movement_Track_Unpause(int actorId) {
	debugC(kDebugScript, "AI_Movement_Track_Unpause(%d)", actorId);
	_vm->_actors[actorId]->movementTrackUnpause();
}

void ScriptBase::AI_Movement_Track_Pause(int actorId) {
	debugC(kDebugScript, "AI_Movement_Track_Pause(%d)", actorId);
	_vm->_actors[actorId]->movementTrackPause();
}

void ScriptBase::AI_Movement_Track_Repeat(int actorId) {
	debugC(kDebugScript, "AI_Movement_Track_Repeat(%d)", actorId);
	_vm->_actors[actorId]->_movementTrack->repeat();
	_vm->_actors[actorId]->movementTrackNext(true);
}

void ScriptBase::AI_Movement_Track_Append_Run_With_Facing(int actorId, int waypointId, int32 delay, int angle) {
	debugC(kDebugScript, "AI_Movement_Track_Append_Run_With_Facing(%d, %d, %d, %d)", actorId, waypointId, delay, angle);
	_vm->_actors[actorId]->_movementTrack->append(waypointId, delay * 1000, angle, true);
}

void ScriptBase::AI_Movement_Track_Append_With_Facing(int actorId, int waypointId, int32 delay, int angle) {
	debugC(kDebugScript, "AI_Movement_Track_Append_With_Facing(%d, %d, %d, %d)", actorId, waypointId, delay, angle);
	_vm->_actors[actorId]->_movementTrack->append(waypointId, delay * 1000, angle, false);
}

void ScriptBase::AI_Movement_Track_Append_Run(int actorId, int waypointId, int32 delay) {
	debugC(kDebugScript, "AI_Movement_Track_Append_Run(%d, %d, %d)", actorId, waypointId, delay);
	_vm->_actors[actorId]->_movementTrack->append(waypointId, delay * 1000, true);
}

void ScriptBase::AI_Movement_Track_Append(int actorId, int waypointId, int32 delay) {
	debugC(kDebugScript, "AI_Movement_Track_Append(%d, %d, %d)", actorId, waypointId, delay);
	_vm->_actors[actorId]->_movementTrack->append(waypointId, delay * 1000, false);
}

void ScriptBase::AI_Movement_Track_Flush(int actorId) {
	debugC(kDebugScript, "AI_Movement_Track_Flush(%d)", actorId);
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
