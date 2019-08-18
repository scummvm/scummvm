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
	Actor_Set_At_Waypoint(kActorHowieLee, 67, 605); // in kSetCT01_CT12
	Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeDefault);
}

bool AIScriptHowieLee::Update() {

	if (_vm->_cutContent) {
		// keep Howie in Acts 2 and 3
		// Howie in Acts 2 and 3 will have a routine with existing waypoints/goals,
		// so no need to add new goals (> 100) for those Acts
		if (Actor_Query_Goal_Number(kActorHowieLee) < 100
		 && Global_Variable_Query(kVariableChapter) == 4
		) {
			Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeGoesToFreeSlotC);
		}

		if (Global_Variable_Query(kVariableChapter) > 3) {
			return true;
		}
	} else {
		if (Actor_Query_Goal_Number(kActorHowieLee) < 100
		 && Global_Variable_Query(kVariableChapter) == 2
		) {
			Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeGoesToFreeSlotC);
		}

		if (Global_Variable_Query(kVariableChapter) > 1) {
			return true;
		}
	}

	if (Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeDefault) {
		Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeMovesInDiner01);
	}

	// In the original code this is used (in Act 1) to get Howie back to the Diner
	// from goals kGoalHowieLeeGoesToCT04GarbageBin or kGoalHowieLeeGoesToFreeSlotH
	// It might cause a blink-in issue, depending when update() will be called for Howie
	// This is kept as a backup while similar code is placed in the CT01 and CT02 scenes InitializeScene()
	// that will handle Howie appearing there immediately (fix for blink in)
	if (Game_Flag_Query(kFlagMcCoyInChinaTown)
	    && !Actor_Query_In_Set(kActorHowieLee, kSetCT01_CT12)) {
		AI_Movement_Track_Flush(kActorHowieLee);
		AI_Movement_Track_Append(kActorHowieLee, 67, 0); // in kSetCT01_CT12
		Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeDefault);
	}

#if BLADERUNNER_ORIGINAL_BUGS
	if ( Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeMovesInDiner01
	 &&  Game_Flag_Query(kFlagCT01BoughtHowieLeeFood)
	 && !Game_Flag_Query(kFlagMcCoyInChinaTown) ) {
		Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeGoesToCT04GarbageBin);
		return true;
	}
#else
	// Prevents possibility of Howie Lee from blinking in/out of existence
	// when the fly-out loop is playing and/or when McCoy enters and exits the Spinner with the spinner doors animation restored
	if ((Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeMovesInDiner01
	     || Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeMovesInDiner02
	     || Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeMovesInDiner03
	     || Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeMovesInDiner06
	     || Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeDefault)
	     &&  Game_Flag_Query(kFlagCT01BoughtHowieLeeFood)
	     && !Game_Flag_Query(kFlagMcCoyInChinaTown)
	     &&  Player_Query_Current_Scene() != kSceneCT01
	     &&  Player_Query_Current_Scene() != kSceneCT12
	) {
		Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeGoesToCT04GarbageBin);
		return true;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	return false;
}

void AIScriptHowieLee::TimerExpired(int timer) {
	//return false;
}

void AIScriptHowieLee::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeMovesInDiner01) {
		Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeMovesInDiner02);
		return; // true;
	}

	if (Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeMovesInDiner02) {
		Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeMovesInDiner03);
		return; // true;
	}

	if (Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeMovesInDiner03) {
		if (_vm->_cutContent) {
			//
			// *after Act 1* Howie will be going to the Garbage Bin "semi-regularly"
			// if McCoy is not in scenes CT01 and CT12 (where Howie is visible working at the diner)
			// otherwise he'll do the short cycle (goals 0, 1, 2, 3)
			if (Global_Variable_Query(kVariableChapter) > 1
			    && Player_Query_Current_Scene() != kSceneCT01
			    && Player_Query_Current_Scene() != kSceneCT12
			    && Player_Query_Current_Scene() != kSceneCT04
			    && Random_Query(1, 5) == 1
			) {
				Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeGoesToCT04GarbageBin);
				return; // true;
			} else {
				Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeDefault);
				return; // true;
			}
		} else {
			Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeDefault);
			return; // true;
		}
	}

#if BLADERUNNER_ORIGINAL_BUGS
	if (Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeMovesInDiner03) {
			Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeGoesToCT04GarbageBin);
			return; // true;
	}
#endif // BLADERUNNER_ORIGINAL_BUGS

	if (Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeGoesToCT04GarbageBin) {
		Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeGoesToFreeSlotH);
		return; // true;
	}

	if (Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeGoesToFreeSlotH) {
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeDefault);
		return; // true;
#else
		// don't blink-in and also only return to diner routine if McCoy is actually in Chinatown
		if (Game_Flag_Query(kFlagMcCoyInChinaTown)
		    && Player_Query_Current_Scene() != kSceneCT01
		    && Player_Query_Current_Scene() != kSceneCT12
		) {
			Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeDefault);
			return; // true;
		} else {
			// return to "garbage bin" goal instead of blinking in
			Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeGoesToCT04GarbageBin);
			return; // true;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
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
	if (Actor_Query_Goal_Number(kActorHowieLee) == kGoalHowieLeeGoesToCT04GarbageBin
	 && Actor_Query_In_Set(kActorHowieLee, kSetCT03_CT04)
	) {
		if ( Game_Flag_Query(kFlagCT04HomelessKilledByMcCoy)
		 && !Game_Flag_Query(kFlagCT04HomelessBodyInDumpster)
		 && !Game_Flag_Query(kFlagCT04HomelessBodyFound)
#if BLADERUNNER_ORIGINAL_BUGS
#else
		 && !Game_Flag_Query(kFlagCT04HomelessBodyThrownAway)
#endif // BLADERUNNER_ORIGINAL_BUGS
		) {
			Game_Flag_Set(kFlagCT04HomelessBodyFound);
			// return false;
		}

		if (Game_Flag_Query(kFlagCT04HomelessKilledByMcCoy)
		 &&  Game_Flag_Query(kFlagCT04HomelessBodyInDumpster)
		 && !Game_Flag_Query(kFlagCT04HomelessBodyFound)
#if BLADERUNNER_ORIGINAL_BUGS
#else
		 && !Game_Flag_Query(kFlagCT04HomelessBodyThrownAway)
#endif // BLADERUNNER_ORIGINAL_BUGS
		 &&  Random_Query(1, 10) == 1
		) {
			Game_Flag_Set(kFlagCT04HomelessBodyFound);
			// return true;
		}
		// return false;
	}
}

void AIScriptHowieLee::OtherAgentEnteredThisScene(int otherActorId) {
#if BLADERUNNER_ORIGINAL_BUGS
#else
	// prevent player from actually meeting hovering Howie half-body in kSetCT03_CT04
	if (otherActorId == kActorMcCoy
	    && Actor_Query_In_Set(kActorHowieLee, kSetCT03_CT04)
	) {
		AI_Movement_Track_Flush(kActorHowieLee);
		AI_Movement_Track_Append(kActorHowieLee, 67, 0); // in kSetCT01_CT12
		Actor_Set_Goal_Number(kActorHowieLee, kGoalHowieLeeDefault);
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
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
	case kGoalHowieLeeMovesInDiner01:
		AI_Movement_Track_Flush(kActorHowieLee);
		if (Random_Query(1, 2) == 1) {
			AI_Movement_Track_Append_With_Facing(kActorHowieLee, 67, Random_Query(3, 10), 720); // in kSetCT01_CT12
		} else {
			AI_Movement_Track_Append_With_Facing(kActorHowieLee, 69, Random_Query(3, 10), 640); // in kSetCT01_CT12
		}
		AI_Movement_Track_Repeat(kActorHowieLee);
		return true;
	case kGoalHowieLeeMovesInDiner02:
		AI_Movement_Track_Flush(kActorHowieLee);
		if (Random_Query(1, 2) == 1) {
			AI_Movement_Track_Append_With_Facing(kActorHowieLee, 68, Random_Query(3, 10), 641); // in kSetCT01_CT12
		} else {
			AI_Movement_Track_Append_With_Facing(kActorHowieLee, 67, Random_Query(3, 10), 720); // in kSetCT01_CT12
		}
		AI_Movement_Track_Repeat(kActorHowieLee);
		return true;
	case kGoalHowieLeeMovesInDiner03:
		AI_Movement_Track_Flush(kActorHowieLee);
		if (Random_Query(1, 2) == 1) {
			AI_Movement_Track_Append_With_Facing(kActorHowieLee, 69, Random_Query(3, 10), 640); // in kSetCT01_CT12
		} else {
			AI_Movement_Track_Append_With_Facing(kActorHowieLee, 68, Random_Query(3, 10), 641); // in kSetCT01_CT12
		}
		AI_Movement_Track_Repeat(kActorHowieLee);
		return true;
	case kGoalHowieLeeGoesToCT04GarbageBin:
		AI_Movement_Track_Flush(kActorHowieLee);
		AI_Movement_Track_Append(kActorHowieLee, 66, 30);  // in kSetCT03_CT04
		AI_Movement_Track_Repeat(kActorHowieLee);
		return true;
	case kGoalHowieLeeGoesToFreeSlotH:
		AI_Movement_Track_Flush(kActorHowieLee);
		AI_Movement_Track_Append(kActorHowieLee, 40, 60);  // in kSetFreeSlotH
		AI_Movement_Track_Repeat(kActorHowieLee);
		return true;
	case kGoalHowieLeeMovesInDiner06: // Unused goal?
		AI_Movement_Track_Flush(kActorHowieLee);
		AI_Movement_Track_Append(kActorHowieLee, 67, 90); // in kSetCT01_CT12
		AI_Movement_Track_Repeat(kActorHowieLee);
		return true;
	case kGoalHowieLeeStopMoving:
		AI_Movement_Track_Flush(kActorHowieLee);
		return false;
	case kGoalHowieLeeGoesToFreeSlotC:
		AI_Movement_Track_Flush(kActorHowieLee);
		Actor_Put_In_Set(kActorHowieLee, kSetFreeSlotC);
		Actor_Set_At_Waypoint(kActorHowieLee, 35, 0); // in kSetFreeSlotC
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
