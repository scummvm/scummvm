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
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/clues.h"
#include "bladerunner/combat.h"
#include "bladerunner/gameflags.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/settings.h"
#include "bladerunner/set_effects.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/slice_animations.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/vector.h"
#include "bladerunner/waypoints.h"

#include "bladerunner/script/ai_00_mccoy.h"
#include "bladerunner/script/aiscript_officer_leroy.h"

namespace BladeRunner {

bool Script::open(const Common::String &name) {
	delete _currentScript;

	if (name == "RC01") { _currentScript = new ScriptRC01(_vm); return true; }

	return false;
}

Script::~Script() {
	delete _currentScript;
}

void Script::InitializeScene() {
	_inScriptCounter++;
	_currentScript->InitializeScene();
	_inScriptCounter--;
}

void Script::SceneLoaded() {
	_inScriptCounter++;
	_currentScript->SceneLoaded();
	_inScriptCounter--;
}

bool Script::ClickedOn3DObject(const char *objectName) {
	if (_inScriptCounter > 0)
		return true;

	_inScriptCounter++;
	bool result = _currentScript->ClickedOn3DObject(objectName);
	_inScriptCounter--;
	return result;
}

bool Script::ClickedOn2DRegion(int region) {
	if (_inScriptCounter > 0)
		return true;

	_inScriptCounter++;
	bool result = _currentScript->ClickedOn2DRegion(region);
	_inScriptCounter--;
	return result;
}

void Script::SceneFrameAdvanced(int frame) {
	_inScriptCounter++;
	_currentScript->SceneFrameAdvanced(frame);
	_inScriptCounter--;
}

void Script::SceneActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) {
	_inScriptCounter++;
	_currentScript->SceneActorChangedGoal(actorId, newGoal, oldGoal, currentSet);
	_inScriptCounter--;
}

void Script::PlayerWalkedIn() {
	_inScriptCounter++;
	_currentScript->PlayerWalkedIn();
	_inScriptCounter--;
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
	_vm->_actors[actorId]->setAtWaypoint(waypointId, angle, 0, 0);
}

bool ScriptBase::Region_Check(int left, int top, int right, int down) {
	//TODO: return _vm->_mouse.x >= left && _vm->_mouse.y >= top && _vm->_mouse.x <= right && _vm->_mouse.y <= down;
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

void ScriptBase::Actor_Face_Heading(int actorId, int heading) {
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

void ScriptBase::Non_Player_Actor_Combat_Mode_On(int actorId, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, int a14) {
	_vm->_actors[actorId]->combatModeOn(a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14);
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
	//_vm->ADQ->flush(1,1)
	Actor_Says_With_Pause(actorId, sentenceId, 0.5f, animationMode);
}

void ScriptBase::Actor_Says_With_Pause(int actorId, int sentenceId, float pause, int animationMode) {
	_vm->gameWaitForActive();
	_vm->loopActorSpeaking();
	//_vm->ADQ->flush(1,1)

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
		actor->changeAnimationMode(0, false);
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
	_vm->gameWaitForActive();
	_vm->loopActorSpeaking();
	//_vm->ADQ->flush(1,1)

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
	_vm->_voiceoverActor->speechPlay(sentenceId, true);
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
	if (_vm->_actors[actorId]->getSetId() != _vm->_actors[otherActorId]->getSetId())
		return 0.0f;
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
	return _vm->_sceneObjects->isBetweenTwoXZ(actorId, x1, z1, x2, z1)
		|| _vm->_sceneObjects->isBetweenTwoXZ(actorId, x1 - 12.0f, z1 - 12.0f, x2 - 12.0f, z2 - 12.0f)
		|| _vm->_sceneObjects->isBetweenTwoXZ(actorId, x1 + 12.0f, z1 - 12.0f, x2 + 12.0f, z2 - 12.0f)
		|| _vm->_sceneObjects->isBetweenTwoXZ(actorId, x1 + 12.0f, z1 + 12.0f, x2 + 12.0f, z2 + 12.0f)
		|| _vm->_sceneObjects->isBetweenTwoXZ(actorId, x1 - 12.0f, z1 + 12.0f, x2 - 12.0f, z2 + 12.0f);
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
	_vm->_actors[actorId]->changeAnimationMode(animationMode, 0);
}

int ScriptBase::Actor_Query_Animation_Mode(int actorId) {
	return _vm->_actors[actorId]->getAnimationMode();
}

// ScriptBase::Loop_Actor_Walk_To_Actor
// ScriptBase::Loop_Actor_Walk_To_Item

bool ScriptBase::Loop_Actor_Walk_To_Scene_Object(int actorId, const char *objectName, int distance, int a4, int a5) {
	_vm->gameWaitForActive();

	_vm->_actors[actorId]->loopWalkToSceneObject(objectName);

	return false;
}

// ScriptBase::Loop_Actor_Walk_To_Waypoint

void ScriptBase::Loop_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int a4, int a5, int a6, int a7) {
	_vm->loopActorWalkToXYZ(actorId, x, y, z, a4, a5, a6, a7);
}

// ScriptBase::Async_Actor_Walk_To_Waypoint
// ScriptBase::Async_Actor_Walk_To_XYZ
// ScriptBase::Actor_Force_Stop_Walking
// ScriptBase::Loop_Actor_Travel_Stairs
// ScriptBase::Loop_Actor_Travel_Ladder

void ScriptBase::Actor_Clue_Add_To_Database(int actorId, int clueId, int unknown, bool clueAcquired, bool unknownFlag, int fromActorId) {
	_vm->_actors[actorId]->addClueToDatabase(clueId, unknown, clueAcquired, unknownFlag, fromActorId);
}

void ScriptBase::Actor_Clue_Acquire(int actorId, int clueId, byte unknownFlag, int fromActorId) {
	_vm->_actors[actorId]->acquireClue(clueId, unknownFlag, fromActorId);
}

void ScriptBase::Actor_Clue_Lose(int actorId, int clueId) {
	_vm->_actors[actorId]->loseClue(clueId);
}

bool ScriptBase::Actor_Clue_Query(int actorId, int clueId) {
	return _vm->_actors[actorId]->hasClue(clueId);
}

void ScriptBase::Actor_Clues_Transfer_New_To_Mainframe(int actorId) {
	_vm->_actors[actorId]->copyClues(99);
}

void ScriptBase::Actor_Clues_Transfer_New_From_Mainframe(int actorId) {
	_vm->_voiceoverActor->copyClues(actorId);
}

void ScriptBase::Actor_Set_Invisible(int actorId, bool isInvisible) {
	_vm->_actors[actorId]->setInvisible(isInvisible);
}

void ScriptBase::Actor_Set_Immunity_To_Obstacles(int actorId, bool isImmune) {
	_vm->_actors[actorId]->setImmunityToObstacles(isImmune);
}

// ScriptBase::Item_Add_To_World
// ScriptBase::Item_Remove_From_World
// ScriptBase::Item_Spin_In_World
// ScriptBase::Item_Flag_As_Target
// ScriptBase::Item_Flag_As_Non_Target
// ScriptBase::Item_Pickup_Spin_Effect

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

// ScriptBase::Sound_Play_Speech_Line
// ScriptBase::Sound_Left_Footstep_Walk
// ScriptBase::Sound_Right_Footstep_Walk
// ScriptBase::Sound_Left_Footstep_Run
// ScriptBase::Sound_Right_Footstep_Run
// ScriptBase::Sound_Walk_Shuffle_Stop
// ScriptBase::Footstep_Sounds_Set
// ScriptBase::Footstep_Sound_Override_On
// ScriptBase::Footstep_Sound_Override_Off
// ScriptBase::Music_Play
// ScriptBase::Music_Adjust
// ScriptBase::Music_Stop
// ScriptBase::Music_Is_Playing
// ScriptBase::Overlay_Play
// ScriptBase::Overlay_Remove

void ScriptBase::Scene_Loop_Set_Default(int a) {
	// debug("Scene_Loop_Set_Default(%d)", a);

	_vm->_scene->loopSetDefault(a);
	// _vm->_scene->_defaultLoop = a;
}

void ScriptBase::Scene_Loop_Start_Special(int a, int b, int c) {
	// debug("Scene_Loop_Start_Special(%d, %d, %d)", a, b, c);

	_vm->_scene->loopStartSpecial(a, b, c);
	// _vm->_scene->_field_24_loop_start_special_param_1 = a;
}

void ScriptBase::Outtake_Play(int id, int noLocalization, int container) {
	_vm->outtakePlay(id, noLocalization, container);
}

void ScriptBase::Ambient_Sounds_Add_Sound(int id, int time1, int time2, int volume1, int volume2, int pan1begin, int pan1end, int pan2begin, int pan2end, int priority, int unk) {
	_vm->_ambientSounds->addSound(id, time1, time2, volume1, volume2, pan1begin, pan1end, pan2begin, pan2end, priority, unk);
}

// ScriptBase::Ambient_Sounds_Remove_Sound
// ScriptBase::Ambient_Sounds_Add_Speech_Sound
// ScriptBase::Ambient_Sounds_Remove_Speech_Sound
// ScriptBase::Ambient_Sounds_Play_Sound
// ScriptBase::Ambient_Sounds_Play_Speech_Sound

void ScriptBase::Ambient_Sounds_Remove_All_Non_Looping_Sounds(int time) {
	// _vm->_ambientSounds->removeAllNonLoopingSounds(time);
}

void ScriptBase::Ambient_Sounds_Add_Looping_Sound(int id, int volume, int pan, int fadeInTime) {
	_vm->_ambientSounds->addLoopingSound(id, volume, pan, fadeInTime);
}

// ScriptBase::Ambient_Sounds_Adjust_Looping_Sound
// ScriptBase::Ambient_Sounds_Remove_Looping_Sound

void ScriptBase::Ambient_Sounds_Remove_All_Looping_Sounds(int time) {
	// _vm->_ambientSounds->removeAllLoopingSounds(time);
}

void ScriptBase::Setup_Scene_Information(float actorX, float actorY, float actorZ, int actorFacing) {
	_vm->_scene->setActorStart(Vector3(actorX, actorY, actorZ), actorFacing);
}

// ScriptBase::Dialogue_Menu_Appear
// ScriptBase::Dialogue_Menu_Disappear
// ScriptBase::Dialogue_Menu_Clear_List
// ScriptBase::Dialogue_Menu_Add_To_List
// ScriptBase::Dialogue_Menu_Add_DONE_To_List
// ScriptBase::Dialogue_Menu_Add_To_List_Never_Repeat_Once_Selected
// ScriptBase::DM_Add_To_List
// ScriptBase::DM_Add_To_List_Never_Repeat_Once_Selected
// ScriptBase::Dialogue_Menu_Remove_From_List
// ScriptBase::Dialogue_Menu_Query_Input
// ScriptBase::Dialogue_Menu_Query_List_Size

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

// ScriptBase::World_Waypoint_Set
// ScriptBase::World_Waypoint_Reset
// ScriptBase::World_Waypoint_Query_X
// ScriptBase::World_Waypoint_Query_Y
// ScriptBase::World_Waypoint_Query_Z
// ScriptBase::Combat_Cover_Waypoint_Set_Data
// ScriptBase::Combat_Flee_Waypoint_Set_Data
// ScriptBase::Police_Maze_Target_Track_Add
// ScriptBase::Police_Maze_Query_Score
// ScriptBase::Police_Maze_Zero_Score
// ScriptBase::Police_Maze_Increment_Score
// ScriptBase::Police_Maze_Decrement_Score
// ScriptBase::Police_Maze_Set_Score
// ScriptBase::Police_Maze_Set_Pause_State
// ScriptBase::CDB_Set_Crime
// ScriptBase::CDB_Set_Clue_Asset_Type
// ScriptBase::SDB_Set_Actor
// ScriptBase::SDB_Add_Photo_Clue
// ScriptBase::SDB_Set_Name
// ScriptBase::SDB_Set_Sex
// ScriptBase::SDB_Add_Identity_Clue
// ScriptBase::SDB_Add_MO_Clue
// ScriptBase::SDB_Add_Whereabouts_Clue
// ScriptBase::SDB_Add_Replicant_Clue
// ScriptBase::SDB_Add_Non_Replicant_Clue
// ScriptBase::SDB_Add_Other_Clue
// ScriptBase::Spinner_Set_Selectable_Destination_Flag
// ScriptBase::Spinner_Query_Selectable_Destination_Flag
// ScriptBase::Spinner_Interface_Choose_Dest
// ScriptBase::ESPER_Flag_To_Activate
// ScriptBase::Voight_Kampff_Activate
// ScriptBase::Elevator_Activate
// ScriptBase::View_Score_Board
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

void ScriptBase::Disable_Shadows(int *animationsIdsList, int listSize) {
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
	_vm->_sceneObjects->setRetired(actorId, true);
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

void ScriptBase::Set_Fog_Color(char* fogName, float r, float g, float b) {
	_vm->_scene->_set->_effects->setFogColor(fogName, r, g, b);
}

void ScriptBase::Set_Fog_Density(char* fogName, float density) {
	_vm->_scene->_set->_effects->setFogDensity(fogName, density);
}

// ScriptBase::ADQ_Flush
// ScriptBase::ADQ_Add
// ScriptBase::ADQ_Add_Pause

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

AIScripts::AIScripts(BladeRunnerEngine *vm)
	: _vm(vm),
	  _inScriptCounter(0)
{
	for (int i = 0; i != 100; ++i)
		_AIScripts[i] = 0;

	_AIScripts[0]  = new AIScript_McCoy(_vm);
	_AIScripts[23] = new AIScript_Officer_Leroy(_vm);
}

void AIScripts::Initialize(int actor)
{
	if (_AIScripts[actor])
		_AIScripts[actor]->Initialize();
}

void AIScripts::UpdateAnimation(int actor, int *animation, int *frame)
{
	_inScriptCounter++;
	if (_AIScripts[actor])
		_AIScripts[actor]->UpdateAnimation(animation, frame);
	_inScriptCounter--;
}

void AIScripts::ChangeAnimationMode(int actor, int mode)
{
	_inScriptCounter++;
	if (_AIScripts[actor])
		_AIScripts[actor]->ChangeAnimationMode(mode);
	_inScriptCounter--;
}

} // End of namespace BladeRunner
