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

AIScriptKlein::AIScriptKlein(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptKlein::Initialize() {
	_animationState = 0;
	_animationFrame = 0;
	_animationStateNext = 0;
	Actor_Put_In_Set(kActorKlein, kSetPS07);
	Actor_Set_At_XYZ(kActorKlein, 338.0f, 0.22f, -612.0f, 768);
	Actor_Set_Goal_Number(kActorKlein, kGoalKleinDefault);
}

bool AIScriptKlein::Update() {
	if (_vm->_cutContent) {
		if (Global_Variable_Query(kVariableChapter) > 3 && Actor_Query_Goal_Number(kActorKlein) < kGoalKleinAwayAtEndOfActThree) {
			Actor_Set_Goal_Number(kActorKlein, kGoalKleinAwayAtEndOfActThree);
			return true;
		}
	} else {
		// original behavior - Klein disappears after Act 1
		if (Global_Variable_Query(kVariableChapter) > 1 && Actor_Query_Goal_Number(kActorKlein) < kGoalKleinAwayAtEndOfActOne) {
			Actor_Set_Goal_Number(kActorKlein, kGoalKleinAwayAtEndOfActOne);
			return true;
		}
	}

	if (Actor_Query_Goal_Number(kActorKlein) < kGoalKleinIsAnnoyedByMcCoyInit
	    && Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) < 40
	) {
		Actor_Set_Goal_Number(kActorKlein, kGoalKleinIsAnnoyedByMcCoyInit);
	}

	if (Player_Query_Current_Scene() == kScenePS07 && Actor_Query_Goal_Number(kActorKlein) == kGoalKleinDefault) {
		Actor_Set_Goal_Number(kActorKlein, kGoalKleinMovingInLab01);
		return true;
	}

	if (!_vm->_cutContent) {
		// Original behavior:
		// The following if-clauses and flags circumvent the manual's explicit instruction
		// that McCoy should upload his clues on the Mainframe, so that Dino Klein can acquire them.
		if (Actor_Clue_Query(kActorMcCoy, kClueOfficersStatement)
		    && !Game_Flag_Query(kFlagMcCoyHasOfficersStatement)
		) {
			Game_Flag_Set(kFlagMcCoyHasOfficersStatement);
		}
		if (Actor_Clue_Query(kActorMcCoy, kCluePaintTransfer)
		    && !Game_Flag_Query(kFlagMcCoyHasPaintTransfer)
		) {
			Game_Flag_Set(kFlagMcCoyHasPaintTransfer);
		}
		if (Actor_Clue_Query(kActorMcCoy, kClueShellCasings)
		    && !Game_Flag_Query(kFlagMcCoyHasShellCasings)
		) {
			Game_Flag_Set(kFlagMcCoyHasShellCasings);
		}
		if (Actor_Clue_Query(kActorMcCoy, kClueChromeDebris)
		    && !Game_Flag_Query(kFlagMcCoyHasChromeDebris)
		) {
			Game_Flag_Set(kFlagMcCoyHasChromeDebris);
		}
	}

	// The following deals with the case that Klein gets annoyed by McCoy
	if (Player_Query_Current_Scene() == kScenePS07
	    && ((_vm->_cutContent && Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) < 40)
	        || (!_vm->_cutContent && Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) < 35) )
	    && !Game_Flag_Query(kFlagPS07KleinInsulted)
	) {
		// kActorTimerAIScriptCustomTask2 causes the "Klein is annoyed dialogue" to occur after a few seconds
		AI_Countdown_Timer_Reset(kActorKlein, kActorTimerAIScriptCustomTask2);
		if (_vm->_cutContent) {
			// original's 5 seconds is too slow. Reduce it to 2 seconds
			Actor_Set_Goal_Number(kActorKlein, kGoalKleinIsAnnoyedByMcCoyPreInit);
			AI_Countdown_Timer_Start(kActorKlein, kActorTimerAIScriptCustomTask2, 2);
		} else {
			AI_Countdown_Timer_Start(kActorKlein, kActorTimerAIScriptCustomTask2, 5);
		}
		Game_Flag_Set(kFlagPS07KleinInsulted);
		return true;
	}

	// The following deals with how Klein recovers from being annoyed at McCoy
	if (Actor_Query_Goal_Number(kActorKlein) == kGoalKleinIsAnnoyedByMcCoyFinal) {
		if (Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) > 20
		    && Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) < 40
		) {
			// when insulted, slowly increase friendliness again, until it's at 40 or greater
			Actor_Modify_Friendliness_To_Other(kActorKlein, kActorMcCoy, 2);
#if !BLADERUNNER_ORIGINAL_BUGS
			if (Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) < 40) {
				// if after the increase (+2) it is still lower than 40 then keep being annoyed
				Actor_Set_Goal_Number(kActorKlein, kGoalKleinIsAnnoyedByMcCoyInit);
				return true;
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
		}

#if BLADERUNNER_ORIGINAL_BUGS
		AI_Movement_Track_Flush(kActorKlein);
		Actor_Set_Goal_Number(kActorKlein, kGoalKleinDefault);
#else
		// don't go to Default if Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) <= 20
		// and also reset kFlagPS07KleinInsulted if the friendliness is now above 40
		if (Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) >= 40)
		{
			if (Game_Flag_Query(kFlagPS07KleinInsulted)) {
				Game_Flag_Reset(kFlagPS07KleinInsulted);
				// don't reset the kFlagPS07KleinInsultedTalk
			}
			AI_Movement_Track_Flush(kActorKlein);
			Actor_Set_Goal_Number(kActorKlein, kGoalKleinDefault);
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		return true;
	}
	return false;
}

void AIScriptKlein::TimerExpired(int timer) {
	if (timer == kActorTimerAIScriptCustomTask2) {
#if !BLADERUNNER_ORIGINAL_BUGS
		// This timer expiration was buggy; it would play the short dialogue version
		// even when the timer expires even if McCoy has left the room and is somewhere else
		// The fix is to return when the player is somewhere else
		if (Player_Query_Current_Set() != kSetPS07
		    || !Actor_Query_Is_In_Current_Set(kActorKlein)
		    || !Game_Flag_Query(kFlagPS07KleinInsulted)
		) {
			if (Actor_Query_Goal_Number(kActorKlein) == kGoalKleinIsAnnoyedByMcCoyPreInit) {
				Actor_Set_Goal_Number(kActorKlein, kGoalKleinDefault);
			}
			return;
		}
		AI_Movement_Track_Flush(kActorKlein);
#endif
		if (!Game_Flag_Query(kFlagPS07KleinInsultedTalk)
#if BLADERUNNER_ORIGINAL_BUGS
			// this is redundant now because we return in the added code above if Klein is not insulted
			// (and the flag now gets reset when Klein calms down)
		    && Game_Flag_Query(kFlagPS07KleinInsulted)
			// this is redundant now because we return in the added code above if Klein is not in the current set
		    && Actor_Query_Is_In_Current_Set(kActorKlein)
#endif
		) {
			// Klein is annoyed - full dialogue
			Actor_Face_Actor(kActorKlein, kActorMcCoy, true);
			Actor_Says(kActorKlein, 10, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 4120, kAnimationModeTalk);
			Actor_Says(kActorKlein, 20, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 4125, kAnimationModeTalk);
			Game_Flag_Set(kFlagPS07KleinInsultedTalk);
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_Goal_Number(kActorKlein, kGoalKleinIsAnnoyedByMcCoyInit);
#else
			if (Actor_Query_Goal_Number(kActorKlein) != kGoalKleinIsAnnoyedByMcCoyInit) {
				Actor_Set_Goal_Number(kActorKlein, kGoalKleinIsAnnoyedByMcCoyInit);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
		} else {
			// Klein is annoyed - short dialogue
			Actor_Says(kActorKlein, 10, kAnimationModeTalk);
#if BLADERUNNER_ORIGINAL_BUGS
			Actor_Set_Goal_Number(kActorKlein, kGoalKleinIsAnnoyedByMcCoyInit);
#else
			if (Actor_Query_Goal_Number(kActorKlein) != kGoalKleinIsAnnoyedByMcCoyInit) {
				Actor_Set_Goal_Number(kActorKlein, kGoalKleinIsAnnoyedByMcCoyInit);
			}
#endif // BLADERUNNER_ORIGINAL_BUGS
		}
		// return true;
	}
	//return false;
}

void AIScriptKlein::CompletedMovementTrack() {
	// Normal behavior
	if (Actor_Query_Goal_Number(kActorKlein) == kGoalKleinMovingInLab01) {
		Actor_Set_Goal_Number(kActorKlein, kGoalKleinMovingInLab02);
		return; // true;
	}
	if (Actor_Query_Goal_Number(kActorKlein) == kGoalKleinMovingInLab02) {
		Actor_Set_Goal_Number(kActorKlein, kGoalKleinMovingInLab01);
		return; // true;
	}

	// Annoyed behavior
	if (Actor_Query_Goal_Number(kActorKlein) == kGoalKleinIsAnnoyedByMcCoyInit) {
		Actor_Set_Goal_Number(kActorKlein, kGoalKleinIsAnnoyedByMcCoy01);
		if (_vm->_cutContent) {
			return;
		}
		// NOTE: original was missing return here
	}
	if (Actor_Query_Goal_Number(kActorKlein) == kGoalKleinIsAnnoyedByMcCoy01) {
		Actor_Set_Goal_Number(kActorKlein, kGoalKleinIsAnnoyedByMcCoy02);
		if (_vm->_cutContent) {
			return;
		}
		// NOTE: original was missing return here
	}
	if (Actor_Query_Goal_Number(kActorKlein) == kGoalKleinIsAnnoyedByMcCoy02) {
		Actor_Set_Goal_Number(kActorKlein, kGoalKleinIsAnnoyedByMcCoyFinal);
		return; // true;
	}
	if (Actor_Query_Goal_Number(kActorKlein) == kGoalKleinIsAnnoyedByMcCoyFinal) {
#if BLADERUNNER_ORIGINAL_BUGS
		Actor_Set_Goal_Number(kActorKlein, kGoalKleinDefault);
#else
		if (Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) < 40) {
			Actor_Set_Goal_Number(kActorKlein, kGoalKleinIsAnnoyedByMcCoyInit);
		} else {
			if (Game_Flag_Query(kFlagPS07KleinInsulted)) {
				Game_Flag_Reset(kFlagPS07KleinInsulted);
				// don't reset the kFlagPS07KleinInsultedTalk
			}
			Actor_Set_Goal_Number(kActorKlein, kGoalKleinDefault);
		}
#endif // BLADERUNNER_ORIGINAL_BUGS
		return; // true;
	}
	// return false;
}

void AIScriptKlein::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptKlein::ClickedByPlayer() {
	//return false;
}

void AIScriptKlein::EnteredSet(int setId) {
	// return false;
}

void AIScriptKlein::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptKlein::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptKlein::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptKlein::ShotAtAndMissed() {
	// return false;
}

bool AIScriptKlein::ShotAtAndHit() {
	return false;
}

void AIScriptKlein::Retired(int byActorId) {
	// return false;
}

int AIScriptKlein::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptKlein::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalKleinMovingInLab01:
		AI_Movement_Track_Flush(kActorKlein);
		AI_Movement_Track_Append(kActorKlein, 73, Random_Query(3, 20));  // kSetPS07
		AI_Movement_Track_Repeat(kActorKlein);
		break;

	case kGoalKleinMovingInLab02:
		AI_Movement_Track_Flush(kActorKlein);
		AI_Movement_Track_Append(kActorKlein, 74, Random_Query(10, 20)); // kSetPS07
		AI_Movement_Track_Repeat(kActorKlein);
		break;

	case kGoalKleinGotoLabSpeaker:
		AI_Movement_Track_Flush(kActorKlein);
		AI_Movement_Track_Append(kActorKlein, 31, 3);  // kSetPS07
		AI_Movement_Track_Repeat(kActorKlein);
		break;

	case kGoalKleinIsAnnoyedByMcCoyPreInit:
		// aux goal (added)
		break;

	case kGoalKleinIsAnnoyedByMcCoyInit:
		AI_Movement_Track_Flush(kActorKlein);
		AI_Movement_Track_Append(kActorKlein, 32, 5);  // kSetPS07 (hidden spot)
		AI_Movement_Track_Repeat(kActorKlein);
		break;

	case kGoalKleinIsAnnoyedByMcCoy01:
		AI_Movement_Track_Flush(kActorKlein);
		if (_vm->_cutContent) {
			AI_Movement_Track_Append(kActorKlein, 35, Random_Query(8, 24)); // kSetFreeSlotC
		} else {
			// this never really gets triggered in the original game
			AI_Movement_Track_Append(kActorKlein, 35, 60); // kSetFreeSlotC
		}
		AI_Movement_Track_Repeat(kActorKlein);
		break;

	case kGoalKleinIsAnnoyedByMcCoy02:
		AI_Movement_Track_Flush(kActorKlein);
		AI_Movement_Track_Append(kActorKlein, 32, 5);  // kSetPS07 (hidden spot)
		AI_Movement_Track_Repeat(kActorKlein);
		break;

	case kGoalKleinIsAnnoyedByMcCoyFinal:
		// Note: Original was missing the kGoalKleinIsAnnoyedByMcCoyFinal case
		//       so we just "break" for the original behavior
		if (_vm->_cutContent) {
			AI_Movement_Track_Flush(kActorKlein);
			AI_Movement_Track_Append(kActorKlein, 74, Random_Query(10, 20)); // kSetPS07
			AI_Movement_Track_Repeat(kActorKlein);
		}
		break;

	case kGoalKleinAwayAtEndOfActThree:
		// fall-through
	case kGoalKleinAwayAtEndOfActOne:
		AI_Movement_Track_Flush(kActorKlein);
		Actor_Put_In_Set(kActorKlein, kSetFreeSlotC);
		Actor_Set_At_Waypoint(kActorKlein, 35, 0);    //  kSetFreeSlotC
		break;
	}
	return false;
}

bool AIScriptKlein::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (Actor_Query_Goal_Number(kActorKlein) == kGoalKleinMovingInLab01
		    || Actor_Query_Goal_Number(kActorKlein) == kGoalKleinMovingInLab02
		) {
			*animation = kModelAnimationKleinWorkingOnInstruments;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationKleinWorkingOnInstruments)) {
				_animationFrame = 0;
			}
		} else if (!Game_Flag_Query(kFlagKleinAnimation1)
		           && Actor_Query_Goal_Number(kActorKlein) == kGoalKleinGotoLabSpeaker
		) {
			*animation = kModelAnimationKleinStandingIdle;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationKleinStandingIdle)) {
				_animationFrame = 0;
				if (Random_Query(1, 10) == 1) {
					Game_Flag_Set(kFlagKleinAnimation1);
				}
			}
		} else {
			if (Game_Flag_Query(kFlagKleinAnimation3)
			    && Actor_Query_Goal_Number(kActorKlein) == kGoalKleinGotoLabSpeaker
			) {
				--_animationFrame;
				if (_animationFrame < 0) {
					_animationFrame = 0;
				}
			} else {
				++_animationFrame;
			}

			*animation = kModelAnimationKleinTalkScratchBackOfHead;
			if (_animationFrame <= 9) {
				if (Game_Flag_Query(kFlagKleinAnimation3)) {
					Game_Flag_Reset(kFlagKleinAnimation3);
				}
			}
			if (_animationFrame == 14) {
				if (Random_Query(1, 5) == 1) {
					Game_Flag_Set(kFlagKleinAnimation2);
				}
			}
			if (_animationFrame == 15) {
				if (Game_Flag_Query(kFlagKleinAnimation2)) {
					Game_Flag_Reset(kFlagKleinAnimation2);
					Game_Flag_Set(kFlagKleinAnimation3);
				}
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationKleinTalkScratchBackOfHead)) {
				_animationFrame = 0;
				Game_Flag_Reset(kFlagKleinAnimation1);
			}
		}
		break;

	case 1:
		*animation = kModelAnimationKleinWalking;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationKleinWalking)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = kModelAnimationKleinTalkSmallLeftHandMove;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationKleinTalkSmallLeftHandMove)) {
			_animationFrame = 0;
		}
		break;

	case 3:
		*animation = kModelAnimationKleinTalkRightHandTouchFace;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationKleinTalkRightHandTouchFace)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = kModelAnimationKleinTalkSmallLeftHandMove;
		}
		break;

	case 4:
		*animation = kModelAnimationKleinTalkWideHandMotion;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationKleinTalkWideHandMotion)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = kModelAnimationKleinTalkSmallLeftHandMove;
		}
		break;

	case 5:
		*animation = kModelAnimationKleinTalkSuggestOrAsk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationKleinTalkSuggestOrAsk)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = kModelAnimationKleinTalkSmallLeftHandMove;
		}
		break;

	case 6:
		*animation = kModelAnimationKleinTalkDismissive;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationKleinTalkDismissive)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = kModelAnimationKleinTalkSmallLeftHandMove;
		}
		break;

	case 7:
		*animation = kModelAnimationKleinTalkRaisingBothHands;
		++_animationFrame;
		if (_animationFrame>= Slice_Animation_Query_Number_Of_Frames(kModelAnimationKleinTalkRaisingBothHands)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = kModelAnimationKleinTalkSmallLeftHandMove;
		}
		break;

	case 8:
		_animationFrame = 0;
		*animation      = _animationNext;
		_animationState = _animationStateNext;
		break;

	default:
		// Dummy placeholder, kModelAnimationZubenWalking (399) is a Zuben animation
		*animation = kModelAnimationZubenWalking;
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptKlein::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		_animationState = 0;
		_animationFrame = 0;
		break;

	case kAnimationModeWalk:
		if (_animationState > 1) {
			_animationState = 1;
			_animationFrame = 0;
		} else if (_animationState == 0) {
			_animationState = 8;
			_animationStateNext = 1;
			_animationNext = kModelAnimationKleinWalking;
		}
		break;

	case kAnimationModeTalk:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 8;
			_animationStateNext = 2;
			_animationNext = kModelAnimationKleinTalkSmallLeftHandMove;
		}
		break;

	case 12:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 8;
			_animationStateNext = 3;
			_animationNext = kModelAnimationKleinTalkRightHandTouchFace;
		}
		break;

	case 13:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 8;
			_animationStateNext = 4;
			_animationNext = kModelAnimationKleinTalkWideHandMotion;
		}
		break;

	case 14:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 8;
			_animationStateNext = 5;
			_animationNext = kModelAnimationKleinTalkSuggestOrAsk;
		}
		break;

	case 15:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 8;
			_animationStateNext = 6;
			_animationNext = kModelAnimationKleinTalkDismissive;
		}
		break;

	case 16:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 8;
			_animationStateNext = 7;
			_animationNext = kModelAnimationKleinTalkRaisingBothHands;
		}
		break;
	}
	return true;
}

void AIScriptKlein::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptKlein::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptKlein::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 73 || waypointId == 74) {
		Actor_Face_Heading(kActorKlein, 768, false);
	}
	if (waypointId == 31) {
		Actor_Face_Heading(kActorKlein, 216, false);
	}
	return true;
}

void AIScriptKlein::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
