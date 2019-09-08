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

#include "bladerunner/bladerunner.h"
#include "bladerunner/game_constants.h"

#include "common/str.h"

namespace BladeRunner {

class BladeRunnerEngine;

class ScriptBase {
friend class SceneScript;

protected:
	BladeRunnerEngine *_vm;

public:
	ScriptBase(BladeRunnerEngine *vm);
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
	void Non_Player_Actor_Combat_Mode_On(int actorId, int initialState, bool rangedAttack, int enemyId, int waypointType, int animationModeCombatIdle, int animationModeCombatWalk, int animationModeCombatRun, int fleeRatio, int coverRatio, int attackRatio, int damage, int range, bool unstoppable);
	void Non_Player_Actor_Combat_Mode_Off(int actorId);
	void Actor_Set_Health(int actorId, int hp, int maxHp);
	void Actor_Set_Targetable(int actorId, bool targetable);
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
	bool Loop_Actor_Walk_To_Actor(int actorId, int otherActorId, int proximity, bool interruptible, bool run);
	bool Loop_Actor_Walk_To_Item(int actorId, int itemId, int proximity, bool interruptible, bool run);
	bool Loop_Actor_Walk_To_Scene_Object(int actorId, const char *objectName, int proximity, bool interruptible, bool run);
	bool Loop_Actor_Walk_To_Waypoint(int actorId, int waypointId, int proximity, bool interruptible, bool run);
	bool Loop_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int proximity, bool interruptible, bool run, bool force);
	void Async_Actor_Walk_To_Waypoint(int actorId, int waypointId, int proximity, bool run);
	void Async_Actor_Walk_To_XYZ(int actorId, float x, float y, float z, int proximity, bool run);
	void Actor_Force_Stop_Walking(int actorId);
	void Loop_Actor_Travel_Stairs(int actorId, int stepCount, bool up, int animationModeEnd);
	void Loop_Actor_Travel_Ladder(int actorId, int stepCount, bool up, int animationModeEnd);
	void Actor_Clue_Add_To_Database(int actorId, int clueId, int weight, bool clueAcquired, bool unknownFlag, int fromActorId);
	void Actor_Clue_Acquire(int actorId, int clueId, bool unknownFlag, int fromActorId);
	void Actor_Clue_Lose(int actorId, int clueId);
	bool Actor_Clue_Query(int actorId, int clueId);
	bool Actor_Clues_Transfer_New_To_Mainframe(int actorId);
	bool Actor_Clues_Transfer_New_From_Mainframe(int actorId);
	void Actor_Set_Invisible(int actorId, bool isInvisible);
	void Actor_Set_Immunity_To_Obstacles(int actorId, bool isImmune);
	void Item_Add_To_World(int itemId, int animationId, int setId, float x, float y, float z, signed int facing, int height, int width, bool isTargetable, bool isObstacle, bool isPoliceMazeEnemy, bool updateOnly);
	void Item_Remove_From_World(int itemId);
	void Item_Spin_In_World(int itemId);
	void Item_Flag_As_Target(int itemId);
	void Item_Flag_As_Non_Target(int itemId);
	void Item_Pickup_Spin_Effect(int animationId, int x, int y);
	void Item_Pickup_Spin_Effect_From_Actor(int animationId, int actorId, int xOffset = 0, int yOffset = 0); // new for restored content mostly
	bool Item_Query_Visible(int itemId);
	void Set_Subtitle_Text_On_Screen(Common::String displayText);
#if BLADERUNNER_ORIGINAL_BUGS
#else
	void Screen_Effect_Skip(int effectInc);
	void Screen_Effect_Restore(int effectInc);
	void Screen_Effect_Restore_All();
#endif // BLADERUNNER_ORIGINAL_BUGS
	int Animation_Open();
	int Animation_Close();
	int Animation_Start();
	int Animation_Stop();
	int Animation_Skip_To_Frame();
	void Delay(uint32 miliseconds);
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
	void Sound_Play_Speech_Line(int actorId, int sentenceId, int volume, int a4, int priority);
	void Sound_Left_Footstep_Walk(int actorId);
	void Sound_Right_Footstep_Walk(int actorId);
	void Sound_Left_Footstep_Run(int actorId);
	void Sound_Right_Footstep_Run(int actorId);
	// Sound_Walk_Shuffle_Stop
	void Footstep_Sounds_Set(int index, int value);
	void Footstep_Sound_Override_On(int footstepSoundOverride);
	void Footstep_Sound_Override_Off();
	bool Music_Play(int musicId, int volume, int pan, int32 timeFadeIn, int32 timePlay, int loop, int32 timeFadeOut);
	void Music_Adjust(int volume, int pan, uint32 delay);
	void Music_Stop(uint32 delay);
	bool Music_Is_Playing();
	void Overlay_Play(const char *overlay, int loopId, bool loopForever, bool startNow, int a5);
	void Overlay_Remove(const char *overlay);
	void Scene_Loop_Set_Default(int loopId);
	void Scene_Loop_Start_Special(int sceneLoopMode, int loopId, bool immediately);
	void Outtake_Play(int id, int noLocalization = false, int container = -1);
	void Ambient_Sounds_Add_Sound(int sfxId, uint32 timeMin, uint32 timeMax, int volumeMin, int volumeMax, int panStartMin, int panStartMax, int panEndMin, int panEndMax, int priority, int unk);
	void Ambient_Sounds_Remove_Sound(int sfxId, bool stopPlaying);
	void Ambient_Sounds_Add_Speech_Sound(int actorId, int sentenceId, uint32 timeMin, uint32 timeMax, int volumeMin, int volumeMax, int panStartMin, int panStartMax, int panEndMin, int panEndMax, int priority, int unk);
	// Ambient_Sounds_Remove_Speech_Sound
	void Ambient_Sounds_Play_Sound(int sfxId, int volume, int panStart, int panEnd, int priority);
	void Ambient_Sounds_Play_Speech_Sound(int actorId, int sfxId, int volume, int panStart, int panEnd, int priority);
	void Ambient_Sounds_Remove_All_Non_Looping_Sounds(bool stopPlaying);
	void Ambient_Sounds_Add_Looping_Sound(int sfxId, int volume, int pan, uint32 delay);
	void Ambient_Sounds_Adjust_Looping_Sound(int sfxId, int volume, int pan, uint32 delay);
	void Ambient_Sounds_Remove_Looping_Sound(int sfxId, uint32 delay);
	void Ambient_Sounds_Remove_All_Looping_Sounds(uint32 delay);
	void Setup_Scene_Information(float actorX, float actorY, float actorZ, int actorFacing);
	bool Dialogue_Menu_Appear(int x, int y);
	bool Dialogue_Menu_Disappear();
	bool Dialogue_Menu_Clear_List();
	bool Dialogue_Menu_Add_To_List(int answer);
	bool Dialogue_Menu_Add_DONE_To_List(int answer);
	bool Dialogue_Menu_Add_To_List_Never_Repeat_Once_Selected(int answer);
	bool DM_Add_To_List(int answer, int priorityPolite, int priorityNormal, int prioritySurly);
	bool DM_Add_To_List_Never_Repeat_Once_Selected(int answer, int priorityPolite, int priorityNormal, int prioritySurly);
	bool Dialogue_Menu_Clear_Never_Repeat_Was_Selected_Flag(int answer);
	bool Dialogue_Menu_Remove_From_List(int answer);
	int Dialogue_Menu_Query_Input();
	int Dialogue_Menu_Query_List_Size();
	void Scene_Exit_Add_2D_Exit(int index, int left, int top, int right, int down, int type);
	void Scene_Exit_Remove(int index);
	void Scene_Exits_Disable();
	void Scene_Exits_Enable();
	void Scene_2D_Region_Add(int index, int left, int top, int right, int down);
	void Scene_2D_Region_Remove(int index);
	void World_Waypoint_Set(int waypointId, int setId, float x, float y, float z);
	// World_Waypoint_Reset
	float World_Waypoint_Query_X(int waypointId);
	float World_Waypoint_Query_Y(int waypointId);
	float World_Waypoint_Query_Z(int waypointId);
	void Combat_Cover_Waypoint_Set_Data(int coverWaypointId, int a2, int setId, int a4, float x, float y, float z);
	void Combat_Flee_Waypoint_Set_Data(int fleeWaypointId, int a2, int setId, int a4, float x, float y, float z, int a8);
	void Police_Maze_Target_Track_Add(int itemId, float startX, float startY, float startZ, float endX, float endY, float endZ, int steps, const int* instructions, bool isActive);
	int Police_Maze_Query_Score();
	void Police_Maze_Zero_Score();
	void Police_Maze_Increment_Score(int delta);
	void Police_Maze_Decrement_Score(int delta);
	void Police_Maze_Set_Score(int value);
	void Police_Maze_Set_Pause_State(bool state);
	void CDB_Set_Crime(int clueId, int crimeId);
	void CDB_Set_Clue_Asset_Type(int clueId, int assetType);
	void SDB_Set_Actor(int suspectId, int actorId);
	bool SDB_Add_Photo_Clue(int suspectId, int clueId, int shapeId);
	void SDB_Set_Name(int suspectId);
	void SDB_Set_Sex(int suspectId, int sex);
	bool SDB_Add_Identity_Clue(int suspectId, int clueId);
	bool SDB_Add_MO_Clue(int suspectId, int clueId);
	bool SDB_Add_Whereabouts_Clue(int suspectId, int clueId);
	bool SDB_Add_Replicant_Clue(int suspectId, int clueId);
	bool SDB_Add_Non_Replicant_Clue(int suspectId, int clueId);
	bool SDB_Add_Other_Clue(int suspectId, int clueId);
	void Spinner_Set_Selectable_Destination_Flag(int destination, bool selectable);
	// Spinner_Query_Selectable_Destination_Flag(int destination);
	int Spinner_Interface_Choose_Dest(int loopId, bool immediately);
	void ESPER_Flag_To_Activate();
	void Voight_Kampff_Activate(int actorId, int calibrationRatio);
	int Elevator_Activate(int elevatorId);
	void View_Score_Board();
	int Query_Score(int a0);
	void Set_Score(int a0, int a1);
	void Give_McCoy_Ammo(int ammoType, int ammo);
	void Assign_Player_Gun_Hit_Sounds(int ammoType, int soundId1, int soundId2, int soundId3);
	void Assign_Player_Gun_Miss_Sounds(int ammoType, int soundId1, int soundId2, int soundId3);
	void Disable_Shadows(int animationsIdsList[], int listSize);
	bool Query_System_Currently_Loading_Game();
	void Actor_Retired_Here(int actorId, int width, int height, bool retired, int retiredByActorId);
	void Clickable_Object(const char *objectName);
	void Unclickable_Object(const char *objectName);
	void Obstacle_Object(const char *objectName, bool updateWalkpath);
	void Unobstacle_Object(const char *objectName, bool updateWalkpath);
	void Obstacle_Flag_All_Objects(bool isObstacle);
	void Combat_Target_Object(const char *objectName);
	void Un_Combat_Target_Object(const char *objectName);
	void Set_Fade_Color(float r, float g, float b);
	void Set_Fade_Density(float density);
	void Set_Fog_Color(const char *fogName, float r, float g, float b);
	void Set_Fog_Density(const char *fogName, float density);
	void ADQ_Flush();
	void ADQ_Add(int actorId, int sentenceId, int animationMode);
	void ADQ_Add_Pause(int32 delay);
	void ADQ_Wait_For_All_Queued_Dialogue();
	bool Game_Over();
	void Autosave_Game(int textId);
	void I_Sez(const char *str);

	void AI_Countdown_Timer_Start(int actorId, signed int timer, int32 seconds);
	void AI_Countdown_Timer_Reset(int actorId, int timer);
	void AI_Movement_Track_Unpause(int actorId);
	void AI_Movement_Track_Pause(int actorId);
	void AI_Movement_Track_Repeat(int actorId);
	void AI_Movement_Track_Append_Run_With_Facing(int actorId, int waypointId, int32 delay, int angle);
	void AI_Movement_Track_Append_With_Facing(int actorId, int waypointId, int32 delay, int angle);
	void AI_Movement_Track_Append_Run(int actorId, int waypointId, int32 delay);
	void AI_Movement_Track_Append(int actorId, int waypointId, int32 delay);
	void AI_Movement_Track_Flush(int actorId);

	void ESPER_Add_Photo(const char *name, int photoId, int shapeId);
	void ESPER_Define_Special_Region(int regionId, int innerLeft, int innerTop, int innerRight, int innerBottom, int outerLeft, int outerTop, int outerRight, int outerBottom, int selectionLeft, int selectionTop, int selectionRight, int selectionBottom, const char *name);

	void KIA_Play_Actor_Dialogue(int actorId, int sentenceId);
	void KIA_Play_Slice_Model(int sliceModelId);
	void KIA_Play_Photograph(int photographId);

	void VK_Play_Speech_Line(int actorId, int sentenceId, float duration);
	void VK_Add_Question(int intensity, int sentenceId, int relatedSentenceId);
	void VK_Subject_Reacts(int intensity, int humanResponse, int replicantResponse, int anxiety);
	void VK_Eye_Animates(int loopId);
};

} // End of namespace BladeRunner

#endif
