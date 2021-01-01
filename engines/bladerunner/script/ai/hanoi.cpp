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

AIScriptHanoi::AIScriptHanoi(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_resumeIdleAfterFramesetCompletesFlag = false;
	// _varChooseIdleAnimation can have valid values: 0, 1
	_varChooseIdleAnimation = 0;
	_varNumOfTimesToHoldCurrentFrame = 0;
	_var4 = 1;
}

void AIScriptHanoi::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_resumeIdleAfterFramesetCompletesFlag = false;
	_varChooseIdleAnimation = 0;
	_varNumOfTimesToHoldCurrentFrame = 0;
	_var4 = 1;

	Actor_Set_Goal_Number(kActorHanoi, 0);
}

bool AIScriptHanoi::Update() {
	if (Actor_Query_Goal_Number(kActorHolloway) == kGoalHollowayGoToNR07) {
		AI_Countdown_Timer_Reset(kActorHanoi, kActorTimerAIScriptCustomTask0);
	}

	if (Global_Variable_Query(kVariableChapter) == 3
	 && Actor_Query_Goal_Number(kActorHanoi) < kGoalHanoiDefault
	) {
		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR03GoToDefaultPosition);
	}

	if (Player_Query_Current_Scene() != kSceneNR03
	 && Actor_Query_Goal_Number(kActorHanoi) == kGoalHanoiNR08Left
	) {
		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR03GoToDefaultPosition);
	}

	if (Player_Query_Current_Scene() == kSceneNR03
	 && Actor_Query_Goal_Number(kActorHanoi) != kGoalHanoiNR03StartGuarding
	 && Actor_Query_Goal_Number(kActorHanoi) != kGoalHanoiNR08WatchShow
	 && Actor_Query_Goal_Number(kActorHanoi) != kGoalHanoiNR08Leave
	 && Actor_Query_Goal_Number(kActorHanoi) != kGoalHanoiNR08Left
	) {
		// McCoy close to table swivel
		if (Actor_Query_Inch_Distance_From_Waypoint(kActorMcCoy, 364) < 420) {
			if (Actor_Query_Goal_Number(kActorHanoi) == kGoalHanoiNR03GoToDefaultPosition) {
				Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR03GoToSwivelTable);
			}
		} else if (Actor_Query_Goal_Number(kActorHanoi) == kGoalHanoiNR03GoToSwivelTable) {
			Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR03GoToDefaultPosition);
		}

		// McCoy close to office door
		if (Actor_Query_Inch_Distance_From_Waypoint(kActorMcCoy, 361) < 240) {
			if (Actor_Query_Goal_Number(kActorHanoi) == kGoalHanoiNR03GoToDefaultPosition) {
				Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR03GoToOfficeDoor);
			}
		} else if (Actor_Query_Goal_Number(kActorHanoi) == kGoalHanoiNR03GoToOfficeDoor) {
			Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR03GoToDefaultPosition);
		}

		// McCoy close to dancer
		if (Actor_Query_Inch_Distance_From_Actor(kActorMcCoy, kActorHysteriaPatron1) < 120
		 && Actor_Query_Which_Set_In(kActorHanoi) == kSetNR03
		 && Actor_Query_Goal_Number(kActorHanoi) != kGoalHanoiNR03GoToDancer
		) {
			Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR03GoToDancer);
		}
	}

	return false;
}

void AIScriptHanoi::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask0) {
		if (Actor_Query_Goal_Number(kActorHanoi) == kGoalHanoiNR03StartGuarding) {
			Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR03GoToDefaultPosition);
			return; //true;
		}

		if (Actor_Query_Goal_Number(kActorHanoi) != kGoalHanoiThrowOutMcCoy) {
			Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR07TalkToMcCoy);
			return; //true;
		}
	}
	return; //false;
}

void AIScriptHanoi::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorHanoi)) {
	case kGoalHanoiNR07TalkToMcCoy:
		Actor_Says(kActorHanoi, 130, 3);
		Actor_Says(kActorDektora, 540, 30);
		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR07GrabMcCoy);
		break;

	case kGoalHanoiNR07GrabMcCoy:
		Actor_Face_Actor(kActorHanoi, kActorMcCoy, true);
		Actor_Face_Actor(kActorMcCoy, kActorHanoi, true);
		Actor_Change_Animation_Mode(kActorHanoi, 23);
		Actor_Set_Invisible(kActorMcCoy, true);
		Actor_Says(kActorMcCoy, 3595, kAnimationModeTalk);
		Actor_Says(kActorHanoi, 140, kAnimationModeTalk);
		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiThrowOutMcCoy);
		break;

	case kGoalHanoiNR03GoToDancer:
		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR03GoToDefaultPosition);
		break;

	case kGoalHanoiNR08Leave:
		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR08Left);
		break;

	case kGoalHanoiNR04Enter:
		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR04ShootMcCoy);
		break;

	default:
		return; //false;
	}

	return; //true;
}

void AIScriptHanoi::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptHanoi::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorHanoi) == kGoalHanoiNR08WatchShow
	 || Actor_Query_Goal_Number(kActorHanoi) == kGoalHanoiNR08Leave
	) {
		Actor_Face_Actor(kActorMcCoy, kActorHanoi, true);
		Actor_Says(kActorMcCoy, 8915, 11);

		if (Actor_Query_Goal_Number(kActorHanoi) == kGoalHanoiNR08WatchShow) {
			Actor_Says(kActorHanoi, 210, kAnimationModeTalk);
		}
	}
}

void AIScriptHanoi::EnteredSet(int setId) {
	// return false;
}

void AIScriptHanoi::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptHanoi::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptHanoi::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (Player_Query_Current_Scene() == kSceneNR03
	 && otherActorId == kActorMcCoy
	 && combatMode
	) {
		Player_Set_Combat_Mode(false);
#if BLADERUNNER_ORIGINAL_BUGS
		// redundant call to lose control here
		Player_Loses_Control();
#endif
		Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiThrowOutMcCoy);
		return; //true;
	}
	return; //false;
}

void AIScriptHanoi::ShotAtAndMissed() {
	// return false;
}

bool AIScriptHanoi::ShotAtAndHit() {
	return false;
}

void AIScriptHanoi::Retired(int byActorId) {
	// return false;
}

int AIScriptHanoi::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptHanoi::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == 0) {
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append(kActorHanoi, 39, 0);
		AI_Movement_Track_Repeat(kActorHanoi);

		return true;
	}

	switch (newGoalNumber) {
	case kGoalHanoiDefault:
		AI_Countdown_Timer_Start(kActorHanoi, kActorTimerAIScriptCustomTask0, 45);
		break;

	case kGoalHanoiResetTimer:
		AI_Countdown_Timer_Reset(kActorHanoi, kActorTimerAIScriptCustomTask0);
		break;

	case kGoalHanoiNR07TalkToMcCoy:
		if (Actor_Query_Which_Set_In(kActorMcCoy) == kSetNR07
		 && Actor_Query_In_Set(kActorDektora, kSetNR07)
		) {
			Player_Loses_Control();
			Actor_Put_In_Set(kActorHanoi, kSetNR07);
			Actor_Set_At_XYZ(kActorHanoi, -102.0f, -73.5f, -233.0f, 0);
			Async_Actor_Walk_To_Waypoint(kActorMcCoy, 338, 0, false);
			AI_Movement_Track_Flush(kActorHanoi);
			AI_Movement_Track_Append(kActorHanoi, 336, 1);
			AI_Movement_Track_Repeat(kActorHanoi);
		} else {
			Actor_Set_Goal_Number(kActorHanoi, 0);
		}
		break;

	case kGoalHanoiNR07GrabMcCoy:
		if (Actor_Query_Which_Set_In(kActorMcCoy) != kSetNR07) {
			return false;
		}
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append(kActorHanoi, 337, 0);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case 204:
		Actor_Says(kActorHanoi, 210, kAnimationModeTalk);
		Actor_Change_Animation_Mode(kActorHanoi, 23);
		break;

	case kGoalHanoiNR03GoToDefaultPosition:
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append_With_Facing(kActorHanoi, 362, 0, 300);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case kGoalHanoiNR03GoToSwivelTable:
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append_With_Facing(kActorHanoi, 363, 0, 500);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case kGoalHanoiNR03GoToOfficeDoor:
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append_With_Facing(kActorHanoi, 361, 0, 457);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case kGoalHanoiNR03GoToDancer:
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append_With_Facing(kActorHanoi, 365, Random_Query(15, 20), 600);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case kGoalHanoiNR03StartGuarding:
		Actor_Put_In_Set(kActorHanoi, kSetNR03);
		Actor_Set_At_Waypoint(kActorHanoi, 362, 300);
		AI_Countdown_Timer_Reset(kActorHanoi, kActorTimerAIScriptCustomTask0);
		AI_Countdown_Timer_Start(kActorHanoi, kActorTimerAIScriptCustomTask0, 6);
		break;

	case kGoalHanoiThrowOutMcCoy:
		Game_Flag_Set(kFlagNR03McCoyThrownOut);
		AI_Countdown_Timer_Reset(kActorHanoi, kActorTimerAIScriptCustomTask0);
#if BLADERUNNER_ORIGINAL_BUGS
		Player_Loses_Control();
#else
		// Lose control only if not already lost control (like in the case of Dektora's dressing room NR07 time-out)
		if (Player_Has_Control()) {
			Player_Loses_Control();
		}
#endif
		Player_Set_Combat_Mode(false); // this is missing in ITA and ESP versions of the game
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
		Actor_Set_Invisible(kActorMcCoy, true);
		AI_Movement_Track_Flush(kActorHanoi);
		Actor_Put_In_Set(kActorHanoi, kSetNR01);
		Actor_Set_At_XYZ(kActorHanoi, -444.0f, 24.0f, -845.0f, 512);
		Actor_Change_Animation_Mode(kActorHanoi, 78);
		Set_Enter(kSetNR01, kSceneNR01);
		break;

	case kGoalHanoiNR08WatchShow:
		AI_Movement_Track_Flush(kActorHanoi);
		Actor_Put_In_Set(kActorHanoi, kSetNR05_NR08);
		Actor_Set_At_XYZ(kActorHanoi, -1387.51f, 0.32f, 288.16f, 292);
		break;

	case kGoalHanoiNR08Leave:
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append(kActorHanoi, 439, 0);
		AI_Movement_Track_Append(kActorHanoi, 39, 45);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case kGoalHanoiNR08Left:
		break;

	case kGoalHanoiNR04Enter:
		Actor_Put_In_Set(kActorHanoi, kSetNR04);
		Actor_Set_At_XYZ(kActorHanoi, -47.0f, 0.0f, 334.0f, 535);
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append(kActorHanoi, 549, 0);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case kGoalHanoiNR04ShootMcCoy:
		Actor_Face_Actor(kActorHanoi, kActorMcCoy, true);
		Actor_Change_Animation_Mode(kActorHanoi, kAnimationModeCombatAttack);
		Actor_Retired_Here(kActorMcCoy, 12, 12, true, -1);
		break;

	case 9999:
		AI_Movement_Track_Flush(kActorHanoi);
		break;

	default:
		return false;
	}

	return true;
}

bool AIScriptHanoi::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_varChooseIdleAnimation > 0) {
			*animation = kModelAnimationHanoiScratchesBackIdle;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				*animation = kModelAnimationHanoiLooksAroundIdle;
				_animationFrame = 0;
				_varChooseIdleAnimation = 0;
			}
			break;
		}

		*animation = kModelAnimationHanoiLooksAroundIdle;
		if (_varNumOfTimesToHoldCurrentFrame != 0) {
			--_varNumOfTimesToHoldCurrentFrame;
			if (!Random_Query(0, 6)) {
				_var4 = -_var4;
			}
		} else {
			_animationFrame += _var4;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}

			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
			}

			if (_animationFrame == 5
			 || _animationFrame == 15
			 || _animationFrame == 11
			 || _animationFrame == 0
			) {
				_varNumOfTimesToHoldCurrentFrame = Random_Query(5, 12);
			}

			if (_animationFrame >= 10
			 && _animationFrame <= 13
			) {
				_varNumOfTimesToHoldCurrentFrame = Random_Query(0, 1);
			}

			if (_animationFrame == 0) {
				if (!Random_Query(0, 4)) {
					_varChooseIdleAnimation = 1;
				}
			}
		}
		break;

	case 1:
		if (_varChooseIdleAnimation > 0) {
			*animation = kModelAnimationHanoiScratchesBackIdle;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(*animation) / 2) {
				_animationFrame += 2;
			} else {
				_animationFrame -= 2;
			}

			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(*animation) - 1
			 || _animationFrame <= 0
			) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		} else {
			_animationFrame = 0;
			_animationState = _animationStateNext;
			*animation = _animationNext;
		}
		break;

	case 2:
		*animation = kModelAnimationHanoiGrabsMcCoy;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = kModelAnimationHanoiHoldsMcCoyUp;
		}
		break;

	case 3:
		*animation = kModelAnimationHanoiHoldsMcCoyUp;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case 4:
		*animation = kModelAnimationHanoiHoldsMcCoyUpAndTalks;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = kModelAnimationHanoiHoldsMcCoyUp;
		}
		break;

	case 5:
		*animation = kModelAnimationHanoiGrabsMcCoy;
		--_animationFrame;
		if (_animationFrame == 0) {
			_animationState = 0;
			_animationFrame = 0;
			*animation = kModelAnimationHanoiLooksAroundIdle;

			Actor_Face_Actor(kActorMcCoy, kActorHanoi, true);
			Actor_Set_Invisible(kActorMcCoy, false);

			if (Actor_Query_In_Set(kActorHanoi, kSetNR01)) {
				AI_Movement_Track_Flush(kActorHanoi);
				AI_Movement_Track_Append(kActorHanoi, 350, 0);
				AI_Movement_Track_Append(kActorHanoi, 39, 0);
				AI_Movement_Track_Repeat(kActorHanoi);
			}
		}
		break;

	case 6:
		*animation = kModelAnimationSadikPicksUpAndThrowsMcCoy;  // Sadik is used in this animation, but he is well hidden
		++_animationFrame;
		if (_animationFrame > 26) {
			Actor_Change_Animation_Mode(kActorHanoi, kAnimationModeIdle);
			_animationState = 0;
			_animationFrame = 0;
			*animation = kModelAnimationHanoiLooksAroundIdle;
			Actor_Set_Goal_Number(kActorMcCoy, kGoalMcCoyNR01ThrownOut);
			Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR03GoToDefaultPosition);
		}
		break;

	case 7:
		*animation = kModelAnimationHanoiWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case 8:
		*animation = kModelAnimationHanoiCombatIdle;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case 9:
		*animation = kModelAnimationHanoiCombatKicksDoorIn;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			Actor_Change_Animation_Mode(kActorHanoi, kAnimationModeCombatIdle);
			_animationState = 8;
			_animationFrame = 0;
			*animation = kModelAnimationHanoiCombatIdle;
			Actor_Set_Goal_Number(kActorHanoi, kGoalHanoiNR04ShootMcCoy);
		}
		break;

	case 10:
		*animation = kModelAnimationHanoiCombatFiresGun;
		++_animationFrame;

		if (_animationFrame == 4) {
			Ambient_Sounds_Play_Sound(kSfxSHOTCOK1, 77, 0, 0, 20);
		}

		if (_animationFrame == 6) {
			Ambient_Sounds_Play_Sound(kSfxSHOTGUN1, 97, 0, 0, 20);
		}

		if (_animationFrame == 5) {
			Actor_Force_Stop_Walking(kActorMcCoy);
			Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
		}

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			Actor_Change_Animation_Mode(kActorHanoi, kAnimationModeCombatIdle);
			_animationFrame = 0;
			_animationState = 8;
			*animation = kModelAnimationHanoiCombatIdle;
		}
		break;

	case 11:
		*animation = kModelAnimationHanoiPunchUpAttack;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = kModelAnimationHanoiLooksAroundIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 12:
		*animation = kModelAnimationHanoiGotHitOrViolentHeadNod;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = kModelAnimationHanoiCombatIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 13:
		*animation = kModelAnimationHanoiShotDead;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
			++_animationFrame;
		}
		break;

	case 14:
		*animation = kModelAnimationHanoiCalmTalk;
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			// _resumeIdleAfterFramesetCompletesFlag is never set so it's always false, thus this does not evaluate true
			_animationState = 0;
		} else {
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
			}
		}
		break;

	case 15:
		*animation = kModelAnimationHanoiExplainTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = kModelAnimationHanoiCalmTalk;
		}
		break;

	case 16:
		*animation = kModelAnimationHanoiDownwardsNodTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = kModelAnimationHanoiCalmTalk;
		}
		break;

	case 17:
		*animation = kModelAnimationHanoiDenyTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = kModelAnimationHanoiCalmTalk;
		}
		break;

	case 18:
		*animation = kModelAnimationHanoiSlightBowingTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = kModelAnimationHanoiCalmTalk;
		}
		break;

	case 19:
		*animation = kModelAnimationHanoiLaughTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = kModelAnimationHanoiCalmTalk;
		}
		break;

	case 20:
		*animation = kModelAnimationHanoiMockTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = kModelAnimationHanoiCalmTalk;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptHanoi::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (_animationState == 2
		 || _animationState == 3
		) {
			_animationState = 3;
		} else {
			_animationState = 0;
		}
		_animationFrame = 0;
		break;

	case kAnimationModeWalk:
		_animationState = 7;
		_animationFrame = 0;
		break;

	case kAnimationModeTalk:
		if (_animationState == 3) {
			_animationState = 4;
			_animationFrame = 0;
		} else {
			_animationStateNext = 14;
			_animationNext = kModelAnimationHanoiCalmTalk;
			_animationState = 1;
		}
		break;

	case kAnimationModeCombatIdle:
		_animationState = 8;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatAttack:
		_animationState = 10;
		_animationFrame = 0;
		break;

	case 12:
		_animationStateNext = 15;
		_animationNext = kModelAnimationHanoiExplainTalk;
		_animationState = 1;
		break;

	case 13:
		_animationStateNext = 16;
		_animationNext = kModelAnimationHanoiDownwardsNodTalk;
		_animationState = 1;
		break;

	case 14:
		_animationStateNext = 17;
		_animationNext = kModelAnimationHanoiDenyTalk;
		_animationState = 1;
		break;

	case 15:
		_animationStateNext = 18;
		_animationNext = kModelAnimationHanoiSlightBowingTalk;
		_animationState = 1;
		break;

	case 16:
		// Used when Hanoi says "You're real cute" to McCoy (in NR03), 
		// when McCoy first tries to sit on the rotating couch
#if BLADERUNNER_ORIGINAL_BUGS
		// TODO a bug? uses kModelAnimationHanoiSlightBowingTalk (654) again like case 15
		_animationStateNext = 18;
		_animationNext = kModelAnimationHanoiSlightBowingTalk;
#else
		_animationStateNext = 19;
		_animationNext = kModelAnimationHanoiLaughTalk;
#endif
		_animationState = 1;
		break;

	case 17:
		_animationStateNext = 20;
		_animationNext = kModelAnimationHanoiMockTalk;
		_animationState = 1;
		break;

	case kAnimationModeHit:
		// fall through
	case kAnimationModeCombatHit:
		_animationState = 12;
		_animationFrame = 0;
		break;

	case 23:
		if (_animationState != 3
		 && _animationState != 4
		) {
			Actor_Set_Invisible(kActorMcCoy, true);
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 5;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelAnimationHanoiGrabsMcCoy) - 1;
		}
		break;

	case kAnimationModeDie:
		_animationState = 13;
		_animationFrame = 0;
		break;

	case 71:
		_animationState = 9;
		_animationFrame = 0;
		break;

	case 78:
		_animationState = 6;
		_animationFrame = 16;
		break;
	}

	return true;
}

void AIScriptHanoi::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptHanoi::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptHanoi::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 365) {
		Actor_Face_Actor(kActorHanoi, kActorHysteriaPatron1, true);
	}

	return true;
}

void AIScriptHanoi::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
