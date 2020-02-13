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
#include "bladerunner/vector.h"

namespace BladeRunner {

enum kMaggieStates {
	kMaggieStateIdle         = 0,
	kMaggieStateWalking      = 1,
	kMaggieStateJumping      = 2,
	kMaggieStateHappyA       = 3,
	kMaggieStateHappyB       = 4,
	kMaggieStateLayingDown   = 5,
	kMaggieStateLayingIdle   = 6,
	kMaggieStateStandingUp   = 7,
	kMaggieStateGoingToSleep = 8,
	kMaggieStateSleeping     = 9,
	kMaggieStateWakingUp     = 10,
	kMaggieStateBombIdle     = 11,
	kMaggieStateBombWalk     = 12,
	kMaggieStateBombJumping  = 13,
	kMaggieStateExploding    = 14,
	kMaggieStateDeadExploded = 15,
	kMaggieStateDead         = 16
};

AIScriptMaggie::AIScriptMaggie(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	var_45F3F8 = 0;
	var_45F3FC = 0;
	var_45F400 = 0;
	var_45F404 = 0;
	var_45F408 = 0;
}

void AIScriptMaggie::Initialize() {
	_animationState = kMaggieStateIdle;
	_animationFrame = 0;
	_animationStateNext = 0;
	_animationNext = 0;
	var_45F3F8 = 0;
	var_45F3FC = 0;
	var_45F400 = 0;
	var_45F404 = 0;
	var_45F408 = 0;
	Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Default);
}

bool AIScriptMaggie::Update() {
	int goal = Actor_Query_Goal_Number(kActorMaggie);

	if (Actor_Query_Which_Set_In(kActorMaggie) == kSetMA02_MA04
	 && Global_Variable_Query(kVariableChapter) == 4
	) {
		Actor_Put_In_Set(kActorMaggie, kSetFreeSlotG);
		Actor_Set_At_Waypoint(kActorMaggie, 39, 0);
	}

	if (goal == kGoalMaggieKP05WillExplode) {
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05Explode);
	} else if (goal == kGoalMaggieKP05WalkToMcCoy
	        && Actor_Query_Inch_Distance_From_Actor(kActorMcCoy, kActorMaggie) < 60
	) {
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05Explode);
	}

	if (Global_Variable_Query(kVariableChapter) == 5) {
		if (Actor_Query_Goal_Number(kActorMaggie) < 400) {
			Actor_Set_Goal_Number(kActorMaggie, 400);
		}
		return true;
	}
	return false;
}

void AIScriptMaggie::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask0) {
		int goal = Actor_Query_Goal_Number(kActorMaggie);
		if (goal == kGoalMaggieMA02Wait) {
			AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
			if (Random_Query(0, 4)) {
				AI_Movement_Track_Flush(kActorMaggie);
				AI_Movement_Track_Append(kActorMaggie, randomWaypointMA02(), 0);
				AI_Movement_Track_Repeat(kActorMaggie);
			} else {
				Actor_Change_Animation_Mode(kActorMaggie, 54);
			}
			return; //true
		}

		if (goal == kGoalMaggieMA02SitDown) {
			AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
			Actor_Change_Animation_Mode(kActorMaggie, 55);
			return; //true
		}
	}
	return; //false
}

void AIScriptMaggie::CompletedMovementTrack() {
	int goal = Actor_Query_Goal_Number(kActorMaggie);
	if (goal == 0 || goal > 9) {
		if (goal == kGoalMaggieKP05WalkToMcCoy) {
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05WillExplode);
			return; //true
		}
	} else {
		if (goal == kGoalMaggieMA02WalkToEntrance) {
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02WalkToMcCoy);
			return; //true
		}

		if (goal == kGoalMaggieMA02Wait) {
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
			AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
			AI_Countdown_Timer_Start(kActorMaggie, kActorTimerAIScriptCustomTask0, Random_Query(1, 5));
			return; //true
		}

		if (goal == 9) {
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
			Actor_Change_Animation_Mode(kActorMaggie, 54);
			return; //true
		}
	}
	Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
	return; //true
}

void AIScriptMaggie::ReceivedClue(int clueId, int fromActorId) {
}

void AIScriptMaggie::ClickedByPlayer() {
	if (!Game_Flag_Query(kFlagMcCoyIsHelpingReplicants)
	 &&  Global_Variable_Query(kVariableChapter) == 5
	) {
		if (Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieKP05WalkToMcCoy) {
			Actor_Set_Targetable(kActorMaggie, true);
			AI_Movement_Track_Flush(kActorMaggie);
			Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
			Actor_Says(kActorMcCoy, 2400, kAnimationModeFeeding);
		}
		return; // true
	}

	if (_animationState == kMaggieStateDead) {
		return; // false
	}

	Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);

	float mccoy_x, mccoy_y, mccoy_z;
	Actor_Query_XYZ(kActorMcCoy, &mccoy_x, &mccoy_y, &mccoy_z);
	if (distanceToActor(kActorMaggie, mccoy_x, mccoy_y, mccoy_z) > 60.0f) {
		Actor_Says(0, 2430, 18);
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02WalkToMcCoy);
		return; // true
	}

	int v6 = Random_Query(0, 4);
	if (v6 == 1) {
		Actor_Says(kActorMcCoy, 2440, 18);
	}
	if (v6 == 0) {
		Actor_Says(kActorMcCoy, 2435, 13);
	}

	int goal = Actor_Query_Goal_Number(kActorMaggie);
	if (goal == kGoalMaggieMA02Wait) {
		if (Random_Query(0, 1)) {
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
			Actor_Change_Animation_Mode(kActorMaggie, 57);
		} else {
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
			Actor_Change_Animation_Mode(kActorMaggie, 56);
		}
		AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
		AI_Countdown_Timer_Start(kActorMaggie, kActorTimerAIScriptCustomTask0, Random_Query(3, 9));
		return; // true
	}

	if (goal == 10) {
		Actor_Change_Animation_Mode(kActorMaggie, kAnimationModeIdle);
		return; // true
	}

	if (goal == 11) {
		Actor_Change_Animation_Mode(kActorMaggie, 54);
		return; // true
	}

	Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
	return; // true
}

void AIScriptMaggie::EnteredScene(int sceneId) {
}

void AIScriptMaggie::OtherAgentEnteredThisScene(int otherActorId) {
}

void AIScriptMaggie::OtherAgentExitedThisScene(int otherActorId) {
	if (otherActorId == kActorMcCoy
	 && Actor_Query_Which_Set_In(kActorMaggie) == kSetMA02_MA04
	 && Global_Variable_Query(kVariableChapter) < 4
	) {
		AI_Movement_Track_Flush(kActorMaggie);
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Default);
	}
}

void AIScriptMaggie::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
}

void AIScriptMaggie::ShotAtAndMissed() {
}

bool AIScriptMaggie::ShotAtAndHit() {
	AI_Movement_Track_Flush(kActorMaggie);
	Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05WillExplode);
	Actor_Set_Targetable(kActorMaggie, false);
	return false;
}

void AIScriptMaggie::Retired(int byActorId) {
}

int AIScriptMaggie::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptMaggie::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (currentGoalNumber == kGoalMaggieDead) {
		return true;
	}

	switch (newGoalNumber) {
	case kGoalMaggieMA02Default:
		Actor_Put_In_Set(kActorMaggie, kSetMA02_MA04);
		Actor_Set_At_Waypoint(kActorMaggie, 265, 780);
		return true;

	case kGoalMaggieMA02WalkToEntrance:
		Actor_Put_In_Set(kActorMaggie, kSetMA02_MA04);
		Actor_Set_At_Waypoint(kActorMaggie, randomWaypointMA02(), 512);
		AI_Movement_Track_Flush(kActorMaggie);
		AI_Movement_Track_Append(kActorMaggie, 264, 0);
		AI_Movement_Track_Repeat(kActorMaggie);
		return true;

	case kGoalMaggieMA02GetFed:
		Player_Loses_Control();
		AI_Movement_Track_Flush(kActorMaggie);
#if BLADERUNNER_ORIGINAL_BUGS
#else
		// Allows McCoy to perform both animated turns (first towards the BAR-MAIN and then towards Maggie)
		// when Maggie is already too close
		// original bug: When Maggie is close McCoy would alternate between
		// - turning to Maggie and throw food at her
		// - only performing the turn toward the BAR-MAIN and "throw" food to wrong direction
		if (Actor_Query_Inch_Distance_From_Actor(kActorMaggie, kActorMcCoy) <= 85) {
			Delay(500);
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		Loop_Actor_Walk_To_Actor(kActorMaggie, kActorMcCoy, 48, false, false);
		Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
		Actor_Face_Actor(kActorMaggie, kActorMcCoy, false);
		Actor_Says(kActorMcCoy, 2400, kAnimationModeFeeding);
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
		Player_Gains_Control();
		return true;

	case kGoalMaggieMA02WalkToMcCoy:
		AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
		AI_Movement_Track_Flush(kActorMaggie);
		Loop_Actor_Walk_To_Actor(kActorMaggie, kActorMcCoy, 30, false, false);
		Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
		Actor_Change_Animation_Mode(kActorMaggie, 56);
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
		return true;

	case kGoalMaggieMA02Wait:
		AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
		AI_Countdown_Timer_Start(kActorMaggie, kActorTimerAIScriptCustomTask0, Random_Query(3, 9));
		return true;

	case 9:
		if (Random_Query(0, 2) <= 0) {
			Actor_Face_Actor(kActorMaggie, kActorMcCoy, false);
			Actor_Change_Animation_Mode(kActorMaggie, 54);
		} else {
			AI_Movement_Track_Flush(kActorMaggie);
			if (Actor_Query_Which_Set_In(kActorMaggie) == kSetMA02_MA04) {
				AI_Movement_Track_Append(kActorMaggie, randomWaypointMA02(), 486);
			}
			AI_Movement_Track_Repeat(kActorMaggie);
		}
		return true;

	case kGoalMaggieMA02SitDown:
		Actor_Change_Animation_Mode(kActorMaggie, 54);
		_animationState = kMaggieStateLayingIdle;
		_animationFrame = 0;
		AI_Countdown_Timer_Reset(kActorMaggie, kActorTimerAIScriptCustomTask0);
		AI_Countdown_Timer_Start(kActorMaggie, kActorTimerAIScriptCustomTask0, Random_Query(2, 9));
		return true;

	case kGoalMaggieMA02Sleep:
		Actor_Change_Animation_Mode(kActorMaggie, 55);
		_animationState = kMaggieStateSleeping;
		_animationFrame = 0;
		return true;

	case 400:
		Actor_Set_Goal_Number(kActorMaggie, 410);
		break;

	case kGoalMaggieKP05Wait:
		AI_Movement_Track_Flush(kActorMaggie);
		Game_Flag_Set(kFlagMaggieHasBomb);
		Actor_Put_In_Set(kActorMaggie, kSetKP05_KP06);
		Actor_Set_At_XYZ(kActorMaggie, -672.0, 0.0, -428.0, 653);
		Actor_Change_Animation_Mode(kActorMaggie, kAnimationModeIdle);
		break;

	case kGoalMaggieKP05McCoyEntred:
		Scene_Exits_Disable();
		Loop_Actor_Walk_To_XYZ(kActorMaggie, -734.0, 0.0, -432.0, 0, false, false, false);
		Actor_Face_Actor(kActorMaggie, kActorMcCoy, true);
		Actor_Change_Animation_Mode(kActorMaggie, 56);
		Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
		Actor_Says(kActorMcCoy, 2225, kAnimationModeTalk);
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05WalkToMcCoy);
		break;

	case kGoalMaggieKP05WalkToMcCoy:
		if (Actor_Query_Is_In_Current_Set(kActorSteele)) {
			Actor_Says(kActorSteele, 3270, 59);
		}
		AI_Movement_Track_Flush(kActorMaggie);
		AI_Movement_Track_Append(kActorMaggie, 540, 0);
		AI_Movement_Track_Repeat(kActorMaggie);
		break;

	case kGoalMaggieKP05Explode:
		AI_Movement_Track_Flush(kActorMaggie);
		Actor_Face_Actor(kActorMcCoy, kActorMaggie, true);
		Sound_Play(kSfxDOGEXPL1, 50, 0, 0, 100);
		Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieDead);
		Actor_Change_Animation_Mode(kActorMaggie, 51);
		if (Actor_Query_Inch_Distance_From_Actor(kActorMcCoy, kActorMaggie) < 144) {
			Player_Loses_Control();
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
			Actor_Retired_Here(kActorMcCoy, 6, 6, 1, -1);
		} else {
			Delay(3000);
			Scene_Exits_Disable();
			Actor_Says(kActorMcCoy, 2235, 12);
			if (Actor_Query_Is_In_Current_Set(kActorSteele)) {
				Actor_Says(kActorSteele, 1530, 58);
				Actor_Set_Goal_Number(kActorSteele, kGoalSteeleKP05Leave);
			}
			Delay(2000);
			Actor_Says(kActorMcCoy, 2390, 13);
			if (Actor_Query_Goal_Number(kActorSadik) == 411) {
				Actor_Set_Goal_Number(kActorSadik, 412);
			} else { // there is no way how Maggie can explode and Sadik's goal is not 411
				Actor_Set_Goal_Number(kActorClovis, kGoalClovisKP06TalkToMcCoy);
			}
		}
		break;

	}
	return false;
}

bool AIScriptMaggie::UpdateAnimation(int *animation, int *frame) {
	int goal;
	switch (_animationState) {
	case kMaggieStateDead:
		*animation = 871;
		_animationFrame = 0;
		break;

	case kMaggieStateDeadExploded:
		*animation = 874;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(874) - 1;
		break;

	case kMaggieStateExploding:
		*animation = 874;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(874) - 1) {
			_animationState = kMaggieStateDeadExploded;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			Actor_Put_In_Set(kActorMaggie, kSetFreeSlotI);
			Actor_Set_At_Waypoint(kActorMaggie, 41, 0);
		}
		break;

	case kMaggieStateBombJumping:
		*animation = 873;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(873)) {
			_animationState = kMaggieStateBombIdle;
			_animationFrame = 0;
			*animation = 875;
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieKP05WillExplode);
		}
		break;

	case kMaggieStateBombWalk:
		*animation = 872;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(872)) {
			_animationFrame = 0;
		}
		break;

	case kMaggieStateBombIdle:
		*animation = 875;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(875)) {
			_animationFrame = 0;
		}
		break;

	case kMaggieStateWakingUp:
		*animation = 876;
		_animationFrame--;
		if (_animationFrame > 0) {
			break;
		}
		_animationState = kMaggieStateLayingIdle;
		_animationFrame = 0;
		*animation = 867;
		goal = Actor_Query_Goal_Number(kActorMaggie);
		if (goal == kGoalMaggieMA02GetFed) {
			_animationState = kMaggieStateStandingUp;
			_animationFrame = 0;
			*animation = 868;
		} else if (goal == kGoalMaggieMA02WalkToMcCoy) {
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02SitDown);
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02WalkToMcCoy);
		} else {
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02SitDown);
		}
		break;

	case kMaggieStateSleeping:
		*animation = 876;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(876) - 1;
		break;

	case kMaggieStateGoingToSleep:
		*animation = 876;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(876) - 1) {
			_animationState = kMaggieStateSleeping;
			Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Sleep);
		}
		break;

	case kMaggieStateStandingUp:
		*animation = 868;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(868)) {
			*animation = 864;
			_animationState = kMaggieStateIdle;
			_animationFrame = 0;
			if (Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieMA02SitDown) {
				Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02Wait);
			} else if (Actor_Query_Goal_Number(kActorMaggie) == kGoalMaggieMA02WalkToMcCoy) {
				Actor_Set_Goal_Number(kActorMaggie, 12); // this is never used
				Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02WalkToMcCoy);
			}
		}
		break;

	case kMaggieStateLayingIdle:
		*animation = 867;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(867)) {
			_animationFrame = 0;
		}
		break;

	case kMaggieStateLayingDown:
		*animation = 866;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(866)) {
			_animationState = kMaggieStateLayingIdle;
			_animationFrame = 0;
			*animation = 867;
			if (Actor_Query_Goal_Number(kActorMaggie) == 9) {
				Actor_Set_Goal_Number(kActorMaggie, kGoalMaggieMA02SitDown);
			}
		}
		break;

	case kMaggieStateHappyB:
		*animation = 865;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(865)) {
			_animationFrame = 0;
			var_45F3F8--;
			if (var_45F3F8 <= 0) {
				Actor_Change_Animation_Mode(kActorMaggie, kAnimationModeIdle);
				*animation = 864;
			}
		}
		break;

	case kMaggieStateHappyA:
		*animation = 870;
		if (_animationFrame == 1) {
			// one of kSfxDOGBARK1, kSfxDOGBARK3
			Sound_Play(Random_Query(kSfxDOGBARK1, kSfxDOGBARK3), 50, 0, 0, 50);
		}
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			var_45F3FC--;
			if (var_45F3FC <= 0) {
				Actor_Change_Animation_Mode(kActorMaggie, kAnimationModeIdle);
				*animation = 864;
				_animationState = kMaggieStateIdle;
			}
			_animationFrame = 0;
		}
		break;

	case kMaggieStateJumping:
		*animation = 869;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(869)) {
			Actor_Change_Animation_Mode(kActorMaggie, kAnimationModeIdle);
			*animation = 864;
			_animationState = kMaggieStateIdle;
			_animationFrame = 0;
		}
		break;

	case kMaggieStateWalking:
		*animation = 863;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(863)) {
			_animationFrame = 0;
		}
		break;

	case kMaggieStateIdle:
		*animation = 864;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(864)) {
			_animationFrame = 0;
		}
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptMaggie::ChangeAnimationMode(int mode) {
	if (mode == kAnimationModeWalk) {
		if (Game_Flag_Query(kFlagMaggieHasBomb)) {
			_animationState = kMaggieStateBombWalk;
			_animationFrame = 0;
		} else {
			_animationState = kMaggieStateWalking;
			_animationFrame = 0;
		}
		return true;
	}
	if (mode == kAnimationModeIdle) {
		if (Game_Flag_Query(kFlagMaggieHasBomb)) {
			_animationState = kMaggieStateBombIdle;
			_animationFrame = kMaggieStateIdle;
		} else {
			switch (_animationState) {
			case kMaggieStateGoingToSleep:
			case kMaggieStateSleeping:
				_animationState = kMaggieStateWakingUp;
				break;
			case kMaggieStateIdle:
				_animationState = kMaggieStateStandingUp;
				_animationFrame = 0;
				break;
			case kMaggieStateLayingDown:
				_animationState = kMaggieStateStandingUp;
				_animationFrame = 0;
				break;
			case kMaggieStateJumping:
			case kMaggieStateStandingUp:
			case kMaggieStateWakingUp:
				break;
			default:
				_animationState = kMaggieStateIdle;
				_animationFrame = 0;
				break;
			}
		}
		return true;
	}

	switch (mode) {
	case 51:
		_animationState = kMaggieStateExploding;
		_animationFrame = 0;
		Sound_Play(kSfxDOGHURT1, 50, 0, 0, 50);
		break;

	case kAnimationModeFeeding:
		if (Game_Flag_Query(kFlagMaggieHasBomb)) {
			_animationState = kMaggieStateBombJumping;
			_animationFrame = 0;
		} else {
			_animationState = kMaggieStateJumping;
			_animationFrame = 0;
		}
		break;

	case 54:
		if (_animationState <= kMaggieStateSleeping) {
			if (_animationState > 0) {
				if (_animationState == kMaggieStateSleeping) {
					_animationState = kMaggieStateWakingUp;
					_animationFrame = 0;
				}
			} else {
				_animationState = kMaggieStateLayingDown;
				_animationFrame = 0;
			}
		}
		break;

	case 55:
		if (_animationState == kMaggieStateLayingIdle) {
			_animationState = kMaggieStateGoingToSleep;
			_animationFrame = 0;
		}
		break;

	case 56:
		if (_animationState != 3) {
			_animationFrame = 0;
			_animationState = kMaggieStateHappyA;
		}
		var_45F3FC = Random_Query(2, 6);
		break;

	case 57:
		if (_animationState != kMaggieStateHappyB) {
			_animationFrame = 0;
			_animationState = kMaggieStateHappyB;
		}
		var_45F3F8 = Random_Query(2, 6);
		Sound_Play(kSfxDOGTAIL1, 50, 0, 0, 50);
		break;

	case 88:
		_animationState = kMaggieStateDead;
		_animationFrame = 0;
		break;

	}
	return true;
}

void AIScriptMaggie::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptMaggie::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptMaggie::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptMaggie::FledCombat() {
}

int AIScriptMaggie::randomWaypointMA02() {
	switch (Random_Query(0, 3)) {
	case 0:
		return 264;

	case 1:
		return 265;

	case 2:
		return 266;

	default:
		return 267;
	}
}

float AIScriptMaggie::distanceToActor(int actorId, float x, float y, float z) {
	float actorX, actorY, actorZ;
	Actor_Query_XYZ(actorId, &actorX, &actorY, &actorZ);
	return sqrt(static_cast<float>((z - actorZ) * (z - actorZ) + (y - actorY) * (y - actorY) + (x - actorX) * (x - actorX)));
}

} // End of namespace BladeRunner
