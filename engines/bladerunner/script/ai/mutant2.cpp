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

AIScriptMutant2::AIScriptMutant2(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_resumeIdleAfterFramesetCompletesFlag = false;
	_var1 = 1;
}

void AIScriptMutant2::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_resumeIdleAfterFramesetCompletesFlag = false;
	_var1 = 1;

	Actor_Put_In_Set(kActorMutant2, kSetFreeSlotG);
	Actor_Set_At_Waypoint(kActorMutant2, 39, 0);
	Actor_Set_Goal_Number(kActorMutant2, 400);
}

bool AIScriptMutant2::Update() {
	if (Global_Variable_Query(kVariableChapter) == 4) {
		switch (Actor_Query_Goal_Number(kActorMutant2)) {
		case 401:
			if (Actor_Query_Which_Set_In(kActorMutant2) == Player_Query_Current_Set()
			 && (Actor_Query_Friendliness_To_Other(kActorMutant2, kActorMcCoy) < 20
			  || Actor_Query_Combat_Aggressiveness(kActorMutant2) >= 60
			 )
			) {
				Actor_Set_Goal_Number(kActorMutant2, 410);
			}
			break;

		case 404:
			if (!Game_Flag_Query(kFlagMutantsPaused)) {
				Actor_Set_Goal_Number(kActorMutant2, 403);
			}
			break;

		case 410:
			if (Actor_Query_Which_Set_In(kActorMutant2) != Player_Query_Current_Set()) {
				Non_Player_Actor_Combat_Mode_Off(kActorMutant2);
				Actor_Set_Goal_Number(kActorMutant2, 403);
			}
			break;

		case 599:
			if (Actor_Query_Which_Set_In(kActorMutant2) != Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorMutant2, 403);
			}
			break;
		}

		if (Game_Flag_Query(kFlagMutantsPaused)
		 && Actor_Query_Goal_Number(kActorMutant2) != 599
		) {
			Actor_Set_Goal_Number(kActorMutant2, 404);
		}
	} else if (Global_Variable_Query(kVariableChapter) == 5
	        && Actor_Query_Goal_Number(kActorMutant2) != 590
	) {
		if (Actor_Query_Which_Set_In(kActorMutant2) != Player_Query_Current_Set()) {
			Actor_Set_Goal_Number(kActorMutant2, 590);
		}
	}

	return false;
}

void AIScriptMutant2::TimerExpired(int timer) {
	//return false;
}

void AIScriptMutant2::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorMutant2) == 401)
		Actor_Set_Goal_Number(kActorMutant2, 403);
}

void AIScriptMutant2::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptMutant2::ClickedByPlayer() {
	//return false;
}

void AIScriptMutant2::EnteredSet(int setId) {
	// return false;
}

void AIScriptMutant2::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptMutant2::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptMutant2::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (Actor_Query_Which_Set_In(kActorMutant2) == Player_Query_Current_Set()
	 && Actor_Query_Goal_Number(kActorMutant2) != 599
	) {
		if (otherActorId == kActorMcCoy) {
			if (combatMode) {
				Actor_Modify_Combat_Aggressiveness(kActorMutant2, 10);
			} else {
				Actor_Modify_Combat_Aggressiveness(kActorMutant2, -10);
			}
		} else if (otherActorId == kActorFreeSlotA
		        || otherActorId == kActorMutant1
		        || otherActorId == kActorMutant3
		) {
			Actor_Modify_Combat_Aggressiveness(kActorMutant2, 5);
		} else {
			Actor_Modify_Combat_Aggressiveness(kActorMutant2, -10);
		}
	}
}

void AIScriptMutant2::ShotAtAndMissed() {
	if (Actor_Query_Goal_Number(kActorMutant2) != 410) {
		Actor_Modify_Combat_Aggressiveness(kActorMutant2, -5);
		Actor_Modify_Friendliness_To_Other(kActorMutant2, kActorMcCoy, -5);
	}
}

bool AIScriptMutant2::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorMutant2) != 410) {
		Actor_Modify_Combat_Aggressiveness(kActorMutant2, -10);
		Actor_Modify_Friendliness_To_Other(kActorMutant2, kActorMcCoy, -10);
	}

	return false;
}

void AIScriptMutant2::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorMutant2, 599);
}

int AIScriptMutant2::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptMutant2::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 400:
		AI_Movement_Track_Flush(kActorMutant2);
		AI_Movement_Track_Append(kActorMutant2, 39, 0);
		AI_Movement_Track_Repeat(kActorMutant2);

		if (Game_Flag_Query(kFlagCT04HomelessKilledByMcCoy)) {
			Actor_Set_Combat_Aggressiveness(kActorMutant2, 60);
			Actor_Set_Friendliness_To_Other(kActorMutant2, kActorMcCoy, 30);
		}
		return true;

	case 401:
		Actor_Set_Targetable(kActorMutant2, true);
		AI_Movement_Track_Flush(kActorMutant2);
		AI_Movement_Track_Append(kActorMutant2, 39, 0);

		switch (Random_Query(1, 8)) {
		case 1:
			AI_Movement_Track_Append(kActorMutant2, 182, 0);
			AI_Movement_Track_Append(kActorMutant2, 183, 2);
			AI_Movement_Track_Append(kActorMutant2, 184, 0);
			AI_Movement_Track_Repeat(kActorMutant2);
			break;

		case 2:
			AI_Movement_Track_Append(kActorMutant2, 296, 0);
			AI_Movement_Track_Append(kActorMutant2, 297, 0);
			AI_Movement_Track_Repeat(kActorMutant2);
			break;

		case 3:
			AI_Movement_Track_Append(kActorMutant2, 176, 0);
			AI_Movement_Track_Append(kActorMutant2, 177, 0);
			AI_Movement_Track_Append(kActorMutant2, 178, 2);
			AI_Movement_Track_Append(kActorMutant2, 177, 0);
			AI_Movement_Track_Append(kActorMutant2, 176, 1);
			AI_Movement_Track_Append(kActorMutant2, 39, 45);
			AI_Movement_Track_Repeat(kActorMutant2);
			break;
		case 4:
			AI_Movement_Track_Append(kActorMutant2, 298, 0);
			AI_Movement_Track_Append(kActorMutant2, 300, 0);
			AI_Movement_Track_Repeat(kActorMutant2);
			break;

		case 5:
			AI_Movement_Track_Append(kActorMutant2, 301, 0);
			AI_Movement_Track_Append(kActorMutant2, 302, 2);
			AI_Movement_Track_Append(kActorMutant2, 303, 0);
			AI_Movement_Track_Append(kActorMutant2, 304, 0);
			AI_Movement_Track_Append(kActorMutant2, 305, 0);
			AI_Movement_Track_Append(kActorMutant2, 304, 0);
			AI_Movement_Track_Append(kActorMutant2, 306, 5);
			AI_Movement_Track_Append(kActorMutant2, 39, 40);
			AI_Movement_Track_Repeat(kActorMutant2);
			break;

		case 6:
			AI_Movement_Track_Append(kActorMutant2, 307, 0);
			AI_Movement_Track_Append(kActorMutant2, 308, 0);
			AI_Movement_Track_Append(kActorMutant2, 309, 1);
			AI_Movement_Track_Append(kActorMutant2, 310, 2);
			AI_Movement_Track_Append(kActorMutant2, 311, 0);
			AI_Movement_Track_Repeat(kActorMutant2);
			break;

		case 7:
			switch (Random_Query(1, 3)) {        // eeh? bug?
			case 1:
				AI_Movement_Track_Append(kActorMutant2, 532, 0);
				AI_Movement_Track_Append(kActorMutant2, 533, 0);
				AI_Movement_Track_Append(kActorMutant2, 534, 1);
				AI_Movement_Track_Append(kActorMutant2, 535, 1);
				AI_Movement_Track_Append(kActorMutant2, 533, 0);
				AI_Movement_Track_Append(kActorMutant2, 532, 0);
				AI_Movement_Track_Repeat(kActorMutant2);
				break;

			case 2:
				AI_Movement_Track_Append(kActorMutant2, 532, 0);
				AI_Movement_Track_Append(kActorMutant2, 533, 0);
				AI_Movement_Track_Append(kActorMutant2, 535, 2);
				AI_Movement_Track_Append(kActorMutant2, 534, 1);
				AI_Movement_Track_Append(kActorMutant2, 533, 0);
				AI_Movement_Track_Append(kActorMutant2, 532, 0);
				AI_Movement_Track_Repeat(kActorMutant2);
				break;

			case 3:
				AI_Movement_Track_Append(kActorMutant2, 536, 0);
				AI_Movement_Track_Append(kActorMutant2, 537, 0);
				AI_Movement_Track_Append(kActorMutant2, 538, 2);
				AI_Movement_Track_Append(kActorMutant2, 537, 0);
				AI_Movement_Track_Append(kActorMutant2, 536, 0);
				AI_Movement_Track_Repeat(kActorMutant2);
				break;

			case 4:
				AI_Movement_Track_Append(kActorMutant2, 532, 0);
				AI_Movement_Track_Append(kActorMutant2, 533, 0);
				AI_Movement_Track_Append(kActorMutant2, 534, 3);
				AI_Movement_Track_Append(kActorMutant2, 533, 0);
				AI_Movement_Track_Append(kActorMutant2, 532, 0);
				AI_Movement_Track_Repeat(kActorMutant2);
				break;

			case 5:
				AI_Movement_Track_Append(kActorMutant2, 532, 0);
				AI_Movement_Track_Append(kActorMutant2, 533, 0);
				AI_Movement_Track_Append(kActorMutant2, 535, 1);
				AI_Movement_Track_Append(kActorMutant2, 533, 0);
				AI_Movement_Track_Append(kActorMutant2, 532, 0);
				AI_Movement_Track_Repeat(kActorMutant2);
				break;

			default:
				return true;
			}
			break;

		case 8:
			AI_Movement_Track_Append(kActorMutant2, 176, 0);
			AI_Movement_Track_Append(kActorMutant2, 177, 0);
			AI_Movement_Track_Append(kActorMutant2, 176, 0);
			AI_Movement_Track_Repeat(kActorMutant2);
			break;

		default:
			AI_Movement_Track_Append(kActorMutant2, 39, 60);
			AI_Movement_Track_Repeat(kActorMutant2);
			break;
		}
		return true;

	case 403:
		Actor_Set_Targetable(kActorMutant2, false);
		Actor_Set_Goal_Number(kActorMutant2, 401);
		return true;

	case 404:
		AI_Movement_Track_Flush(kActorMutant2);
		AI_Movement_Track_Append(kActorMutant2, 39, 0);
		AI_Movement_Track_Repeat(kActorMutant2);
		return true;

	case 410:
		switch (Actor_Query_Which_Set_In(kActorMutant2)) {
		case kSetUG01:
			Non_Player_Actor_Combat_Mode_On(kActorMutant2, kActorCombatStateIdle, false, kActorMcCoy, 11, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			break;

		case kSetUG04:
			// fall through
		case kSetUG05:
			// fall through
		case kSetUG06:
			Non_Player_Actor_Combat_Mode_On(kActorMutant2, kActorCombatStateIdle, false, kActorMcCoy, 10, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			break;

		case kSetUG10:
			// fall through
		case kSetUG12:
			// fall through
		case kSetUG14:
			Non_Player_Actor_Combat_Mode_On(kActorMutant2, kActorCombatStateIdle, false, kActorMcCoy, 14, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			break;
		}
		return true;

	case 590:
		AI_Movement_Track_Flush(kActorMutant2);
		AI_Movement_Track_Append(kActorMutant2, 39, 100);
		AI_Movement_Track_Repeat(kActorMutant2);
		return true;

	case 599:
		AI_Movement_Track_Flush(kActorMutant2);
		Actor_Change_Animation_Mode(kActorMutant2, 48);
		Actor_Set_Intelligence(kActorMutant2, 20);
		Actor_Set_Health(71, 10 * Query_Difficulty_Level() + 50, 10 * Query_Difficulty_Level() + 50);

		if (Game_Flag_Query(kFlagCT04HomelessKilledByMcCoy)) {
			Actor_Set_Combat_Aggressiveness(kActorMutant2, 60);
			Actor_Set_Friendliness_To_Other(kActorMutant2, kActorMcCoy, 30);
		} else {
			Actor_Set_Combat_Aggressiveness(kActorMutant2, 40);
			Actor_Set_Friendliness_To_Other(kActorMutant2, kActorMcCoy, 50);
		}

		Actor_Modify_Friendliness_To_Other(kActorMutant1, kActorMcCoy, -15);
		Actor_Modify_Friendliness_To_Other(kActorMutant3, kActorMcCoy, -20);
		Actor_Modify_Combat_Aggressiveness(kActorMutant1, 10);
		Actor_Modify_Combat_Aggressiveness(kActorMutant3, 15);
		return true;
	}

	return false;
}

bool AIScriptMutant2::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationMutant2Idle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant2Idle)) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = kModelAnimationMutant2Walking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant2Walking)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationMutant2Running;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant2Running)) {
			_animationFrame = 0;
		}
		break;

	case 3:
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationMutant2Idle;
			_animationState = 0;
		} else {
			*animation = kModelAnimationMutant2YellOrHurt;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant2YellOrHurt)) {
				_animationFrame = 0;
			}
		}
		break;

	case 4:
		*animation = kModelAnimationMutant2YellOrHurt;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant2YellOrHurt)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = kModelAnimationMutant2CalmTalk;
		}
		break;

	case 5:
		*animation = kModelAnimationMutant2MeleeAttack;
		++_animationFrame;
		if (_animationFrame == 7) {
			int snd;
			if (Random_Query(1, 2) == 1) {
				snd = 9010;
			} else {
				snd = 9015;
			}
			Sound_Play_Speech_Line(kActorMutant2, snd, 75, 0, 99);
		}
		if (_animationFrame == 9) {
			Actor_Combat_AI_Hit_Attempt(kActorMutant2);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant2MeleeAttack)) {
			Actor_Change_Animation_Mode(kActorMutant2, kAnimationModeIdle);
		}
		break;

	case 6:
		*animation = kModelAnimationMutant2ShotDead;
		++_animationFrame;
		if (_animationFrame == 1) {
			Sound_Play(kSfxYELL1M2, 100, 0, 0, 50);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			Actor_Change_Animation_Mode(kActorMutant2, 88);
		}
		break;

	case 7:
		*animation = kModelAnimationMutant2ShotDead;
		// TODO why "- 2" here? 
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant2ShotDead) - 2;
		break;

	case 8:
		*animation = kModelAnimationMutant2ShotDead;
		_animationFrame += _var1;
		if (_animationFrame == 4) {
			_var1 = -1;
			Sound_Play(kSfxHURT1M2, 100, 0, 0, 50);
		} else {
			if (_animationFrame == 0) {
				Actor_Change_Animation_Mode(kActorMutant2, kAnimationModeIdle);
			}
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptMutant2::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState >= 3 && _animationState <= 4) {
			_resumeIdleAfterFramesetCompletesFlag = true;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;

	case 1:
		// fall through
	case 7:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case 2:
		// fall through
	case 8:
		_animationState = 2;
		_animationFrame = 0;
		break;

	case 3:
		_animationState = 3;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 4:
		if (_animationState >= 3 && _animationState <= 4) {
			_resumeIdleAfterFramesetCompletesFlag = true;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;

	case 6:
		_animationState = 5;
		_animationFrame = 0;
		break;

	case 12:
		_animationState = 3;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 21:
		// fall through
	case 22:
		_animationState = 8;
		_animationFrame = 0;
		_var1 = 1;
		break;

	case kAnimationModeDie:
		_animationState = 6;
		_animationFrame = 0;
		break;

	case 88:
		_animationState = 7;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant2ShotDead) - 1;
		break;
	}

	return true;
}

void AIScriptMutant2::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptMutant2::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptMutant2::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptMutant2::FledCombat() {
	Actor_Set_Goal_Number(kActorMutant2, 403);
}

} // End of namespace BladeRunner
