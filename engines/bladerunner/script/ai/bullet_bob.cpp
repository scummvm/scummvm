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

AIScriptBulletBob::AIScriptBulletBob(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	// _varChooseIdleAnimation can have valid values: 0, 1
	_varChooseIdleAnimation = 0;
	_var2 = 6;
	_var3 = 1;
	_varNumOfTimesToHoldCurrentFrame = 0;
}

void AIScriptBulletBob::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_varChooseIdleAnimation = 0;
	_var2 = 6;
	_var3 = 1;
	_varNumOfTimesToHoldCurrentFrame = 0;

	Actor_Set_Goal_Number(kActorBulletBob, kGoalBulletBobDefault);
	Actor_Set_Targetable(kActorBulletBob, true);
}

bool AIScriptBulletBob::Update() {
	if (Game_Flag_Query(kFlagRC04McCoyShotBob)
	 && Actor_Query_Goal_Number(kActorBulletBob) != kGoalBulletBobDead
	) {
		Actor_Set_Goal_Number(kActorBulletBob, kGoalBulletBobDead);
	}

	if ( Player_Query_Combat_Mode()
	 &&  Player_Query_Current_Scene() == kSceneRC04
	 && !Game_Flag_Query(kFlagRC04McCoyCombatMode)
	 &&  Global_Variable_Query(kVariableChapter) < 4
	) {
		AI_Countdown_Timer_Reset(kActorBulletBob, kActorTimerAIScriptCustomTask2);
		AI_Countdown_Timer_Start(kActorBulletBob, kActorTimerAIScriptCustomTask2, 10);
		Actor_Set_Goal_Number(kActorBulletBob, kGoalBulletBobWarningMcCoy);
		Actor_Modify_Friendliness_To_Other(kActorBulletBob, kActorMcCoy, -15);
		Game_Flag_Set(kFlagRC04McCoyCombatMode);
	} else if ( Actor_Query_Goal_Number(kActorBulletBob) == kGoalBulletBobWarningMcCoy
	        && !Player_Query_Combat_Mode()
	) {
		AI_Countdown_Timer_Reset(kActorBulletBob, kActorTimerAIScriptCustomTask2);
		Game_Flag_Reset(kFlagRC04McCoyCombatMode);
		Game_Flag_Set(kFlagRC04McCoyWarned);
		Actor_Set_Goal_Number(kActorBulletBob, kGoalBulletBobDefault);
	}

	if ( Actor_Query_Goal_Number(kActorBulletBob) == kGoalBulletBobShootMcCoy
	 && !Game_Flag_Query(kFlagRC04BobShootMcCoy)
	 &&  _animationState == 0
	) {
		Actor_Face_Heading(kActorBulletBob, 208, false);
		_animationFrame = 0;
		_animationState = 2;
		Actor_Set_Goal_Number(kActorBulletBob, kGoalBulletBobWillShotMcCoy);
		Game_Flag_Set(kFlagRC04BobShootMcCoy);
		return true;
	}

	if (Game_Flag_Query(kFlagRC04McCoyWarned)
	 && Player_Query_Combat_Mode()
	 && Actor_Query_Goal_Number(kActorBulletBob) != kGoalBulletBobDead
	) {
		Actor_Set_Goal_Number(kActorBulletBob, kGoalBulletBobShootMcCoy);
		return true;
	}

	return false;
}

void AIScriptBulletBob::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask2
	 && Actor_Query_Goal_Number(kActorBulletBob) == kGoalBulletBobWarningMcCoy
	) {
		Actor_Set_Goal_Number(kActorBulletBob, kGoalBulletBobShootMcCoy);
		AI_Countdown_Timer_Reset(kActorBulletBob, kActorTimerAIScriptCustomTask2);
		return; //true;
	}
	return; //false;
}

void AIScriptBulletBob::CompletedMovementTrack() {
	//return false;
}

void AIScriptBulletBob::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptBulletBob::ClickedByPlayer() {
	//return false;
}

void AIScriptBulletBob::EnteredSet(int setId) {
	// return false;
}

void AIScriptBulletBob::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptBulletBob::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptBulletBob::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptBulletBob::ShotAtAndMissed() {
	// return false;
}

bool AIScriptBulletBob::ShotAtAndHit() {
	Global_Variable_Increment(kVariableBobShot, 1);
	if (Global_Variable_Query(kVariableBobShot) > 0) {
		Actor_Set_Targetable(kActorBulletBob, false);
		Actor_Set_Goal_Number(kActorBulletBob, kGoalBulletBobGone);
		_animationFrame = 0;
 		_animationState = 3;
		Ambient_Sounds_Play_Speech_Sound(kActorGordo, 9000, 100, 0, 0, 0); // not a typo, it's really from Gordo
		Actor_Face_Heading(kActorBulletBob, 281, false);
	}

	return false;
}

void AIScriptBulletBob::Retired(int byActorId) {
	// return false;
}

int AIScriptBulletBob::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptBulletBob::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == kGoalBulletBobDefault
	 && Game_Flag_Query(kFlagRC04McCoyWarned)
	 && Player_Query_Current_Scene() == kSceneRC04
	) {
		Actor_Says(kActorBulletBob, 140, 16);
		return true;
	}

	if ( newGoalNumber == kGoalBulletBobWarningMcCoy
	 && !Game_Flag_Query(kFlagRC04McCoyWarned)
	 &&  Player_Query_Current_Scene() == kSceneRC04
	) {
		Actor_Says(kActorBulletBob, 120, 37);
		Actor_Says(kActorMcCoy, 4915, 13);
		return true;
	}

	if ( newGoalNumber == kGoalBulletBobDead
	 && !Actor_Clue_Query(kActorMcCoy, kClueVKBobGorskyReplicant)
	) {
		Delay(2000);
		Actor_Voice_Over(2100, kActorVoiceOver);
		Actor_Voice_Over(2110, kActorVoiceOver);
		Actor_Voice_Over(2120, kActorVoiceOver);
		Actor_Voice_Over(2130, kActorVoiceOver);
		return true;
	}

	if (newGoalNumber == kGoalBulletBobShotMcCoy) {
		Scene_Exits_Disable();
		Actor_Force_Stop_Walking(kActorMcCoy);
		Ambient_Sounds_Play_Speech_Sound(kActorMcCoy, 9900, 100, 0, 0, 0);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
		Actor_Retired_Here(kActorMcCoy, 6, 6, 1, -1);
		Scene_Exits_Enable();
		return true;
	}

	return false;
}

bool AIScriptBulletBob::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_varChooseIdleAnimation == 1) {
#if BLADERUNNER_ORIGINAL_BUGS
			// TODO a bug? Why use kModelAnimationBulletBobSittingHeadMoveDownThink (516) here 
			//             and leave kModelAnimationBulletBobSittingHeadMoveAround (515) unused?
			//             Also below kModelAnimationBulletBobSittingHeadMoveAround (515) is used 
			//             in Slice_Animation_Query_Number_Of_Frames(),
			//             even though the count of frames is identical in both 515 and 516 framesets
			*animation = kModelAnimationBulletBobSittingHeadMoveDownThink;
#else
			*animation = kModelAnimationBulletBobSittingHeadMoveAround;
#endif
			if (_varNumOfTimesToHoldCurrentFrame > 0) {
				--_varNumOfTimesToHoldCurrentFrame;
			} else {
				if (++_animationFrame == 6) {
					_varNumOfTimesToHoldCurrentFrame = Random_Query(4, 8);
				}
				if (_animationFrame == 11) {
					_varNumOfTimesToHoldCurrentFrame = Random_Query(2, 6);
				}
				// frames in frameset kModelAnimationBulletBobSittingHeadMoveAround    (515): 16
				// frames in frameset kModelAnimationBulletBobSittingHeadMoveDownThink (kModelAnimationBulletBobSittingHeadMoveDownThink): 16
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingHeadMoveAround)) {
					_animationFrame = 0;
					_varChooseIdleAnimation = 0;
					_var3 = 2 * Random_Query(0, 1) - 1;
					_var2 = Random_Query(3, 7);
					_varNumOfTimesToHoldCurrentFrame = Random_Query(0, 4);
				}
			}
		} else if (_varChooseIdleAnimation == 0) {
			*animation = kModelAnimationBulletBobSittingIdle;
			if (_varNumOfTimesToHoldCurrentFrame > 0) {
				--_varNumOfTimesToHoldCurrentFrame;
			} else {
				_animationFrame += _var3;
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingIdle) - 1;
				} else if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingIdle)) {
					_animationFrame = 0;
				}
				if (!--_var2) {
					_var3 = 2 * Random_Query(0, 1) - 1;
					_var2 = Random_Query(3, 7);
					_varNumOfTimesToHoldCurrentFrame = Random_Query(0, 4);
				}
				if (_animationFrame == 0) {
					_varChooseIdleAnimation = Random_Query(0, 1);
				}
			}
		}
		break;

	case 1:
		*animation = kModelAnimationBulletBobSittingCombatIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingCombatIdle)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationBulletBobSittingCombatFiresGun;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingCombatFiresGun)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = kModelAnimationBulletBobSittingCombatIdle;
		}
		if (_animationFrame == 10) {
			Sound_Play(kSfxSHOTCOK1, 75, 0, 0, 50);
		}
		if (_animationFrame == 5) {
			Sound_Play(kSfxSHOTGUN1, 90, 0, 0, 50);
			Actor_Set_Goal_Number(kActorBulletBob, kGoalBulletBobShotMcCoy);
		}
		break;

	case 3:
		*animation = kModelAnimationBulletBobSittingCombatShotDead;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingCombatShotDead) - 1) {
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingCombatShotDead) - 1;
			_animationState = 16;
			Game_Flag_Set(kFlagRC04McCoyShotBob);
		}
		break;

	case 4:
		break;

	case 5:
		*animation = kModelAnimationBulletBobSittingThumbsUp;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingThumbsUp)) {
			*animation = kModelAnimationBulletBobSittingIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 6:
		*animation = kModelAnimationBulletBobSittingCalmTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingCalmTalk)) {
			_animationFrame = 0;
		}
		break;

	case 7:
		*animation = kModelAnimationBulletBobSittingDismissiveTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingDismissiveTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationBulletBobSittingCalmTalk;
		}
		break;

	case 8:
		*animation = kModelAnimationBulletBobSittingSuggestTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingSuggestTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationBulletBobSittingCalmTalk;
		}
		break;

	case 9:
		*animation = kModelAnimationBulletBobSittingQuickSuggestTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingQuickSuggestTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationBulletBobSittingCalmTalk;
		}
		break;

	case 10:
		*animation = kModelAnimationBulletBobSittingExplainTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingExplainTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationBulletBobSittingCalmTalk;
		}
		break;

	case 11:
		*animation = kModelAnimationBulletBobSittingGossipTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingGossipTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationBulletBobSittingCalmTalk;
		}
		break;

	case 12:
		*animation = kModelAnimationBulletBobSittingHeadNodLeftTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingHeadNodLeftTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationBulletBobSittingCalmTalk;
		}
		break;

	case 13:
		*animation = kModelAnimationBulletBobSittingPersistentTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingPersistentTalk)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = kModelAnimationBulletBobSittingCalmTalk;
		}
		break;

	case 14:
		*animation = kModelAnimationBulletBobSittingCombatRetrievesGun;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingCombatRetrievesGun)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = kModelAnimationBulletBobSittingCombatIdle;
		}
		break;

	case 15:
		if (_varChooseIdleAnimation == 1) {
			*animation = kModelAnimationBulletBobSittingHeadMoveDownThink;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingHeadMoveDownThink)) {
				_animationFrame += 2;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingHeadMoveDownThink)) {
					_animationFrame = 0;
					*animation = _animationNext;
					_animationState = _animationStateNext;
				}
			} else {
				_animationFrame -= 2;
				if (_animationFrame <= 0) {
					_animationFrame = 0;
					*animation = _animationNext;
					_animationState = _animationStateNext;
				}
			}
		} else if (_varChooseIdleAnimation == 0) {
			*animation = kModelAnimationBulletBobSittingIdle;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingIdle)) {
				_animationFrame += 2;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingIdle)) {
					_animationFrame = 0;
					*animation = _animationNext;
					_animationState = _animationStateNext;
				}
			} else {
				_animationFrame -= 2;
				if (_animationFrame <= 0) {
					_animationFrame = 0;
					*animation = _animationNext;
					_animationState = _animationStateNext;
				}
			}
		}
		break;

	case 16:
		*animation = kModelAnimationBulletBobSittingCombatShotDead;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingCombatShotDead) - 1;
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptBulletBob::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (_animationState > 4 || _animationState) {
			// TODO "|| _animationState" part of the clause does not make sense (makes the first part redundant). A bug?
			_animationState = 0;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeTalk:
		// fall through
	case 9:
		// fall through
	case 30:
		if (_animationState < 6 || _animationState > 13) {
			_animationState = 15;
			_animationStateNext = 6;
			_animationNext = kModelAnimationBulletBobSittingCalmTalk;
		}
		break;

	case kAnimationModeCombatIdle:
		if (_animationState <= 4 && _animationState == 0) {
			_animationState = 14;
			_animationFrame = 0;
		}
		break;

	case kAnimationModeCombatAttack:
		_animationState = 2;
		_animationFrame = 0;
		break;

	case 10:
		// fall through
	case 31:
		if (_animationState < 6 || _animationState > 13) {
			_animationState = 15;
			_animationStateNext = 7;
			_animationNext = kModelAnimationBulletBobSittingDismissiveTalk;
		}
		break;

	case 11:
		// fall through
	case 33:
		if (_animationState < 6 || _animationState > 13) {
			_animationState = 15;
			_animationStateNext = 9;
			_animationNext = kModelAnimationBulletBobSittingQuickSuggestTalk;
		}
		break;

	case kAnimationModeHit:
		// fall through
	case kAnimationModeCombatHit:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 23:
		_animationState = 5;
		_animationFrame = 0;
		break;

	case 32:
		if (_animationState < 6 || _animationState > 13) {
			_animationState = 15;
			_animationStateNext = 8;
			_animationNext = kModelAnimationBulletBobSittingSuggestTalk;
		}
		break;

	case 34:
		if (_animationState < 6 || _animationState > 13) {
			_animationState = 15;
			_animationStateNext = 10;
			_animationNext = kModelAnimationBulletBobSittingExplainTalk;
		}
		break;

	case 35:
		if (_animationState < 6 || _animationState > 13) {
			_animationState = 15;
			_animationStateNext = 11;
			_animationNext = kModelAnimationBulletBobSittingGossipTalk;
		}
		break;

	case 36:
		if (_animationState < 6 || _animationState > 13) {
			_animationState = 15;
			_animationStateNext = 12;
			_animationNext = kModelAnimationBulletBobSittingHeadNodLeftTalk;
		}
		break;

	case 37:
		if (_animationState < 6 || _animationState > 13) {
			_animationState = 15;
			_animationStateNext = 13;
			_animationNext = kModelAnimationBulletBobSittingPersistentTalk;
		}
		break;

	case kAnimationModeDie:
		_animationState = 4;
		_animationFrame = 0;
		break;

	case 88:
		_animationState = 16;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationBulletBobSittingCombatShotDead) - 1;
		break;

	default:
		_animationState = 0;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptBulletBob::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptBulletBob::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptBulletBob::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptBulletBob::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
