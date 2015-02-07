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

#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/gameflags.h"
#include "bladerunner/scene.h"
#include "bladerunner/vector.h"

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

void Script::SceneFrameAdvanced(int frame) {
	_inScriptCounter++;
	_currentScript->SceneFrameAdvanced(frame);
	_inScriptCounter--;
}

// ScriptBase::Preload
// ScriptBase::Actor_Put_In_Set
// ScriptBase::Actor_Set_At_XYZ
// ScriptBase::Actor_Set_At_Waypoint
// ScriptBase::Region_Check
// ScriptBase::Object_Query_Click
// ScriptBase::Object_Do_Ground_Click
// ScriptBase::Object_Mark_For_Hot_Mouse
// ScriptBase::Actor_Face_Actor
// ScriptBase::Actor_Face_Object
// ScriptBase::Actor_Face_Item
// ScriptBase::Actor_Face_Waypoint
// ScriptBase::Actor_Face_XYZ
// ScriptBase::Actor_Face_Current_Camera
// ScriptBase::Actor_Face_Heading
// ScriptBase::Actor_Query_Friendliness_To_Other
// ScriptBase::Actor_Modify_Friendliness_To_Other
// ScriptBase::Actor_Set_Friendliness_To_Other
// ScriptBase::Actor_Set_Honesty
// ScriptBase::Actor_Set_Intelligence
// ScriptBase::Actor_Set_Stability
// ScriptBase::Actor_Set_Combat_Aggressiveness
// ScriptBase::Actor_Query_Current_HP
// ScriptBase::Actor_Query_Max_HP
// ScriptBase::Actor_Query_Combat_Aggressiveness
// ScriptBase::Actor_Query_Honesty
// ScriptBase::Actor_Query_Intelligence
// ScriptBase::Actor_Query_Stability
// ScriptBase::Actor_Modify_Current_HP
// ScriptBase::Actor_Modify_Max_HP
// ScriptBase::Actor_Modify_Combat_Aggressiveness
// ScriptBase::Actor_Modify_Honesty
// ScriptBase::Actor_Modify_Intelligence
// ScriptBase::Actor_Modify_Stability
// ScriptBase::Actor_Set_Flag_Damage_Anim_If_Moving
// ScriptBase::Actor_Query_Flag_Damage_Anim_If_Moving
// ScriptBase::Actor_Combat_AI_Hit_Attempt
// ScriptBase::Non_Player_Actor_Combat_Mode_On
// ScriptBase::Non_Player_Actor_Combat_Mode_Off
// ScriptBase::Actor_Set_Health
// ScriptBase::Actor_Set_Targetable
// ScriptBase::Actor_Says
// ScriptBase::Actor_Says_With_Pause
// ScriptBase::Actor_Voice_Over
// ScriptBase::Actor_Start_Speech_Sample
// ScriptBase::Actor_Start_Voice_Over_Sample
// ScriptBase::Actor_Query_Which_Set_In
// ScriptBase::Actor_Query_Is_In_Current_Set
// ScriptBase::Actor_Query_In_Set
// ScriptBase::Actor_Query_Inch_Distance_From_Actor
// ScriptBase::Actor_Query_Inch_Distance_From_Waypoint
// ScriptBase::Actor_Query_In_Between_Two_Actors
// ScriptBase::Actor_Set_Goal_Number
// ScriptBase::Actor_Query_Goal_Number
// ScriptBase::Actor_Query_XYZ
// ScriptBase::Actor_Query_Facing_1024
// ScriptBase::Actor_Set_Frame_Rate_FPS
// ScriptBase::Slice_Animation_Query_Number_Of_Frames
// ScriptBase::Actor_Change_Animation_Mode
// ScriptBase::Actor_Query_Animation_Mode
// ScriptBase::Loop_Actor_Walk_To_Actor
// ScriptBase::Loop_Actor_Walk_To_Item
// ScriptBase::Loop_Actor_Walk_To_Scene_Object
// ScriptBase::Loop_Actor_Walk_To_Waypoint
// ScriptBase::Loop_Actor_Walk_To_XYZ
// ScriptBase::Async_Actor_Walk_To_Waypoint
// ScriptBase::Async_Actor_Walk_To_XYZ
// ScriptBase::Actor_Force_Stop_Walking
// ScriptBase::Loop_Actor_Travel_Stairs
// ScriptBase::Loop_Actor_Travel_Ladder
// ScriptBase::Actor_Clue_Add_To_Database
// ScriptBase::Actor_Clue_Acquire
// ScriptBase::Actor_Clue_Lose
// ScriptBase::Actor_Clue_Query
// ScriptBase::Actor_Clues_Transfer_New_To_Mainframe
// ScriptBase::Actor_Clues_Transfer_New_From_Mainframe
// ScriptBase::Actor_Set_Invisible
// ScriptBase::Actor_Set_Immunity_To_Obstacles
// ScriptBase::Item_Add_To_World
// ScriptBase::Item_Remove_From_World
// ScriptBase::Item_Spin_In_World
// ScriptBase::Item_Flag_As_Target
// ScriptBase::Item_Flag_As_Non_Target
// ScriptBase::Item_Pickup_Spin_Effect
// ScriptBase::Animation_Open
// ScriptBase::Animation_Close
// ScriptBase::Animation_Start
// ScriptBase::Animation_Stop
// ScriptBase::Animation_Skip_To_Frame
// ScriptBase::Delay
// ScriptBase::Player_Loses_Control
// ScriptBase::Player_Gains_Control
// ScriptBase::Player_Set_Combat_Mode
// ScriptBase::Player_Query_Combat_Mode
// ScriptBase::Player_Set_Combat_Mode_Access
// ScriptBase::Player_Query_Current_Set
// ScriptBase::Player_Query_Current_Scene
// ScriptBase::Player_Query_Agenda
// ScriptBase::Player_Set_Agenda
// ScriptBase::Query_Difficulty_Level

void ScriptBase::Game_Flag_Set(int flag) {
	_vm->_gameFlags->set(flag);
}

void ScriptBase::Game_Flag_Reset(int flag) {
	_vm->_gameFlags->reset(flag);
}

bool ScriptBase::Game_Flag_Query(int flag) {
	return _vm->_gameFlags->query(flag);
}

// ScriptBase::Set_Enter
// ScriptBase::Chapter_Enter

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

// ScriptBase::Random_Query

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
	debug("Scene_Loop_Set_Default(%d)", a);
}

void ScriptBase::Scene_Loop_Start_Special(int a, int b, int c) {
	debug("Scene_Loop_Start_Special(%d, %d, %d)", a, b, c);
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

void ScriptBase::Scene_Exit_Add_2D_Exit(int a, int b, int c, int d, int e, int f) {
	debug("Scene_Exit_Add_2D_Exit(%d, %d, %d, %d, %d, %d)", a, b, c, d, e, f);
}

// ScriptBase::Scene_Exit_Remove
// ScriptBase::Scene_Exits_Disable
// ScriptBase::Scene_Exits_Enable

void ScriptBase::Scene_2D_Region_Add(int a, int b, int c, int d, int e) {
	debug("Scene_2D_Region_Add(%d, %d, %d, %d, %d)", a, b, c, d, e);
}

// ScriptBase::Scene_2D_Region_Remove
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
// ScriptBase::Set_Score
// ScriptBase::Give_McCoy_Ammo

void ScriptBase::Assign_Player_Gun_Hit_Sounds(int a0, int a1, int a2, int a3) {
	debug("STUB: Assign_Player_Gun_Hit_Sounds(%d, %d, %d, %d)", a0, a1, a2, a3);
}

void ScriptBase::Assign_Player_Gun_Miss_Sounds(int a0, int a1, int a2, int a3) {
	debug("STUB: Assign_Player_Gun_Miss_Sounds(%d, %d, %d, %d)", a0, a1, a2, a3);
}

// ScriptBase::Disable_Shadows
// ScriptBase::Query_System_Currently_Loading_Game
// ScriptBase::Actor_Retired_Here
// ScriptBase::Clickable_Object
// ScriptBase::Unclickable_Object
// ScriptBase::Obstacle_Object
// ScriptBase::Unobstacle_Object
// ScriptBase::Obstacle_Flag_All_Objects
// ScriptBase::Combat_Target_Object
// ScriptBase::Un_Combat_Target_Object
// ScriptBase::Set_Fade_Color
// ScriptBase::Set_Fade_Density
// ScriptBase::Set_Fog_Color
// ScriptBase::Set_Fog_Density
// ScriptBase::ADQ_Flush
// ScriptBase::ADQ_Add
// ScriptBase::ADQ_Add_Pause
// ScriptBase::Game_Over
// ScriptBase::Autosave_Game

void ScriptBase::I_Sez(const char *str) {
	_vm->ISez(str);
}


} // End of namespace BladeRunner
