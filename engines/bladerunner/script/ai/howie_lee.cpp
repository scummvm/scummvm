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

AIScriptHowieLee::AIScriptHowieLee(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	var_45DFB8 = 0;
}

void AIScriptHowieLee::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;
	var_45DFB8 = false;

	Actor_Put_In_Set(kActorHowieLee, kSetCT01_CT12);
	Actor_Set_At_Waypoint(kActorHowieLee, 67, 605);
	Actor_Set_Goal_Number(kActorHowieLee, 0);
}

bool AIScriptHowieLee::Update() {
	if (Actor_Query_Goal_Number(kActorHowieLee) < 100
	 && Global_Variable_Query(kVariableChapter) == 2
	) {
		Actor_Set_Goal_Number(kActorHowieLee, 100);
	}

	if (Global_Variable_Query(kVariableChapter) > 1) {
		return true;
	}

	if (Actor_Query_Goal_Number(kActorHowieLee) == 0) {
		Actor_Set_Goal_Number(kActorHowieLee, 1);
	}

	if ( Game_Flag_Query(kFlagMcCoyInChinaTown)
	 && !Actor_Query_In_Set(kActorHowieLee, kSetCT01_CT12)
	) {
		AI_Movement_Track_Flush(kActorHowieLee);
		AI_Movement_Track_Append(kActorHowieLee, 67, 0);
		Actor_Set_Goal_Number(kActorHowieLee, 0);
	}

	if ( Actor_Query_Goal_Number(kActorHowieLee) == 1
	 &&  Game_Flag_Query(kFlagCT01BoughtHowieLeeFood)
	 && !Game_Flag_Query(kFlagMcCoyInChinaTown)
	) {
		Actor_Set_Goal_Number(kActorHowieLee, 4);
		return true;
	}

	return false;
}

void AIScriptHowieLee::TimerExpired(int timer) {
	//return false;
}

void AIScriptHowieLee::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorHowieLee) == 1) {
		Actor_Set_Goal_Number(kActorHowieLee, 2);
		return; // true;
	}

	if (Actor_Query_Goal_Number(kActorHowieLee) == 2) {
		Actor_Set_Goal_Number(kActorHowieLee, 3);
		return; // true;
	}

	if (Actor_Query_Goal_Number(kActorHowieLee) == 3) {
		Actor_Set_Goal_Number(kActorHowieLee, 0);
		return; // true;
	}

	// Bug/intentional in original game?
	if (Actor_Query_Goal_Number(kActorHowieLee) == 3) {
		Actor_Set_Goal_Number(kActorHowieLee, 4);
		return; // true;
	}

	if (Actor_Query_Goal_Number(kActorHowieLee) == 4) {
		Actor_Set_Goal_Number(kActorHowieLee, 5);
		return; // true;
	}

	if (Actor_Query_Goal_Number(kActorHowieLee) == 5) {
		Actor_Set_Goal_Number(kActorHowieLee, 0);
		return; // true;
	}
	// return false;
}

void AIScriptHowieLee::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptHowieLee::ClickedByPlayer() {
	//return false;
}

void AIScriptHowieLee::EnteredScene(int sceneId) {
	if (Actor_Query_Goal_Number(kActorHowieLee) == 4
	 && Actor_Query_In_Set(kActorHowieLee, kSetCT03_CT04)
	) {
		if ( Game_Flag_Query(kFlagCT04HomelessKilledByMcCoy)
		 && !Game_Flag_Query(kFlagCT04HomelessBodyInDumpster)
		 && !Game_Flag_Query(kFlagCT04HomelessBodyFound)
		) {
			Game_Flag_Set(kFlagCT04HomelessBodyFound);
			// return false;
		}

		if (!Game_Flag_Query(kFlagCT04HomelessKilledByMcCoy)
		 &&  Game_Flag_Query(kFlagCT04HomelessBodyInDumpster)
		 && !Game_Flag_Query(kFlagCT04HomelessBodyFound)
		 &&  Random_Query(1, 10) == 1
		) {
			Game_Flag_Set(kFlagCT04HomelessBodyFound);
			// return true;
		}
		// return false;
	}
}

void AIScriptHowieLee::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptHowieLee::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptHowieLee::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptHowieLee::ShotAtAndMissed() {
	// return false;
}

bool AIScriptHowieLee::ShotAtAndHit() {
	return false;
}

void AIScriptHowieLee::Retired(int byActorId) {
	// return false;
}

int AIScriptHowieLee::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptHowieLee::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 1:
		AI_Movement_Track_Flush(kActorHowieLee);
		if (Random_Query(1, 2) == 1) {
			AI_Movement_Track_Append_With_Facing(kActorHowieLee, 67, Random_Query(3, 10), 720);
		} else {
			AI_Movement_Track_Append_With_Facing(kActorHowieLee, 69, Random_Query(3, 10), 640);
		}
		AI_Movement_Track_Repeat(kActorHowieLee);
		return true;
	case 2:
		AI_Movement_Track_Flush(kActorHowieLee);
		if (Random_Query(1, 2) == 1) {
			AI_Movement_Track_Append_With_Facing(kActorHowieLee, 68, Random_Query(3, 10), 641);
		} else {
			AI_Movement_Track_Append_With_Facing(kActorHowieLee, 67, Random_Query(3, 10), 720);
		}
		AI_Movement_Track_Repeat(kActorHowieLee);
		return true;
	case 3:
		AI_Movement_Track_Flush(kActorHowieLee);
		if (Random_Query(1, 2) == 1) {
			AI_Movement_Track_Append_With_Facing(kActorHowieLee, 69, Random_Query(3, 10), 640);
		} else {
			AI_Movement_Track_Append_With_Facing(kActorHowieLee, 68, Random_Query(3, 10), 641);
		}
		AI_Movement_Track_Repeat(kActorHowieLee);
		return true;
	case 4:
		AI_Movement_Track_Flush(kActorHowieLee);
		AI_Movement_Track_Append(kActorHowieLee, 66, 30);
		AI_Movement_Track_Repeat(kActorHowieLee);
		return true;
	case 5:
		AI_Movement_Track_Flush(kActorHowieLee);
		AI_Movement_Track_Append(kActorHowieLee, 40, 60);
		AI_Movement_Track_Repeat(kActorHowieLee);
		return true;
	case 6:
		AI_Movement_Track_Flush(kActorHowieLee);
		AI_Movement_Track_Append(kActorHowieLee, 67, 90);
		AI_Movement_Track_Repeat(kActorHowieLee);
		return true;
	case 50:
		AI_Movement_Track_Flush(kActorHowieLee);
		return false;
	case 100:
		AI_Movement_Track_Flush(kActorHowieLee);
		Actor_Put_In_Set(kActorHowieLee, kSetFreeSlotC);
		Actor_Set_At_Waypoint(kActorHowieLee, 35, 0);
		return false;
	}
	return false;
}

bool AIScriptHowieLee::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		_animationFrame++;
		if (var_45DFB8) {
			*animation = 673;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(673)) {
				_animationFrame = 0;
				if (Random_Query(0, 2) > 0) {
					var_45DFB8 ^= 1;
				}
			}
		} else {
			*animation = 671;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(671)) {
				_animationFrame = 0;
				if (Random_Query(0, 1) > 0) {
					var_45DFB8 ^= 1;
				}
			}
		}
		break;
	case 1:
		*animation = 674;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(674)) {
			_animationFrame = 0;
			if (_animationState < 3 || _animationState > 8) {
				_animationState = 0;
				_animationFrame = 0;
				var_45DFB8 = Random_Query(0, 1);
			} else {
				Game_Flag_Set(kFlagHowieLeeAnimation1);
			}
			*animation = 673;
		}
		break;
	case 2:
		*animation = 672;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(672)) {
			_animationFrame = 0;
		}
		break;
	case 3:
		if (Game_Flag_Query(kFlagHowieLeeAnimation1)) {
			Game_Flag_Reset(kFlagHowieLeeAnimation1);
			_animationState = 0;
			_animationFrame = 0;
			var_45DFB8 = Random_Query(0, 1);
			*animation = 671;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(676)) {
				_animationFrame = 0;
			}
			*animation = 676;
		}
		break;
	case 4:
		*animation = 677;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(677)) {
			_animationState = 3;
			_animationFrame = 0;
			*animation = 676;
		}
		break;
	case 5:
		*animation = 678;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(678)) {
			_animationState = 3;
			_animationFrame = 0;
			*animation = 676;
		}
		break;
	case 6:
		*animation = 679;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(679)) {
			_animationState = 3;
			_animationFrame = 0;
			*animation = 676;
		}
		break;
	case 7:
		*animation = 680;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(680)) {
			_animationState = 3;
			_animationFrame = 0;
			*animation = 676;
		}
		break;
	case 8:
		*animation = 681;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(681)) {
			_animationState = 3;
			_animationFrame = 0;
			*animation = 676;
		}
		break;
	default:
		*animation = 399;
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptHowieLee::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (_animationState < 3 || _animationState > 8) {
			_animationState = 0;
			_animationFrame = 0;
			var_45DFB8 = Random_Query(0, 1);
		} else {
			Game_Flag_Set(kFlagHowieLeeAnimation1);
		}
		break;
	case kAnimationModeWalk:
		if (_animationState != 2) {
			_animationState = 2;
			_animationFrame = 0;
		}
		break;
	case kAnimationModeTalk:
		_animationState = 3;
		_animationFrame = 0;
		break;
	case 12:
		_animationState = 4;
		_animationFrame = 0;
		break;
	case 13:
		_animationState = 5;
		_animationFrame = 0;
		break;
	case 14:
		_animationState = 6;
		_animationFrame = 0;
		break;
	case 15:
		_animationState = 7;
		_animationFrame = 0;
		break;
	case 16:
		_animationState = 8;
		_animationFrame = 0;
		break;
	case 43:
		_animationState = 1;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptHowieLee::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptHowieLee::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptHowieLee::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 67 && Random_Query(1, 2) == 2) {
		Actor_Face_Heading(kActorHowieLee, 850, 0);
		_animationFrame = 0;
		_animationState = 1;
	}
	return true;
}

void AIScriptHowieLee::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
