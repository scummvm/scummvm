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

AIScriptMoraji::AIScriptMoraji(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 1;
	_varNumOfTimesToHoldCurrentFrame = 0;
}

void AIScriptMoraji::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 1;
	_varNumOfTimesToHoldCurrentFrame = 0;

	Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiDefault);
}

bool AIScriptMoraji::Update() {
	if ( Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiDefault
	 &&  Player_Query_Current_Scene() == kSceneDR05
	 && !Game_Flag_Query(kFlagDR05BombActivated)
	) {
		AI_Countdown_Timer_Reset(kActorMoraji, kActorTimerAIScriptCustomTask2);
		int bombTime = 30; // Original value
		if (_vm->_cutContent && Query_Difficulty_Level() == kGameDifficultyEasy)
			bombTime += 10; // Extend the bomb timer duration when in Dermo Design (where Moraji is chained)
		AI_Countdown_Timer_Start(kActorMoraji, kActorTimerAIScriptCustomTask2, bombTime);
		Game_Flag_Set(kFlagDR05BombActivated);
		return true;
	}

	if (Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiScream) {
		Actor_Says(kActorMoraji, 80, 13);
		_animationState = 9;
		_animationFrame = -1;
		Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiGetUp);
	}

	return false;
}

void AIScriptMoraji::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask2) {
		AI_Countdown_Timer_Reset(kActorMoraji, kActorTimerAIScriptCustomTask2);

		if (Actor_Query_Goal_Number(kActorMoraji) != kGoalMorajiJump
		 && Actor_Query_Goal_Number(kActorMoraji) != kGoalMorajiLayDown
		 && Actor_Query_Goal_Number(kActorMoraji) != kGoalMorajiPerished
		) {
			Game_Flag_Set(kFlagDR05BombWillExplode);
		}
		return; //true;
	}

	return; //false;
}

void AIScriptMoraji::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiRunOut) {
		AI_Countdown_Timer_Reset(kActorMoraji, kActorTimerAIScriptCustomTask2);
		Game_Flag_Set(kFlagDR05BombWillExplode);
		_animationState = 3;

		return; //true;
	}
	return; //false
}

void AIScriptMoraji::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptMoraji::ClickedByPlayer() {
	//return false;
}

void AIScriptMoraji::EnteredSet(int setId) {
	// return false;
}

void AIScriptMoraji::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptMoraji::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptMoraji::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptMoraji::ShotAtAndMissed() {
	// return false;
}

bool AIScriptMoraji::ShotAtAndHit() {
	if (Actor_Query_Goal_Number(kActorMoraji)) {
		if (Actor_Query_Goal_Number(kActorMoraji) == kGoalMorajiLayDown) {
			Game_Flag_Set(kFlagDR04McCoyShotMoraji);
			Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiDie);
			Actor_Set_Goal_Number(kActorOfficerGrayford, kGoalOfficerGrayfordArrivesToDR04);
			return true;
		} else {
			return false;
		}
	} else {
		Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiShot);
		return true;
	}
}

void AIScriptMoraji::Retired(int byActorId) {
	// return false;
}

int AIScriptMoraji::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptMoraji::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalMorajiDefault:
		Actor_Put_In_Set(kActorMoraji, kSetDR05);
		Actor_Set_At_XYZ(kActorMoraji, 50.0f, 0.30f, 35.0f, 414);
		Actor_Set_Targetable(kActorMoraji, true);
		return false;

	case kGoalMorajiShot:
		// applies only when shot inside the Dermo Design Lab
		Actor_Set_Targetable(kActorMoraji, false);
#if BLADERUNNER_ORIGINAL_BUGS
		Sound_Play(kSfxFEMHURT2, 100, 0, 0, 50);	// Original code has female scream here (FEMHURT2)
#else
		Sound_Play_Speech_Line(kActorMoraji, 9020, 50, 0, 50); // fix: Use Moraji's death SPCHSFX, also lower volume
#endif // BLADERUNNER_ORIGINAL_BUGS
		_animationState = 10;
		_animationFrame = 0;
		Actor_Retired_Here(kActorMoraji, 60, 16, true, -1);
		return true;

	case kGoalMorajiFreed:
		Actor_Set_Targetable(kActorMoraji, false);
		_animationState = 8;
		_animationFrame = 1;
		return true;

	case kGoalMorajiRunOut:
		_animationState = 3;
		_animationFrame = 0;
		AI_Movement_Track_Flush(kActorMoraji);
		AI_Movement_Track_Append_Run(kActorMoraji, 95, 0);
		AI_Movement_Track_Append_Run(kActorMoraji, 96, 0);
		AI_Movement_Track_Append_Run(kActorMoraji, 97, 0);
		AI_Movement_Track_Append_Run(kActorMoraji, 98, 0);
		AI_Movement_Track_Repeat(kActorMoraji);
		return true;

	case kGoalMorajiJump:
		_animationState = 11;
		return true;

	case kGoalMorajiLayDown:
		Actor_Retired_Here(kActorMoraji, 60, 16, 0, -1);
		Actor_Set_Targetable(kActorMoraji, true);
		return true;

	case kGoalMorajiDie:
		// Added check here to have Moraji death speech SFX
		// when shot by McCoy outside the Dermo Design Lab
		if (_vm->_cutContent && Game_Flag_Query(kFlagDR04McCoyShotMoraji)) {
			// original code used no voice here
			Sound_Play_Speech_Line(kActorMoraji, 9020, 50, 0, 50); // Use Moraji's death SPCHSFX, also lower volume
		}
		_animationFrame = -1;
		_animationState = 13;
		return true;

	case kGoalMorajiDead:
		Actor_Set_Targetable(kActorMoraji, false);
		_animationState = 14;
		Actor_Retired_Here(kActorMoraji, 60, 16, 1, -1);
		return true;
		break;

	case kGoalMorajiChooseFate:
		if (Player_Query_Current_Scene() == kSceneDR05) {
			Game_Flag_Set(kFlagDR05ViewExplosion);
			Set_Enter(kSetDR01_DR02_DR04, kSceneDR04);
		} else {
			if (Actor_Query_In_Set(kActorMoraji, kSetDR05)) {
				Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiPerished);
			} else {
				Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiJump);
			}
			Game_Flag_Set(kFlagDR05JustExploded);
		}
		return true;

	case kGoalMorajiPerished:
		AI_Movement_Track_Flush(kActorMoraji);
		AI_Movement_Track_Append(kActorMoraji, 41, 0);
		AI_Movement_Track_Repeat(kActorMoraji);
		return true;
	}

	return false;
}

bool AIScriptMoraji::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = kModelAnimationMorajiCuffedIdle;
		if (_varNumOfTimesToHoldCurrentFrame > 0) {
			--_varNumOfTimesToHoldCurrentFrame;
		} else {
			_animationFrame += _var1;
			if (Random_Query(0, 10) == 0) {
				_var1 = -_var1;
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			}
			_varNumOfTimesToHoldCurrentFrame = Random_Query(0, 1);
		}
		break;

	case 1:
		_animationFrame = 0;
		*animation = _animationNext;
		_animationState = _animationStateNext;
		break;

	case 2:
		// Dummy placeholder, kModelAnimationIzoWalking (290) is an Izo animation
		*animation = kModelAnimationIzoWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationIzoWalking))
			_animationFrame = 0;
		break;

	case 3:
		*animation = kModelAnimationMorajiRunning;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMorajiRunning))
			_animationFrame = 0;
		break;

	case 4:
		break;

	case 5:
		*animation = kModelAnimationMorajiCuffedFastTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMorajiCuffedFastTalk)) {
			_animationFrame = 0;
			_animationState = Random_Query(0, 2) + 5;
		}
		break;

	case 6:
		*animation = kModelAnimationMorajiCuffedMoreFastTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMorajiCuffedMoreFastTalk)) {
			_animationFrame = 0;
			_animationState = 5;
			*animation = kModelAnimationMorajiCuffedMoreFastTalk;
		}
		break;

	case 7:
		*animation = kModelAnimationMorajiCuffedPointingTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMorajiCuffedPointingTalk)) {
			_animationFrame = 0;
			_animationState = 5;
			*animation = kModelAnimationMorajiCuffedPointingTalk;
		}
		break;

	case 8:
		*animation = kModelAnimationMorajiCuffedSomethingExplodingToHisLeft;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMorajiCuffedSomethingExplodingToHisLeft)) {
			_animationFrame = 0;
			_animationState = 0;
			*animation = kModelAnimationMorajiCuffedIdle;
			Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiScream);
		}
		if (_animationFrame == 6) {
			Ambient_Sounds_Play_Sound(kSfxCHAINBRK, 69, 0, 0, 20);
		}
		break;

	case 9:
		*animation = kModelAnimationMorajiSittingGetsUp;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMorajiSittingGetsUp)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = kModelAnimationMorajiRunning;
			Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiRunOut);
		}
		break;

	case 10:
		*animation = kModelAnimationMorajiCuffedShotDead;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(kModelAnimationMorajiCuffedShotDead) - 1) {
			++_animationFrame;
		}
		break;

	case 11:
		*animation = kModelAnimationMorajiRunningDivesForward;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMorajiRunningDivesForward)) {
			Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiLayDown);
			_animationFrame = 0;
			_animationState = 12;
			*animation = kModelAnimationMorajiLayingForwardTalk;
		}
		break;

	case 12:
		*animation = kModelAnimationMorajiLayingForwardTalk;
		if (_varNumOfTimesToHoldCurrentFrame > 0) {
			--_varNumOfTimesToHoldCurrentFrame;
		} else {
			_animationFrame += _var1;
			if (!Random_Query(0, 5)) {
				_var1 = -_var1;
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			}
			_varNumOfTimesToHoldCurrentFrame = Random_Query(0, 2);
		}
		break;

	case 13:
		*animation = kModelAnimationMorajiLayingForwardDies;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationMorajiLayingForwardDies) - 1) {
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			Actor_Set_Goal_Number(kActorMoraji, kGoalMorajiDead);
		}
		break;

	case 14:
		*animation = kModelAnimationMorajiLayingForwardDies;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationMorajiLayingForwardDies) - 1;
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptMoraji::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (_animationState != 12
		 && Actor_Query_Goal_Number(kActorMoraji) != 11
		) {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeWalk:
		_animationState = 2;
		_animationFrame = 0;
		break;

	case kAnimationModeRun:
		if (_animationState == 0) {
			_animationState = 1;
			_animationStateNext = 3;
			_animationNext = kModelAnimationMorajiRunning;
		} else if (_animationState != 3) {
			_animationState = 3;
			_animationFrame = 0;
		}
		break;

	case 3:
		if (_animationState != 12) {
			if (_animationState == 0) {
				_animationState = 1;
				_animationStateNext = 5;
				_animationNext = 734;
			} else {
				_animationState = 5;
				_animationFrame = 0;
			}
		}
		break;

	case 12:
		if (_animationState == 0) {
			_animationState = 1;
			_animationStateNext = 6;
			_animationNext = kModelAnimationMorajiCuffedMoreFastTalk;
		} else {
			_animationState = 6;
			_animationFrame = 0;
		}
		break;

	case 13:
		if (_animationState == 0) {
			_animationState = 1;
			_animationStateNext = 7;
			_animationNext = kModelAnimationMorajiCuffedPointingTalk;
		} else {
			_animationState = 7;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeDie:
		_animationState = 13;
		_animationFrame = -1;
		break;
	}

	return true;
}

void AIScriptMoraji::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptMoraji::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptMoraji::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 96)
		AI_Countdown_Timer_Reset(kActorMoraji, kActorTimerAIScriptCustomTask2);

	return true;
}

void AIScriptMoraji::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
