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
	void Preload(int animationId);
	void Actor_Put_In_Set(int id, int set);
	void Actor_Set_At_XYZ(int actorId, float x, float y, float z, int direction);
	void Actor_Set_At_Waypoint(int actorId, int waypointId, int angle);
	bool Region_Check(int left, int top, int right, int down);
	bool Object_Query_Click(const char *objectName1, const char *objectName2);
	void Object_Do_Ground_Click();
	bool Object_Mark_For_Hot_Mouse(const char *objectName);
	void Actor_Face_Actor(int actorId, int otherActorId, bool animate);
	void Actor_Face_Object(int actorId, const char *objectName, bool animate);
	void Actor_Face_Item(int actorId, int itemId, bool animate);
	void Actor_Face_Waypoint(int actorId, int waypointId, bool animate);
	void Actor_Face_XYZ(int actorId, float x, float y, float z, bool animate);
	void Actor_Face_Current_Camera(int actorId, bool animate);
	void Actor_Face_Heading(int actorId, int heading);
	int Actor_Query_Friendliness_To_Other(int actorId, int otherActorId);
	void Actor_Modify_Friendliness_To_Other(int actorId, int otherActorId, signed int change);
	void Actor_Set_Friendliness_To_Other(int actorId, int otherActorId, int friendliness);
	void Actor_Set_Honesty(int actorId, int honesty);
	void Actor_Set_Intelligence(int actorId, int intelligence);
	void Actor_Set_Stability(int actorId, int stability);
	void Actor_Set_Combat_Aggressiveness(int actorId, int combatAggressiveness);
	int Actor_Query_Current_HP(int actorId);
	int Actor_Query_Max_HP(int actorId);
	int Actor_Query_Combat_Aggressiveness(int actorId);
	int Actor_Query_Honesty(int actorId);
	int Actor_Query_Intelligence(int actorId);
	int Actor_Query_Stability(int actorId);
	void Actor_Modify_Current_HP(int actorId, signed int change);
	void Actor_Modify_Max_HP(int actorId, signed int change);
	void Actor_Modify_Combat_Aggressiveness(int actorId, signed int change);
	void Actor_Modify_Honesty(int actorId, signed int change);
	void Actor_Modify_Intelligence(int actorId, signed int change);
	void Actor_Modify_Stability(int actorId, signed int change);
	void Actor_Set_Flag_Damage_Anim_If_Moving(int actorId, bool value);
	bool Actor_Query_Flag_Damage_Anim_If_Moving(int actorId);
	void Actor_Combat_AI_Hit_Attempt(int actorId);
	void Non_Player_Actor_Combat_Mode_On(int actorId, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, int a14);
	void Non_Player_Actor_Combat_Mode_Off(int actorId);
	void Actor_Set_Health(int actor, int hp, int maxHp);
	void Actor_Set_Targetable(int actor, bool targetable);
	void Actor_Says(int actorId, int sentenceId, int animationMode);
	void Actor_Says_With_Pause(int actorId, int sentenceId, float pause, int animationMode);
	void Actor_Voice_Over(int sentenceId, int actorId);
	void Actor_Start_Speech_Sample(int actorId, int sentenceId);
	void Actor_Start_Voice_Over_Sample(int sentenceId);
	int Actor_Query_Which_Set_In(int actorId);
	bool Actor_Query_Is_In_Current_Set(int actorId);
	bool Actor_Query_In_Set(int actorId, int setId);
	int Actor_Query_Inch_Distance_From_Actor(int actorId, int otherActorId);
	int Actor_Query_Inch_Distance_From_Waypoint(int actorId, int waypointId);
	bool Actor_Query_In_Between_Two_Actors(int actorId, int otherActor1Id, int otherActor2Id);
	void Actor_Set_Goal_Number(int actorId, int goalNumber);
	int Actor_Query_Goal_Number(int actorId);
	void Actor_Query_XYZ(int actorId, float *x, float *y, float *z);
	int Actor_Query_Facing_1024(int actorId);
	void Actor_Set_Frame_Rate_FPS(int actorId, int fps);
	int Slice_Animation_Query_Number_Of_Frames(int animationId);
	void Actor_Change_Animation_Mode(int actorId, int animationMode);
	int Actor_Query_Animation_Mode(int actorId);
	// Loop_Actor_Walk_To_Actor
	// Loop_Actor_Walk_To_Item
	bool Loop_Actor_Walk_To_Scene_Object(int actorId, const char *objectName, int distance, int a4, int a5);
	// Loop_Actor_Walk_To_Waypoint
	void Loop_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int a4, int a5, int a6, int a7);
	// Async_Actor_Walk_To_Waypoint
	// Async_Actor_Walk_To_XYZ
	// Actor_Force_Stop_Walking
	// Loop_Actor_Travel_Stairs
	// Loop_Actor_Travel_Ladder
	void Actor_Clue_Add_To_Database(int actorId, int clueId, int unknown, bool clueAcquired, bool unknownFlag, int fromActorId);
	void Actor_Clue_Acquire(int actorId, int clueId, byte unknownFlag, int fromActorId);
	void Actor_Clue_Lose(int actorId, int clueId);
	bool Actor_Clue_Query(int actorId, int clueId);
	void Actor_Clues_Transfer_New_To_Mainframe(int actorId);
	void Actor_Clues_Transfer_New_From_Mainframe(int actorId);
	void Actor_Set_Invisible(int actorId, bool isInvisible);
	void Actor_Set_Immunity_To_Obstacles(int actorId, bool isImmune);
	// Item_Add_To_World
	// Item_Remove_From_World
	// Item_Spin_In_World
	// Item_Flag_As_Target
	// Item_Flag_As_Non_Target
	// Item_Pickup_Spin_Effect
	int Animation_Open();
	int Animation_Close();
	int Animation_Start();
	int Animation_Stop();
	int Animation_Skip_To_Frame();
	void Delay(int miliseconds);
	void Player_Loses_Control();
	void Player_Gains_Control();
	void Player_Set_Combat_Mode(bool activate);
	bool Player_Query_Combat_Mode();
	void Player_Set_Combat_Mode_Access(bool enable);
	int Player_Query_Current_Set();
	int Player_Query_Current_Scene();
	int Player_Query_Agenda();
	void Player_Set_Agenda(int agenda);
	int Query_Difficulty_Level();
	void Game_Flag_Set(int flag);
	void Game_Flag_Reset(int flag);
	bool Game_Flag_Query(int flag);
	void Set_Enter(int setId, int sceneId);
	void Chapter_Enter(int chapter, int setId, int sceneId);
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
	void Scene_Exit_Add_2D_Exit(int index, int left, int top, int right, int down, int type);
	void Scene_Exit_Remove(int index);
	void Scene_Exits_Disable();
	void Scene_Exits_Enable();
	void Scene_2D_Region_Add(int index, int left, int top, int right, int down);
	void Scene_2D_Region_Remove(int index);
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
	void Give_McCoy_Ammo(int ammoType, int ammo);
	void Assign_Player_Gun_Hit_Sounds(int row, int soundId1, int soundId2, int soundId3);
	void Assign_Player_Gun_Miss_Sounds(int row, int soundId1, int soundId2, int soundId3);
	void Disable_Shadows(int *animationsIdsList, int listSize);
	bool Query_System_Currently_Loading_Game();
	void Actor_Retired_Here(int actorId, int width, int height, int retired, int retiredByActorId);
	void Clickable_Object(const char *objectName);
	void Unclickable_Object(const char *objectName);
	void Obstacle_Object(const char *objectName, bool updateWalkpath);
	void Unobstacle_Object(const char *objectName, bool updateWalkpath);
	void Obstacle_Flag_All_Objects(bool isObstacle);
	void Combat_Target_Object(const char *objectName);
	void Un_Combat_Target_Object(const char *objectName);
	void Set_Fade_Color(float r, float g, float b);
	void Set_Fade_Density(float density);
	void Set_Fog_Color(char* fogName, float r, float g, float b);
	void Set_Fog_Density(char* fogName, float density);
	// ADQ_Flush
	// ADQ_Add
	// ADQ_Add_Pause
	bool Game_Over();
	void Autosave_Game(int textId);
	void I_Sez(const char *str);
};

class SceneScriptBase : public ScriptBase {
public:
	SceneScriptBase(BladeRunnerEngine *vm)
		: ScriptBase(vm)
	{}

	virtual void InitializeScene() = 0;
	virtual void SceneLoaded() = 0;
	virtual bool ClickedOn3DObject(const char *objectName) = 0;
	virtual bool ClickedOn2DRegion(int region) = 0;
	virtual void SceneFrameAdvanced(int frame) = 0;
	virtual void SceneActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) = 0;
	virtual void PlayerWalkedIn() = 0;
};

/*
 * Scene Scripts
 */

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
	bool ClickedOn3DObject(const char *objectName);
	bool ClickedOn2DRegion(int region);
	void SceneFrameAdvanced(int frame);
	void SceneActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet);
	void PlayerWalkedIn();
};

#define DECLARE_SCRIPT(name) \
class Script##name : public SceneScriptBase { \
public: \
	Script##name(BladeRunnerEngine *vm) \
		: SceneScriptBase(vm) \
	{} \
	void InitializeScene(); \
	void SceneLoaded(); \
	bool ClickedOn3DObject(const char *objectName); \
	bool ClickedOn2DRegion(int region); \
	void SceneFrameAdvanced(int frame); \
	void SceneActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet); \
	void PlayerWalkedIn(); \
private:
#define END_SCRIPT };

DECLARE_SCRIPT(RC01)
	void sub_403850();
END_SCRIPT

#undef DECLARE_SCRIPT

/*
 * Actor Scripts
 */

class AIScriptBase : public ScriptBase {
public:
	AIScriptBase(BladeRunnerEngine *vm)
		: ScriptBase(vm)
	{}

	virtual void Initialize() = 0;
	virtual void UpdateAnimation(int *animation, int *frame) = 0;
	virtual void ChangeAnimationMode(int mode) = 0;
};

class AIScripts {
public:
	BladeRunnerEngine *_vm;
	int                _inScriptCounter;
	AIScriptBase      *_AIScripts[100];

	AIScripts(BladeRunnerEngine *vm);
	~AIScripts();

	void Initialize(int actor);
	void UpdateAnimation(int actor, int *animation, int *frame);
	void ChangeAnimationMode(int actor, int mode);
};

} // End of namespace BladeRunner

#endif
