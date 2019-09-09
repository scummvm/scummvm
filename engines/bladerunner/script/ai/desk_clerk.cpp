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

AIScriptDeskClerk::AIScriptDeskClerk(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag1 = false;
	_flag2 = false;
	_var3 = 75;
}

void AIScriptDeskClerk::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag1 = false;
	_flag2 = false;
	_var3 = 75;
	Actor_Set_Goal_Number(kActorDeskClerk, kGoalDeskClerkDefault);
}

bool AIScriptDeskClerk::Update() {
	if (Actor_Query_Goal_Number(kActorDeskClerk) == kGoalDeskClerkKnockedOut
	 && Player_Query_Current_Set() != kSetCT01_CT12
	 && Player_Query_Current_Set() != kSetCT03_CT04
	 && Player_Query_Current_Set() != kSetCT08_CT51_UG12
	 && Player_Query_Current_Set() != kSetCT02
	 && Player_Query_Current_Set() != kSetCT05
	 && Player_Query_Current_Set() != kSetCT06
	 && Player_Query_Current_Set() != kSetCT07
	 && Player_Query_Current_Set() != kSetCT09
	 && Player_Query_Current_Set() != kSetCT10
	 && Player_Query_Current_Set() != kSetCT11
	) {
		Actor_Set_Goal_Number(kActorDeskClerk, kGoalDeskClerkRecovered);
	}

	if (Global_Variable_Query(kVariableChapter) == 5
	 && Actor_Query_Goal_Number(kActorDeskClerk) < kGoalDeskClerkGone
	) {
		Actor_Set_Goal_Number(kActorDeskClerk, kGoalDeskClerkGone);
	}

	return false;
}

void AIScriptDeskClerk::TimerExpired(int timer) {
	//return false;
}

void AIScriptDeskClerk::CompletedMovementTrack() {
	//return false;
}

void AIScriptDeskClerk::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptDeskClerk::ClickedByPlayer() {
	//return false;
}

void AIScriptDeskClerk::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptDeskClerk::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptDeskClerk::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptDeskClerk::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptDeskClerk::ShotAtAndMissed() {
	// return false;
}

bool AIScriptDeskClerk::ShotAtAndHit() {
	return false;
}

void AIScriptDeskClerk::Retired(int byActorId) {
	// return false;
}

int AIScriptDeskClerk::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptDeskClerk::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalDeskClerkDefault:
		// fall through
	case kGoalDeskClerkRecovered:
		Actor_Put_In_Set(kActorDeskClerk, kSetCT09);
		Actor_Set_At_XYZ(kActorDeskClerk, 282.0f, 360.52f, 743.0f, 513);
		break;
	case kGoalDeskClerkKnockedOut:
		// fall through
	case kGoalDeskClerkGone:
		Actor_Put_In_Set(kActorDeskClerk, kSetFreeSlotH);
		Actor_Set_At_Waypoint(kActorDeskClerk, 40, 0);
		break;
	}
	return false;
}

bool AIScriptDeskClerk::UpdateAnimation(int *animation, int *frame) {

	switch (_animationState) {
	case 0:
		if (_flag1) {
			*animation = 662;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(662)) {
				_animationFrame = 0;
				_flag1 = false;
				*animation = 661;
				_var3 = Random_Query(50, 100);
			}
		} else {
			if (_var3 != 0) {
				--_var3;
			}

			*animation = 661;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(661)) {
				_animationFrame = 0;

				if (_var3 == 0) {
					*animation = 662;
					_flag1 = true;
				}
			}
		}
		break;

	case 1:
		*animation = 663;

		if (_animationFrame == 0
		 && _flag2
		) {
			*animation = 661;
			_animationState = 0;
			_flag1 = false;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
		}
		break;

	case 2:
		*animation = 664;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(664)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = 663;
		}
		break;

	case 3:
		*animation = 665;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(665)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = 663;
		}
		break;

	case 4:
		*animation = 666;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(666)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = 663;
		}
		break;

	case 5:
		*animation = 667;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(667)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = 663;
		}
		break;

	case 6:
		*animation = 668;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(668)) {
			_animationFrame = 0;
		}
		break;

	case 7:
		*animation = 669;

		if (_animationFrame == 0
		 && _flag2
		) {
			Actor_Change_Animation_Mode(kActorDeskClerk, 72);
			*animation = 668;
			_animationState = 6;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
		}
		break;

	case 8:
		*animation = 670;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(670) - 2) {
			Ambient_Sounds_Play_Sound(kSfxZUBLAND1, 40, 30, 30, 99);
			Actor_Set_Goal_Number(kActorDeskClerk, kGoalDeskClerkKnockedOut);
			Actor_Change_Animation_Mode(kActorDeskClerk, kAnimationModeIdle);
			*animation = 661;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;
	}

	*frame = _animationFrame;
	return true;
}

bool AIScriptDeskClerk::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		switch (_animationState) {
		case 0:
			_animationState = 8;
			_animationFrame = 0;
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			_flag2 = true;
			break;

		case 6:
			Actor_Change_Animation_Mode(kActorDeskClerk, 72);
			break;

		default:
			_animationState = 0;
			_animationFrame = 0;
			_flag1 = false;
			_var3 = Random_Query(70, 140);
			break;
		}
		break;

	case kAnimationModeTalk:
		_animationState = 1;
		_animationFrame = 0;
		_flag2 = false;
		break;

	case 12:
		_animationState = 2;
		_animationFrame = 0;
		_flag2 = false;
		break;

	case 13:
		_animationState = 3;
		_animationFrame = 0;
		_flag2 = false;
		break;

	case 14:
		_animationState = 4;
		_animationFrame = 0;
		_flag2 = false;
		break;

	case 15:
		_animationState = 5;
		_animationFrame = 0;
		_flag2 = false;
		break;

	case 26:
		_animationState = 8;
		_animationFrame = 0;
		break;

	case 58:
		_animationState = 7;
		_animationFrame = 0;
		_flag2 = false;
		break;

	case 72:
		if (_animationState != 6) {
			_animationState = 6;
			_animationFrame = 0;
		}
		break;
	}

	return true;
}

void AIScriptDeskClerk::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptDeskClerk::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptDeskClerk::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptDeskClerk::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
