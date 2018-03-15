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

#include "bladerunner/script/ai_script.h"

namespace BladeRunner {

AIScriptIzo::AIScriptIzo(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = 0;
	_var1 = 6;
	_var2 = 1;
	_var3 = 0;
	_var4 = 0;
}

void AIScriptIzo::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = 0;
	_var1 = 6;
	_var2 = 1;
	_var3 = 0;
	_var4 = 0;

	Actor_Set_Goal_Number(kActorIzo, 0);
	Actor_Put_In_Set(kActorIzo, kSetHC01_HC02_HC03_HC04);
	Actor_Set_At_XYZ(kActorIzo, 591.0f, 0.14f, 25.0f, 540);
	World_Waypoint_Set(349, 70, -14.7f, -4.01f, 224.5f);
}

bool AIScriptIzo::Update() {
	if (Actor_Query_Goal_Number(kActorIzo) == 100 && Player_Query_Current_Scene() == kSceneUG02) {
		Actor_Set_Targetable(kActorIzo, 1);
		Actor_Set_Goal_Number(kActorIzo, 101);
		return true;
	}

	if (Global_Variable_Query(kVariableChapter) == 3
			&& Actor_Query_Goal_Number(kActorIzo) == 599
			&& Actor_Query_Which_Set_In(kActorIzo) == 70) {
		Actor_Put_In_Set(kActorIzo, kSetFreeSlotI);
		Actor_Set_At_Waypoint(kActorIzo, 41, 0);
	}
	if (!Actor_Query_Goal_Number(kActorIzo) && Player_Query_Current_Scene() == kSceneTB02) {
		Actor_Set_Goal_Number(kActorIzo, 155);
	}
	if (Global_Variable_Query(kVariableChapter) == 4
			&& Actor_Query_Goal_Number(kActorIzo) < 599
			&& Actor_Query_Goal_Number(kActorIzo) < 300
			&& Actor_Query_Goal_Number(kActorIzo) != 180) {
		Actor_Set_Goal_Number(kActorIzo, 300);
	}
	if (Global_Variable_Query(kVariableChapter) == 5 && Actor_Query_Goal_Number(kActorIzo) < 400) {
		Actor_Set_Goal_Number(kActorIzo, 400);
	}

	return false;
}

void AIScriptIzo::TimerExpired(int timer) {
	//return false;
}

void AIScriptIzo::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorIzo)) {
	case 3:
		Actor_Set_Goal_Number(kActorIzo, 101);
		Player_Gains_Control();
		return; //true;

	case 101:
		Actor_Set_Goal_Number(kActorIzo, 102);
		return; //true;

	case 110:
	case 111:
		Actor_Set_Goal_Number(kActorIzo, 114);
		Actor_Set_Goal_Number(kActorSteele, 100);
		Scene_Exits_Enable();
		return; //true;

	case 114:
		Scene_Exits_Enable();
		return; //true;

	case 120:
		Game_Flag_Set(164);
		Actor_Set_Goal_Number(kActorIzo, 180);
		Scene_Exits_Enable();
		return; //true;

	case 150:
		Actor_Set_Goal_Number(kActorIzo, 155);
		return; //true;

	case 155:
		Actor_Set_Goal_Number(kActorIzo, 150);
		return; //true;

	case 300:
		Actor_Set_Goal_Number(kActorIzo, 301);
		return; //true;

	case 301:
		Actor_Set_Goal_Number(kActorIzo, 300);
		return; //true;
	}

	return; //false;
}

void AIScriptIzo::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptIzo::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorIzo) > 500) {
		Actor_Face_Actor(kActorMcCoy, kActorIzo, 1);
		Actor_Says(kActorMcCoy, 8585, 13);
		return; //true;
	}

	if (Actor_Query_Goal_Number(kActorIzo) == 101 && Player_Query_Current_Set() == 75) {
		Player_Loses_Control();
		Actor_Set_Goal_Number(kActorIzo, 100);
		Actor_Face_Actor(kActorMcCoy, kActorIzo, 1);
		Actor_Says(kActorMcCoy, 5460, 16);
		Actor_Face_Actor(kActorIzo, kActorMcCoy, 1);
		Actor_Says(kActorIzo, 700, 17);
		Actor_Says(kActorMcCoy, 5465, 14);
		someDialog();
	}

	if (Actor_Query_Goal_Number(kActorIzo) == 110) {
		Actor_Face_Actor(kActorMcCoy, kActorIzo, 1);
		Actor_Says(kActorMcCoy, 2715, 14);
		Actor_Set_Goal_Number(kActorIzo, 111);
		Actor_Says(kActorMcCoy, 1800, 14);
		Actor_Set_Goal_Number(kActorSteele, 100);
		return; //true;
	}

	return; //false;
}

void AIScriptIzo::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptIzo::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptIzo::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptIzo::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (Actor_Query_Goal_Number(kActorIzo) == 110) {
		Game_Flag_Query(44);
	}
	return; //false;
}

void AIScriptIzo::ShotAtAndMissed() {
	if (Actor_Query_Goal_Number(kActorIzo) != 110) {
		return; //false;
	}

	Actor_Set_Goal_Number(kActorIzo, 111);

	return; //true;
}

bool AIScriptIzo::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorIzo) == 110
			|| Actor_Query_Goal_Number(kActorIzo) == 111
			|| Actor_Query_Goal_Number(kActorIzo) == 114) {
		Actor_Set_Health(kActorIzo, 50, 50);

		if (Actor_Query_Goal_Number(kActorIzo) == 110) {
			Actor_Set_Goal_Number(kActorIzo, 111);
		}
		return true;
	} else {
		AI_Movement_Track_Flush(kActorIzo);
		Global_Variable_Increment(19, 1);
		if (!Game_Flag_Query(444) && Global_Variable_Query(19) == 1) {
			Game_Flag_Set(444);
			_animationFrame = 0;
			_animationState = 19;
			Actor_Retired_Here(kActorIzo, 36, 12, 1, -1);
			Actor_Set_Goal_Number(kActorIzo, 199);
		}
		return false;
	}
}

void AIScriptIzo::Retired(int byActorId) {
	if (!Actor_Query_In_Set(kActorIzo, kSetKP07)) {
		return; //false;
	}

	Global_Variable_Decrement(51, 1);
	Actor_Set_Goal_Number(kActorIzo, 599);

	if (Global_Variable_Query(51)) {
		return; //false;
	}

	Player_Loses_Control();
	Delay(2000);
	Player_Set_Combat_Mode(0);
	Loop_Actor_Walk_To_XYZ(kActorMcCoy, -12.0f, -41.58f, 72.0f, 0, 1, 0, 0);
	Ambient_Sounds_Remove_All_Non_Looping_Sounds(1);
	Ambient_Sounds_Remove_All_Looping_Sounds(1);
	Game_Flag_Set(579);
	Game_Flag_Reset(653);
	Set_Enter(kSetKP05_KP06, kSetKP03);

	return; //true;
}

int AIScriptIzo::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptIzo::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == 200
		|| newGoalNumber == 199
		|| newGoalNumber == 198
		|| newGoalNumber == 180
		|| newGoalNumber == 103) {
			Spinner_Set_Selectable_Destination_Flag(6, 1);
		}

	switch (newGoalNumber) {
	case 0:
		AI_Movement_Track_Flush(kActorIzo);
		return true;

	case 1:
		AI_Movement_Track_Flush(kActorIzo);
		Actor_Face_Heading(kActorIzo, 520, 0);
		_animationState = 32;
		_animationFrame = -1;
		return true;

	case 2:
		_animationState = 34;
		_animationFrame = -1;
		return true;

	case 3:
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Append_Run(kActorIzo, 149, 0);
		AI_Movement_Track_Append_Run(kActorIzo, 152, 0);
		AI_Movement_Track_Repeat(kActorIzo);
		Scene_Exit_Add_2D_Exit(1, 394, 229, 485, 371, 1);
		Game_Flag_Set(402);
		return true;

	case 100:
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Repeat(kActorIzo);
		return true;

	case 101:
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Append_Run(kActorIzo, 153, 0);
		AI_Movement_Track_Append(kActorIzo, 154, 0);
		AI_Movement_Track_Repeat(kActorIzo);
		return true;

	case 102:
		return true;

	case 103:
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Append(kActorIzo, 153, 0);
		AI_Movement_Track_Append(kActorIzo, 39, 120);
		AI_Movement_Track_Append(kActorIzo, 33, 0);
		AI_Movement_Track_Repeat(kActorIzo);
		return 1;

	case 110:
		Game_Flag_Set(486);
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Append(kActorIzo, 349, 0);
		AI_Movement_Track_Repeat(kActorIzo);
		return true;

	case 111:
		Game_Flag_Set(486);
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Append_Run(kActorIzo, 349, 0);
		AI_Movement_Track_Repeat(kActorIzo);
		return true;

	case 114:
		Game_Flag_Set(486);
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Append_Run(kActorIzo, 174, 0);
		AI_Movement_Track_Repeat(kActorIzo);
		return true;

	case 115:
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Append(kActorIzo, 39, 60);
		if (Game_Flag_Query(44)) {
			AI_Movement_Track_Append(kActorIzo, 33, 0);
		} else {
			AI_Movement_Track_Append(kActorIzo, 34, 0);
		}
		AI_Movement_Track_Repeat(kActorIzo);
		Game_Flag_Set(486);
		return true;

	case 120:
		Actor_Set_Targetable(kActorIzo, 0);
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Append(kActorIzo, 174, 0);
		AI_Movement_Track_Append(kActorIzo, 33, 0);
		AI_Movement_Track_Repeat(kActorIzo);
		return true;

	case 150:
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Append(kActorIzo, 39, Random_Query(15, 30));
		AI_Movement_Track_Append(kActorIzo, 149, 0);
		AI_Movement_Track_Append(kActorIzo, 323, Random_Query(90, 120));
		AI_Movement_Track_Repeat(kActorIzo);
		return true;

	case 155:
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Append(kActorIzo, 149, 0);
		if (Game_Flag_Query(44)) {
			AI_Movement_Track_Append(kActorIzo, 39, 5);
			AI_Movement_Track_Append(kActorIzo, 34, Random_Query(10, 20));
			AI_Movement_Track_Append(kActorIzo, 39, 5);
			AI_Movement_Track_Append(kActorIzo, 33, Random_Query(10, 20));
		} else {
			AI_Movement_Track_Append(kActorIzo, 39, Random_Query(5, 15));
			AI_Movement_Track_Append(kActorIzo, 34, Random_Query(20, 40));
		}
		AI_Movement_Track_Repeat(kActorIzo);
		return true;

	case 160:
		AI_Movement_Track_Flush(kActorIzo);
		return true;

	case 180:
		Game_Flag_Set(486);
		Actor_Put_In_Set(kActorIzo, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorIzo, 33, 0);
		return true;

	case 198:
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Append(kActorIzo, 41, 0);
		AI_Movement_Track_Repeat(kActorIzo);
		Actor_Set_Goal_Number(kActorIzo, 999);
		return true;

	case 199:
		Game_Flag_Set(486);
		AI_Movement_Track_Flush(kActorIzo);
		Ambient_Sounds_Play_Speech_Sound(kActorIzo, 9000, 100, 0, 0, 0);
		Actor_Change_Animation_Mode(kActorIzo, 48);
		Actor_Set_Goal_Number(kActorIzo, 999);
		Scene_Exits_Enable();
		Actor_Retired_Here(kActorIzo, 36, 12, 1, -1);
		return true;

	case 200:
		Game_Flag_Set(486);
		return true;

	case 300:
		AI_Movement_Track_Flush(kActorIzo);
		modifyWaypoints();
		AI_Movement_Track_Repeat(kActorIzo);
		return true;

	case 301:
		AI_Movement_Track_Flush(kActorIzo);
		AI_Movement_Track_Append(kActorIzo, 34, 1);
		AI_Movement_Track_Repeat(kActorIzo);
		return true;

	case 400:
		AI_Movement_Track_Flush(kActorIzo);
		Actor_Put_In_Set(kActorIzo, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorIzo, 33, 0);
		return true;

	case 999:
		Actor_Set_Goal_Number(kActorIzo, 599);
		return true;

	case 9999:
		return true;

	}

	return false;
}

bool AIScriptIzo::UpdateAnimation(int *animation, int *frame) {
	return true;
}

bool AIScriptIzo::ChangeAnimationMode(int mode) {
	return true;
}

void AIScriptIzo::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptIzo::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptIzo::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptIzo::FledCombat() {
	// return false;
}

void AIScriptIzo::someDialog() {
	int input;

	Dialogue_Menu_Clear_List();
	DM_Add_To_List_Never_Repeat_Once_Selected(0, 7, 4, -1);

	if (Actor_Clue_Query(0, 179)) {
		DM_Add_To_List_Never_Repeat_Once_Selected(10, 8, -1, -1);
	}

	DM_Add_To_List_Never_Repeat_Once_Selected(20, 3, 7, 4);
	DM_Add_To_List_Never_Repeat_Once_Selected(30, -1, 3, 7);
	Dialogue_Menu_Appear(320, 240);

	input = Dialogue_Menu_Query_Input();

	Dialogue_Menu_Disappear();

	switch (input) {
	case 0:
		Actor_Says(kActorMcCoy, 5470, 15);
		Actor_Says(kActorMcCoy, 710, 13);
		Actor_Set_Goal_Number(kActorIzo, 103);
		Player_Gains_Control();
		break;

	case 10:
		if (Game_Flag_Query(44) == 1) {
			Actor_Says(kActorMcCoy, 5475, 18);
			Actor_Says(kActorIzo, 720, 12);
			Actor_Says(kActorMcCoy, 5485, 13);
			Actor_Says(kActorIzo, 740, 14);
			Actor_Says(kActorMcCoy, 5495, 12);
			Actor_Says(kActorIzo, 750, 15);
			Actor_Says(kActorIzo, 760, 17);
			Actor_Says(kActorMcCoy, 5500, 12);
			Actor_Says(kActorIzo, 770, 15);
			Actor_Says(kActorIzo, 780, 15);
			Actor_Says(kActorMcCoy, 5505, 12);
			Actor_Says(kActorIzo, 790, 15);
		} else {
			Actor_Says(kActorMcCoy, 5510, 15);
			Actor_Says(kActorIzo, 820, 13);
			Actor_Says(kActorMcCoy, 5520, 13);
			Actor_Says(kActorIzo, 830, 13);
			Actor_Says(kActorIzo, 840, 13);
		}
		Actor_Set_Goal_Number(kActorIzo, 103);
		Player_Gains_Control();
		break;

	case 20:
		Actor_Says(kActorMcCoy, 5480, 18);
		Actor_Set_Goal_Number(kActorIzo, 103);
		Player_Gains_Control();
		break;

	case 30:
		Player_Set_Combat_Mode(kActorSteele);
		Actor_Set_Goal_Number(kActorIzo, 103);
		Player_Gains_Control();
		break;
	}
}

void AIScriptIzo::modifyWaypoints() {
	switch (Random_Query(1, 10) - 1) {
	case 0:
		World_Waypoint_Set(484, 54, -212.58f, 23.38f, -1859.45f);
		World_Waypoint_Set(485, 54, 355.49f, 31.66f, -859.81f);
		World_Waypoint_Set(486, 11, -323.89f, -24.0f, 35.58f);
		World_Waypoint_Set(487, 11, -211.89f, -24.0f, 35.58f);
		AI_Movement_Track_Append(kActorIzo, 484, 1);
		AI_Movement_Track_Append(kActorIzo, 485, 10);
		AI_Movement_Track_Append(kActorIzo, 486, 1);
		AI_Movement_Track_Append(kActorIzo, 487, 15);
		AI_Movement_Track_Append(kActorIzo, 486, 1);
		AI_Movement_Track_Append(kActorIzo, 485, 1);
		AI_Movement_Track_Append(kActorIzo, 484, 1);
		break;

	case 1:
		World_Waypoint_Set(484, 13, -1335.0f, 0.0f, -542.0f);
		World_Waypoint_Set(485, 13, -1027.0f, 0.0f, -542.0f);
		AI_Movement_Track_Append(kActorIzo, 484, 1);
		AI_Movement_Track_Append(kActorIzo, 485, 20);
		AI_Movement_Track_Append(kActorIzo, 484, 1);
		break;

	case 2:
		World_Waypoint_Set(484, 37, -352.16f, 8.0f, -379.24f);
		World_Waypoint_Set(485, 37, 108.2f, 8.0f, -934.80f);
		AI_Movement_Track_Append(kActorIzo, 484, 1);
		AI_Movement_Track_Append(kActorIzo, 485, 1);
		break;

	case 3:
		World_Waypoint_Set(484, 39, 589.59f, 47.76f, -1153.76f);
		World_Waypoint_Set(485, 39, 481.59f, 47.76f, -429.76f);
		World_Waypoint_Set(486, 38, 524.0f, 47.76f, -562.0f);
		World_Waypoint_Set(487, 38, -10.0f, 47.76f, -327.0f);
		AI_Movement_Track_Append(kActorIzo, 484, 1);
		AI_Movement_Track_Append(kActorIzo, 485, 1);
		AI_Movement_Track_Append(kActorIzo, 486, 1);
		AI_Movement_Track_Append(kActorIzo, 487, 1);
		break;

	default:
		AI_Movement_Track_Append(kActorIzo, 34, 60);
		break;
	}
}

} // End of namespace BladeRunner
