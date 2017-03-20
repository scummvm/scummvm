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
	ScriptBase(BladeRunnerEngine *vm) : _vm(vm) {}
	virtual ~ScriptBase() {}

protected:
	void Preload(int animationId);
	void Actor_Put_In_Set(int actorId, int set);
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
	void Actor_Face_Heading(int actorId, int heading, bool animate);
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
	bool Loop_Actor_Walk_To_Actor(int actorId, int otherActorId, int a3, int a4, bool running);
	bool Loop_Actor_Walk_To_Item(int actorId, int itemId, int a3, int a4, bool run);
	bool Loop_Actor_Walk_To_Scene_Object(int actorId, const char *objectName, int distance, bool a4, bool run);
	bool Loop_Actor_Walk_To_Waypoint(int actorId, int waypointId, int a3, int a4, bool run);
	bool Loop_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int a4, int a5, bool run, int a7);
	void Async_Actor_Walk_To_Waypoint(int actorId, int waypointId, int a3, int run);
	void Async_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int a5, bool run);
	void Actor_Force_Stop_Walking(int actorId);
	bool Loop_Actor_Travel_Stairs(int actorId, int a2, int a3, int a4);
	bool Loop_Actor_Travel_Ladder(int actorId, int a2, int a3, int a4);
	void Actor_Clue_Add_To_Database(int actorId, int clueId, int unknown, bool clueAcquired, bool unknownFlag, int fromActorId);
	void Actor_Clue_Acquire(int actorId, int clueId, byte unknownFlag, int fromActorId);
	void Actor_Clue_Lose(int actorId, int clueId);
	bool Actor_Clue_Query(int actorId, int clueId);
	void Actor_Clues_Transfer_New_To_Mainframe(int actorId);
	void Actor_Clues_Transfer_New_From_Mainframe(int actorId);
	void Actor_Set_Invisible(int actorId, bool isInvisible);
	void Actor_Set_Immunity_To_Obstacles(int actorId, bool isImmune);
	void Item_Add_To_World(int itemId, int animationId, int setId, float x, float y, float z, signed int facing, int height, int width, bool isTargetable, bool isObstacle, bool isPoliceMazeEnemy, bool updateOnly);
	void Item_Remove_From_World(int itemId);
	void Item_Spin_In_World(int itemId);
	void Item_Flag_As_Target(int itemId);
	void Item_Flag_As_Non_Target(int itemId);
	void Item_Pickup_Spin_Effect(int a1, int a2, int a3);
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
	void Sound_Play_Speech_Line(int actorId, int speechId, int a3, int a4, int a5);
	void Sound_Left_Footstep_Walk(int actorId);
	void Sound_Right_Footstep_Walk(int actorId);
	void Sound_Left_Footstep_Run(int actorId);
	void Sound_Right_Footstep_Run(int actorId);
	// Sound_Walk_Shuffle_Stop
	void Footstep_Sounds_Set(int index, int value);
	void Footstep_Sound_Override_On(int footstepSoundOverride);
	void Footstep_Sound_Override_Off();
	bool Music_Play(int a1, int a2, int a3, int a4, int a5, int a6, int a7);
	void Music_Adjust(int a1, int a2, int a3);
	void Music_Stop(int a1);
	bool Music_Is_Playing();
	void Overlay_Play(const char *overlay, int a2, int a3, int a4, int a5);
	void Overlay_Remove(const char *overlay);
	void Scene_Loop_Set_Default(int);
	void Scene_Loop_Start_Special(int, int, int);
	void Outtake_Play(int id, int noLocalization = false, int container = -1);
	void Ambient_Sounds_Add_Sound(int id, int time1, int time2, int volume1, int volume2, int pan1begin, int pan1end, int pan2begin, int pan2end, int priority, int unk);
	void Ambient_Sounds_Remove_Sound(int id, bool a2);
	void Ambient_Sounds_Add_Speech_Sound(int id, int unk1, int time1, int time2, int volume1, int volume2, int pan1begin, int pan1end, int pan2begin, int pan2end, int priority, int unk2);
	// Ambient_Sounds_Remove_Speech_Sound
	int Ambient_Sounds_Play_Sound(int a1, int a2, int a3, int a4, int a5);
	// Ambient_Sounds_Play_Speech_Sound
	void Ambient_Sounds_Remove_All_Non_Looping_Sounds(int time);
	void Ambient_Sounds_Add_Looping_Sound(int id, int volume, int pan, int fadeInTime);
	void Ambient_Sounds_Adjust_Looping_Sound(int id, int panBegin, int panEnd, int a4);
	void Ambient_Sounds_Remove_Looping_Sound(int id, bool a2);
	void Ambient_Sounds_Remove_All_Looping_Sounds(int time);
	void Setup_Scene_Information(float actorX, float actorY, float actorZ, int actorFacing);
	bool Dialogue_Menu_Appear(int x, int y);
	bool Dialogue_Menu_Disappear();
	bool Dialogue_Menu_Clear_List();
	bool Dialogue_Menu_Add_To_List(int answer);
	bool Dialogue_Menu_Add_DONE_To_List(int answer);
	// Dialogue_Menu_Add_To_List_Never_Repeat_Once_Selected
	bool DM_Add_To_List(int answer, int a2, int a3, int a4);
	bool DM_Add_To_List_Never_Repeat_Once_Selected(int answer, int a2, int a3, int a4);
	void Dialogue_Menu_Remove_From_List(int answer);
	int Dialogue_Menu_Query_Input();
	int Dialogue_Menu_Query_List_Size();
	void Scene_Exit_Add_2D_Exit(int index, int left, int top, int right, int down, int type);
	void Scene_Exit_Remove(int index);
	void Scene_Exits_Disable();
	void Scene_Exits_Enable();
	void Scene_2D_Region_Add(int index, int left, int top, int right, int down);
	void Scene_2D_Region_Remove(int index);
	void World_Waypoint_Set(int waypointId, int sceneId, float x, float y, float z);
	// World_Waypoint_Reset
	float World_Waypoint_Query_X(int waypointId);
	float World_Waypoint_Query_Y(int waypointId);
	float World_Waypoint_Query_Z(int waypointId);
	void Combat_Cover_Waypoint_Set_Data(int combatCoverId, int a2, int sceneId, int a4, float x, float y, float z);
	void Combat_Flee_Waypoint_Set_Data(int combatFleeWaypointId, int a2, int sceneId, int a4, float x, float y, float z, int a8);
	void Police_Maze_Target_Track_Add(int itemId, float startX, float startY, float startZ, float endX, float endY, float endZ, int steps, signed int data[], bool a10);
	// Police_Maze_Query_Score
	// Police_Maze_Zero_Score
	// Police_Maze_Increment_Score
	// Police_Maze_Decrement_Score
	// Police_Maze_Set_Score
	void Police_Maze_Set_Pause_State(int a1);
	void CDB_Set_Crime(int crimeId, int value);
	void CDB_Set_Clue_Asset_Type(int assetId, int type);
	void SDB_Set_Actor(int suspectId, int actorId);
	bool SDB_Add_Photo_Clue(int suspectId, int a2, int a3);
	void SDB_Set_Name(int suspectId);
	void SDB_Set_Sex(int suspectId, int sex);
	bool SDB_Add_Identity_Clue(int suspectId, int clueId);
	bool SDB_Add_MO_Clue(int suspectId, int clueId);
	bool SDB_Add_Whereabouts_Clue(int suspectId, int clueId);
	bool SDB_Add_Replicant_Clue(int suspectId, int clueId);
	bool SDB_Add_Non_Replicant_Clue(int suspectId, int clueId);
	bool SDB_Add_Other_Clue(int suspectId, int clueId);
	void Spinner_Set_Selectable_Destination_Flag(int a1, int a2);
	// Spinner_Query_Selectable_Destination_Flag
	int Spinner_Interface_Choose_Dest(int a1, int a2);
	void ESPER_Flag_To_Activate();
	bool Voight_Kampff_Activate(int a1, int a2);
	int Elevator_Activate(int elevator);
	void View_Score_Board();
	// Query_Score
	void Set_Score(int a0, int a1);
	void Give_McCoy_Ammo(int ammoType, int ammo);
	void Assign_Player_Gun_Hit_Sounds(int row, int soundId1, int soundId2, int soundId3);
	void Assign_Player_Gun_Miss_Sounds(int row, int soundId1, int soundId2, int soundId3);
	void Disable_Shadows(int animationsIdsList[], int listSize);
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
	void Set_Fog_Color(const char* fogName, float r, float g, float b);
	void Set_Fog_Density(const char* fogName, float density);
	void ADQ_Flush();
	void ADQ_Add(int actorId, int sentenceId, int animationMode);
	void ADQ_Add_Pause(int delay);
	bool Game_Over();
	void Autosave_Game(int textId);
	void I_Sez(const char *str);

	void AI_Countdown_Timer_Start(int actorId, signed int timer, int seconds);
	void AI_Countdown_Timer_Reset(int actorId, int timer);
	void AI_Movement_Track_Unpause(int actorId);
	void AI_Movement_Track_Pause(int actorId);
	void AI_Movement_Track_Repeat(int actorId);
	void AI_Movement_Track_Append_Run_With_Facing(int actorId, int waypointId, int delay, int angle);
	void AI_Movement_Track_Append_With_Facing(int actorId, int waypointId, int delay, int angle);
	void AI_Movement_Track_Append_Run(int actorId, int waypointId, int delay);
	void AI_Movement_Track_Append(int actorId, int waypointId, int delay);
	void AI_Movement_Track_Flush(int actorId);

	void ESPER_Add_Photo(const char* fileName, int a2, int a3);
	void ESPER_Define_Special_Region(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11, int a12, int a13, const char *name);

	void KIA_Play_Actor_Dialogue(int a1, int a2);
	void KIA_Play_Slice_Model(int a1);
	void KIA_Play_Photograph(int a1);

	void VK_Play_Speech_Line(int actorIndex, int a2, float a3);
	void VK_Add_Question(int a1, int a2, int a3);
	void VK_Subject_Reacts(int a1, int a2, int a3, int a4);
	void VK_Eye_Animates(int a1);
};

class SceneScriptBase : public ScriptBase {
public:
	SceneScriptBase(BladeRunnerEngine *vm) : ScriptBase(vm) {}

	virtual void InitializeScene() = 0;
	virtual void SceneLoaded() = 0;
	virtual bool MouseClick(int x, int y) = 0;
	virtual bool ClickedOn3DObject(const char *objectName, bool a2) = 0;
	virtual bool ClickedOnActor(int actorId) = 0;
	virtual bool ClickedOnItem(int itemId, bool a2) = 0;
	virtual bool ClickedOnExit(int exitId) = 0;
	virtual bool ClickedOn2DRegion(int region) = 0;
	virtual void SceneFrameAdvanced(int frame) = 0;
	virtual void ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet) = 0;
	virtual void PlayerWalkedIn() = 0;
	virtual void PlayerWalkedOut() = 0;
	virtual void DialogueQueueFlushed(int a1) = 0;
};

/*
 * Scene Scripts
 */

class Script {
protected:
	BladeRunnerEngine *_vm;
	int                _inScriptCounter;
	SceneScriptBase   *_currentScript;

public:
	Script(BladeRunnerEngine *vm)
		: _vm(vm),
		  _inScriptCounter(0),
		  _currentScript(nullptr) {
	}
	~Script();

	bool open(const Common::String &name);

	void InitializeScene();
	void SceneLoaded();
	bool MouseClick(int x, int y);
	bool ClickedOn3DObject(const char *objectName, bool a2);
	bool ClickedOnActor(int actorId);
	bool ClickedOnItem(int itemId, bool a2);
	bool ClickedOnExit(int exitId);
	bool ClickedOn2DRegion(int region);
	void SceneFrameAdvanced(int frame);
	void ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet);
	void PlayerWalkedIn();
	void PlayerWalkedOut();
	void DialogueQueueFlushed(int a1);
	bool IsInsideScript() { return _inScriptCounter > 0; }
};

#define DECLARE_SCRIPT(name) \
class Script##name : public SceneScriptBase { \
public: \
	Script##name(BladeRunnerEngine *vm) \
		: SceneScriptBase(vm) \
	{} \
	void InitializeScene(); \
	void SceneLoaded(); \
	bool MouseClick(int x, int y); \
	bool ClickedOn3DObject(const char *objectName, bool a2); \
	bool ClickedOnActor(int actorId); \
	bool ClickedOnItem(int itemId, bool a2); \
	bool ClickedOnExit(int exitId); \
	bool ClickedOn2DRegion(int region); \
	void SceneFrameAdvanced(int frame); \
	void ActorChangedGoal(int actorId, int newGoal, int oldGoal, bool currentSet); \
	void PlayerWalkedIn(); \
	void PlayerWalkedOut(); \
	void DialogueQueueFlushed(int a1); \
private:
#define END_SCRIPT };

DECLARE_SCRIPT(AR01)
END_SCRIPT

DECLARE_SCRIPT(AR02)
	void sub_402694();
	void sub_402AE0();
	void sub_402CE4();
END_SCRIPT

DECLARE_SCRIPT(BB01)
END_SCRIPT

DECLARE_SCRIPT(BB02)
END_SCRIPT

DECLARE_SCRIPT(BB03)
END_SCRIPT

DECLARE_SCRIPT(BB04)
END_SCRIPT

DECLARE_SCRIPT(BB05)
END_SCRIPT

DECLARE_SCRIPT(BB06)
END_SCRIPT

DECLARE_SCRIPT(BB07)
END_SCRIPT

DECLARE_SCRIPT(BB08)
END_SCRIPT

DECLARE_SCRIPT(BB09)
END_SCRIPT

DECLARE_SCRIPT(BB10)
END_SCRIPT

DECLARE_SCRIPT(BB11)
END_SCRIPT

DECLARE_SCRIPT(BB12)
END_SCRIPT

DECLARE_SCRIPT(BB51)
END_SCRIPT

DECLARE_SCRIPT(CT01)
	void sub_40269C();
END_SCRIPT

DECLARE_SCRIPT(CT02)
	void sub_401ACC();
END_SCRIPT

DECLARE_SCRIPT(CT03)
END_SCRIPT

DECLARE_SCRIPT(CT04)
	void sub_401D4C();
END_SCRIPT

DECLARE_SCRIPT(CT05)
END_SCRIPT

DECLARE_SCRIPT(CT06)
END_SCRIPT

DECLARE_SCRIPT(CT07)
END_SCRIPT

DECLARE_SCRIPT(CT08)
END_SCRIPT

DECLARE_SCRIPT(CT09)
END_SCRIPT

DECLARE_SCRIPT(CT10)
	void sub_401844();
END_SCRIPT

DECLARE_SCRIPT(CT11)
END_SCRIPT

DECLARE_SCRIPT(CT12)
END_SCRIPT

DECLARE_SCRIPT(CT51)
END_SCRIPT

DECLARE_SCRIPT(DR01)
END_SCRIPT

DECLARE_SCRIPT(DR02)
END_SCRIPT

DECLARE_SCRIPT(DR03)
	void sub_401B18();
END_SCRIPT

DECLARE_SCRIPT(DR04)
	bool sub_401160();
END_SCRIPT

DECLARE_SCRIPT(DR05)
END_SCRIPT

DECLARE_SCRIPT(DR06)
END_SCRIPT

DECLARE_SCRIPT(HC01)
	void sub_402384();
	void sub_40346C();
END_SCRIPT

DECLARE_SCRIPT(HC02)
END_SCRIPT

DECLARE_SCRIPT(HC03)
END_SCRIPT

DECLARE_SCRIPT(HC04)
	void sub_401B90();
END_SCRIPT

DECLARE_SCRIPT(HF01)
	void sub_4026B4();
	void sub_4032DC();
	void sub_403484();
END_SCRIPT

DECLARE_SCRIPT(HF02)
END_SCRIPT

DECLARE_SCRIPT(HF03)
	void sub_401C80();
END_SCRIPT

DECLARE_SCRIPT(HF04)
END_SCRIPT

DECLARE_SCRIPT(HF05)
	void sub_402370();
	void sub_402970();
	void sub_402AE4();
	void sub_403738();
	void sub_403A34(int actorId);
	void sub_403F0C();
	void sub_40410C();
	void sub_4042E4();
	void sub_404474();
	int sub_404858();
	int sub_4048C0();
END_SCRIPT

DECLARE_SCRIPT(HF06)
	void sub_401EF4();
	void sub_4023E0();
END_SCRIPT

DECLARE_SCRIPT(HF07)
	int sub_401864();
END_SCRIPT

DECLARE_SCRIPT(KP01)
END_SCRIPT

DECLARE_SCRIPT(KP02)
END_SCRIPT

DECLARE_SCRIPT(KP03)
	void sub_401E54();
END_SCRIPT

DECLARE_SCRIPT(KP04)
END_SCRIPT

DECLARE_SCRIPT(KP05)
END_SCRIPT

DECLARE_SCRIPT(KP06)
END_SCRIPT

DECLARE_SCRIPT(KP07)
END_SCRIPT

DECLARE_SCRIPT(MA01)
END_SCRIPT

DECLARE_SCRIPT(MA02)
	void sub_401E4C();
	bool sub_401F7C();
	void sub_402044();
END_SCRIPT

//MA03 does not exists

DECLARE_SCRIPT(MA04)
	bool sub_402758();
	bool sub_402820();
	bool sub_402888();
	void sub_4028A8();
	void sub_402F2C();
	void sub_4032A0();
	void sub_4034D8();
	void sub_403864();
	void sub_403DA8();
END_SCRIPT

DECLARE_SCRIPT(MA05)
	bool sub_401990();
END_SCRIPT

DECLARE_SCRIPT(MA06)
	bool sub_4012C0();
	void sub_4014E4();
END_SCRIPT

DECLARE_SCRIPT(MA07)
END_SCRIPT

DECLARE_SCRIPT(MA08)
END_SCRIPT

DECLARE_SCRIPT(NR01)
END_SCRIPT

DECLARE_SCRIPT(NR02)
	void sub_402134();
END_SCRIPT

DECLARE_SCRIPT(NR03)
	void sub_40259C(int frame);
	void sub_402994();
END_SCRIPT

DECLARE_SCRIPT(NR04)
	void sub_401DB0();
	void sub_402860(int frame);
	void sub_402960();
END_SCRIPT

DECLARE_SCRIPT(NR05)
	void sub_401F74(int frame);
	void sub_4020B4();
	void sub_4022DC();
	void sub_402A48(int actorId);
	void sub_402B9C();
END_SCRIPT

DECLARE_SCRIPT(NR06)
	void sub_401BAC();
END_SCRIPT

DECLARE_SCRIPT(NR07)
	void sub_4018D4();
	void sub_401A10();
	void sub_401C60();
	void sub_401EF4();
	void sub_4020F0();
	void sub_402284();
	void sub_402510();
	void sub_402614();
	void sub_402738();
	void sub_4028FC();
END_SCRIPT

DECLARE_SCRIPT(NR08)
	void sub_4021B4();
END_SCRIPT

DECLARE_SCRIPT(NR09)
	void sub_40172C();
END_SCRIPT

DECLARE_SCRIPT(NR10)
END_SCRIPT

DECLARE_SCRIPT(NR11)
	void sub_4027D0(int actorId, signed int frame);
	void sub_4028EC();
END_SCRIPT

DECLARE_SCRIPT(PS01)
END_SCRIPT

DECLARE_SCRIPT(PS02)
	void sub_4018BC();
END_SCRIPT

DECLARE_SCRIPT(PS03)
END_SCRIPT

DECLARE_SCRIPT(PS04)
	void sub_4017E4();
END_SCRIPT

DECLARE_SCRIPT(PS05)
	void sub_401B34();
	void sub_401C30();
END_SCRIPT

DECLARE_SCRIPT(PS06)
END_SCRIPT

DECLARE_SCRIPT(PS07)
	void sub_401D60();
END_SCRIPT

// PS08 does not exits

DECLARE_SCRIPT(PS09)
	void sub_402090();
END_SCRIPT

DECLARE_SCRIPT(PS10)
	void sub_402238();
END_SCRIPT

DECLARE_SCRIPT(PS11)
	void sub_402744();
END_SCRIPT

DECLARE_SCRIPT(PS12)
	void sub_4028C4();
END_SCRIPT

DECLARE_SCRIPT(PS13)
	void sub_40267C();
END_SCRIPT

DECLARE_SCRIPT(PS14)
END_SCRIPT

DECLARE_SCRIPT(PS15)
END_SCRIPT

DECLARE_SCRIPT(RC01)
	void sub_403850();
	void sub_4037AC();
END_SCRIPT

DECLARE_SCRIPT(RC02)
	void sub_402A7C();
END_SCRIPT

DECLARE_SCRIPT(RC03)
	void sub_402834();
END_SCRIPT

DECLARE_SCRIPT(RC04)
	void sub_401DF4();
END_SCRIPT

DECLARE_SCRIPT(RC51)
END_SCRIPT

DECLARE_SCRIPT(TB02)
	void sub_402644();
	void sub_402B50();
END_SCRIPT

DECLARE_SCRIPT(TB03)
END_SCRIPT

DECLARE_SCRIPT(TB05)
END_SCRIPT

DECLARE_SCRIPT(TB06)
END_SCRIPT

DECLARE_SCRIPT(TB07)
	void sub_401B0C();
END_SCRIPT

DECLARE_SCRIPT(UG01)
END_SCRIPT

DECLARE_SCRIPT(UG02)
	bool sub_402354();
END_SCRIPT

DECLARE_SCRIPT(UG03)
END_SCRIPT

DECLARE_SCRIPT(UG04)
END_SCRIPT

DECLARE_SCRIPT(UG05)
	int sub_4021B0();
	void sub_402218();
END_SCRIPT

DECLARE_SCRIPT(UG06)
END_SCRIPT

DECLARE_SCRIPT(UG07)
END_SCRIPT

DECLARE_SCRIPT(UG08)
END_SCRIPT

DECLARE_SCRIPT(UG09)
END_SCRIPT

DECLARE_SCRIPT(UG10)
END_SCRIPT

// UG11 does not exists

DECLARE_SCRIPT(UG12)
END_SCRIPT

DECLARE_SCRIPT(UG13)
	void sub_40223C();
	void sub_4023D8();
	void sub_4025E0();
	void sub_402960();
	int sub_402AD0();
	void sub_402AD4();
	void sub_402E24();
END_SCRIPT

DECLARE_SCRIPT(UG14)
END_SCRIPT

DECLARE_SCRIPT(UG15)
END_SCRIPT

DECLARE_SCRIPT(UG16)
	void sub_401D78();
END_SCRIPT

DECLARE_SCRIPT(UG17)
END_SCRIPT

DECLARE_SCRIPT(UG18)
	void sub_402734();
	void sub_402DE8();
	void sub_402F8C();
	void sub_403114();
	void sub_403278();
	void sub_403588();
END_SCRIPT

DECLARE_SCRIPT(UG19)
END_SCRIPT

#undef DECLARE_SCRIPT

/*
 * Actor Scripts
 */

class AIScriptBase : public ScriptBase {
public:
	AIScriptBase(BladeRunnerEngine *vm) : ScriptBase(vm) {}

	virtual void Initialize() = 0;
	virtual bool Update() = 0;
	virtual void TimerExpired(int timer) = 0;
	virtual void CompletedMovementTrack() = 0;
	virtual void ReceivedClue(int clueId, int fromActorId) = 0;
	virtual void ClickedByPlayer() = 0;
	virtual void EnteredScene(int setId) = 0;
	virtual void OtherAgentEnteredThisScene(int otherActorId) = 0;
	virtual void OtherAgentExitedThisScene(int otherActorId) = 0;
	virtual void OtherAgentEnteredCombatMode(int otherActorId, int combatMode) = 0;
	virtual void ShotAtAndMissed() = 0;
	virtual void ShotAtAndHit() = 0;
	virtual void Retired(int byActorId) = 0;
	virtual int GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) = 0;
	virtual bool GoalChanged(int currentGoalNumber, int newGoalNumber) = 0;
	virtual bool UpdateAnimation(int *animation, int *frame) = 0;
	virtual bool ChangeAnimationMode(int mode) = 0;
	virtual void QueryAnimationState(int *animationState, int *a2, int *a3, int *a4) = 0;
	virtual void SetAnimationState(int animationState, int a2, int a3, int a4) = 0;
	virtual bool ReachedMovementTrackWaypoint(int a1) = 0;
	virtual void FledCombat() = 0;
};

class AIScripts {
private:
	BladeRunnerEngine *_vm;
	int                _inScriptCounter;
	int                _actorsCount;
	AIScriptBase     **_AIScripts;
	bool              *_actorUpdating;
public:
	AIScripts(BladeRunnerEngine *vm, int actorsCount);
	~AIScripts();

	void Initialize(int actor);
	void Update(int actor);
	void TimerExpired(int actor, int timer);
	void EnteredScene(int actor, int setId);
	void OtherAgentEnteredThisScene(int actor, int otherActorId);
	void OtherAgentExitedThisScene(int actor, int otherActorId);
	void GoalChanged(int actor, int currentGoalNumber, int newGoalNumber);
	void UpdateAnimation(int actor, int *animation, int *frame);
	void ChangeAnimationMode(int actor, int mode);

	bool IsInsideScript() { return _inScriptCounter > 0; }
};

} // End of namespace BladeRunner

#endif
