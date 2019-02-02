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

AIScriptOfficerGrayford::AIScriptOfficerGrayford(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 0;
	_var2 = 0;
	_var3 = 0;
}

void AIScriptOfficerGrayford::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 0;
	_var2 = 0;
	_var3 = 0;

	Actor_Put_In_Set(kActorOfficerGrayford, kSetFreeSlotG);
	Actor_Set_At_Waypoint(kActorOfficerGrayford, 39, 0);
	Actor_Set_Goal_Number(kActorOfficerGrayford, 0);
}

bool AIScriptOfficerGrayford::Update() {
	if (Global_Variable_Query(kVariableChapter) == 4 && Actor_Query_Goal_Number(kActorOfficerGrayford) < 300) {
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 300);
	} else if (Global_Variable_Query(kVariableChapter) == 5 && Actor_Query_Goal_Number(kActorOfficerGrayford) < 400) {
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 400);
	} else if (!Game_Flag_Query(kFlagMcCoyInDNARow)
			&& Actor_Query_Goal_Number(kActorOfficerGrayford) > 102
			&& Actor_Query_Goal_Number(kActorOfficerGrayford) < 110) {
		Actor_Set_Goal_Number(kActorOfficerGrayford, 110);
	} else if (Actor_Query_Goal_Number(kActorOfficerGrayford) == 0) {
		Actor_Set_Goal_Number(kActorOfficerGrayford, 1);
	} else 	if (Actor_Query_Goal_Number(kActorOfficerGrayford) == 10) {
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 0);
	} else if (Actor_Query_Goal_Number(kActorOfficerGrayford) == 102) {
		Actor_Set_Goal_Number(kActorOfficerGrayford, 103);
	} else if (Game_Flag_Query(629)) {
		AI_Movement_Track_Unpause(kActorGenwalkerA);
		AI_Movement_Track_Unpause(kActorGenwalkerB);
		AI_Movement_Track_Unpause(kActorGenwalkerC);
	} else if (Actor_Query_Goal_Number(kActorOfficerGrayford) == 310
				&& Actor_Query_Which_Set_In(kActorOfficerGrayford) != Player_Query_Current_Set()) {
		Non_Player_Actor_Combat_Mode_Off(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 305);
	} else if (Actor_Query_Goal_Number(kActorOfficerGrayford) == 599
				&& Actor_Query_Which_Set_In(kActorOfficerGrayford) != Player_Query_Current_Set()) {
		Actor_Set_Health(kActorOfficerGrayford, 50, 50);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 305);
	} else if (Actor_Query_Goal_Number(kActorOfficerGrayford) == 305) {
		switch (Actor_Query_Which_Set_In(kActorOfficerGrayford)) {
		case kSetRC03:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 18, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG01:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 11, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG04:
		case kSetUG05:
		case kSetUG06:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 10, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG08:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 13, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG10:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 14, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG12:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 16, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetUG14:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 17, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetMA07:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 7, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetNR01:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 3, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetDR01_DR02_DR04:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 0, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetBB01:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 1, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		case kSetCT11:
			if (Actor_Query_Which_Set_In(kActorOfficerGrayford) == Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 310);
				Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 5, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			}
			break;

		}
	}
	return false;
}

void AIScriptOfficerGrayford::TimerExpired(int timer) {
	if (timer == 2) {
		AI_Countdown_Timer_Reset(kActorOfficerGrayford, 2);
		if (Actor_Query_Goal_Number(kActorOfficerGrayford) == 104) {
			Actor_Set_Goal_Number(kActorOfficerGrayford, 105);
		} else if (Actor_Query_Goal_Number(kActorOfficerGrayford) == 105) {
			Actor_Set_Goal_Number(kActorOfficerGrayford, 104);
		}
	}
}

void AIScriptOfficerGrayford::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorOfficerGrayford)) {
	case 1:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 2);
		break;

	case 2:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 3);
		break;

	case 3:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 4);
		break;

	case 4:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 5);
		break;

	case 5:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 6);
		break;

	case 6:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 7);
		break;

	case 7:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 8);
		break;

	case 8:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 9);
		break;

	case 9:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 10);
		break;

	case 104:
	case 105:
		if (Random_Query(0, 2)) {
			Actor_Change_Animation_Mode(kActorOfficerGrayford, 43);
		} else {
			AI_Countdown_Timer_Reset(kActorOfficerGrayford, 2);
			AI_Countdown_Timer_Start(kActorOfficerGrayford, 2, Random_Query(6, 12));
		}
		Actor_Face_Waypoint(kActorOfficerGrayford, 97, true);
		// return false;
		break;

	case 305:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 306);
		break;

	case 307:
		Non_Player_Actor_Combat_Mode_On(kActorOfficerGrayford, kActorCombatStateIdle, true, kActorMcCoy, 12, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, -1, -1, 15, 300, false);
		break;

	case 308:
		Actor_Change_Animation_Mode(kActorOfficerGrayford, kAnimationModeCombatIdle);
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		break;

	}

	// return true;
}

void AIScriptOfficerGrayford::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptOfficerGrayford::ClickedByPlayer() {
	switch (Actor_Query_Goal_Number(kActorOfficerGrayford)) {
	case 1:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 99);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		if (Random_Query(1, 2) == 1) {
			Actor_Says(kActorMcCoy, 5075, 14);
		} else {
			Actor_Says(kActorMcCoy, 4515, 13);
			Actor_Says(kActorOfficerGrayford, 230, 13);
		}
		Actor_Set_Goal_Number(kActorOfficerGrayford, 1);
		break;

	case 2:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 99);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		if (Random_Query(1, 2) == 1) {
			Actor_Says(kActorMcCoy, 5075, 14);
		} else {
			Actor_Says(kActorMcCoy, 4515, 13);
			Actor_Says(kActorOfficerGrayford, 330, 13);
		}
		Actor_Set_Goal_Number(kActorOfficerGrayford, 2);
		break;

	case 3:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 99);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		if (Random_Query(1, 2) == 1) {
			Actor_Says(kActorMcCoy, 5075, 14);
		} else {
			Actor_Says(kActorMcCoy, 5075, 14); // bug in the original? Matches the above statement
		}
		Actor_Set_Goal_Number(kActorOfficerGrayford, 3);
		break;

	case 4:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 99);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		if (Random_Query(1, 2) == 1) {
			Actor_Says(kActorMcCoy, 5075, 14);
			Actor_Says(kActorOfficerGrayford, 160, 13);
		} else {
			Actor_Says(kActorMcCoy, 4515, 13);
			Actor_Says(kActorOfficerGrayford, 330, 13);
		}
		Actor_Set_Goal_Number(kActorOfficerGrayford, 4);
		break;

	case 7:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 99);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Says(kActorMcCoy, 4515, 14);
		Actor_Says(kActorOfficerGrayford, 330, 13);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 7);
		break;

	case 8:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 99);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		Actor_Says(kActorMcCoy, 5075, 13);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 8);
		break;

	case 104:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 199);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Says(kActorMcCoy, 1005, kAnimationModeTalk);
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Countdown_Timer_Reset(kActorOfficerGrayford, 2);
		if (_animationState == 35 || _animationState == 34) {
			_animationState = 37;
			_animationFrame = 0;
		}
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		Actor_Says(kActorOfficerGrayford, 190, 19);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 104);
		break;

	case 105:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 199);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Says(kActorMcCoy, 1005, kAnimationModeTalk);
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Countdown_Timer_Reset(kActorOfficerGrayford, 2);
		if (_animationState == 35 || _animationState == 34) {
			_animationState = 37;
			_animationFrame = 0;
		}
		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, 1);
		Actor_Says(kActorOfficerGrayford, 190, 19);
		Actor_Set_Goal_Number(kActorOfficerGrayford, 105);
		break;

	default:
		return; //false;
		break;
	}

	return; //true;
}

void AIScriptOfficerGrayford::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptOfficerGrayford::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptOfficerGrayford::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptOfficerGrayford::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptOfficerGrayford::ShotAtAndMissed() {
	// return false;
}

bool AIScriptOfficerGrayford::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorOfficerGrayford) == 307)
		Actor_Set_Health(kActorOfficerGrayford, 50, 50);

	return false;
}

void AIScriptOfficerGrayford::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorOfficerGrayford, 599);
	Game_Flag_Set(kFlagMcCoyRetiredHuman);
}

int AIScriptOfficerGrayford::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptOfficerGrayford::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 1:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 82, Random_Query(5, 20));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 2:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 76, Random_Query(10, 20));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 3:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 77, Random_Query(5, 15));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 4:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 78, Random_Query(5, 15));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 5:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 6:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 79, Random_Query(5, 15));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 7:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 80, 1);
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 8:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 81, Random_Query(5, 15));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 9:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 82, Random_Query(5, 15));
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 101:
		Player_Loses_Control();
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Put_In_Set(kActorOfficerGrayford, kSetDR01_DR02_DR04);
		Actor_Set_At_Waypoint(kActorOfficerGrayford, 110, 0);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Loop_Actor_Walk_To_Waypoint(kActorOfficerGrayford, 111, 0, false, true);

		_animationState = 23;
		_animationFrame = kActorMcCoy;

		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);

		if (Game_Flag_Query(kFlagDR04McCoyShotMoraji)) {
			Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyArrested);
		} else {
			Actor_Says(kActorMcCoy, 960, 15);
			Actor_Says(kActorMcCoy, 965, 18);
			_animationState = 24;
		}
		return true;

	case 102:
		return true;

	case 103:
		Actor_Says(kActorOfficerGrayford, 120, 19);
		Actor_Says_With_Pause(kActorMcCoy, 970, 0.2f, 13);
		Actor_Says(kActorMcCoy, 975, 12);

		if (Actor_Clue_Query(kActorMcCoy, kClueMorajiInterview) == 1) {
			Actor_Says(kActorMcCoy, 980, 16);
			Actor_Says_With_Pause(kActorOfficerGrayford, 130, 0.1f, 13);
			Actor_Says(kActorMcCoy, 985, 14);
			Actor_Says_With_Pause(kActorMcCoy, 990, 0.0f, 17);
			Actor_Says_With_Pause(kActorOfficerGrayford, 140, 1.0f, 16);
			Actor_Says_With_Pause(kActorOfficerGrayford, 150, 0.0f, 17);
			Actor_Says(kActorOfficerGrayford, 160, 15);
			Actor_Says_With_Pause(kActorMcCoy, 995, 0.3f, 14);
		}

		Player_Gains_Control();

		if (Actor_Query_Goal_Number(kActorMoraji) == 23) {
			Actor_Face_Actor(kActorOfficerGrayford, kActorMoraji, 1);
		} else {
			Actor_Face_Waypoint(kActorOfficerGrayford, 97, 1);
		}

		Actor_Change_Animation_Mode(kActorOfficerGrayford, 43);

		if (Player_Query_Current_Scene() == 28) {
			Actor_Says(kActorOfficerGrayford, 170, kAnimationModeTalk);
		}
		return true;

	case 104:
		AI_Countdown_Timer_Reset(kActorOfficerGrayford, 2);
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 112, 0);
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 105:
		AI_Countdown_Timer_Reset(kActorOfficerGrayford, 2);
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 113, 0);
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 106:
		Actor_Face_Actor(kActorMcCoy, kActorOfficerGrayford, true);
		Actor_Says(kActorMcCoy, 1000, 14);
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Countdown_Timer_Reset(kActorOfficerGrayford, 2);

		if (_animationState == 35 || _animationState == 34) {
			_animationState = 37;
			_animationFrame = 0;
		}

		Actor_Face_Actor(kActorOfficerGrayford, kActorMcCoy, true);
		Actor_Says(kActorOfficerGrayford, 180, 18);
		Actor_Set_Goal_Number(kActorOfficerGrayford, currentGoalNumber);
		break;

	case 110:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append(kActorOfficerGrayford, 35, 0);
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		AI_Movement_Track_Flush(kActorMoraji);
		AI_Movement_Track_Append(kActorMoraji, 41, 0);
		AI_Movement_Track_Repeat(kActorMoraji);
		return true;

	case 300:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 305);
		return true;

	case 305:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		switch (Random_Query(1, 10)) {
		case 1:
			AI_Movement_Track_Append(kActorOfficerGrayford, 398, 15);
			AI_Movement_Track_Append(kActorOfficerGrayford, 399, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 400, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 401, 0);
			AI_Movement_Track_Append_With_Facing(kActorOfficerGrayford, 402, 3, 276);
			AI_Movement_Track_Append(kActorOfficerGrayford, 403, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 404, 15);
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			Actor_Set_Goal_Number(kActorOfficerGrayford, 305);
			return true;

		case 2:
			AI_Movement_Track_Append(kActorOfficerGrayford, 385, 10);
			AI_Movement_Track_Append(kActorOfficerGrayford, 242, 2);
			AI_Movement_Track_Append(kActorOfficerGrayford, 386, 2);
			AI_Movement_Track_Append(kActorOfficerGrayford, 387, 15);
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			Actor_Set_Goal_Number(kActorOfficerGrayford, 305);
			return true;

		case 3:
			AI_Movement_Track_Append(kActorOfficerGrayford, 390, 10);
			AI_Movement_Track_Append(kActorOfficerGrayford, 391, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 392, 5);
			AI_Movement_Track_Append(kActorOfficerGrayford, 345, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 393, 15);
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			Actor_Set_Goal_Number(kActorOfficerGrayford, 305);
			return true;

		case 4:
			AI_Movement_Track_Append(kActorOfficerGrayford, 381, 15);
			AI_Movement_Track_Append(kActorOfficerGrayford, 382, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 383, 15);
			AI_Movement_Track_Append(kActorOfficerGrayford, 382, 3);
			AI_Movement_Track_Append(kActorOfficerGrayford, 384, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			Actor_Set_Goal_Number(kActorOfficerGrayford, 305);
			return true;

		case 5:
			AI_Movement_Track_Append(kActorOfficerGrayford, 388, 10);
			AI_Movement_Track_Append(kActorOfficerGrayford, 389, 10);
			AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			Actor_Set_Goal_Number(kActorOfficerGrayford, 305);
			return true;

		case 6:
			AI_Movement_Track_Append(kActorOfficerGrayford, 385, 10);
			AI_Movement_Track_Append(kActorOfficerGrayford, 242, 2);
			AI_Movement_Track_Append(kActorOfficerGrayford, 386, 2);
			AI_Movement_Track_Append(kActorOfficerGrayford, 387, 15);
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			Actor_Set_Goal_Number(kActorOfficerGrayford, 305);
			return true;

		case 7:
			AI_Movement_Track_Append(kActorOfficerGrayford, 394, 15);
			AI_Movement_Track_Append(kActorOfficerGrayford, 395, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 396, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 397, 15);
			AI_Movement_Track_Append(kActorOfficerGrayford, 396, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 395, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 430, 15);
			AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			Actor_Set_Goal_Number(kActorOfficerGrayford, 305);
			return true;

		case 8:
			switch (Random_Query(1, 7)) {
			case 1:
				AI_Movement_Track_Append(kActorOfficerGrayford, 302, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 407, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 408, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				break; // and go to case 9 below

			case 2:
				AI_Movement_Track_Append(kActorOfficerGrayford, 536, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 537, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 538, 5);
				AI_Movement_Track_Append(kActorOfficerGrayford, 537, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 536, 0);
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				break; // and go to case 9 below

			case 3:
				AI_Movement_Track_Append(kActorOfficerGrayford, 296, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 409, 2);
				AI_Movement_Track_Append(kActorOfficerGrayford, 296, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				break; // and go to case 9 below

			case 4:
				AI_Movement_Track_Append(kActorOfficerGrayford, 411, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 412, 5);
				AI_Movement_Track_Append(kActorOfficerGrayford, 411, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				break; // and go to case 9 below

			case 5:
				AI_Movement_Track_Append(kActorOfficerGrayford, 413, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 414, 0);
				AI_Movement_Track_Append_With_Facing(kActorOfficerGrayford, 431, 0, 1017);
				AI_Movement_Track_Append(kActorOfficerGrayford, 432, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				break; // and go to case 9 below

			case 6:
				AI_Movement_Track_Append(kActorOfficerGrayford, 415, 0);
				AI_Movement_Track_Append_With_Facing(kActorOfficerGrayford, 416, 0, 620);
				AI_Movement_Track_Append(kActorOfficerGrayford, 417, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 418, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				break; // and go to case 9 below

			case 7:
				AI_Movement_Track_Append(kActorOfficerGrayford, 405, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 406, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				return false;

			}
			// fall through
			// TODO bug in the game? there should be nothing track related after AI_Movement_Track_Repeat

		case 9:
			if (Random_Query(0, 1)) {
				AI_Movement_Track_Append(kActorOfficerGrayford, 433, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 434, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 435, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
			} else {
				AI_Movement_Track_Append(kActorOfficerGrayford, 420, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 422, 2);
				AI_Movement_Track_Append(kActorOfficerGrayford, 421, 1);
				AI_Movement_Track_Append_With_Facing(kActorOfficerGrayford, 422, 4, 182);
				AI_Movement_Track_Append(kActorOfficerGrayford, 420, 10);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
				// TODO bug in the game? Same code bellow looks like a case 10 and are from set 84 whereas upper one are from set 81
				AI_Movement_Track_Append(kActorOfficerGrayford, 310, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 307, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 309, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 310, 0);
				AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
				AI_Movement_Track_Repeat(kActorOfficerGrayford);
			}
			return false;

		case 10:
			AI_Movement_Track_Append(kActorOfficerGrayford, 310, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 307, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 309, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 310, 0);
			AI_Movement_Track_Append(kActorOfficerGrayford, 35, 30);
			AI_Movement_Track_Repeat(kActorOfficerGrayford);
			return false;

		}
		return false;

	case 306:
		Actor_Set_Goal_Number(kActorOfficerGrayford, 305);
		return true;

	case 307:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append_With_Facing(kActorOfficerGrayford, 419, 0, 512);
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 308:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		AI_Movement_Track_Append_Run(kActorOfficerGrayford, 440, 0);
		AI_Movement_Track_Append_Run(kActorOfficerGrayford, 441, 0);
		AI_Movement_Track_Repeat(kActorOfficerGrayford);
		return true;

	case 399:
		AI_Movement_Track_Flush(kActorOfficerGrayford);
		Actor_Put_In_Set(kActorOfficerGrayford, kSetTB02_TB03);
		Actor_Set_At_XYZ(kActorOfficerGrayford, -173.89f, 0.0f, 2084.22f, 859);
		Actor_Change_Animation_Mode(kActorOfficerGrayford, kAnimationModeCombatIdle);
		return true;

	case 599:
		_animationState = 32;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(624) - 1;
		return true;

	}
	return false;
}

bool AIScriptOfficerGrayford::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (!_var1) {
			*animation = 625;
		}
		if (_var1 == 1) {
			*animation = 626;
		}
		if (_var1 == 2) {
			*animation = 627;
		}
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_var1 = 0;
			if (!Random_Query(0, 1)) {
				_var1 = Random_Query(1, 2);
			}
		}
		break;

	case 1:
		*animation = 618;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(618)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = 619;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(619)) {
			_animationFrame = 0;
		}
		break;

	case 5:
		*animation = 611;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(611)) {
			_animationFrame = 0;
		}
		break;

	case 6:
		*animation = 610;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(610)) {
			_animationFrame = 0;
		}
		break;

	case 9:
		if (!_animationFrame && _var2) {
			*animation = 625;
			_animationState = 0;
			_var1 = 0;
			_var2 = 0;
		} else {
			*animation = 629;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(629)) {
				_animationFrame = 0;
				_animationState = Random_Query(9, 11);
			}
		}
		break;

	case 10:
		*animation = 630;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(630)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 629;
		}
		break;

	case 11:
		*animation = 631;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(631)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 629;
		}
		break;

	case 12:
		*animation = 632;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(632)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 629;
		}
		break;

	case 13:
		*animation = 633;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(633)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 629;
		}
		break;

	case 14:
		*animation = 634;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(634)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 629;
		}
		break;

	case 15:
		*animation = 635;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(635)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 629;
		}
		break;

	case 16:
		*animation = 636;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(636)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 629;
		}
		break;

	case 17:
		*animation = 637;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(637)) {
			_animationFrame = 0;
			_animationState = 9;
			*animation = 629;
		}
		break;

	case 18:
	case 19:
		*animation = 605;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(605)) {
			_animationFrame = 0;
		}
		break;

	case 20:
		*animation = 615;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(615)) {
			_animationFrame = 0;
			_animationState = 19;
		}
		break;

	case 21:
		*animation = 616;
		_animationFrame++;
		if (_animationFrame == 11) {
			Ambient_Sounds_Play_Sound(556, 25, 0, 0, 25);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = 625;
			_animationFrame = 0;
			_animationState = 0;
			_var1 = 0;
			if (Actor_Query_Goal_Number(kActorOfficerGrayford) == 101) {
				Actor_Set_Goal_Number(kActorOfficerGrayford, 102);
			}
		}
		break;

	case 22:
		*animation = 617;
		_animationFrame++;
		if (_animationFrame == 3) {
			int snd;
			if (Random_Query(1, 2) == 1) {
				snd = 9010;
			} else {
				snd = 9015;
			}
			Sound_Play_Speech_Line(kActorOfficerGrayford, snd, 75, 0, 99);
		}
		if (_animationFrame == 5) {
			Actor_Combat_AI_Hit_Attempt(kActorOfficerGrayford);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(617)) {
			_animationState = 19;
			_animationFrame = 0;
			Actor_Change_Animation_Mode(kActorOfficerGrayford, 4);
		}
		break;

	case 23:
		*animation = 617;
		if (_animationFrame < 2) {
			_animationFrame++;
		}
		break;

	case 24:
		*animation = 617;
		_animationFrame--;
		if (_animationFrame < 0) {
			_animationFrame = 0;
			_animationState = 21;
			*animation = 616;
		}
		break;

	case 27:
		*animation = 608;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(608) - 1) {
			_animationFrame = 0;
			_animationState = 19;
			*animation = 625;
			Actor_Change_Animation_Mode(kActorOfficerGrayford, 4);
		}
		break;

	case 28:
		*animation = 609;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(609) - 1) {
			_animationFrame = 0;
			_animationState = 19;
			*animation = 625;
			Actor_Change_Animation_Mode(kActorOfficerGrayford, 4);
		}
		break;

	case 29:
		*animation = 622;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(622) - 1) {
			*animation = 605;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorOfficerGrayford, 0);
		}
		break;

	case 30:
		*animation = 623;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(623) - 1) {
			*animation = 605;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorOfficerGrayford, 0);
		}
		break;

	case 31:
		*animation = 612;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(612) - 1) {
			_animationFrame++;
		}
		break;

	case 32:
		*animation = 624;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(624) - 1) {
			_animationFrame++;
		}
		break;

	case 34:
		*animation = 639;
		if (_var3) {
			_var3--;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(639)) {
				if (Random_Query(0, 1)) {
					*animation = 641;
					_animationState = 37;
				} else {
					*animation = 638;
					_animationState = 35;
				}
				_animationFrame = 0;
			} else {
				if (_animationFrame == 12) {
					_var3 = Random_Query(5, 18);
				}
			}
		}
		break;

	case 35:
		*animation = 638;
		if (_var3) {
			_var3--;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(638)) {
				if (Random_Query(0, 1)) {
					*animation = 641;
					_animationState = 37;
				} else {
					*animation = 639;
					_animationState = 34;
				}
				_animationFrame = 0;
			} else {
				if (_animationFrame == 10) {
					_var3 = Random_Query(5, 18);
				}
			}
		}
		break;

	case 36:
		*animation = 640;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(640)) {
			_animationFrame = 0;
			_animationState = 34;
			*animation = 639;
		}
		break;

	case 37:
		*animation = 641;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(641)) {
			*animation = 625;
			_animationState = 0;
			_animationFrame = 0;

			switch (Actor_Query_Goal_Number(kActorOfficerGrayford)) {
			case 103:
			case 104:
				Actor_Set_Goal_Number(kActorOfficerGrayford, 105);
				break;

			case 105:
				Actor_Set_Goal_Number(kActorOfficerGrayford, 104);
				break;
			}
		}
		break;

	default:
		*animation = 399;
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptOfficerGrayford::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		switch (_animationState) {
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
			_var2 = 1;
			break;

		case 18:
			{
				int tmp = _animationFrame;
				Actor_Change_Animation_Mode(kActorOfficerGrayford, 4);
				_animationFrame = tmp;
				_animationState = 19;
				break;
			}

		case 19:
			_animationState = 21;
			_animationFrame = 0;
			break;

		case 20:
		case 21:
		case 34:
		case 35:
		case 36:
		case 37:
			return true;

		default:
			_animationState = 0;
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeWalk:
		if (Actor_Query_Goal_Number(kActorOfficerGrayford) == 101) {
			_animationState = 6;
			_animationFrame = 0;
		} else if (_animationState != 1) {
			_animationState = 1;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeRun:
		if (Actor_Query_Goal_Number(kActorOfficerGrayford) == 101) {
			_animationState = 5;
			_animationFrame = 0;
		} else if (_animationState != 2) {
			_animationState = 2;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeTalk:
		if (_animationState != 36 && _animationState != 34) {
			_animationState = 9;
			_animationFrame = 0;
			_var2 = 0;
		}
		break;

	case kAnimationModeCombatIdle:
		switch (_animationState) {
		case 5:
		case 6:
		case 7:
		case 8:
			_animationState = 19;
			_animationFrame = 0;
			break;

		case 19:
		case 20:
		case 22:
		case 24:
			return true;

		case 23:
			_animationState = 24;
			_animationFrame = 0;
			break;

		default:
			_animationState = 20;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeCombatAim:
		_animationState = 23;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatAttack:
		_animationFrame = 0;
		_animationState = 22;
		break;

	case kAnimationModeCombatWalk:
		_animationState = 6;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatRun:
		_animationState = 5;
		_animationFrame = 0;
		break;

	case 12:
		_animationState = 10;
		_animationFrame = 0;
		_var2 = 0;
		break;

	case 13:
		_animationState = 11;
		_animationFrame = 0;
		_var2 = 0;
		break;

	case 14:
		_animationState = 12;
		_animationFrame = 0;
		_var2 = 0;
		break;

	case 15:
		_animationState = 13;
		_animationFrame = 0;
		_var2 = 0;
		break;

	case 16:
		_animationState = 14;
		_animationFrame = 0;
		_var2 = 0;
		break;

	case 17:
		_animationState = 15;
		_animationFrame = 0;
		_var2 = 0;
		break;

	case 18:
		_animationState = 16;
		_animationFrame = 0;
		_var2 = 0;
		break;

	case 19:
		_animationState = 17;
		_animationFrame = 0;
		_var2 = 0;
		break;

	case kAnimationModeHit:
		switch (_animationState) {
		case 19:
		case 20:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
			if (Random_Query(0, 1)) {
				_animationState = 27;
			} else {
				_animationState = 28;
			}
			_animationFrame = 0;
			break;

		case 21:
			if (Random_Query(0, 1)) {
				_animationState = 29;
			} else {
				_animationState = 30;
			}
			_animationFrame = 0;
			break;
		}
		break;

	case kAnimationModeCombatHit:
		if (Random_Query(0, 1)) {
			_animationState = 27;
		} else {
			_animationState = 28;
		}
		_animationFrame = 0;
		break;

	case 43:
		_animationState = 36;
		_animationFrame = 0;
		break;

	case kAnimationModeWalkUp:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case kAnimationModeWalkDown:
		_animationState = 4;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalkUp:
		_animationState = 7;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatWalkDown:
		_animationState = 7;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		switch (_animationState) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 7:
			_animationState = 20;
			_animationFrame = 0;
			break;

		case 5:
		case 6:
			return true;

		default:
			_animationState = 32;
			_animationFrame = 0;
			break;
		}
		break;

	case 58:
		_animationState = 18;
		_animationFrame = 0;
		break;

	}

	return true;
}

void AIScriptOfficerGrayford::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptOfficerGrayford::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptOfficerGrayford::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptOfficerGrayford::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
