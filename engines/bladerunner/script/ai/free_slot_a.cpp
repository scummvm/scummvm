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
//#include "common/debug.h"

namespace BladeRunner {

AIScriptFreeSlotA::AIScriptFreeSlotA(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 0;
	_var2 = 1;
	_fallSpeed = 0.0f;
	_fallHeightCurrent = 0.0f; // not initialized in original
	_fallHeightTarget = 0.0f; // not initialized in original
}

void AIScriptFreeSlotA::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 0;
	_var2 = 1;
	_fallSpeed = 0.0f;
	_fallHeightCurrent = 0.0f; // not initialized in original
	_fallHeightTarget  = 0.0f; // not initialized in original

	World_Waypoint_Set(525, kSetKP02, -780.0f, -615.49f, 2611.0f);
	// TODO: A bug? world waypoint 526 is unused
	World_Waypoint_Set(526, kSetKP02, -780.0f, -615.49f, 2759.0f);
}

bool AIScriptFreeSlotA::Update() {
	switch (Global_Variable_Query(kVariableChapter)) {
	case 4:
		// Act 4
		if (Actor_Query_Which_Set_In(kActorMcCoy) == kSetUG15
		 && Actor_Query_Which_Set_In(kActorFreeSlotA) == kSetUG15
		) {
			int goal = Actor_Query_Goal_Number(kActorFreeSlotA);
			if ((goal == kGoalFreeSlotAUG15RunToOtherSide
			     || goal == kGoalFreeSlotAUG15RunBack)
			    && Actor_Query_Inch_Distance_From_Actor(kActorFreeSlotA, kActorMcCoy) <= 48
			) {
				Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAUG15Attack);
			} else if (goal == kGoalFreeSlotAUG15Fall) {
				float x, y, z;

				Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
				_fallHeightCurrent += _fallSpeed;
				if (_fallHeightTarget < _fallHeightCurrent) {
					_fallSpeed -= 0.2f;
				} else {
					_fallHeightCurrent = _fallHeightTarget;
					Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotADefault);
				}
				Actor_Set_At_XYZ(kActorFreeSlotA, x, _fallHeightCurrent, z, Actor_Query_Facing_1024(kActorFreeSlotA));
			}
			return true;
		}

		switch (Actor_Query_Goal_Number(kActorFreeSlotA)) {
		case kGoalFreeSlotAWalkAround:
#if BLADERUNNER_ORIGINAL_BUGS
			if (Actor_Query_Which_Set_In(kActorFreeSlotA) == Player_Query_Current_Set()
				&& Actor_Query_Inch_Distance_From_Actor(kActorFreeSlotA, kActorMcCoy) <= 48
			) {
				Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAAttackMcCoy);
			}
#else
			if (Actor_Query_Which_Set_In(kActorFreeSlotA) == Player_Query_Current_Set()
				&& Actor_Query_Inch_Distance_From_Actor(kActorFreeSlotA, kActorMcCoy) <= 54
			) {
				Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAAttackMcCoy);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
			break;

		case kGoalFreeSlotAAttackMcCoy:
			if (Actor_Query_Which_Set_In(kActorFreeSlotA) != Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAWalkAround);
			}
			break;

		case kGoalFreeSlotAGone:
			if (Actor_Query_Which_Set_In(kActorFreeSlotA) != Player_Query_Current_Set()) {
				Game_Flag_Reset(kFlagRatWalkingAround);
				Game_Flag_Reset(kFlagUG15BridgeWillBreak);
				Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotADefault);
			}
			break;

		default:
			if (!Game_Flag_Query(kFlagRatWalkingAround)) {
				Game_Flag_Set(kFlagRatWalkingAround);
				Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAWalkAround);
				Actor_Set_Targetable(kActorFreeSlotA, true);
			}
		}

		return true;

	case 5:
		// Act 5
		if (Actor_Query_Goal_Number(kActorFreeSlotA) < kGoalFreeSlotAAct5Default) {
			AI_Movement_Track_Flush(kActorFreeSlotA);
			Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAAct5Default);
		} else {
#if BLADERUNNER_ORIGINAL_BUGS
			if (Actor_Query_Goal_Number(kActorFreeSlotA) == kGoalFreeSlotAAct5Prepare
		        && Actor_Query_Which_Set_In(kActorMcCoy) == kSetKP02
			) {
				Actor_Set_Targetable(kActorFreeSlotA, true);
				Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAAct5KP02Attack);
			}
#else
			// copying the behavior of Rat B (kActorFreeSlotB) because it works without the bug
			// Fixes the bug where the Rat A is killed in KP02 but when McCoy re-enters it is there alive
			// in idle mode and non-target-able
			if (Actor_Query_Goal_Number(kActorFreeSlotA) != kGoalFreeSlotAAct5Prepare
		        || Actor_Query_Which_Set_In(kActorMcCoy) != kSetKP02
			) {
				if (Actor_Query_Goal_Number(kActorFreeSlotA) == kGoalFreeSlotAGone) {
					if (Actor_Query_Which_Set_In(kActorFreeSlotA) != Player_Query_Current_Set()) {
						Non_Player_Actor_Combat_Mode_Off(kActorFreeSlotA);
						Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAAct5Default);
						return true;
					}
				}
				return false;
			}
			Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAAct5KP02Attack);
			Actor_Set_Targetable(kActorFreeSlotA, true);
			return true;
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		return true;

	default:
		return false;
	}
}

void AIScriptFreeSlotA::TimerExpired(int timer) {
	//return false;
}

void AIScriptFreeSlotA::CompletedMovementTrack() {
//	debug("Rat A completed move with Goal: %d", Actor_Query_Goal_Number(kActorFreeSlotA));
	switch (Actor_Query_Goal_Number(kActorFreeSlotA)) {
	case kGoalFreeSlotAUG15WalkOut:
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAUG15RunToOtherSide);
		break;

	case kGoalFreeSlotAUG15RunToOtherSide:
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAUG15RunBack);
		break;

	case kGoalFreeSlotAUG15RunBack:
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAUG15Wait);
		break;

	case kGoalFreeSlotAWalkAround:
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAWalkAroundRestart);
		break;

	case kGoalFreeSlotAWalkAroundRestart:
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAWalkAround);
		break;

	case kGoalFreeSlotAAct5Default:
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAAct5Prepare);
		break;

	case kGoalFreeSlotAAct5KP02Attack:
		Non_Player_Actor_Combat_Mode_On(kActorFreeSlotA, kActorCombatStateIdle, false, kActorMcCoy, 8, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 5, 300, false);
		break;

	default:
		return; //false;
	}

	return; //true;
}

void AIScriptFreeSlotA::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptFreeSlotA::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorFreeSlotA) == kGoalFreeSlotAGone) {
		Actor_Face_Actor(kActorMcCoy, kActorFreeSlotA, true);
		if (_vm->_cutContent && !Game_Flag_Query(kFlagMcCoyCommentsOnHoodooRats)) {
			Game_Flag_Set(kFlagMcCoyCommentsOnHoodooRats);
			Actor_Voice_Over(1060, kActorVoiceOver);  // Hoodoo rats
			// Note: Quote 1070 is *boop* in ENG version.
			// However, it is similar to 1060 quote in FRA, DEU, ESP and ITA versions
			//          with the only difference being not mentioning the "Hoodoo Rats" name.
			//          It uses a generic "rats" in its place.
			Actor_Voice_Over(1080, kActorVoiceOver);
			Actor_Voice_Over(1090, kActorVoiceOver);
		} else {
			if (Random_Query(1, 2) == 1) {
				Actor_Says(kActorMcCoy, 8655, 16);
			} else {
				Actor_Says(kActorMcCoy, 8665, 16);
			}
		}
	}
	//return false;
}

void AIScriptFreeSlotA::EnteredSet(int setId) {
	// return false;
}

void AIScriptFreeSlotA::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptFreeSlotA::OtherAgentExitedThisSet(int otherActorId) {
#if !BLADERUNNER_ORIGINAL_BUGS
	if (otherActorId == kActorMcCoy && Actor_Query_Goal_Number(kActorFreeSlotA) == kGoalFreeSlotAGone) {
		if (Global_Variable_Query(kVariableChapter) == 4) {
			Game_Flag_Reset(kFlagRatWalkingAround);
			Game_Flag_Reset(kFlagUG15BridgeWillBreak);
			Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotADefault);
		} else if (Global_Variable_Query(kVariableChapter) == 5) {
			Non_Player_Actor_Combat_Mode_Off(kActorFreeSlotA);
			Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAAct5Default);
		}
	}
#endif // BLADERUNNER_ORIGINAL_BUGS
	// return false;
}

void AIScriptFreeSlotA::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptFreeSlotA::ShotAtAndMissed() {
	if (Actor_Query_In_Set(kActorFreeSlotA, kSetUG15)) {
		checkIfOnBridge();
	}
}

bool AIScriptFreeSlotA::ShotAtAndHit() {
	if (Actor_Query_In_Set(kActorFreeSlotA, kSetUG15)) {
		checkIfOnBridge();
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAUG15Die);
		return true;
	}
	return false;
}

void AIScriptFreeSlotA::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAGone);
}

int AIScriptFreeSlotA::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptFreeSlotA::GoalChanged(int currentGoalNumber, int newGoalNumber) {
//	debug("Rat A goal changed from %d to: %d", currentGoalNumber, newGoalNumber);
	switch (newGoalNumber) {
	case kGoalFreeSlotAUG15Wait:
		AI_Movement_Track_Flush(kActorFreeSlotA);
		Actor_Change_Animation_Mode(kActorFreeSlotA, kAnimationModeIdle);
		Actor_Set_Targetable(kActorFreeSlotA, false);
		break;

	case kGoalFreeSlotAUG15WalkOut:
		// kSetUG15
		Actor_Force_Stop_Walking(kActorMcCoy);
		AI_Movement_Track_Flush(kActorFreeSlotA);
		World_Waypoint_Set(444, kSetUG15, -48.75f, 44.66f, 87.57f);
		AI_Movement_Track_Append(kActorFreeSlotA, 444, 1);
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		break;

	case kGoalFreeSlotAUG15RunToOtherSide:
		// kSetUG15
		AI_Movement_Track_Flush(kActorFreeSlotA);
		World_Waypoint_Set(444, kSetUG15, -237.0f, 48.07f, 208.0f);
		AI_Movement_Track_Append(kActorFreeSlotA, 444, 1);
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		Actor_Set_Targetable(kActorFreeSlotA, true);
		break;

	case kGoalFreeSlotAUG15RunBack:
		// kSetUG15
		AI_Movement_Track_Flush(kActorFreeSlotA);
		World_Waypoint_Set(444, kSetUG15, 3.52f, 52.28f, 90.68f);
		AI_Movement_Track_Append(kActorFreeSlotA, 444, 0);
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		break;

	case kGoalFreeSlotAUG15Attack:
		Player_Loses_Control();
		Actor_Force_Stop_Walking(kActorMcCoy);
		AI_Movement_Track_Flush(kActorFreeSlotA);
		Actor_Face_Actor(kActorFreeSlotA, kActorMcCoy, true);
		Actor_Change_Animation_Mode(kActorFreeSlotA, kAnimationModeCombatAttack);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
		break;

	case kGoalFreeSlotAUG15Die:
		AI_Movement_Track_Flush(kActorFreeSlotA);
		Actor_Set_Targetable(kActorFreeSlotA, false);
		Game_Flag_Set(kFlagUG15RatShot);
		_animationState = 7;
		_animationFrame = 0;
		break;

	case kGoalFreeSlotAWalkAround:
		AI_Movement_Track_Flush(kActorFreeSlotA);
		goToRandomUGxx();
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		break;

	case kGoalFreeSlotAWalkAroundRestart:
		// kSetFreeSlotG
		AI_Movement_Track_Flush(kActorFreeSlotA);
		AI_Movement_Track_Append(kActorFreeSlotA, 39, 1);
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		break;

	case kGoalFreeSlotAAttackMcCoy:
		Actor_Set_Targetable(kActorFreeSlotA, true);
		Non_Player_Actor_Combat_Mode_On(kActorFreeSlotA, kActorCombatStateIdle, false, kActorMcCoy, 8, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 25, 0, 75, 5, 300, false);
		break;

	case kGoalFreeSlotAUG15Fall:
		Actor_Force_Stop_Walking(kActorFreeSlotA);
		AI_Movement_Track_Flush(kActorFreeSlotA);
		_fallHeightCurrent = 52.46f;
		_fallSpeed = -4.0f;
		_fallHeightTarget = -10.0f;
		if (_animationState != 7
		 && _animationState != 8
		) {
			_animationState = 7;
			_animationFrame = 0;
		}
		break;

	case kGoalFreeSlotAUG15Prepare:
		AI_Movement_Track_Flush(kActorFreeSlotA);
		Actor_Put_In_Set(kActorFreeSlotA, kSetUG15);
		Actor_Set_At_XYZ(kActorFreeSlotA, 3.52f, 52.28f, 90.68f, 700);
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAUG15Wait);
		break;

	case kGoalFreeSlotAAct5Default:
		// kSetFreeSlotG
		AI_Movement_Track_Append(kActorFreeSlotA, 39, 0);
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		break;

	case kGoalFreeSlotAAct5KP02Attack:
		// kSetKP02
		AI_Movement_Track_Flush(kActorFreeSlotA);
		AI_Movement_Track_Append(kActorFreeSlotA, 525, 0);
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		break;

	case kGoalFreeSlotAGone:
		Actor_Set_Health(kActorFreeSlotA, 20, 20);
		Actor_Set_Friendliness_To_Other(kActorFreeSlotA, kActorMcCoy, 40);
		break;

	default:
		return false;
	}

	return true;
}

bool AIScriptFreeSlotA::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationRatIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatIdle)) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = kModelAnimationRatSlowWalk;
		if (_var1) {
			--_var1;
		} else {
			_animationFrame += _var2;
			if (_animationFrame < 8) {
				_var2 = 1;
			} else {
				if (_animationFrame > 8) {
					_var2 = -1;
				} else if (Random_Query(0, 4)) {
					_var2 = -_var2;
				}
			}
			if (_animationFrame >= 7 && _animationFrame <= 9) {
				_var1 = Random_Query(0, 1);
			}
		}
		break;

	case 2:
		*animation = kModelAnimationRatSlowWalk;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatSlowWalk) - 1) {
			*animation = kModelAnimationRatIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 3:
		*animation = kModelAnimationRatRunning;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatRunning)) {
			_animationFrame = 0;
		}
		break;

	case 4:
		*animation = kModelAnimationRatJumpAttack;
		++_animationFrame;
		if (_animationFrame == 1) {
			int snd;
			if (Random_Query(1, 2) == 1) {
				snd = 9010;
			} else {
				snd = 9015;
			}
			Sound_Play_Speech_Line(kActorFreeSlotA, snd, 75, 0, 99);
		}
		if (_animationFrame == 3) {
			Ambient_Sounds_Play_Sound(kSfxRATTY4, 99, 0, 0, 20);
			Actor_Combat_AI_Hit_Attempt(kActorFreeSlotA);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatJumpAttack)) {
			_animationState = 0;
			_animationFrame = 0;
			Actor_Change_Animation_Mode(kActorFreeSlotA, kAnimationModeCombatIdle);
		}
		break;

	case 5:
		// This is an animation for Maggie (exploding) but is also used for generic death states (rats, generic walkers)
		// probably for debug purposes
		*animation = kModelAnimationMaggieExploding;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMaggieExploding) - 1) {
			Actor_Change_Animation_Mode(kActorFreeSlotA, kAnimationModeIdle);
		}
		break;

	case 6:
		if (_animationFrame == 1) {
			Ambient_Sounds_Play_Sound(kSfxRATTY3, 99, 0, 0, 20);
		}
		*animation = kModelAnimationRatHurt;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatHurt)) {
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorFreeSlotA, kAnimationModeIdle);
		}
		break;

	case 7:
		*animation = kModelAnimationRatDying;
		++_animationFrame;
		if (_animationFrame == 1) {
			Ambient_Sounds_Play_Sound(kSfxRATTY5, 99, 0, 0, 25);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatDying) - 1) {
			// -1 in the clause is ok here since the next _animationState is 8 which will maintain the last frame (Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatDying) - 1)
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatDying) - 1;
			_animationState = 8;
			Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAGone);
		}
		break;

	case 8:
		*animation = kModelAnimationRatDying;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationRatDying) - 1;
		break;

	default:
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptFreeSlotA::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if ((unsigned int)(_animationState - 1) > 1) {
			_animationState = 0;
			_animationFrame = 0;
		} else if (_animationState == 1) {
			_animationState = 2;
		}
		break;

	case 1:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 4:
		if ((unsigned int)(_animationState - 1) > 1) {
			_animationState = 0;
			_animationFrame = 0;
		} else if (_animationState == 1) {
			_animationState = 2;
		}
		break;

	case 6:
		_animationState = 4;
		_animationFrame = 0;
		break;

	case 7:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 8:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 21:
		_animationState = 6;
		_animationFrame = 0;
		break;

	case 43:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		_animationState = 7;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptFreeSlotA::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptFreeSlotA::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptFreeSlotA::ReachedMovementTrackWaypoint(int waypointId) {
	if (Actor_Query_Which_Set_In(kActorFreeSlotA) == kSetUG01) {
		if (waypointId == 465) {
			Actor_Change_Animation_Mode(kActorFreeSlotA, 43);
		}
	} else if (Actor_Query_Goal_Number(kActorFreeSlotA) == kGoalFreeSlotAUG15RunToOtherSide) {
		Actor_Face_Actor(kActorFreeSlotA, kActorMcCoy, true);
	}

	return true;
}

void AIScriptFreeSlotA::FledCombat() {
	// return false;
}

void AIScriptFreeSlotA::checkIfOnBridge() {
	if (_vm->_cutContent && Query_Difficulty_Level() == kGameDifficultyEasy) {
		// Make the bridge indestructible on easy mode for the enhanced version
		return;
	}
	float x, y, z;
	Actor_Query_XYZ(kActorFreeSlotA, &x, &y, &z);
	// bug? this should probably check if McCoy is close enough because bridge will break long after rat died and player tries to walk through
	if (-150.0 <= x && x < -30.0f) {
		Game_Flag_Set(kFlagUG15BridgeWillBreak);
	}
}

void AIScriptFreeSlotA::goToRandomUGxx() {
	switch (Random_Query(1, 14)) {
	case 1:
		// kSetUG01
#if BLADERUNNER_ORIGINAL_BUGS
		AI_Movement_Track_Append(kActorFreeSlotA, 450, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 451, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 450, 0);
		break;
#else
		// Don't put rats in UG01 when Lucy is also here
		if (!Actor_Query_In_Set(kActorLucy, kSetUG01)) {
			AI_Movement_Track_Append(kActorFreeSlotA, 450, 1);
			AI_Movement_Track_Append(kActorFreeSlotA, 451, 5);
			AI_Movement_Track_Append(kActorFreeSlotA, 450, 0);
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		// fall through
	case 2:
		// kSetUG01
#if BLADERUNNER_ORIGINAL_BUGS
		World_Waypoint_Set(463, kSetUG01, 144.98f, -50.13f, -175.75f);
		World_Waypoint_Set(464, kSetUG01, 105.6f,  -50.13f, -578.46f);
		World_Waypoint_Set(465, kSetUG01,  62.0f,  -50.13f, -574.0f);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 465, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 5);
		break;
#else
		// Don't put rats in UG01 when Lucy is also here
		if (!Actor_Query_In_Set(kActorLucy, kSetUG01)) {
			World_Waypoint_Set(463, kSetUG01, 144.98f, -50.13f, -175.75f);
			World_Waypoint_Set(464, kSetUG01, 105.6f,  -50.13f, -578.46f);
			World_Waypoint_Set(465, kSetUG01,  62.0f,  -50.13f, -574.0f);
			AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
			AI_Movement_Track_Append(kActorFreeSlotA, 464, 1);
			AI_Movement_Track_Append(kActorFreeSlotA, 465, 5);
			AI_Movement_Track_Append(kActorFreeSlotA, 463, 5);
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		// fall through
	case 3:
		// kSetUG04
		AI_Movement_Track_Append(kActorFreeSlotA, 446, 15);
		AI_Movement_Track_Append(kActorFreeSlotA, 447,  1);
		AI_Movement_Track_Append(kActorFreeSlotA, 449,  1);
		AI_Movement_Track_Append(kActorFreeSlotA, 448,  2);
		AI_Movement_Track_Append(kActorFreeSlotA, 449,  0);
		break;

	case 4:
		// kSetUG04
		World_Waypoint_Set(463, kSetUG04, -22.7f,   6.39f,    33.12f);
		World_Waypoint_Set(464, kSetUG04,  -6.70f, -1.74f,  -362.88f);
		World_Waypoint_Set(465, kSetUG04, 164.0f,  11.87f, -1013.0f);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 2);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 0);
		AI_Movement_Track_Append(kActorFreeSlotA, 465, 0);
		break;

	case 5:
		// kSetUG05
		AI_Movement_Track_Append(kActorFreeSlotA, 457, 15);
		AI_Movement_Track_Append(kActorFreeSlotA, 458,  0);
		AI_Movement_Track_Append(kActorFreeSlotA, 459, 15);
		break;

	case 6:
		// kSetUG06
		AI_Movement_Track_Append(kActorFreeSlotA, 460, 15);
		AI_Movement_Track_Append(kActorFreeSlotA, 461,  5);
		AI_Movement_Track_Append(kActorFreeSlotA, 460, 15);
		break;

	case 7:
		// kSetUG07 or kSetFreeSlotG
#if BLADERUNNER_ORIGINAL_BUGS
		if (Actor_Query_In_Set(kActorClovis, kSetUG07)) {
			// this check is not very effective since Clovis
			// will spawn in the set when McCoy goes to the downwards exit
			// at which point a rat may already be in the set and that could
			// make them collide with Clovis' path
			AI_Movement_Track_Append(kActorFreeSlotA, 39, 10); // kSetFreeSlotG
		} else {
			World_Waypoint_Set(463, kSetUG07,  -88.78f, -12.21f, -184.08f);
			World_Waypoint_Set(464, kSetUG07,  250.0f,  -12.21f, -342.0f);
			World_Waypoint_Set(465, kSetUG07, -164.78f, -12.21f, -832.08f);
			AI_Movement_Track_Append(kActorFreeSlotA, 463, 5);
			AI_Movement_Track_Append(kActorFreeSlotA, 464, 1);
			// TODO: A bug? the waypoint 465 is created but is unused
		}
		break;
#else
		// Don't put rats in UG07 after the UG18 Guzza scene
		// since Clovis may be there too and that does not work well
		if (!Game_Flag_Query(kFlagUG18GuzzaScene)) {
			World_Waypoint_Set(463, kSetUG07,  -88.78f, -12.21f, -184.08f);
			World_Waypoint_Set(464, kSetUG07,  250.0f,  -12.21f, -342.0f);
			World_Waypoint_Set(465, kSetUG07, -164.78f, -12.21f, -832.08f);
			AI_Movement_Track_Append(kActorFreeSlotA, 463, 5);
			AI_Movement_Track_Append(kActorFreeSlotA, 464, 1);
			// TODO: A bug? the waypoint 465 is created but is unused
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		// fall through
	case 8:
		// kSetUG07
#if BLADERUNNER_ORIGINAL_BUGS
		World_Waypoint_Set(463, kSetUG07,  -88.78f, -12.21f, -184.08f);
		World_Waypoint_Set(464, kSetUG07,  250.0f,  -12.21f, -342.0f);
		World_Waypoint_Set(465, kSetUG07, -164.78f, -12.21f, -832.08f);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
		// TODO: A bug? the waypoint 465 is created but is unused
		break;
#else
		// Don't put rats in UG07 after the UG18 Guzza scene
		// since Clovis may be there too and that does not work well
		if (!Game_Flag_Query(kFlagUG18GuzzaScene)) {
			World_Waypoint_Set(463, kSetUG07,  -88.78f, -12.21f, -184.08f);
			World_Waypoint_Set(464, kSetUG07,  250.0f,  -12.21f, -342.0f);
			World_Waypoint_Set(465, kSetUG07, -164.78f, -12.21f, -832.08f);
			AI_Movement_Track_Append(kActorFreeSlotA, 464, 5);
			AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
			// TODO: A bug? the waypoint 465 is created but is unused
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		// fall through
	case 9:
		// kSetUG07
#if BLADERUNNER_ORIGINAL_BUGS
		World_Waypoint_Set(463, kSetUG07,  -88.78f, -12.21f, -184.08f);
		World_Waypoint_Set(464, kSetUG07,  250.0f,  -12.21f, -342.0f);
		World_Waypoint_Set(465, kSetUG07, -164.78f, -12.21f, -832.08f);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 465, 1);
		// TODO: A bug? the waypoint 463 is created but is unused
		break;
#else
		// Don't put rats in UG07 after the UG18 Guzza scene
		// since Clovis may be there too and that does not work well
		if (!Game_Flag_Query(kFlagUG18GuzzaScene)) {
			World_Waypoint_Set(463, kSetUG07,  -88.78f, -12.21f, -184.08f);
			World_Waypoint_Set(464, kSetUG07,  250.0f,  -12.21f, -342.0f);
			World_Waypoint_Set(465, kSetUG07, -164.78f, -12.21f, -832.08f);
			AI_Movement_Track_Append(kActorFreeSlotA, 464, 5);
			AI_Movement_Track_Append(kActorFreeSlotA, 465, 1);
			// TODO: A bug? the waypoint 463 is created but is unused
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		// fall through
	case 10:
		// kSetUG07
#if BLADERUNNER_ORIGINAL_BUGS
		World_Waypoint_Set(463, kSetUG07,  -88.78f, -12.21f, -184.08f);
		World_Waypoint_Set(464, kSetUG07,  250.0f,  -12.21f, -342.0f);
		World_Waypoint_Set(465, kSetUG07, -164.78f, -12.21f, -832.08f);
		AI_Movement_Track_Append(kActorFreeSlotA, 465, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 1);
		// TODO: A bug? the waypoint 463 is created but is unused
		break;
#else
		// Don't put rats in UG07 after the UG18 Guzza scene
		// since Clovis may be there too and that does not work well
		if (!Game_Flag_Query(kFlagUG18GuzzaScene)) {
			World_Waypoint_Set(463, kSetUG07,  -88.78f, -12.21f, -184.08f);
			World_Waypoint_Set(464, kSetUG07,  250.0f,  -12.21f, -342.0f);
			World_Waypoint_Set(465, kSetUG07, -164.78f, -12.21f, -832.08f);
			AI_Movement_Track_Append(kActorFreeSlotA, 465, 5);
			AI_Movement_Track_Append(kActorFreeSlotA, 464, 1);
			// TODO: A bug? the waypoint 463 is created but is unused
			break;
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		// fall through
	case 11:
		// kSetUG09
		World_Waypoint_Set(463, kSetUG09,   91.0f, 156.94f, -498.0f);
		World_Waypoint_Set(464, kSetUG09, -149.0f, 156.94f, -498.0f);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 1);
		break;

	case 12:
		// kSetUG09
		World_Waypoint_Set(463, kSetUG09,   91.0f, 156.94f, -498.0f);
		World_Waypoint_Set(464, kSetUG09, -149.0f, 156.94f, -498.0f);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
		break;

	case 13:
		// kSetUG09
#if BLADERUNNER_ORIGINAL_BUGS
		// this makes the rat appear on the pipe (top left) but this is buggy
		// since it appears floating there
		World_Waypoint_Set(463, kSetUG09, -152.51f, 277.31f, 311.98f);
		World_Waypoint_Set(464, kSetUG09, -124.51f, 275.08f, 319.98f);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 8);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
#else
		// replacing with something more normal
		World_Waypoint_Set(463, kSetUG09,  91.0f,  156.94f, -498.0f);
		World_Waypoint_Set(464, kSetUG09, -29.60f, 156.94f, -498.0f);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
#endif
		break;

	case 14:
		// kSetUG12
		World_Waypoint_Set(463, kSetUG12, -360.67f, 21.39f,   517.55f);
		World_Waypoint_Set(464, kSetUG12, -250.67f, 21.39f,   477.55f);
		World_Waypoint_Set(465, kSetUG12, -248.67f, 21.39f, -1454.45f);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 8);
		AI_Movement_Track_Append(kActorFreeSlotA, 465, 1);
		break;

	default:
		// kSetFreeSlotG
		AI_Movement_Track_Append(kActorFreeSlotA, 39, Random_Query(1, 10));
		break;
	}
}

} // End of namespace BladeRunner
