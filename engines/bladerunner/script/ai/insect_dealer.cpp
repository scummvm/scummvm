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
	_flag1 = false;
	_state = 0;
	_frameDelta = 0;
	_var2 = 0;
	_counter = 0;
}

void AIScriptInsectDealer::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag1 = false;
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

void AIScriptInsectDealer::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptInsectDealer::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptInsectDealer::OtherAgentExitedThisScene(int otherActorId) {
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
			*animation = 545;
			if (_counter) {
				_counter--;
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
			*animation = 546;
			_animationFrame++;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(546)) {
				*animation = 545;
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
		if (_animationFrame == 0 && _flag1) {
			*animation = 545;
			_animationState = 0;
		} else {
			*animation = 548;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(548)) {
				_animationFrame = 0;
				_animationState = 0;
			}
		}
		break;
	case 2:
		*animation = 549;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(549)) {
			*animation = 548;
			_animationFrame = 0;
			_animationState = 1;
		}
		break;
	case 3:
		*animation = 550;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(550)) {
			*animation = 548;
			_animationFrame = 0;
			_animationState = 1;
		}
		break;
	case 4:
		*animation = 551;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(551)) {
			*animation = 548;
			_animationFrame = 0;
			_animationState = 1;
		}
		break;
	case 5:
		*animation = 552;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(552)) {
			*animation = 548;
			_animationFrame = 0;
			_animationState = 1;
		}
		break;
	case 6:
		*animation = 553;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(553)) {
			*animation = 548;
			_animationFrame = 0;
			_animationState = 1;
		}
		break;
	case 7:
		*animation = 554;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(554)) {
			*animation = 548;
			_animationFrame = 0;
			_animationState = 1;
		}
		break;
	case 8:
		*animation = 547;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(547)) {
			*animation = 545;
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
			_flag1 = true;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;
	case 3:
	case 18:
	case 19:
		_animationState = 1;
		_animationFrame = 0;
		_flag1 = false;
		break;
	case 12:
		_animationState = 2;
		_animationFrame = 0;
		_flag1 = false;
		break;
	case 13:
		_animationState = 3;
		_animationFrame = 0;
		_flag1 = false;
		break;
	case 14:
		_animationState = 4;
		_animationFrame = 0;
		_flag1 = false;
		break;
	case 15:
		_animationState = 5;
		_animationFrame = 0;
		_flag1 = false;
		break;
	case 16:
		_animationState = 6;
		_animationFrame = 0;
		_flag1 = false;
		break;
	case 17:
		_animationState = 7;
		_animationFrame = 0;
		_flag1 = false;
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
