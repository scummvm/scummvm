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

AIScriptTyrellGuard::AIScriptTyrellGuard(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptTyrellGuard::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_animDirection = 1;
	_flag1 = false;
	Actor_Set_Goal_Number(kActorTyrellGuard, 0);
}

bool AIScriptTyrellGuard::Update() {
	return false;
}

void AIScriptTyrellGuard::TimerExpired(int timer) {
	if (timer) {
		if (timer == 1) {
			AI_Countdown_Timer_Reset(kActorTyrellGuard, 1);
			Actor_Set_Goal_Number(kActorTyrellGuard, 303);
		}
	} else {
		AI_Countdown_Timer_Reset(kActorTyrellGuard, 0);

		if (Actor_Query_Which_Set_In(0) == kSetTB02_TB03) {
			Actor_Set_Goal_Number(kActorTyrellGuard, 301);
		}
	}
}

void AIScriptTyrellGuard::CompletedMovementTrack() {
	//return false;
}

void AIScriptTyrellGuard::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptTyrellGuard::ClickedByPlayer() {
	//return false;
}

void AIScriptTyrellGuard::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptTyrellGuard::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptTyrellGuard::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptTyrellGuard::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptTyrellGuard::ShotAtAndMissed() {
	// return false;
}

bool AIScriptTyrellGuard::ShotAtAndHit() {
	return false;
}

void AIScriptTyrellGuard::Retired(int byActorId) {
	// return false;
}

int AIScriptTyrellGuard::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptTyrellGuard::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 300:
		if (currentGoalNumber != newGoalNumber) {
			Actor_Change_Animation_Mode(kActorTyrellGuard, 55);
			AI_Countdown_Timer_Start(kActorTyrellGuard, 0, 30);
		}
		return true;

	case 301:
		Actor_Change_Animation_Mode(kActorTyrellGuard, 0);
		Delay(1000);
		Actor_Says(kActorTyrellGuard, 320, 14);
		Actor_Change_Animation_Mode(kActorTyrellGuard, 50);
		Ambient_Sounds_Play_Sound(590, 100, 0, 0, 0);
		Delay(1000);
		Actor_Force_Stop_Walking(0);
		Actor_Set_Goal_Number(0, 500);

		return true;

	case 302:
		AI_Countdown_Timer_Reset(kActorTyrellGuard, 0);
		Actor_Says(kActorTyrellGuard, 310, 14);
		AI_Countdown_Timer_Start(kActorTyrellGuard, 1, 20);

		return true;

	case 303:
		Actor_Change_Animation_Mode(kActorTyrellGuard, 50);
		Ambient_Sounds_Play_Sound(590, 100, 0, 0, 0);
		Delay(1000);
		Actor_Force_Stop_Walking(0);
		Actor_Set_Goal_Number(0, 500);

		return true;

	case 304:
		AI_Countdown_Timer_Reset(kActorTyrellGuard, 1);

		return true;

	default:
		return false;
	}
}

bool AIScriptTyrellGuard::UpdateAnimation(int *animation, int *frame) {
	int frameRes;

	switch (_animationState) {
	case 0:
		*animation = 555;
		_animationFrame++;

		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(555)) {
			frameRes = _animationFrame;
		} else {
			_animationFrame = 0;
			frameRes = 0;
		}
		break;
	case 1:
		*animation = 564;

		if (_animationFrame <= 5) {
			_animDirection = 1;
		} else if (_animationFrame >= 12) {
			_animDirection = -1;
		}

		_animationFrame += _animDirection;
		frameRes = _animationFrame;
		break;
	case 2:
		*animation = 564;
		_animationFrame++;

		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(564)) {
			frameRes = _animationFrame;
		} else {
			_animationFrame = 0;
			frameRes = 0;
			_animationState = 0;
		}
		break;
	case 3:
		frameRes = _animationFrame;
		*animation = 558;

		if (!frameRes && _flag1) {
			*animation = 555;
			_animationState = 0;
		} else {
			_animationFrame = frameRes + 1;

			if (frameRes + 1 < Slice_Animation_Query_Number_Of_Frames(*animation)) {
				frameRes = _animationFrame;
			} else {
				_animationFrame = 0;
				frameRes = 0;
			}
		}
		break;
	case 4:
		*animation = 559;
		_animationFrame++;

		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(559)) {
			frameRes = _animationFrame;
		} else {
			_animationFrame = 0;
			frameRes = 0;
			_animationState = 3;
			*animation = 558;
		}
		break;
	case 5:
		*animation = 560;
		_animationFrame++;

		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(560)) {
			frameRes = _animationFrame;
		} else {
			_animationFrame = 0;
			frameRes = 0;
			_animationState = 3;
			*animation = 558;
		}
		break;
	case 6:
		*animation = 561;
		_animationFrame++;

		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(561)) {
			frameRes = _animationFrame;
		} else {
			_animationFrame = 0;
			frameRes = 0;
			_animationState = 3;
			*animation = 558;
		}
		break;
	case 7:
		*animation = 562;
		_animationFrame++;

		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(562)) {
			frameRes = _animationFrame;
		} else {
			_animationFrame = 0;
			frameRes = 0;
			_animationState = 3;
			*animation = 558;
		}
		break;
	case 8:
		*animation = 557;
		_animationFrame++;

		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(557)) {
			frameRes = _animationFrame;
		} else {
			*animation = 555;
			_animationFrame = 0;
			frameRes = 0;
			_animationState = 0;
		}
		break;
	case 9:
		*animation = 563;
		_animationFrame++;

		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(563)) {
			frameRes = _animationFrame;
		} else {
			*animation = 555;
			_animationFrame = 0;
			frameRes = 0;
			_animationState = 0;
		}
		break;
	case 10:
		*animation = 564;
		_animationFrame++;

		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(564)) {
			frameRes = _animationFrame;
		} else {
			*animation = 555;
			_animationFrame = 0;
			frameRes = 0;
			_animationState = 0;
		}
		break;
	case 11:
		*animation = 565;
		_animationFrame++;

		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(565)) {
			frameRes = _animationFrame;
		} else {
			*animation = 555;
			_animationFrame = 0;
			frameRes = 0;
			_animationState = 0;
		}
		break;
	default:
		frameRes = _animationFrame;
		break;
	}

	*frame = frameRes;

	return true;
}

bool AIScriptTyrellGuard::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		switch (_animationState) {
		case 0:
			_animationState = 8;
			_animationFrame = 0;
			break;
		case 1:
			_animationState = 2;
			break;
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			_flag1 = true;
			break;
		case 8:
			break;
		default:
			_animationState = 0;
			_animationFrame = 0;
			break;
		}
		break;
	case 1:
	case 2:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
		break;
	case 3:
		if (_animationState == 1) {
			_animationState = 2;
		} else if ((_animationState - 1 != 7 && _animationState - 1 != 10) || _animationState - 1 > 10) {
			_animationState = 3;
			_animationFrame = 0;
			_flag1 = false;
		}
		break;
	case 12:
		if (_animationState == 1) {
			_animationState = 2;
		} else if ((_animationState - 1 != 7 && _animationState - 1 != 10) || _animationState - 1 > 10) {
			_animationState = 4;
			_animationFrame = 0;
			_flag1 = false;
		}
		break;
	case 13:
		if (_animationState == 1) {
			_animationState = 2;
		} else if ((_animationState - 1 != 7 && _animationState - 1 != 10) || _animationState - 1 > 10) {
			_animationState = 5;
			_animationFrame = 0;
			_flag1 = false;
		}
		break;
	case 14:
		if (_animationState == 1) {
			_animationState = 2;
		} else if ((_animationState - 1 != 7 && _animationState - 1 != 10) || _animationState - 1 > 10) {
			_animationState = 6;
			_animationFrame = 0;
			_flag1 = false;
		}
		break;
	case 15:
		if (_animationState == 1) {
			_animationState = 2;
		} else if ((_animationState - 1 != 7 && _animationState - 1 != 10) || _animationState - 1 > 10) {
			_animationState = 7;
			_animationFrame = 0;
			_flag1 = false;
		}
		break;
	case 23:
		_animationState = 8;
		_animationFrame = 0;
		break;
	default:
		if (mode - 43 > 12) {
			break;
		}
		if (mode - 43 == 7) {
			_animationState = 11;
			_animationFrame = 0;
		} else if (mode - 43 == 12 || mode == 43) {
			if (_animationState != 1) {
				_animationState = 1;
				_animationFrame = 0;
			}
		}
	}

	return true;
}

void AIScriptTyrellGuard::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptTyrellGuard::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptTyrellGuard::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptTyrellGuard::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
