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

#ifndef BLADERUNNER_SCRIPT_H
#define BLADERUNNER_SCRIPT_H

#include "common/str.h"

namespace BladeRunner {

class BladeRunnerEngine;

class ScriptBase {
protected:
	BladeRunnerEngine *_vm;

public:
	ScriptBase(BladeRunnerEngine *vm)
		: _vm(vm)
	{}
	virtual ~ScriptBase()
	{}

protected:
	// Preload
	// Actor_Put_In_Set
	// Actor_Set_At_XYZ
	// Actor_Set_At_Waypoint
	// Region_Check
	// Object_Query_Click
	// Object_Do_Ground_Click
	// Object_Mark_For_Hot_Mouse
	// Actor_Face_Actor
	// Actor_Face_Object
	// Actor_Face_Item
	// Actor_Face_Waypoint
	// Actor_Face_XYZ
	// Actor_Face_Current_Camera
	// Actor_Face_Heading
	// Actor_Query_Friendliness_To_Other
	// Actor_Modify_Friendliness_To_Other
	// Actor_Set_Friendliness_To_Other
	// Actor_Set_Honesty
	// Actor_Set_Intelligence
	// Actor_Set_Stability
	// Actor_Set_Combat_Aggressiveness
	// Actor_Query_Current_HP
	// Actor_Query_Max_HP
	// Actor_Query_Combat_Aggressiveness
	// Actor_Query_Honesty
	// Actor_Query_Intelligence
	// Actor_Query_Stability
	// Actor_Modify_Current_HP
	// Actor_Modify_Max_HP
	// Actor_Modify_Combat_Aggressiveness
	// Actor_Modify_Honesty
	// Actor_Modify_Intelligence
	// Actor_Modify_Stability
	// Actor_Set_Flag_Damage_Anim_If_Moving
	// Actor_Query_Flag_Damage_Anim_If_Moving
	// Actor_Combat_AI_Hit_Attempt
	// Non_Player_Actor_Combat_Mode_On
	// Non_Player_Actor_Combat_Mode_Off
	// Actor_Set_Health
	// Actor_Set_Targetable
	// Actor_Says
	// Actor_Says_With_Pause
	// Actor_Voice_Over
	// Actor_Start_Speech_Sample
	// Actor_Start_Voice_Over_Sample
	// Actor_Query_Which_Set_In
	// Actor_Query_Is_In_Current_Set
	// Actor_Query_In_Set
	// Actor_Query_Inch_Distance_From_Actor
	// Actor_Query_Inch_Distance_From_Waypoint
	// Actor_Query_In_Between_Two_Actors
	// Actor_Set_Goal_Number
	// Actor_Query_Goal_Number
	// Actor_Query_XYZ
	// Actor_Query_Facing_1024
	// Actor_Set_Frame_Rate_FPS
	// Slice_Animation_Query_Number_Of_Frames
	// Actor_Change_Animation_Mode
	// Actor_Query_Animation_Mode
	// Loop_Actor_Walk_To_Actor
	// Loop_Actor_Walk_To_Item
	// Loop_Actor_Walk_To_Scene_Object
	// Loop_Actor_Walk_To_Waypoint
	// Loop_Actor_Walk_To_XYZ
	// Async_Actor_Walk_To_Waypoint
	// Async_Actor_Walk_To_XYZ
	// Actor_Force_Stop_Walking
	// Loop_Actor_Travel_Stairs
	// Loop_Actor_Travel_Ladder
	void Actor_Clue_Add_To_Database(int a0, int a1, int a2, int a3, int a4, int a5);
	// Actor_Clue_Acquire
	// Actor_Clue_Lose
	// Actor_Clue_Query
	// Actor_Clues_Transfer_New_To_Mainframe
	// Actor_Clues_Transfer_New_From_Mainframe
	// Actor_Set_Invisible
	// Actor_Set_Immunity_To_Obstacles
	// Item_Add_To_World
	// Item_Remove_From_World
	// Item_Spin_In_World
	// Item_Flag_As_Target
	// Item_Flag_As_Non_Target
	// Item_Pickup_Spin_Effect
	// Animation_Open
	// Animation_Close
	// Animation_Start
	// Animation_Stop
	// Animation_Skip_To_Frame
	// Delay
	// Player_Loses_Control
	// Player_Gains_Control
	// Player_Set_Combat_Mode
	// Player_Query_Combat_Mode
	// Player_Set_Combat_Mode_Access
	// Player_Query_Current_Set
	// Player_Query_Current_Scene
	// Player_Query_Agenda
	// Player_Set_Agenda
	// Query_Difficulty_Level
	void Game_Flag_Set(int flag);
	void Game_Flag_Reset(int flag);
	bool Game_Flag_Query(int flag);
	// Set_Enter
	// Chapter_Enter
	int Global_Variable_Set(int, int);
	int Global_Variable_Reset(int);
	int Global_Variable_Query(int);
	int Global_Variable_Increment(int, int);
	int Global_Variable_Decrement(int, int);
	int Random_Query(int min, int max);
	void Sound_Play(int id, int volume, int panFrom, int panTo, int priority);
	// Sound_Play_Speech_Line
	// Sound_Left_Footstep_Walk
	// Sound_Right_Footstep_Walk
	// Sound_Left_Footstep_Run
	// Sound_Right_Footstep_Run
	// Sound_Walk_Shuffle_Stop
	// Footstep_Sounds_Set
	// Footstep_Sound_Override_On
	// Footstep_Sound_Override_Off
	// Music_Play
	// Music_Adjust
	// Music_Stop
	// Music_Is_Playing
	// Overlay_Play
	// Overlay_Remove
	void Scene_Loop_Set_Default(int);
	void Scene_Loop_Start_Special(int, int, int);
	void Outtake_Play(int id, int noLocalization = false, int container = -1);
	void Ambient_Sounds_Add_Sound(int id, int time1, int time2, int volume1, int volume2, int pan1begin, int pan1end, int pan2begin, int pan2end, int priority, int unk);
	// Ambient_Sounds_Remove_Sound
	// Ambient_Sounds_Add_Speech_Sound
	// Ambient_Sounds_Remove_Speech_Sound
	// Ambient_Sounds_Play_Sound
	// Ambient_Sounds_Play_Speech_Sound
	void Ambient_Sounds_Remove_All_Non_Looping_Sounds(int time);
	void Ambient_Sounds_Add_Looping_Sound(int id, int volume, int pan, int fadeInTime);
	// Ambient_Sounds_Adjust_Looping_Sound
	// Ambient_Sounds_Remove_Looping_Sound
	void Ambient_Sounds_Remove_All_Looping_Sounds(int time);
	void Setup_Scene_Information(float actorX, float actorY, float actorZ, int actorFacing);
	// Dialogue_Menu_Appear
	// Dialogue_Menu_Disappear
	// Dialogue_Menu_Clear_List
	// Dialogue_Menu_Add_To_List
	// Dialogue_Menu_Add_DONE_To_List
	// Dialogue_Menu_Add_To_List_Never_Repeat_Once_Selected
	// DM_Add_To_List
	// DM_Add_To_List_Never_Repeat_Once_Selected
	// Dialogue_Menu_Remove_From_List
	// Dialogue_Menu_Query_Input
	// Dialogue_Menu_Query_List_Size
	void Scene_Exit_Add_2D_Exit(int, int, int, int, int, int);
	// Scene_Exit_Remove
	// Scene_Exits_Disable
	// Scene_Exits_Enable
	void Scene_2D_Region_Add(int, int, int, int, int);
	// Scene_2D_Region_Remove
	// World_Waypoint_Set
	// World_Waypoint_Reset
	// World_Waypoint_Query_X
	// World_Waypoint_Query_Y
	// World_Waypoint_Query_Z
	// Combat_Cover_Waypoint_Set_Data
	// Combat_Flee_Waypoint_Set_Data
	// Police_Maze_Target_Track_Add
	// Police_Maze_Query_Score
	// Police_Maze_Zero_Score
	// Police_Maze_Increment_Score
	// Police_Maze_Decrement_Score
	// Police_Maze_Set_Score
	// Police_Maze_Set_Pause_State
	// CDB_Set_Crime
	// CDB_Set_Clue_Asset_Type
	// SDB_Set_Actor
	// SDB_Add_Photo_Clue
	// SDB_Set_Name
	// SDB_Set_Sex
	// SDB_Add_Identity_Clue
	// SDB_Add_MO_Clue
	// SDB_Add_Whereabouts_Clue
	// SDB_Add_Replicant_Clue
	// SDB_Add_Non_Replicant_Clue
	// SDB_Add_Other_Clue
	// Spinner_Set_Selectable_Destination_Flag
	// Spinner_Query_Selectable_Destination_Flag
	// Spinner_Interface_Choose_Dest
	// ESPER_Flag_To_Activate
	// Voight_Kampff_Activate
	// Elevator_Activate
	// View_Score_Board
	// Query_Score
	void Set_Score(int a0, int a1);
	// Give_McCoy_Ammo
	void Assign_Player_Gun_Hit_Sounds(int a0, int a1, int a2, int a3);
	void Assign_Player_Gun_Miss_Sounds(int a0, int a1, int a2, int a3);
	// Disable_Shadows
	// Query_System_Currently_Loading_Game
	// Actor_Retired_Here
	// Clickable_Object
	// Unclickable_Object
	// Obstacle_Object
	// Unobstacle_Object
	// Obstacle_Flag_All_Objects
	// Combat_Target_Object
	// Un_Combat_Target_Object
	// Set_Fade_Color
	// Set_Fade_Density
	// Set_Fog_Color
	// Set_Fog_Density
	// ADQ_Flush
	// ADQ_Add
	// ADQ_Add_Pause
	// Game_Over
	// Autosave_Game
	void I_Sez(const char *str);
};

class SceneScriptBase : public ScriptBase {
public:
	SceneScriptBase(BladeRunnerEngine *vm)
		: ScriptBase(vm)
	{}

	virtual void InitializeScene() = 0;
	virtual void SceneLoaded() = 0;
	virtual void SceneFrameAdvanced(int frame) = 0;
};

class Script {
public:
	BladeRunnerEngine *_vm;
	int                _inScriptCounter;
	SceneScriptBase   *_currentScript;

	Script(BladeRunnerEngine *vm)
		: _vm(vm),
		  _inScriptCounter(0),
		  _currentScript(nullptr)
	{}
	~Script();

	bool open(const Common::String &name);

	void InitializeScene();
	void SceneLoaded();
	void SceneFrameAdvanced(int frame);
};

#define DECLARE_SCRIPT(name) \
class Script##name : public SceneScriptBase { \
public: \
	Script##name(BladeRunnerEngine *vm) \
		: SceneScriptBase(vm) \
	{} \
	void InitializeScene(); \
	void SceneLoaded(); \
	void SceneFrameAdvanced(int frame); \
};

DECLARE_SCRIPT(RC01)

#undef DECLARE_SCRIPT

} // End of namespace BladeRunner

#endif
