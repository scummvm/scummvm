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

enum kRunciterStates {
	kRunciterStateIdle         = 0,
	kRunciterStateWalking      = 1,
	kRunciterStateDying        = 14,
	kRunciterStateDead         = 15
};

AIScriptRunciter::AIScriptRunciter(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	// _varChooseIdleAnimation can have valid values: 0, 1, 2
	_varChooseIdleAnimation = 0;
	var_45CD7C = 0;
	var_45CD80 = 0;
	_varNumOfTimesToHoldCurrentFrame = 0;
	var_45CD88 = 0;
}

void AIScriptRunciter::Initialize() {
	_animationState = 0;
	_animationFrame = 0;
	_animationStateNext = 0;
	_varChooseIdleAnimation = 0;
	var_45CD7C = 6;
	var_45CD80 = 1;
	_varNumOfTimesToHoldCurrentFrame = 0;
	var_45CD88 = 0;
	Actor_Set_Goal_Number(kActorRunciter, kGoalRunciterDefault);
}

bool AIScriptRunciter::Update() {
	if (Actor_Query_Goal_Number(kActorRunciter) == kGoalRunciterDefault
	 && Game_Flag_Query(kFlagRC01PoliceDone)
	) {
		Actor_Set_Goal_Number(kActorRunciter, kGoalRunciterGoToFreeSlotGH);
	}

	if (Global_Variable_Query(kVariableChapter) == 4
	 && Actor_Query_Goal_Number(kActorRunciter) < kGoalRunciterRC02Wait
	) {
		Actor_Set_Goal_Number(kActorRunciter, kGoalRunciterRC02Wait);
	}

	return false;
}

void AIScriptRunciter::TimerExpired(int timer) {}

void AIScriptRunciter::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorRunciter) == kGoalRunciterRC02WalkAround) {
		if (Player_Query_Current_Scene() == kSceneRC02) {
			switch (Random_Query(1, 5)) {
			case 2:
				// fall through
			case 3:
				ADQ_Add(kActorRunciter, 530, -1);
				break;

			case 1:
				// fall through
			case 5:
				ADQ_Add(kActorRunciter, 80, -1);
				break;

			case 4:
				ADQ_Add(kActorRunciter, 930, -1);
				break;
			}
		}
		Actor_Set_Goal_Number(kActorRunciter, 99);
		Actor_Set_Goal_Number(kActorRunciter, kGoalRunciterRC02WalkAround);
		//return true;
	}
	//return false;
}

void AIScriptRunciter::ReceivedClue(int clueId, int fromActorId) {}

void AIScriptRunciter::ClickedByPlayer() {}

void AIScriptRunciter::EnteredSet(int setId) {}

void AIScriptRunciter::OtherAgentEnteredThisSet(int otherActorId) {}

void AIScriptRunciter::OtherAgentExitedThisSet(int otherActorId) {}

void AIScriptRunciter::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if ( Actor_Query_Goal_Number(kActorRunciter) == kGoalRunciterRC02Wait
	 &&  combatMode
	 && !Game_Flag_Query(kFlagRC02RunciterTalkWithGun)
	) {
		Actor_Set_Targetable(kActorRunciter, true);
		Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
		Actor_Says(kActorRunciter, 420, 12);
		Actor_Face_Actor(kActorMcCoy, kActorRunciter, true);
		// TODO revisit setting kActorMcCoy to Combat Aim via Actor_Change_Animation_Mode()
		//      (see notes in Gordo AI script in his CompletedMovementTrack())
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatAim);
		if (Actor_Clue_Query(kActorMcCoy, kClueZubensMotive)) {
			Actor_Says(kActorMcCoy, 4770, -1);
			Actor_Says(kActorRunciter, 590, 13);
			Actor_Says(kActorMcCoy, 4775, -1);
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorRunciter, 600, 17);
#else
			// Runciter is interrupted here
			Actor_Says_With_Pause(kActorRunciter, 600, 0.0f, 17);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Sound_Play(kSfxSHOTCOK1, 100, 0, 100, 50);
			Actor_Says(kActorMcCoy, 4780, -1);
			Actor_Says(kActorRunciter, 610, 18);
			Actor_Says(kActorMcCoy, 4785, -1);
			Actor_Says(kActorRunciter, 620, 15);
			if (Game_Flag_Query(kFlagLucyIsReplicant)) {
				Actor_Says(kActorRunciter, 630, 12);
				Actor_Says(kActorRunciter, 640, 17);
				Actor_Says(kActorMcCoy, 4790, -1);
				Actor_Says(kActorRunciter, 650, 18);
				Actor_Says(kActorRunciter, 660, 19);
				Actor_Clue_Acquire(kActorMcCoy, kClueRuncitersConfession1, true, kActorRunciter);
			} else {
#if BLADERUNNER_ORIGINAL_BUGS
				Actor_Says(kActorRunciter, 670, 18);
#else
				// Runciter is interrupted here
				Actor_Says_With_Pause(kActorRunciter, 670, 0.0f, 18);
#endif // BLADERUNNER_ORIGINAL_BUGS
				Actor_Says(kActorMcCoy, 4795, -1);
				Actor_Says(kActorRunciter, 730, 17);
			}
		} else if (Actor_Clue_Query(kActorMcCoy, kClueEnvelope)) {
			Actor_Says(kActorMcCoy, 4730, -1);
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorRunciter, 480, 17);
#else
			// Runciter is kind of interrupted here
			Actor_Says_With_Pause(kActorRunciter, 480, 0.0f, 17);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorMcCoy, 4735, -1);
			Actor_Says(kActorRunciter, 490, 16);
			Sound_Play(kSfxSHOTCOK1, 100, 0, 100, 50);
			Actor_Says(kActorMcCoy, 4740, -1);
			Actor_Says(kActorRunciter, 500, 18);
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorRunciter, 510, 19);
#else
			// Runciter is kind of interrupted here
			Actor_Says_With_Pause(kActorRunciter, 510, 0.0f, 19);
#endif // BLADERUNNER_ORIGINAL_BUGS
			Actor_Says(kActorMcCoy, 4745, -1);
			Actor_Says(kActorMcCoy, 4750, -1);
			Actor_Says(kActorRunciter, 520, 17);
			Actor_Says(kActorRunciter, 530, 18);
			Actor_Says(kActorRunciter, 540, 16);
		}
		Game_Flag_Set(kFlagRC02RunciterTalkWithGun);
	}
}

void AIScriptRunciter::ShotAtAndMissed() {}

bool AIScriptRunciter::ShotAtAndHit() {
	Actor_Set_Targetable(kActorRunciter, false);
	Actor_Change_Animation_Mode(kActorRunciter, kAnimationModeDie);
	Actor_Set_Goal_Number(kActorRunciter, kGoalRunciterDead);
	Delay(2000);
	if (Actor_Clue_Query(kActorMcCoy, kClueZubensMotive)) {
		Actor_Voice_Over(2050, kActorVoiceOver);
		Actor_Voice_Over(2060, kActorVoiceOver);
	} else {
		Actor_Voice_Over(2070, kActorVoiceOver);
		Actor_Voice_Over(2080, kActorVoiceOver);
		Actor_Voice_Over(2090, kActorVoiceOver);
	}
	Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 3);
	return false;
}

void AIScriptRunciter::Retired(int byActorId) {}

int AIScriptRunciter::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptRunciter::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == kGoalRunciterDefault) {
		Actor_Put_In_Set(kActorRunciter, kSetRC02_RC51);
		Actor_Set_At_Waypoint(kActorRunciter, 92, 567);
		return false;
	}

	if (newGoalNumber == kGoalRunciterRC02WalkAround) {
		AI_Movement_Track_Flush(kActorRunciter);
		if (Random_Query(0, 1) == 1) {
			if (Random_Query(0, 1) == 0) {
				AI_Movement_Track_Append_With_Facing(kActorRunciter, 89, Random_Query(6, 10), 567);
			}
			AI_Movement_Track_Append_With_Facing(kActorRunciter, 93, Random_Query(2, 6), 1002);
			AI_Movement_Track_Append(kActorRunciter, 92, 5);
		} else {
			AI_Movement_Track_Append_With_Facing(kActorRunciter, 91, Random_Query(3, 10), 120);
			if (Random_Query(1, 3) == 1) {
				AI_Movement_Track_Append_With_Facing(kActorRunciter, 93, Random_Query(2, 6), 1002);
			}
			AI_Movement_Track_Append_With_Facing(kActorRunciter, 90, Random_Query(5, 10), 170);
		}
		AI_Movement_Track_Repeat(kActorRunciter);
		return true;
	}

	if (newGoalNumber == kGoalRunciterGoToFreeSlotGH) {
		AI_Movement_Track_Flush(kActorRunciter);
		AI_Movement_Track_Append(kActorRunciter, 39, 120);
		AI_Movement_Track_Append(kActorRunciter, 40, 0);
		AI_Movement_Track_Repeat(kActorRunciter);
		return false;
	}

	if (newGoalNumber == kGoalRunciterRC02Wait) {
		Actor_Put_In_Set(kActorRunciter, kSetRC02_RC51);
		Actor_Set_At_Waypoint(kActorRunciter, 93, 1007);
		return false;
	}
	return false;
}

bool AIScriptRunciter::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case kRunciterStateIdle:
		if (_varChooseIdleAnimation == 0) {
			*animation = kModelAnimationRunciterIdle;
			if (_varNumOfTimesToHoldCurrentFrame > 0) {
				--_varNumOfTimesToHoldCurrentFrame;
			} else {
				_animationFrame += var_45CD80;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterIdle)) {
					_animationFrame = 0;
				}
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterIdle) - 1;
				}
				--var_45CD7C;
				if (var_45CD7C == 0) {
					var_45CD80 = 2 * Random_Query(0, 1) - 1;
					var_45CD7C = Random_Query(6, 14);
					_varNumOfTimesToHoldCurrentFrame = Random_Query(0, 4);
				}
				if (_animationFrame == 0) {
					if (Random_Query(0, 1) == 1) {
						_varChooseIdleAnimation = Random_Query(1, 2);
						var_45CD80 = 1;
						_varNumOfTimesToHoldCurrentFrame = 0;
					}
				}
			}
		} else if (_varChooseIdleAnimation == 1) {
			*animation = kModelAnimationRunciterScratchesWoundIdle;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterScratchesWoundIdle)) {
				_animationFrame = 0;
				_varChooseIdleAnimation = 0;
				*animation = kModelAnimationRunciterIdle;
				var_45CD7C = Random_Query(6, 14);
				var_45CD80 = 2 * Random_Query(0, 1) - 1;
			}
		} else if (_varChooseIdleAnimation == 2) {
			*animation = kModelAnimationRunciterPicksNose;
			if (_varNumOfTimesToHoldCurrentFrame > 0) {
				--_varNumOfTimesToHoldCurrentFrame;
			} else {
				_animationFrame += var_45CD80;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
					_varNumOfTimesToHoldCurrentFrame = Random_Query(5, 15);
					var_45CD80 = -1;
				}
				if (_animationFrame <= 0) {
					_animationFrame = 0;
					_varChooseIdleAnimation = 0;
					*animation = kModelAnimationRunciterIdle;
					var_45CD7C = Random_Query(6, 14);
					var_45CD80 = 2 * Random_Query(0, 1) - 1;
				}
			}
		}
		*frame = _animationFrame;
		break;

	case kRunciterStateWalking:
		*animation = kModelAnimationRunciterWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterWalking)) {
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		break;

	case 2:
		*animation = kModelAnimationRunciterCalmTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterCalmTalk)) {
			_animationFrame = 0;
			if (var_45CD88 > 0) {
				*animation = kModelAnimationRunciterIdle;
				_animationState = 0;
				_varChooseIdleAnimation = 0;
			} else {
				_animationState = 4;
			}
		}
		*frame = _animationFrame;
		break;

	case 4:
		*animation = kModelAnimationRunciterSuggestOnTipToesTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterSuggestOnTipToesTalk)) {
			_animationFrame = 0;
			if (var_45CD88 > 0) {
				*animation = kModelAnimationRunciterIdle;
				_animationState = 0;
				_varChooseIdleAnimation = 0;
			} else {
				*animation = kModelAnimationRunciterCalmTalk;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 5:
		*animation = kModelAnimationRunciterExplainTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterExplainTalk)) {
			_animationFrame = 0;
			if (var_45CD88 > 0) {
				*animation = kModelAnimationRunciterIdle;
				_animationState = 0;
				_varChooseIdleAnimation = 0;
			} else {
				*animation = kModelAnimationRunciterCalmTalk;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 6:
		*animation = kModelAnimationRunciterAngryTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterAngryTalk)) {
			_animationFrame = 0;
			if (var_45CD88 > 0) {
				*animation = kModelAnimationRunciterIdle;
				_animationState = 0;
				_varChooseIdleAnimation = 0;
			} else {
				*animation = kModelAnimationRunciterCalmTalk;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 7:
		*animation = kModelAnimationRunciterQuestionTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterQuestionTalk)) {
			_animationFrame = 0;
			if (var_45CD88 > 0) {
				*animation = kModelAnimationRunciterIdle;
				_animationState = 0;
				_varChooseIdleAnimation = 0;
			} else {
				*animation = kModelAnimationRunciterCalmTalk;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 8:
		*animation = kModelAnimationRunciterOffensiveTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterOffensiveTalk)) {
			_animationFrame = 0;
			if (var_45CD88 > 0) {
				*animation = kModelAnimationRunciterIdle;
				_animationState = 0;
				_varChooseIdleAnimation = 0;
			} else {
				*animation = kModelAnimationRunciterCalmTalk;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 9:
		*animation = kModelAnimationRunciterComplainCryTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterComplainCryTalk)) {
			_animationFrame = 0;
			if (var_45CD88 > 0) {
				*animation = kModelAnimationRunciterIdle;
				_animationState = 0;
				_varChooseIdleAnimation = 0;
			} else {
				*animation = kModelAnimationRunciterCalmTalk;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 10:
		*animation = kModelAnimationRunciterDespairTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterDespairTalk)) {
			_animationFrame = 0;
			if (var_45CD88 > 0) {
				*animation = kModelAnimationRunciterIdle;
				_animationState = 0;
				_varChooseIdleAnimation = 0;
			} else {
				*animation = kModelAnimationRunciterCalmTalk;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 11:
		*animation = kModelAnimationRunciterCannotBelieveTalk;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterCannotBelieveTalk)) {
			_animationFrame = 0;
			if (var_45CD88 > 0) {
				*animation = kModelAnimationRunciterIdle;
				_animationState = 0;
				_varChooseIdleAnimation = 0;
			} else {
				*animation = kModelAnimationRunciterCalmTalk;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 12:
		*animation = kModelAnimationRunciterGestureGive;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterGestureGive)) {
			*animation = kModelAnimationRunciterIdle;
			_animationState = 0;
			_animationFrame = 0;
			_varChooseIdleAnimation = 0;
			Actor_Change_Animation_Mode(kActorRunciter, kAnimationModeCombatIdle);
		}
		*frame = _animationFrame;
		break;

	case 13:
		if (_varChooseIdleAnimation == 0) {
			_animationFrame = 0;
			_animationState = _animationStateNext;
			*animation = _animationNext;
		} else if (_varChooseIdleAnimation == 1) {
			*animation = kModelAnimationRunciterScratchesWoundIdle;
			_animationFrame += 3;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterScratchesWoundIdle)) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		} else if (_varChooseIdleAnimation == 2) {
			*animation = kModelAnimationRunciterPicksNose;
			_animationFrame -= 3;
			if (_animationFrame - 3 < 0) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		}
		*frame = _animationFrame;
		break;

	case kRunciterStateDying:
		*animation = kModelAnimationRunciterShotDead;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterShotDead) - 1) {
			*animation = kModelAnimationRunciterShotDead;
			_animationState = kRunciterStateDead;
		}
		*frame = _animationFrame;
		break;

	case kRunciterStateDead:
		*animation = kModelAnimationRunciterShotDead;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationRunciterShotDead) - 1;
		*frame = _animationFrame;
		break;

	default:
		// Dummy placeholder, kModelAnimationZubenWalking (399) is a Zuben animation
		*animation = kModelAnimationZubenWalking;
		_animationFrame = 0;
		*frame = _animationFrame;
		break;
	}
	return true;
}

bool AIScriptRunciter::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (_animationState >= 2 && _animationState <= 11) {
			var_45CD88 = 1;
		} else {
			_animationState = 0;
			_animationFrame = 0;
			_varChooseIdleAnimation = 0;
		}
		break;

	case kAnimationModeWalk:
		if (_animationState > 1) {
			_animationState = 1;
			_animationFrame = 0;
		} else if (_animationState == 0) {
			_animationState = 13;
			_animationStateNext = 1;
			_animationNext = kModelAnimationRunciterWalking;
		}
		break;

	case kAnimationModeTalk:
		if (_animationState  != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 2;
			_animationNext = kModelAnimationRunciterWalking;
		}
		var_45CD88 = 0;
		break;

	case 12:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 4;
			_animationNext = kModelAnimationRunciterSuggestOnTipToesTalk;
		}
		var_45CD88 = 0;
		break;

	case 13:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 5;
			_animationNext = kModelAnimationRunciterExplainTalk;
		}
		var_45CD88 = 0;
		break;

	case 14:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 6;
			_animationNext = kModelAnimationRunciterAngryTalk;
		}
		var_45CD88 = 0;
		break;

	case 15:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 7;
			_animationNext = kModelAnimationRunciterQuestionTalk;
		}
		var_45CD88 = 0;
		break;

	case 16:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 8;
			_animationNext = kModelAnimationRunciterOffensiveTalk;
		}
		var_45CD88 = 0;
		break;

	case 17:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 9;
			_animationNext = kModelAnimationRunciterComplainCryTalk;
		}
		var_45CD88 = 0;
		break;

	case 18:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 10;
			_animationNext = kModelAnimationRunciterDespairTalk;
		}
		var_45CD88 = 0;
		break;

	case 19:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 11;
			_animationNext = kModelAnimationRunciterCannotBelieveTalk;
		}
		var_45CD88 = 0;
		break;

	case 23:
		_animationState = 12;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		_animationState = 14;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptRunciter::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptRunciter::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptRunciter::ReachedMovementTrackWaypoint(int waypointId) {
	switch (waypointId) {
	case 89:
		Actor_Face_Heading(kActorRunciter, 567, true);
		break;

	case 90:
		Actor_Face_Heading(kActorRunciter, 170, true);
		break;

	case 91:
		Actor_Face_Heading(kActorRunciter, 120, true);
		break;

	case 92:
		Actor_Face_Heading(kActorRunciter, 664, true);
		break;

	case 93:
		Actor_Face_Heading(kActorRunciter, 1002, true);
		break;
	}
	return true;
}

void AIScriptRunciter::FledCombat() {}

} // End of namespace BladeRunner
