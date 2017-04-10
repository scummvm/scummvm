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

#include "bladerunner/bladerunner.h"

#include "bladerunner/actor.h"
#include "bladerunner/actor_combat.h"
#include "bladerunner/adq.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/crimes_database.h"
#include "bladerunner/combat.h"
#include "bladerunner/gameflags.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/items.h"
#include "bladerunner/item_pickup.h"
#include "bladerunner/movement_track.h"
#include "bladerunner/settings.h"
#include "bladerunner/set_effects.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/slice_animations.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/vector.h"
#include "bladerunner/waypoints.h"

namespace BladeRunner {

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
	return _vm->_actors[actorId]->_friendlinessToOther[otherActorId];
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
	return _vm->_actors[actorId]->_currentHP;
}

int ScriptBase::Actor_Query_Max_HP(int actorId) {
	return _vm->_actors[actorId]->_maxHP;
}

int ScriptBase::Actor_Query_Combat_Aggressiveness(int actorId) {
	return _vm->_actors[actorId]->_combatAggressiveness;
}

int ScriptBase::Actor_Query_Honesty(int actorId) {
	return _vm->_actors[actorId]->_honesty;
}

int ScriptBase::Actor_Query_Intelligence(int actorId) {
	return _vm->_actors[actorId]->_intelligence;
}

int ScriptBase::Actor_Query_Stability(int actorId) {
	return _vm->_actors[actorId]->_stability;
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

void ScriptBase::Non_Player_Actor_Combat_Mode_On(int actorId, int a2, int a3, int otherActorId, int a5, int animationModeCombatIdle, int animationModeCombatWalk, int animationModeCombatRun, int a9, int a10, int a11, int a12, int a13, int a14) {
	_vm->_actors[actorId]->combatModeOn(a2, a3, otherActorId, a5, animationModeCombatIdle, animationModeCombatWalk, animationModeCombatRun, a9, a10, a11, a12, a13, a14);
}

void ScriptBase::Non_Player_Actor_Combat_Mode_Off(int actorId) {
	_vm->_actors[actorId]->combatModeOff();
}

void ScriptBase::Actor_Set_Health(int actorId, int hp, int maxHp) {
	_vm->_actors[actorId]->setHealth(hp, maxHp);
}

void ScriptBase::Actor_Set_Targetable(int actorId, bool targetable) {
	_vm->_actors[actorId]->setTargetable(targetable);

}

void ScriptBase::Actor_Says(int actorId, int sentenceId, int animationMode){
	_vm->loopActorSpeaking();
	_vm->_adq->flush(1, true);
	Actor_Says_With_Pause(actorId, sentenceId, 0.5f, animationMode);
}

void ScriptBase::Actor_Says_With_Pause(int actorId, int sentenceId, float pause, int animationMode) {
	_vm->gameWaitForActive();
	_vm->loopActorSpeaking();
	_vm->_adq->flush(1, true);

	Actor *actor = _vm->_actors[actorId];

	if(animationMode != -1) {
		actor->stopWalking(false);
	}

	actor->speechPlay(sentenceId, false);
	bool animationModeChanged = false;
	if(animationMode >= 0) {
		if (actorId != 0) {
			actor->changeAnimationMode(animationMode, false);
			animationModeChanged = true;
		} else if(_vm->_combat->isActive()) {
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

#if 0
void ScriptBase::Actor_Voice_Over(int sentenceId, int actorId) {
	// Wait for any existing speech to end
	_vm->loopActorSpeaking();

	// TODO: Hack - This needs to go through the actor class
	char name[13];
	sprintf(name, "%02d-%04d.AUD", actorId, sentenceId);
	_vm->_audioSpeech->playSpeech(name);

	// warning("start voice over loop");
	while (true)
	{
		_vm->gameTick();
		if (_vm->shouldQuit())
			break;
		if (!_vm->_audioSpeech->isPlaying())
			break;
	}
	// warning("end voice over loop");
}
#endif

void ScriptBase::Actor_Voice_Over(int sentenceId, int actorId) {
	assert(actorId < ACTORS_COUNT);

	_vm->gameWaitForActive();
	_vm->loopActorSpeaking();
	_vm->_adq->flush(1, true);

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
	_vm->_actors[VOICEOVER_ACTOR]->speechPlay(sentenceId, true);
}

int ScriptBase::Actor_Query_Which_Set_In(int actorId) {
	return _vm->_actors[actorId]->getSetId();
}

bool ScriptBase::Actor_Query_Is_In_Current_Set(int actorId) {
	int actorSetId = _vm->_actors[actorId]->getSetId();
	return actorSetId >= 0 && _vm->_scene->getSetId();
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

	return sqrtf(distX * distX + distZ * distZ);
}

bool ScriptBase::Actor_Query_In_Between_Two_Actors(int actorId, int otherActor1Id, int otherActor2Id) {
	float x1 = _vm->_actors[otherActor1Id]->getX();
	float z1 = _vm->_actors[otherActor1Id]->getZ();
	float x2 = _vm->_actors[otherActor2Id]->getX();
	float z2 = _vm->_actors[otherActor2Id]->getZ();
	return _vm->_sceneObjects->isBetweenTwoXZ(actorId + SCENE_OBJECTS_ACTORS_OFFSET, x1, z1, x2, z1)
		|| _vm->_sceneObjects->isBetweenTwoXZ(actorId + SCENE_OBJECTS_ACTORS_OFFSET, x1 - 12.0f, z1 - 12.0f, x2 - 12.0f, z2 - 12.0f)
		|| _vm->_sceneObjects->isBetweenTwoXZ(actorId + SCENE_OBJECTS_ACTORS_OFFSET, x1 + 12.0f, z1 - 12.0f, x2 + 12.0f, z2 - 12.0f)
		|| _vm->_sceneObjects->isBetweenTwoXZ(actorId + SCENE_OBJECTS_ACTORS_OFFSET, x1 + 12.0f, z1 + 12.0f, x2 + 12.0f, z2 + 12.0f)
		|| _vm->_sceneObjects->isBetweenTwoXZ(actorId + SCENE_OBJECTS_ACTORS_OFFSET, x1 - 12.0f, z1 + 12.0f, x2 - 12.0f, z2 + 12.0f);
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

bool ScriptBase::Loop_Actor_Walk_To_Actor(int actorId, int otherActorId, int distance, int a4, bool run) {
	_vm->gameWaitForActive();

	if (actorId == _vm->_walkingActorId) {
		run = true;
	}
	_vm->_playerActorIdle = false;
	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToActor(otherActorId, distance, a4, run, true, &isRunning);
	if (_vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
	if (isRunning == 1) {
		_vm->_walkingActorId = actorId;
	}
	Global_Variable_Set(37, actorId);
	Global_Variable_Set(38, isRunning);
	return result;
}

bool ScriptBase::Loop_Actor_Walk_To_Item(int actorId, int itemId, int a3, int a4, bool run) {
	_vm->gameWaitForActive();

	if (_vm->_walkingActorId == actorId) {
		run = true;
	}
	_vm->_playerActorIdle = false;
	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToItem(itemId, a3, a4, run, true, &isRunning);
	if (_vm->_playerActorIdle == 1) {
		result = true;
		_vm->_playerActorIdle = false;
	}
	if (isRunning == 1) {
		_vm->_walkingActorId = actorId;
	}
	Global_Variable_Set(37, actorId);
	Global_Variable_Set(38, isRunning);
	return result;
}

bool ScriptBase::Loop_Actor_Walk_To_Scene_Object(int actorId, const char *objectName, int destinationOffset, bool a4, bool run) {
	_vm->gameWaitForActive();

	if (_vm->_walkingActorId == actorId) {
		run = true;
	}
	_vm->_playerActorIdle = false;
	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToSceneObject(objectName, destinationOffset, a4, run, true, &isRunning);
	if (_vm->_playerActorIdle) {
		result = true;
		_vm->_playerActorIdle = false;
	}
	if (isRunning == 1) {
		_vm->_walkingActorId = actorId;
	}
	Global_Variable_Set(37, actorId);
	Global_Variable_Set(38, isRunning);
	return result;
}

bool ScriptBase::Loop_Actor_Walk_To_Waypoint(int actorId, int waypointId, int destinationOffset, int a4, bool run) {
	//TODO
	warning("Loop_Actor_Walk_To_Waypoint(%d, %d, %d, %d, %d)", actorId, waypointId, destinationOffset, a4, run);
	return false;
}

bool ScriptBase::Loop_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int destinationOffset, int a5, bool run, int a7) {
	_vm->gameWaitForActive();

	if(_vm->_walkingActorId == actorId) {
		if(a7) {
			_vm->_walkingActorId = -1;
		} else {
			run = true;
		}
	}
	//TODO:
	//PlayerActorIdle = 0;
	bool isRunning;
	bool result = _vm->_actors[actorId]->loopWalkToXYZ(Vector3(x, y, z), destinationOffset, a5, run, 1, &isRunning);

//	if (PlayerActorIdle == 1) {
//		result = 1;
//		PlayerActorIdle = 0;
//	}
	if(isRunning) {
		_vm->_walkingActorId = actorId;
	}
	Global_Variable_Set(37, actorId);
	Global_Variable_Set(38, isRunning);
	return result;
}

void ScriptBase::Async_Actor_Walk_To_Waypoint(int actorId, int waypointId, int destinationOffset, int run) {
	//TODO
	warning("Async_Actor_Walk_To_Waypoint(%d, %d, %d, %d)", actorId, waypointId, destinationOffset, run);
}

void ScriptBase::Async_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int destinationOffset, bool run) {
	//TODO
	warning("Async_Actor_Walk_To_XYZ(%d, %f, %f, %f, %d, %d)", actorId, x, y, z, destinationOffset, run);
}

void ScriptBase::Actor_Force_Stop_Walking(int actorId) {
	//TODO
	warning("Loop_Actor_Travel_Stairs(%d)", actorId);
}

bool ScriptBase::Loop_Actor_Travel_Stairs(int actorId, int a2, int a3, int a4) {
	//TODO
	warning("Loop_Actor_Travel_Stairs(%d, %d, %d, %d)", actorId, a2, a3, a4);
	return false;
}

bool ScriptBase::Loop_Actor_Travel_Ladder(int actorId, int a2, int a3, int a4) {
	//TODO
	warning("Loop_Actor_Travel_Ladder(%d, %d, %d, %d)", actorId,a2,a3,a4);
	return false;
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
	_vm->_actors[actorId]->copyClues(VOICEOVER_ACTOR);
}

void ScriptBase::Actor_Clues_Transfer_New_From_Mainframe(int actorId) {
	_vm->_actors[VOICEOVER_ACTOR]->copyClues(actorId);
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
	warning("Item_Spin_In_World(%d)", itemId);
}

void ScriptBase::Item_Flag_As_Target(int itemId) {
	warning("Item_Flag_As_Target(%d)", itemId);
}

void ScriptBase::Item_Flag_As_Non_Target(int itemId) {
	warning("Item_Flag_As_Non_Target(%d)", itemId);
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
	int endTime = _vm->getTotalPlayTime() + miliseconds;
	while ((int)_vm->getTotalPlayTime() < endTime)
		_vm->gameTick();
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
	const char *name = _vm->_gameInfo->getSfxTrack(id);
	_vm->_audioPlayer->playAud(name, volume, panFrom, panTo, priority);
}

void ScriptBase::Sound_Play_Speech_Line(int actorId, int speechId, int a3, int a4, int a5) {
	//TODO
	warning("Sound_Play_Speech_Line(%d, %d, %d, %d, %d)", actorId, speechId, a3, a4, a5);
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

bool ScriptBase::Music_Play(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
	//TODO
	warning("Music_Play(%d, %d, %d, %d, %d, %d, %d)", a1,  a2,  a3,  a4,  a5,  a6,  a7);
	return false;
}

void ScriptBase::Music_Adjust(int a1, int a2, int a3) {
	//TODO
	warning("Music_Adjust(%d, %d, %d)", a1, a2, a3);
}

void ScriptBase::Music_Stop(int a1) {
	//TODO
	warning("Music_Stop(%d)", a1);
}

bool ScriptBase::Music_Is_Playing() {
	//TODO
	warning("Music_Is_Playing()");
	return false;
}

void ScriptBase::Overlay_Play(const char *overlay, int a2, int a3, int a4, int a5) {
	//TODO
	warning("Overlay_Play(%s, %d, %d, %d, %d)", overlay, a2, a3, a4, a5);
}

void ScriptBase::Overlay_Remove(const char *overlay) {
	//TODO
	warning("Overlay_Remove(%s)", overlay);
}

void ScriptBase::Scene_Loop_Set_Default(int loopId) {
	_vm->_scene->loopSetDefault(loopId);
}

void ScriptBase::Scene_Loop_Start_Special(int sceneLoopMode, int loopId, int c) {
	if (sceneLoopMode == 1) {
		c = 1;
	}
	_vm->_scene->loopStartSpecial(sceneLoopMode, loopId, c);
	if (sceneLoopMode == 1) {
		_vm->_settings->clearNewSetAndScene();
	}
}

void ScriptBase::Outtake_Play(int id, int noLocalization, int container) {
	_vm->outtakePlay(id, noLocalization, container);
}

void ScriptBase::Ambient_Sounds_Add_Sound(int id, int time1, int time2, int volume1, int volume2, int pan1begin, int pan1end, int pan2begin, int pan2end, int priority, int unk) {
	_vm->_ambientSounds->addSound(id, time1, time2, volume1, volume2, pan1begin, pan1end, pan2begin, pan2end, priority, unk);
}

void  ScriptBase::Ambient_Sounds_Remove_Sound(int id, bool a2) {
	//TODO
	warning("Ambient_Sounds_Remove_Sound(%d, %d)", id, a2);
}

void ScriptBase::Ambient_Sounds_Add_Speech_Sound(int id, int unk1, int time1, int time2, int volume1, int volume2, int pan1begin, int pan1end, int pan2begin, int pan2end, int priority, int unk2){
	//TODO
	warning("Ambient_Sounds_Add_Speech_Sound(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)", id, unk1, time1, time2, volume1, volume2, pan1begin, pan1end, pan2begin, pan2end, priority, unk2);
}

// ScriptBase::Ambient_Sounds_Remove_Speech_Sound

int ScriptBase::Ambient_Sounds_Play_Sound(int a1, int a2, int a3, int a4, int a5) {
	//TODO
	warning("Ambient_Sounds_Remove_Sound(%d, %d, %d, %d, %d)", a1, a2, a3, a4, a5);
	return -1;
}

// ScriptBase::Ambient_Sounds_Play_Speech_Sound

void ScriptBase::Ambient_Sounds_Remove_All_Non_Looping_Sounds(int time) {
	//TODO
	warning("Ambient_Sounds_Remove_All_Non_Looping_Sounds(%d)", time);
	// _vm->_ambientSounds->removeAllNonLoopingSounds(time);
}

void ScriptBase::Ambient_Sounds_Add_Looping_Sound(int id, int volume, int pan, int fadeInTime) {
	_vm->_ambientSounds->addLoopingSound(id, volume, pan, fadeInTime);
}

void ScriptBase::Ambient_Sounds_Adjust_Looping_Sound(int id, int panBegin, int panEnd, int a4) {
	//TODO
	warning("Ambient_Sounds_Adjust_Looping_Sound(%d, %d, %d, %d)", id, panBegin, panEnd, a4);
}

void ScriptBase::Ambient_Sounds_Remove_Looping_Sound(int id, bool a2){
	//TODO
	warning("Ambient_Sounds_Remove_Looping_Sound(%d, %d)", id, a2);
}

void ScriptBase::Ambient_Sounds_Remove_All_Looping_Sounds(int time) {
	//TODO
	warning("Ambient_Sounds_Remove_All_Looping_Sounds(%d)", time);
	// _vm->_ambientSounds->removeAllLoopingSounds(time);
}

void ScriptBase::Setup_Scene_Information(float actorX, float actorY, float actorZ, int actorFacing) {
	_vm->_scene->setActorStart(Vector3(actorX, actorY, actorZ), actorFacing);
}

bool ScriptBase::Dialogue_Menu_Appear(int x, int y) {
	//TODO
	warning("Dialogue_Menu_Appear(%d, %d)", x, y);
	return false;
}

bool ScriptBase::Dialogue_Menu_Disappear() {
	//TODO
	warning("Dialogue_Menu_Disappear()");
	return false;
}

bool ScriptBase::Dialogue_Menu_Clear_List() {
	//TODO
	warning("Dialogue_Menu_Clear_List()");
	return false;
}

bool ScriptBase::Dialogue_Menu_Add_To_List(int answer) {
	//TODO
	warning("Dialogue_Menu_Add_To_List(%d)", answer);
	return false;
}

bool ScriptBase::Dialogue_Menu_Add_DONE_To_List(int answerValue) {
	//TODO
	warning("Dialogue_Menu_Add_DONE_To_List(%d)", answerValue);
	return false;
}

// Dialogue_Menu_Add_To_List_Never_Repeat_Once_Selected

bool ScriptBase::DM_Add_To_List(int answer, int a2, int a3, int a4) {
	//TODO
	warning("DM_Add_To_List(%d, %d, %d, %d)", answer, a2, a3, a4);
	return false;
}

bool ScriptBase::DM_Add_To_List_Never_Repeat_Once_Selected(int answer, int a2, int a3, int a4) {
	//TODO
	warning("DM_Add_To_List_Never_Repeat_Once_Selected(%d, %d, %d, %d)", answer, a2, a3, a4);
	return false;
}

void ScriptBase::Dialogue_Menu_Remove_From_List(int answer) {
	//TODO
	warning("Dialogue_Menu_Remove_From_List(%d)", answer);
}

int ScriptBase::Dialogue_Menu_Query_Input() {
	//TODO
	warning("Dialogue_Menu_Query_Input()");
	return 0;
}

int ScriptBase::Dialogue_Menu_Query_List_Size() {
	//TODO
	warning("Dialogue_Menu_Query_List_Size()");
	return 0;
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

void ScriptBase::Combat_Cover_Waypoint_Set_Data(int combatCoverId, int type, int setId, int sceneId, float x, float y, float z) {
	//TODO
	warning("Combat_Cover_Waypoint_Set_Data(%d, %d, %d, %d, %f, %f, %f)", combatCoverId, type, setId, sceneId, x, y, z);
}

void ScriptBase::Combat_Flee_Waypoint_Set_Data(int combatFleeWaypointId, int type, int setId, int sceneId, float x, float y, float z, int a8) {
	//TODO
	warning("Combat_Cover_Waypoint_Set_Data(%d, %d, %d, %d, %f, %f, %f, %d)", combatFleeWaypointId, type, setId, sceneId, x, y, z, a8);
}

void ScriptBase::Police_Maze_Target_Track_Add(int itemId, float startX, float startY, float startZ, float endX, float endY, float endZ, int steps, signed int data[], bool a10) {
	//TODO
	warning("Police_Maze_Target_Track_Add(%d, %f, %f, %f, %f, %f, %f, %d, %p, %d)", itemId,  startX,  startY,  startZ,  endX,  endY,  endZ,  steps,  (void *)data,  a10);

}

// ScriptBase::Police_Maze_Query_Score
// ScriptBase::Police_Maze_Zero_Score
// ScriptBase::Police_Maze_Increment_Score
// ScriptBase::Police_Maze_Decrement_Score
// ScriptBase::Police_Maze_Set_Score

void ScriptBase::Police_Maze_Set_Pause_State(int a1) {
	//TODO
	warning("Police_Maze_Set_Pause_State(%d)", a1);
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

void ScriptBase::Spinner_Set_Selectable_Destination_Flag(int a1, int a2) {
	//TODO
	warning("Spinner_Set_Selectable_Destination_Flag(%d, %d)", a1, a2);
}

// ScriptBase::Spinner_Query_Selectable_Destination_Flag

int ScriptBase::Spinner_Interface_Choose_Dest(int a1, int a2) {
	//TODO
	warning("Spinner_Interface_Choose_Dest(%d, %d)", a1, a2);
	return -1;
}

// ScriptBase::Spinner_Set_Selectable_Destination_Flag
// ScriptBase::Spinner_Query_Selectable_Destination_Flag
// ScriptBase::Spinner_Interface_Choose_Dest

void ScriptBase::ESPER_Flag_To_Activate() {
	//TODO
	warning("ESPER_Flag_To_Activate()");
}

bool ScriptBase::Voight_Kampff_Activate(int a1, int a2){
	//TODO
	warning("Voight_Kampff_Activate(%d, %d)", a1, a2);
	return false;
}

int ScriptBase::Elevator_Activate(int elevator) {
	//TODO
	warning("Elevator_Activate(%d)", elevator);
	return 0;
}

void ScriptBase::View_Score_Board() {
	//TODO
	warning("View_Score_Board()");
}
// ScriptBase::Query_Score

void ScriptBase::Set_Score(int a0, int a1) {
	// debug("STUB: Set_Score(%d, %d)", a0, a1);
}

void ScriptBase::Give_McCoy_Ammo(int ammoType, int ammo) {
	_vm->_settings->addAmmo(ammoType, ammo);
}

void ScriptBase::Assign_Player_Gun_Hit_Sounds(int row, int soundId1, int soundId2, int soundId3) {
	_vm->_combat->setHitSoundId(row, 0, soundId1);
	_vm->_combat->setHitSoundId(row, 1, soundId2);
	_vm->_combat->setHitSoundId(row, 2, soundId3);
}

void ScriptBase::Assign_Player_Gun_Miss_Sounds(int row, int soundId1, int soundId2, int soundId3) {
	_vm->_combat->setMissSoundId(row, 0, soundId1);
	_vm->_combat->setMissSoundId(row, 1, soundId2);
	_vm->_combat->setMissSoundId(row, 2, soundId3);
}

void ScriptBase::Disable_Shadows(int animationsIdsList[], int listSize) {
	_vm->_sliceRenderer->disableShadows(animationsIdsList, listSize);
}

bool ScriptBase::Query_System_Currently_Loading_Game() {
	return _vm->_gameIsLoading;
}

void ScriptBase::Actor_Retired_Here(int actorId, int width, int height, int retired, int retiredByActorId) {
	Actor *actor = _vm->_actors[actorId];
	Vector3 actorPosition;
	actor->getXYZ(&actorPosition.x, &actorPosition.y, &actorPosition.z);
	actor->retire(retired, width, height, retiredByActorId);
	actor->setAtXYZ(actorPosition, actor->getFacing(), true, 0, true);
	_vm->_sceneObjects->setRetired(actorId + SCENE_OBJECTS_ACTORS_OFFSET, true);
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

void ScriptBase::Set_Fog_Color(const char* fogName, float r, float g, float b) {
	_vm->_scene->_set->_effects->setFogColor(fogName, r, g, b);
}

void ScriptBase::Set_Fog_Density(const char* fogName, float density) {
	_vm->_scene->_set->_effects->setFogDensity(fogName, density);
}

void ScriptBase::ADQ_Flush() {
	_vm->_adq->flush(0, true);
}

void ScriptBase::ADQ_Add(int actorId, int sentenceId, int animationMode) {
	_vm->_adq->add(actorId, sentenceId, animationMode);
}

void ScriptBase::ADQ_Add_Pause(int delay) {
	_vm->_adq->addPause(delay);
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
		_vm->_actors[actorId]->countdownTimerStart(timer, 1000 * seconds);
	}
}

void ScriptBase::AI_Countdown_Timer_Reset(int actorId, int timer) {
	if (timer >= 0 && timer <= 2) {
		_vm->_actors[actorId]->countdownTimerReset(timer);
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
	_vm->_actors[actorId]->_movementTrack->append(waypointId, delay, angle, 1);
}

void ScriptBase::AI_Movement_Track_Append_With_Facing(int actorId, int waypointId, int delay, int angle) {
	_vm->_actors[actorId]->_movementTrack->append(waypointId, delay, angle, 0);
}

void ScriptBase::AI_Movement_Track_Append_Run(int actorId, int waypointId, int delay) {
	_vm->_actors[actorId]->_movementTrack->append(waypointId, delay, 1);
}

void ScriptBase::AI_Movement_Track_Append(int actorId, int waypointId, int delay) {
	_vm->_actors[actorId]->_movementTrack->append(waypointId, delay, 0);
}

void ScriptBase::AI_Movement_Track_Flush(int actorId) {
	_vm->_actors[actorId]->_movementTrack->flush();
	_vm->_actors[actorId]->stopWalking(false);
}

void ScriptBase::KIA_Play_Actor_Dialogue(int a1, int a2) {
	//TODO
	warning("KIA_Play_Actor_Dialogue(%d, %d)", a1, a2);
}

void ScriptBase::KIA_Play_Slice_Model(int a1) {
	//TODO
	warning("KIA_Play_Slice_Model(%d)", a1);
}

void ScriptBase::KIA_Play_Photograph(int a1) {
	//TODO
	warning("KIA_Play_Photograph(%d)", a1);
}

void ScriptBase::ESPER_Add_Photo(const char* fileName, int a2, int a3) {
	//TODO
	warning("ESPER_Add_Photo(%s, %d, %d)", fileName, a2, a3);
}

void ScriptBase::ESPER_Define_Special_Region(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, const char *name) {
	//TODO
	warning("ESPER_Define_Special_Region(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %s)",  a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,  a11,  a12,  a13, name);
}

void ScriptBase::VK_Add_Question(int a1, int a2, int a3) {
	//TODO
	warning("VK_Add_Question(%d, %d, %d)", a1, a2, a3);
}

void ScriptBase::VK_Eye_Animates(int a1) {
	//TODO
	warning("VK_Eye_Animates(%d)", a1);
}

void ScriptBase::VK_Subject_Reacts(int a1, int a2, int a3, int a4) {
	//TODO
	warning("VK_Subject_Reacts(%d, %d, %d, %d)", a1, a2, a3, a4);
}

void ScriptBase::VK_Play_Speech_Line(int actorIndex, int a2, float a3) {
	//TODO
	warning("VK_Play_Speech_Line(%d, %d, %g)", actorIndex, a2, a3);
}

} // End of namespace BladeRunner
