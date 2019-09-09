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

AIScriptMutant1::AIScriptMutant1(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = false;
}

void AIScriptMutant1::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = 0;

	Actor_Put_In_Set(kActorMutant1, kSetFreeSlotG);
	Actor_Set_At_Waypoint(kActorMutant1, 39, 0);
	Actor_Set_Goal_Number(kActorMutant1, 400);
}

bool AIScriptMutant1::Update() {
	if (Global_Variable_Query(kVariableChapter) == 4) {
		switch (Actor_Query_Goal_Number(kActorMutant1)) {
		case 400:
			if (!Game_Flag_Query(kFlagMutantsActive)
			&&  Game_Flag_Query(kFlagUG06Chapter4Started)
			) {
				Actor_Set_Goal_Number(kActorMutant1, 401);
				Actor_Set_Goal_Number(kActorMutant2, 401);
				Actor_Set_Goal_Number(kActorMutant3, 401);
				Actor_Set_Targetable(kActorMutant1, true);
				Actor_Set_Targetable(kActorMutant2, true);
				Actor_Set_Targetable(kActorMutant3, true);
				Game_Flag_Set(kFlagMutantsActive);
			}
			break;

		case 401:
			if (Actor_Query_Which_Set_In(kActorMutant1) == Player_Query_Current_Set()
			 && (Actor_Query_Friendliness_To_Other(kActorMutant1, kActorMcCoy) < 30
			  || Actor_Query_Combat_Aggressiveness(kActorMutant1) >= 60
			 )
			) {
				Actor_Set_Goal_Number(kActorMutant1, 410);
			}
			break;

		case 404:
			if (!Game_Flag_Query(kFlagMutantsPaused)) {
				Actor_Set_Goal_Number(kActorMutant1, 403);
			}
			break;

		case 410:
			if (Actor_Query_Which_Set_In(kActorMutant1) != Player_Query_Current_Set()) {
				Non_Player_Actor_Combat_Mode_Off(kActorMutant1);
				Actor_Set_Goal_Number(kActorMutant1, 403);
			}
			break;

		case 599:
			if (Actor_Query_Which_Set_In(kActorMutant1) != Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorMutant1, 411);
			}
			break;
		}

		if (Game_Flag_Query(kFlagMutantsPaused)
		&& Actor_Query_Goal_Number(kActorMutant1) != 599
		) {
			Actor_Set_Goal_Number(kActorMutant1, 404);
		}
	} else if (Global_Variable_Query(kVariableChapter) == 5
	        && Actor_Query_Goal_Number(kActorMutant1) != 590
	) {
		if (Actor_Query_Which_Set_In(kActorMutant1) != Player_Query_Current_Set()) {
			Actor_Set_Goal_Number(kActorMutant1, 590);
		}
	}

	return false;
}

void AIScriptMutant1::TimerExpired(int timer) {
	//return false;
}

void AIScriptMutant1::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorMutant1) == 401) {
		Actor_Set_Goal_Number(kActorMutant1, 403);
	}
}

void AIScriptMutant1::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptMutant1::ClickedByPlayer() {
	//return false;
}

void AIScriptMutant1::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptMutant1::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptMutant1::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptMutant1::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (Actor_Query_Which_Set_In(kActorMutant1) == Player_Query_Current_Set()
	 && Actor_Query_Goal_Number(kActorMutant1) != 599
	) {
		if (otherActorId == kActorMcCoy) {
			if (combatMode) {
				Actor_Modify_Combat_Aggressiveness(kActorMutant1, 10);
			} else {
				Actor_Modify_Combat_Aggressiveness(kActorMutant1, -10);
			}
		} else if (otherActorId == kActorFreeSlotA
		        || otherActorId == kActorMutant2
		        || otherActorId == kActorMutant3
		) {
			Actor_Modify_Combat_Aggressiveness(kActorMutant1, 10);
		} else {
			Actor_Modify_Combat_Aggressiveness(kActorMutant1, -10);
		}
	}
}

void AIScriptMutant1::ShotAtAndMissed() {
	if (Actor_Query_Goal_Number(kActorMutant1) != 410) {
		Actor_Modify_Combat_Aggressiveness(kActorMutant1, 10);
		Actor_Modify_Friendliness_To_Other(kActorMutant1, kActorMcCoy, -10);
	}
}

bool AIScriptMutant1::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorMutant1) != 410) {
		Actor_Modify_Combat_Aggressiveness(kActorMutant1, 15);
		Actor_Modify_Friendliness_To_Other(kActorMutant1, kActorMcCoy, -15);
	}

	return false;
}

void AIScriptMutant1::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorMutant1, 599);
}

int AIScriptMutant1::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptMutant1::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 400:
		AI_Movement_Track_Flush(kActorMutant1);
		AI_Movement_Track_Append(kActorMutant1, 39, 0);
		AI_Movement_Track_Repeat(kActorMutant1);

		if (Game_Flag_Query(kFlagCT04HomelessKilledByMcCoy)) {
			Actor_Set_Combat_Aggressiveness(kActorMutant1, 70);
			Actor_Set_Friendliness_To_Other(kActorMutant1, kActorMcCoy, 20);
		}
		return true;

	case 401:
		Actor_Set_Targetable(kActorMutant1, true);
		AI_Movement_Track_Flush(kActorMutant1);
		AI_Movement_Track_Append(kActorMutant1, 39, 0);

		switch (Random_Query(1, 8)) {
		case 1:
			AI_Movement_Track_Append(kActorMutant1, 182, 0);
			AI_Movement_Track_Append(kActorMutant1, 183, 2);
			AI_Movement_Track_Append(kActorMutant1, 184, 0);
			AI_Movement_Track_Repeat(kActorMutant1);
			break;

		case 2:
			AI_Movement_Track_Append(kActorMutant1, 296, 0);
			AI_Movement_Track_Append(kActorMutant1, 297, 0);
			AI_Movement_Track_Repeat(kActorMutant1);
			break;

		case 3:
			AI_Movement_Track_Append(kActorMutant1, 176, 0);
			AI_Movement_Track_Append(kActorMutant1, 177, 0);
			AI_Movement_Track_Append(kActorMutant1, 178, 2);
			AI_Movement_Track_Append(kActorMutant1, 177, 0);
			AI_Movement_Track_Append(kActorMutant1, 176, 1);
			AI_Movement_Track_Append(kActorMutant1, 39, 45);
			AI_Movement_Track_Repeat(kActorMutant1);
			break;

		case 4:
			AI_Movement_Track_Append(kActorMutant1, 298, 0);
			AI_Movement_Track_Append(kActorMutant1, 300, 0);
			AI_Movement_Track_Repeat(kActorMutant1);
			break;

		case 5:
			AI_Movement_Track_Append(kActorMutant1, 301, 0);
			AI_Movement_Track_Append(kActorMutant1, 302, 2);
			AI_Movement_Track_Append(kActorMutant1, 303, 0);
			AI_Movement_Track_Append(kActorMutant1, 304, 0);
			AI_Movement_Track_Append(kActorMutant1, 305, 0);
			AI_Movement_Track_Append(kActorMutant1, 304, 0);
			AI_Movement_Track_Append(kActorMutant1, 306, 0);
			AI_Movement_Track_Append(kActorMutant1, 39, 60);
			AI_Movement_Track_Repeat(kActorMutant1);
			break;

		case 6:
			AI_Movement_Track_Append(kActorMutant1, 307, 0);
			AI_Movement_Track_Append(kActorMutant1, 308, 0);
			AI_Movement_Track_Append(kActorMutant1, 309, 1);
			AI_Movement_Track_Append(kActorMutant1, 310, 3);
			AI_Movement_Track_Append(kActorMutant1, 311, 0);
			AI_Movement_Track_Repeat(kActorMutant1);
			break;

		case 7:
			switch (Random_Query(1, 5)) {
			case 1:
				AI_Movement_Track_Append(kActorMutant1, 532, 0);
				AI_Movement_Track_Append(kActorMutant1, 533, 0);
				AI_Movement_Track_Append(kActorMutant1, 534, 2);
				AI_Movement_Track_Append(kActorMutant1, 535, 3);
				AI_Movement_Track_Append(kActorMutant1, 533, 0);
				AI_Movement_Track_Append(kActorMutant1, 532, 0);
				AI_Movement_Track_Repeat(kActorMutant1);
				break;

			case 2:
				AI_Movement_Track_Append(kActorMutant1, 532, 0);
				AI_Movement_Track_Append(kActorMutant1, 533, 0);
				AI_Movement_Track_Append(kActorMutant1, 535, 2);
				AI_Movement_Track_Append(kActorMutant1, 534, 3);
				AI_Movement_Track_Append(kActorMutant1, 533, 0);
				AI_Movement_Track_Append(kActorMutant1, 532, 0);
				AI_Movement_Track_Repeat(kActorMutant1);
				break;

			case 3:
				AI_Movement_Track_Append(kActorMutant1, 536, 0);
				AI_Movement_Track_Append(kActorMutant1, 537, 0);
				AI_Movement_Track_Append(kActorMutant1, 538, 2);
				AI_Movement_Track_Append(kActorMutant1, 537, 0);
				AI_Movement_Track_Append(kActorMutant1, 536, 0);
				AI_Movement_Track_Repeat(kActorMutant1);
				break;

			case 4:
				AI_Movement_Track_Append(kActorMutant1, 532, 0);
				AI_Movement_Track_Append(kActorMutant1, 533, 0);
				AI_Movement_Track_Append(kActorMutant1, 534, 3);
				AI_Movement_Track_Append(kActorMutant1, 533, 0);
				AI_Movement_Track_Append(kActorMutant1, 532, 0);
				AI_Movement_Track_Repeat(kActorMutant1);
				break;

			case 5:
				AI_Movement_Track_Append(kActorMutant1, 532, 0);
				AI_Movement_Track_Append(kActorMutant1, 533, 0);
				AI_Movement_Track_Append(kActorMutant1, 535, 1);
				AI_Movement_Track_Append(kActorMutant1, 533, 0);
				AI_Movement_Track_Append(kActorMutant1, 532, 0);
				AI_Movement_Track_Repeat(kActorMutant1);
				break;

			default:
				return true;
			}
			break;

		case 8:
			if (Game_Flag_Query(kFlagUG07Empty)) {
				AI_Movement_Track_Append(kActorMutant1, 418, 0);
				AI_Movement_Track_Append(kActorMutant1, 417, 0);
				AI_Movement_Track_Append(kActorMutant1, 539, 0);
				AI_Movement_Track_Repeat(kActorMutant1);
			} else {
				Actor_Set_Goal_Number(kActorMutant1, 403);
			}
			break;

		default:
			AI_Movement_Track_Append(kActorMutant1, 39, 60);
			AI_Movement_Track_Repeat(kActorMutant1);
			break;
		}
		return true;

	case 403:
		Actor_Set_Targetable(kActorMutant1, false);
		Actor_Set_Goal_Number(kActorMutant1, 401);
		return true;

	case 404:
		AI_Movement_Track_Flush(kActorMutant1);
		AI_Movement_Track_Append(kActorMutant1, 39, 0);
		AI_Movement_Track_Repeat(kActorMutant1);
		return true;

	case 410:
		switch (Actor_Query_Which_Set_In(kActorMutant1)) {
		case kSetUG01:
			Non_Player_Actor_Combat_Mode_On(kActorMutant1, kActorCombatStateIdle, false, kActorMcCoy, 11, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			break;

		case kSetUG04:
		case kSetUG05:
		case kSetUG06:
			Non_Player_Actor_Combat_Mode_On(kActorMutant1, kActorCombatStateIdle, false, kActorMcCoy, 10, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			break;

		case kSetUG07:
			Non_Player_Actor_Combat_Mode_On(kActorMutant1, kActorCombatStateIdle, false, kActorMcCoy, 12, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			break;

		case kSetUG10:
		case kSetUG12:
		case kSetUG14:
			Non_Player_Actor_Combat_Mode_On(kActorMutant1, kActorCombatStateIdle, false, kActorMcCoy, 14, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			break;
		}
		return true;

	case 411:
		AI_Movement_Track_Flush(kActorMutant1);
		Actor_Set_Intelligence(kActorMutant1, 40);
		Actor_Set_Health(kActorMutant1, 10 * Query_Difficulty_Level() + 30, 10 * Query_Difficulty_Level() + 30);

		if (Game_Flag_Query(kFlagCT04HomelessKilledByMcCoy)) {
			Actor_Set_Combat_Aggressiveness(kActorMutant1, 70);
			Actor_Set_Friendliness_To_Other(kActorMutant1, kActorMcCoy, 20);
		} else {
			Actor_Set_Combat_Aggressiveness(kActorMutant1, 40);
			Actor_Set_Friendliness_To_Other(kActorMutant1, kActorMcCoy, 45);
		}

		Actor_Modify_Friendliness_To_Other(kActorMutant2, kActorMcCoy, -10);
		Actor_Modify_Friendliness_To_Other(kActorMutant3, kActorMcCoy, -20);
		Actor_Modify_Combat_Aggressiveness(kActorMutant2, 10);
		Actor_Modify_Combat_Aggressiveness(kActorMutant3, 15);
		Actor_Set_Goal_Number(kActorMutant1, 403);
		return true;

	case 590:
		AI_Movement_Track_Flush(kActorMutant1);
		AI_Movement_Track_Append(kActorMutant1, 39, 100);
		AI_Movement_Track_Repeat(kActorMutant1);
		return true;

	case 599:
		AI_Movement_Track_Flush(kActorMutant1);
		Actor_Change_Animation_Mode(kActorMutant1, 48);
		Actor_Modify_Friendliness_To_Other(kActorMutant2, kActorMcCoy, -10);
		Actor_Modify_Friendliness_To_Other(kActorMutant3, kActorMcCoy, -20);
		Actor_Modify_Combat_Aggressiveness(kActorMutant2, 10);
		Actor_Modify_Combat_Aggressiveness(kActorMutant3, 15);
		return true;

	default:
		break;
	}

	return false;
}

bool AIScriptMutant1::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = 894;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(894)) {
			_animationFrame = 0;
		}
		break;

	case 1:
	case 2:
		*animation = 893;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(893)) {
			_animationFrame = 0;
		}
		break;

	case 3:
		if (!_animationFrame && _flag) {
			*animation = 894;
			_animationState = 0;
		} else {
			*animation = 896;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(896)) {
				_animationFrame = 0;
			}
		}
		break;

	case 4:
		*animation = 896;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(896)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 896;
		}
		break;

	case 5:
		*animation = 897;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(897)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 896;
		}
		break;

	case 6:
		*animation = 898;
		_animationFrame++;
		if (_animationFrame == 5) {
			int snd;

			if (Random_Query(1, 2) == 1) {
				snd = 9010;
			} else {
				snd = 9015;
			}
			Sound_Play_Speech_Line(kActorMutant1, snd, 75, 0, 99);
		}
		if (_animationFrame == 9) {
			Actor_Combat_AI_Hit_Attempt(kActorMutant1);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(898)) {
			Actor_Change_Animation_Mode(kActorMutant1, kAnimationModeIdle);
		}
		break;

	case 7:
		*animation = 899;
		_animationFrame++;
		if (_animationFrame == 1) {
			Ambient_Sounds_Play_Sound(kSfxHURT1M1, 99, 0, 0, 25);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(899)) {
			Actor_Change_Animation_Mode(kActorMutant1, kAnimationModeIdle);
		}
		break;

	case 8:
		*animation = 900;
		_animationFrame++;
		if (_animationFrame == 1) {
			Sound_Play(kSfxYELL1M1, 100, 0, 0, 50);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(900)) {
			Actor_Change_Animation_Mode(kActorMutant1, 88);
		}
		break;

	case 9:
		*animation = 900;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(900) - 1;
		break;

	case 10:
		*animation = 899;
		_animationFrame++;
		if (_animationFrame == 9) {
			Sound_Play(kSfxHURT1M1, 100, 0, 0, 50);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(899)) {
			Actor_Change_Animation_Mode(kActorMutant1, kAnimationModeIdle);
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptMutant1::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState >= 3 && _animationState <= 5) {
			_flag = 1;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;

	case 1:
	case 7:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case 2:
		_animationState = 2;
		_animationFrame = 0;
		break;

	case 3:
		_animationState = 3;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 4:
		if (_animationState >= 3 && _animationState <= 5) {
			_flag = 1;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;

	case 6:
		_animationState = 6;
		_animationFrame = 0;
		break;

	case 8:
		_animationState = 2;
		_animationFrame = 0;
		break;

	case 12:
		_animationState = 3;
		_animationFrame = 0;
		_flag = 0;
		break;

	case 13:
		_animationState = 3;
		_animationFrame = 0;
		_flag = 0;
		break;
	case 21:
	case 22:
		_animationState = 10;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		_animationState = 8;
		_animationFrame = 0;
		break;

	case 88:
		_animationState = 9;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(900) - 1;
		break;
	}

	return true;
}

void AIScriptMutant1::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptMutant1::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptMutant1::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptMutant1::FledCombat() {
	Actor_Set_Goal_Number(kActorMutant1, 403);
}

} // End of namespace BladeRunner
