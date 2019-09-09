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

AIScriptChew::AIScriptChew(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = false;
	_var1 = 0;
	_var2 = 0;
	_var3 = 1;
}

void AIScriptChew::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = false;
	_var1 = 0;
	_var2 = 0;
	_var3 = 1;
}

bool AIScriptChew::Update() {
	return false;
}

void AIScriptChew::TimerExpired(int timer) {
	//return false;
}

void AIScriptChew::CompletedMovementTrack() {
	//return false;
}

void AIScriptChew::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptChew::ClickedByPlayer() {
	//return false;
}

void AIScriptChew::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptChew::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptChew::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptChew::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptChew::ShotAtAndMissed() {
	// return false;
}

bool AIScriptChew::ShotAtAndHit() {
	return false;
}

void AIScriptChew::Retired(int byActorId) {
	// return false;
}

int AIScriptChew::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptChew::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptChew::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_var2 == 0) {
			*animation = 777;
			if (_var1) {
				_var1--;
			} else {
				_animationFrame++;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(777)) {
					_animationFrame = 0;
					_var3 = 1;
					_var2 = Random_Query(0, 2);
				} else if (!Random_Query(0, 1)) {
					_var1 = 1;
					if (!Random_Query(0, 3)) {
						_var3 = -_var3;
					}
				}
			}
		} else if (_var2 == 1) {
			*animation = 778;
			_animationFrame += _var3;
			if (_animationFrame <= 6) {
				_var3 = 1;
			}
			if (_animationFrame == 13) {
				if (!Random_Query(0, 1)) {
					_var3 = -1;
				}
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(778)) {
				_animationFrame = 0;
				_var2 = Random_Query(0, 2);
				_var3 = 1;
			}
		} else if (_var2 == 2) {
			*animation = 779;
			_animationFrame += _var3;
			if (_animationFrame <= 8) {
				_var3 = 1;
			}
			if (_animationFrame == 16) {
				if (!Random_Query(0, 1)) {
					_var3 = -1;
				}
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(779)) {
				_animationFrame = 0;
				_var2 = Random_Query(0, 2);
				_var3 = 1;
			}
		}
		break;

	case 1:
		if (!_var2) {
			*animation = 777;
		}
		if (_var2 == 1) {
			*animation = 778;
		}
		if (_var2 == 2) {
			*animation = 779;
		}
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame += 2;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		} else {
			_animationFrame -= 2;
			if (_animationFrame <= 0) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		}
		break;

	case 2:
		*animation = 780;
		if (!_animationFrame && _flag) {
			*animation = 777;
			_animationState = 0;
			_var2 = 0;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(780)) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = 781;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(781)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 780;
		}
		break;

	case 4:
		*animation = 782;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(782)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 780;
		}
		break;

	case 5:
		*animation = 783;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(783)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 780;
		}
		break;

	case 6:
		*animation = 784;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(784)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 780;
		}
		break;

	case 7:
		*animation = 785;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(785)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 780;
		}
		break;

	case 8:
		*animation = 786;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(786)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 780;
		}
		break;

	case 9:
		*animation = 787;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(787)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 780;
		}
		break;

	case 10:
		*animation = 775;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(775)) {
			*animation = 777;
			_animationFrame = 0;
			_animationState = 0;
			_var2 = 0;
		}
		break;

	case 11:
		*animation = 776;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(776) - 1) {
			_animationFrame++;
		}
		break;

	case 12:
		*animation = 773;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(773)) {
			_animationFrame = 0;
		}
		break;

	case 13:
		*animation = 774;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(774)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 780;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptChew::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		switch (_animationState) {
		case 0:
			return true;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			_flag = 1;
			break;
		default:
			_animationState = 0;
			_var2 = 0;
			_animationFrame = 0;
			break;
		}
		break;

	case 1:
		_animationFrame = 0;
		_animationState = 12;
		break;

	case 3:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState) {
				_animationState = 2;
				_animationFrame = 0;
				_flag = 0;
			} else {
				_animationStateNext = 2;
				_animationNext = 780;
				_animationState = 1;
			}
		}
		break;

	case 12:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState) {
				_animationState = 3;
				_animationFrame = 0;
				_flag = 0;
			} else {
				_animationStateNext = 3;
				_animationNext = 781;
				_animationState = 1;
			}
		}
		break;

	case 13:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState) {
				_animationState = 4;
				_animationFrame = 0;
				_flag = 0;
			} else {
				_animationStateNext = 4;
				_animationNext = 782;
				_animationState = 1;
			}
		}
		break;

	case 14:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState) {
				_animationState = 5;
				_animationFrame = 0;
				_flag = 0;
			} else {
				_animationStateNext = 5;
				_animationNext = 783;
				_animationState = 1;
			}
		}
		break;

	case 15:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState) {
				_animationState = 6;
				_animationFrame = 0;
				_flag = 0;
			} else {
				_animationStateNext = 6;
				_animationNext = 784;
				_animationState = 1;
			}
		}
		break;

	case 16:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState) {
				_animationState = 7;
				_animationFrame = 0;
				_flag = 0;
			} else {
				_animationStateNext = 7;
				_animationNext = 785;
				_animationState = 1;
			}
		}
		break;

	case 17:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState) {
				_animationState = 8;
				_animationFrame = 0;
				_flag = 0;
			} else {
				_animationStateNext = 8;
				_animationNext = 786;
				_animationState = 1;
			}
		}
		break;

	case 18:
		if (_animationState < 2 || _animationState > 9) {
			if (_animationState) {
				_animationState = 9;
				_animationFrame = 0;
				_flag = 0;
			} else {
				_animationStateNext = 9;
				_animationNext = 787;
				_animationState = 1;
			}
		}
		break;

	case 43:
		if (_animationState || (!_animationState && _var2 != 1 && _var2 != 2)) {
			Actor_Change_Animation_Mode(kActorChew, kAnimationModeIdle);
			_var2 = Random_Query(1, 2);
		}
		break;

	case kAnimationModeDie:
		_animationFrame = 0;
		_animationState = 11;
		break;

	default:
		break;
	}

	return true;
}

void AIScriptChew::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptChew::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptChew::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptChew::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
