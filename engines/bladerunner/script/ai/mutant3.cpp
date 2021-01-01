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

AIScriptMutant3::AIScriptMutant3(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 1;
	_resumeIdleAfterFramesetCompletesFlag = false;
}

void AIScriptMutant3::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 1;
	_resumeIdleAfterFramesetCompletesFlag = false;

	Actor_Put_In_Set(kActorMutant3, kSetFreeSlotG);
	Actor_Set_At_Waypoint(kActorMutant3, 39, 0);
	Actor_Set_Goal_Number(kActorMutant3, 400);
}

bool AIScriptMutant3::Update() {
	if (Global_Variable_Query(kVariableChapter) == 4) {
		switch (Actor_Query_Goal_Number(kActorMutant3)) {
		case 401:
			if (Actor_Query_Which_Set_In(kActorMutant3) == Player_Query_Current_Set()
			 && (Actor_Query_Friendliness_To_Other(kActorMutant3, kActorMcCoy) < 40
			  || Actor_Query_Combat_Aggressiveness(kActorMutant3) >= 60
			 )
			) {
				Actor_Set_Goal_Number(kActorMutant3, 410);
			}
			break;

		case 404:
			if (!Game_Flag_Query(kFlagMutantsPaused)) {
				Actor_Set_Goal_Number(kActorMutant3, 401);
			}
			break;

		case 410:
			if (Actor_Query_Which_Set_In(kActorMutant3) != Player_Query_Current_Set()) {
				Non_Player_Actor_Combat_Mode_Off(kActorMutant3);
				Actor_Set_Goal_Number(kActorMutant3, 403);
			}
			break;

		case 599:
			if (Actor_Query_Which_Set_In(kActorMutant3) != Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorMutant3, 403);
			}
			break;
		}

		if (Game_Flag_Query(kFlagMutantsPaused)
		 && Actor_Query_Goal_Number(kActorMutant3) != 599
		) {
			Actor_Set_Goal_Number(kActorMutant3, 404);
		}
	} else if (Global_Variable_Query(kVariableChapter) == 5
	        && Actor_Query_Goal_Number(kActorMutant3) != 590
	) {
		if (Actor_Query_Which_Set_In(kActorMutant3) != Player_Query_Current_Set()) {
			Actor_Set_Goal_Number(kActorMutant3, 590);
		}
	}
	return false;
}

void AIScriptMutant3::TimerExpired(int timer) {
	//return false;
}

void AIScriptMutant3::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorMutant3)) {
	case 201:
		Actor_Set_Goal_Number(kActorMutant3, 400);
		break;

	case 401:
		Actor_Set_Goal_Number(kActorMutant3, 403);
		break;
	}
}

void AIScriptMutant3::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptMutant3::ClickedByPlayer() {
	//return false;
}

void AIScriptMutant3::EnteredSet(int setId) {
	// return false;
}

void AIScriptMutant3::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptMutant3::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptMutant3::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (Actor_Query_Which_Set_In(kActorMutant3) == Player_Query_Current_Set()
	 && Actor_Query_Goal_Number(kActorMutant3) != 599
	) {
		if (otherActorId == kActorMcCoy) {
			if (combatMode) {
				Actor_Modify_Combat_Aggressiveness(kActorMutant3, 10);
			} else {
				Actor_Modify_Combat_Aggressiveness(kActorMutant3, -10);
			}
		} else if (otherActorId == kActorFreeSlotA
		        || otherActorId == kActorMutant1
		        || otherActorId == kActorMutant2
		) {
			Actor_Modify_Combat_Aggressiveness(kActorMutant3, 5);
		} else {
			Actor_Modify_Combat_Aggressiveness(kActorMutant3, -10);
		}
	}
}

void AIScriptMutant3::ShotAtAndMissed() {
	if (Actor_Query_Goal_Number(kActorMutant3) != 410) {
		Actor_Modify_Combat_Aggressiveness(kActorMutant3, 15);
		Actor_Modify_Friendliness_To_Other(kActorMutant3, kActorMcCoy, -15);
	}
}

bool AIScriptMutant3::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorMutant3) != 410) {
		Actor_Modify_Combat_Aggressiveness(kActorMutant3, 20);
		Actor_Modify_Friendliness_To_Other(kActorMutant3, kActorMcCoy, -20);
	}
	return false;
}

void AIScriptMutant3::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorMutant3, 599);
}

int AIScriptMutant3::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptMutant3::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 201:
		AI_Movement_Track_Flush(kActorMutant3);
		AI_Movement_Track_Append(kActorMutant3, 305, 3);
		AI_Movement_Track_Append(kActorMutant3, 306, 0);
		AI_Movement_Track_Append(kActorMutant3, 39, 0);
		AI_Movement_Track_Repeat(kActorMutant3);
		break;

	case 400:
		AI_Movement_Track_Flush(kActorMutant3);
		AI_Movement_Track_Append(kActorMutant3, 39, 0);
		AI_Movement_Track_Repeat(kActorMutant3);

		if (Game_Flag_Query(kFlagCT04HomelessKilledByMcCoy)) {
			Actor_Set_Combat_Aggressiveness(kActorMutant3, 80);
			Actor_Set_Friendliness_To_Other(kActorMutant3, kActorMcCoy, 20);
		}
		break;

	case 401:
		Actor_Set_Targetable(kActorMutant3, true);
		AI_Movement_Track_Flush(kActorMutant3);
		AI_Movement_Track_Append(kActorMutant3, 39, 0);

		switch (Random_Query(1, 8)) {
		case 1:
			AI_Movement_Track_Append(kActorMutant3, 182, 0);
			AI_Movement_Track_Append(kActorMutant3, 183, 2);
			AI_Movement_Track_Append(kActorMutant3, 184, 0);
			AI_Movement_Track_Repeat(kActorMutant3);
			break;

		case 2:
			AI_Movement_Track_Append(kActorMutant3, 296, 0);
			AI_Movement_Track_Append(kActorMutant3, 297, 0);
			AI_Movement_Track_Repeat(kActorMutant3);
			break;

		case 3:
			AI_Movement_Track_Append(kActorMutant3, 176, 0);
			AI_Movement_Track_Append(kActorMutant3, 177, 0);
			AI_Movement_Track_Append(kActorMutant3, 178, 2);
			AI_Movement_Track_Append(kActorMutant3, 177, 0);
			AI_Movement_Track_Append(kActorMutant3, 176, 1);
			AI_Movement_Track_Append(kActorMutant3, 39, 45);
			AI_Movement_Track_Repeat(kActorMutant3);
			break;

		case 4:
			AI_Movement_Track_Append(kActorMutant3, 298, 0);
			AI_Movement_Track_Append(kActorMutant3, 300, 0);
			AI_Movement_Track_Repeat(kActorMutant3);
			break;

		case 5:
			AI_Movement_Track_Append(kActorMutant3, 301, 0);
			AI_Movement_Track_Append(kActorMutant3, 302, 2);
			AI_Movement_Track_Append(kActorMutant3, 303, 0);
			AI_Movement_Track_Append(kActorMutant3, 304, 2);
			AI_Movement_Track_Append(kActorMutant3, 305, 0);
			AI_Movement_Track_Append(kActorMutant3, 304, 0);
			AI_Movement_Track_Append(kActorMutant3, 306, 0);
			AI_Movement_Track_Append(kActorMutant3, 39, 60);
			AI_Movement_Track_Repeat(kActorMutant3);
			break;

		case 6:
			AI_Movement_Track_Append(kActorMutant3, 307, 0);
			AI_Movement_Track_Append(kActorMutant3, 308, 0);
			AI_Movement_Track_Append(kActorMutant3, 309, 1);
			AI_Movement_Track_Append(kActorMutant3, 310, 3);
			AI_Movement_Track_Append(kActorMutant3, 311, 0);
			AI_Movement_Track_Repeat(kActorMutant3);
			break;

		case 7:
			switch (Random_Query(1, 5)) {
			case 1:
				AI_Movement_Track_Append(kActorMutant3, 532, 0);
				AI_Movement_Track_Append(kActorMutant3, 533, 0);
				AI_Movement_Track_Append(kActorMutant3, 534, 0);
				AI_Movement_Track_Append(kActorMutant3, 535, 0);
				AI_Movement_Track_Append(kActorMutant3, 533, 0);
				AI_Movement_Track_Append(kActorMutant3, 532, 0);
				AI_Movement_Track_Repeat(kActorMutant3);
				break;

			case 2:
				AI_Movement_Track_Append(kActorMutant3, 532, 0);
				AI_Movement_Track_Append(kActorMutant3, 533, 0);
				AI_Movement_Track_Append(kActorMutant3, 535, 1);
				AI_Movement_Track_Append(kActorMutant3, 534, 3);
				AI_Movement_Track_Append(kActorMutant3, 533, 0);
				AI_Movement_Track_Append(kActorMutant3, 532, 0);
				AI_Movement_Track_Repeat(kActorMutant3);
				break;

			case 3:
				AI_Movement_Track_Append(kActorMutant3, 536, 0);
				AI_Movement_Track_Append(kActorMutant3, 537, 0);
				AI_Movement_Track_Append(kActorMutant3, 538, 2);
				AI_Movement_Track_Append(kActorMutant3, 537, 0);
				AI_Movement_Track_Append(kActorMutant3, 536, 0);
				AI_Movement_Track_Repeat(kActorMutant3);
				break;

			case 4:
				AI_Movement_Track_Append(kActorMutant3, 532, 0);
				AI_Movement_Track_Append(kActorMutant3, 533, 0);
				AI_Movement_Track_Append(kActorMutant3, 534, 3);
				AI_Movement_Track_Append(kActorMutant3, 533, 0);
				AI_Movement_Track_Append(kActorMutant3, 532, 0);
				AI_Movement_Track_Repeat(kActorMutant3);
				break;

			case 5:
				AI_Movement_Track_Append(kActorMutant3, 532, 0);
				AI_Movement_Track_Append(kActorMutant3, 533, 0);
				AI_Movement_Track_Append(kActorMutant3, 535, 1);
				AI_Movement_Track_Append(kActorMutant3, 533, 0);
				AI_Movement_Track_Append(kActorMutant3, 532, 0);
				AI_Movement_Track_Repeat(kActorMutant3);
				break;
			}
			break;

		case 8:
			AI_Movement_Track_Append(kActorMutant3, 176, 0);
			AI_Movement_Track_Append(kActorMutant3, 177, 0);
			AI_Movement_Track_Append(kActorMutant3, 176, 0);
			AI_Movement_Track_Repeat(kActorMutant3);
			break;

		default:
			AI_Movement_Track_Append(kActorMutant3, 39, 60);
			AI_Movement_Track_Repeat(kActorMutant3);
			break;
		}
		break;

	case 403:
		Actor_Set_Targetable(kActorMutant3, false);
		Actor_Set_Goal_Number(kActorMutant3, 401);
		break;

	case 404:
		AI_Movement_Track_Flush(kActorMutant3);
		AI_Movement_Track_Append(kActorMutant3, 39, 0);
		AI_Movement_Track_Repeat(kActorMutant3);
		break;

	case 410:
		switch (Actor_Query_Which_Set_In(kActorMutant3)) {
		case kSetUG01:
			Non_Player_Actor_Combat_Mode_On(kActorMutant3, kActorCombatStateIdle, false, kActorMcCoy, 11, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			break;

		case kSetUG04:
			// fall through
		case kSetUG05:
			// fall through
		case kSetUG06:
			Non_Player_Actor_Combat_Mode_On(kActorMutant3, kActorCombatStateIdle, false, kActorMcCoy, 10, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			break;

		case kSetUG10:
			// fall through
		case kSetUG12:
			// fall through
		case kSetUG14:
			Non_Player_Actor_Combat_Mode_On(kActorMutant3, kActorCombatStateIdle, false, kActorMcCoy, 14, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, -1, -1, -1, 10, 300, false);
			break;
		}
		break;

	case 590:
		AI_Movement_Track_Flush(kActorMutant3);
		AI_Movement_Track_Append(kActorMutant3, 39, 100);
		AI_Movement_Track_Repeat(kActorMutant3);
		break;

	case 599:
		AI_Movement_Track_Flush(kActorMutant3);
		Actor_Change_Animation_Mode(kActorMutant3, 48);
		Actor_Set_Intelligence(kActorMutant3, 40);
		Actor_Set_Health(kActorMutant3, 10 * Query_Difficulty_Level() + 50, 10 * Query_Difficulty_Level() + 50);

		if (Game_Flag_Query(kFlagCT04HomelessKilledByMcCoy)) {
			Actor_Set_Combat_Aggressiveness(kActorMutant3, 80);
			Actor_Set_Friendliness_To_Other(kActorMutant3, kActorMcCoy, 20);
		} else {
			Actor_Set_Combat_Aggressiveness(kActorMutant3, 50);
			Actor_Set_Friendliness_To_Other(kActorMutant3, kActorMcCoy, 40);
		}

		Actor_Modify_Friendliness_To_Other(kActorMutant1, kActorMcCoy, 20);
		Actor_Modify_Friendliness_To_Other(kActorMutant2, kActorMcCoy, 15);
		Actor_Modify_Combat_Aggressiveness(kActorMutant1, 10);
		Actor_Modify_Combat_Aggressiveness(kActorMutant2, 10);
		break;

	default:
		return false;
	}

	return true;
}

bool AIScriptMutant3::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationMutant3Idle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant3Idle)) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = kModelAnimationMutant3Walking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant3Walking)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationMutant3Running;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant3Running)) {
			_animationFrame = 0;
		}
		break;

	case 3:
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationMutant3Idle;
			_animationState = 0;
		} else {
			*animation = kModelAnimationMutant3YellOrHurt;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant3YellOrHurt)) {
				_animationFrame = 0;
			}
		}
		break;

	case 4:
		*animation = kModelAnimationMutant3YellOrHurt;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant3YellOrHurt)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = kModelAnimationMutant3CalmTalk;
		}
		break;

	case 5:
		*animation = kModelAnimationMutant3PicksUpAndThrowsRock;
		++_animationFrame;
		if (_animationFrame == 9) {
			int snd;
			if (Random_Query(1, 2) == 1) {
				snd = 9010;
			} else {
				snd = 9015;
			}
			Sound_Play_Speech_Line(kActorMutant3, snd, 75, 0, 99);
		}
		if (_animationFrame == 11) {
			Actor_Combat_AI_Hit_Attempt(kActorMutant3);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			Actor_Change_Animation_Mode(kActorMutant3, kAnimationModeIdle);
		}
		break;

	case 6:
		*animation = kModelAnimationMutant3ShotDead;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant3ShotDead)) {
			Actor_Change_Animation_Mode(kActorMutant3, 88);
		}
		break;

	case 7:
		*animation = kModelAnimationMutant3ShotDead;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant3ShotDead) - 1;
		break;

	case 8:
		*animation = kModelAnimationMutant3CrouchedWaiting;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant3CrouchedWaiting)) {
			_animationFrame = 0;
		}
		break;

	case 9:
		*animation = kModelAnimationMutant3CrouchedFromStanding;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant3CrouchedFromStanding)) {
			_animationFrame = 0;
			_animationState = 8;
			*animation = kModelAnimationMutant3CrouchedWaiting;
		}
		break;

	case 10:
		*animation = kModelAnimationMutant3CrouchedToStanding;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant3CrouchedToStanding)) {
			*animation = kModelAnimationMutant3Idle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 11:
		*animation = kModelAnimationMutant3ShotDead;
		_animationFrame += _var1;
		if (_animationFrame == 3) {
			Sound_Play(kSfxHURT1M3, 100, 0, 0, 50);
			_var1 = -1;
		} else {
			if (_animationFrame == 0) {
				Actor_Change_Animation_Mode(kActorMutant3, kAnimationModeIdle);
			}
		}
		break;

	default:
		// Dummy placeholder, kModelAnimationZubenIdle (406) is a Zuben animation
		*animation = kModelAnimationZubenIdle;
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptMutant3::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		switch (_animationState) {
		case 3:
			// fall through
		case 4:
			_resumeIdleAfterFramesetCompletesFlag = true;
			break;

		case 8:
			_animationState = 10;
			_animationFrame = 0;
			break;

		case 10:
			return true;

		default:
			_animationState = 0;
			_animationFrame = 0;
			break;
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
		// fall through
	case 12:
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

	case 21:
		// fall through
	case 22:
		_animationState = 11;
		_animationFrame = 0;
		_var1 = 1;
		break;

	case 88:
		_animationState = 7;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationMutant3ShotDead) - 1;
		break;

 	case 43:
		if ((unsigned int)(_animationState - 8) > 1) {
			_animationState = 9;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeDie:
		_animationState = 6;
		_animationFrame = 0;
		break;
	}

	return true;
}

void AIScriptMutant3::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptMutant3::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptMutant3::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptMutant3::FledCombat() {
	Actor_Set_Goal_Number(kActorMutant3, 403);
}

} // End of namespace BladeRunner
