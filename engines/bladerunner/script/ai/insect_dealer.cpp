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

AIScriptInsectDealer::AIScriptInsectDealer(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_resumeIdleAfterFramesetCompletesFlag = false;
	_state = 0;
	_frameDelta = 0;
	_var2 = 0; // is set on some occasions but is never checked. Unused.
	_counter = 0;
}

void AIScriptInsectDealer::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_resumeIdleAfterFramesetCompletesFlag = false;
	_state = 0;
	_frameDelta = 1;
	_var2 = 6;
	_counter = 0;

	Actor_Put_In_Set(kActorInsectDealer, kSetAR01_AR02);
	Actor_Set_At_XYZ(kActorInsectDealer, -414.0f, 0.0f, -1199.0f, 371);
	Actor_Set_Goal_Number(kActorInsectDealer, 0);
}

bool AIScriptInsectDealer::Update() {
	if (Global_Variable_Query(kVariableChapter) == 5
	 && Actor_Query_Goal_Number(kActorInsectDealer) < 400
	)
		Actor_Set_Goal_Number(kActorInsectDealer, 400);

	return false;
}

void AIScriptInsectDealer::TimerExpired(int timer) {
	//return false;
}

void AIScriptInsectDealer::CompletedMovementTrack() {
	//return false;
}

void AIScriptInsectDealer::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptInsectDealer::ClickedByPlayer() {
	//return false;
}

void AIScriptInsectDealer::EnteredSet(int setId) {
	// return false;
}

void AIScriptInsectDealer::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptInsectDealer::OtherAgentExitedThisSet(int otherActorId) {
	// return false;
}

void AIScriptInsectDealer::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptInsectDealer::ShotAtAndMissed() {
	// return false;
}

bool AIScriptInsectDealer::ShotAtAndHit() {
	return false;
}

void AIScriptInsectDealer::Retired(int byActorId) {
	// return false;
}

int AIScriptInsectDealer::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptInsectDealer::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == 400) {
		Actor_Put_In_Set(kActorInsectDealer, kSetFreeSlotH);
		Actor_Set_At_Waypoint(kActorInsectDealer, 40, 0);

		if (!Game_Flag_Query(kFlagAR02DektoraBoughtScorpions)) {
			Game_Flag_Set(kFlagAR02DektoraBoughtScorpions);
#if BLADERUNNER_ORIGINAL_BUGS
			Item_Remove_From_World(kItemScorpions);
#else
			if (Game_Flag_Query(kFlagScorpionsInAR02)) {
				Game_Flag_Reset(kFlagScorpionsInAR02);
				Item_Remove_From_World(kItemScorpions);
			}
#endif
		}
	}

	return false;
}

bool AIScriptInsectDealer::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		switch (_state) {
		case 0:
			*animation = kModelAnimationInsectDealerSittingIdle;
			if (_counter > 0) {
				--_counter;
				if (Random_Query(0, 6) == 0) {
					_frameDelta = -_frameDelta;
				}
			} else {
				_animationFrame += _frameDelta;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
					_animationFrame = 0;
					if (Random_Query(0, 2) == 0) {
						_state = 2 * Random_Query(0, 1);
					}
				}
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(*animation) - 1;
				}

				_counter = Random_Query(0, 1);
				if (_animationFrame == 0) {
					_state = Random_Query(0, 1);
				}
			}
			break;

		case 1:
			*animation = kModelAnimationInsectDealerSittingScratchEarIdle;
			++_animationFrame;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationInsectDealerSittingScratchEarIdle)) {
				*animation = kModelAnimationInsectDealerSittingIdle;
				_animationFrame = 0;
				_state = 0;
				_var2 = Random_Query(6, 14);
				_frameDelta = 2 * Random_Query(0, 1) - 1;
			}
			break;

		case 2:
			// TODO: test... actor will be stuck
			break;
		}
		break;

	case 1:
		if (_animationFrame == 0 && _resumeIdleAfterFramesetCompletesFlag) {
			*animation = kModelAnimationInsectDealerSittingIdle;
			_animationState = 0;
		} else {
			*animation = kModelAnimationInsectDealerSittingCalmTalk;
			++_animationFrame;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationInsectDealerSittingCalmTalk)) {
				_animationFrame = 0;
				_animationState = 0;
			}
		}
		break;

	case 2:
		*animation = kModelAnimationInsectDealerSittingExplainTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationInsectDealerSittingExplainTalk)) {
			*animation = kModelAnimationInsectDealerSittingCalmTalk;
			_animationFrame = 0;
			_animationState = 1;
		}
		break;

	case 3:
		*animation = kModelAnimationInsectDealerSittingPointingTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationInsectDealerSittingPointingTalk)) {
			*animation = kModelAnimationInsectDealerSittingCalmTalk;
			_animationFrame = 0;
			_animationState = 1;
		}
		break;

	case 4:
		*animation = kModelAnimationInsectDealerSittingHeadMoveTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationInsectDealerSittingHeadMoveTalk)) {
			*animation = kModelAnimationInsectDealerSittingCalmTalk;
			_animationFrame = 0;
			_animationState = 1;
		}
		break;

	case 5:
		*animation = kModelAnimationInsectDealerSittingSuggestTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationInsectDealerSittingSuggestTalk)) {
			*animation = kModelAnimationInsectDealerSittingCalmTalk;
			_animationFrame = 0;
			_animationState = 1;
		}
		break;

	case 6:
		*animation = kModelAnimationInsectDealerSittingGossipTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationInsectDealerSittingGossipTalk)) {
			*animation = kModelAnimationInsectDealerSittingCalmTalk;
			_animationFrame = 0;
			_animationState = 1;
		}
		break;

	case 7:
		*animation = kModelAnimationInsectDealerSittingDescriptiveTalk;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationInsectDealerSittingDescriptiveTalk)) {
			*animation = kModelAnimationInsectDealerSittingCalmTalk;
			_animationFrame = 0;
			_animationState = 1;
		}
		break;

	case 8:
		*animation = kModelAnimationInsectDealerSittingGestureGive;
		++_animationFrame;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(kModelAnimationInsectDealerSittingGestureGive)) {
			*animation = kModelAnimationInsectDealerSittingIdle;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;
	}

	*frame = _animationFrame;
	return true;
}

bool AIScriptInsectDealer::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState > 0 && _animationState <= 7) {
			_resumeIdleAfterFramesetCompletesFlag = true;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;

	case 3:
		// fall through
	case 18:
		// fall through
	case 19:
		_animationState = 1;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 12:
		_animationState = 2;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 13:
		_animationState = 3;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 14:
		_animationState = 4;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 15:
		_animationState = 5;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 16:
		_animationState = 6;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 17:
		_animationState = 7;
		_animationFrame = 0;
		_resumeIdleAfterFramesetCompletesFlag = false;
		break;

	case 23:
		_animationState = 8;
		_animationFrame = 0;
		break;

	default:
		break;
	}

	return true;
}

void AIScriptInsectDealer::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptInsectDealer::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptInsectDealer::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptInsectDealer::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
