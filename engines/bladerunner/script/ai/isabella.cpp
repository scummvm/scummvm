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

AIScriptIsabella::AIScriptIsabella(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 0;
	_var2 = 0;
	_var3 = 0;
	_var4 = 1;
}

void AIScriptIsabella::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 0;
	_var2 = 0;
	_var3 = 0;
	_var4 = 1;
}

bool AIScriptIsabella::Update() {
	return false;
}

void AIScriptIsabella::TimerExpired(int timer) {
	//return false;
}

void AIScriptIsabella::CompletedMovementTrack() {
	//return false;
}

void AIScriptIsabella::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptIsabella::ClickedByPlayer() {
	//return false;
}

void AIScriptIsabella::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptIsabella::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptIsabella::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptIsabella::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptIsabella::ShotAtAndMissed() {
	// return false;
}

bool AIScriptIsabella::ShotAtAndHit() {
	return false;
}

void AIScriptIsabella::Retired(int byActorId) {
	// return false;
}

int AIScriptIsabella::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptIsabella::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	return false;
}

bool AIScriptIsabella::UpdateAnimation(int *animation, int *frame) {
	bool flag;

	switch (_animationState) {
	case 0:
		if (_var3 == 1) {
			*animation = 839;
			_animationFrame++;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(839) - 1) {
				_animationFrame = 0;
			}
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(839) - 1;
			}
			if (!_animationFrame) {
				_var3 = 0;
				_var4 = 2 * Random_Query(0, 1) - 1;
			}
		} else if (_var3 == 0) {
			*animation = 838;
			if (_var2) {
				_var2--;
				if (_var2 == 0) {
					_var4 = 2 * Random_Query(0, 1) - 1;
				}
			} else {
				_animationFrame += _var4;
				if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(838) - 1) {
					_animationFrame = 0;
				}
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(838) - 1;
				}
				if (_animationFrame == 1) {
					if (!Random_Query(0, 1)) {
						_var2 = Random_Query(4, 8);
					}
				}
				if (_animationFrame == 11) {
					if (!Random_Query(0, 1)) {
						_var2 = Random_Query(4, 8);
					}
				}
				if (_animationFrame == 16) {
					if (!Random_Query(0, 1)) {
						_var2 = Random_Query(4, 8);
					}
				}
				if (!_animationFrame) {
					if (!Random_Query(0, 2)) {
						_var3 = 1;
					}
				}
			}
		}
		break;

	case 1:
		*animation = 840;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(840) - 1) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(840) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = 838;
			_animationState = 0;
		}
		break;

	case 2:
		*animation = 841;
		if (_animationFrame < 2 && _var1) {
			_animationFrame = 0;
			_animationState = 0;
		} else {
			_animationFrame++;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(841) - 1) {
				_animationFrame = 0;
			} else {
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(841) - 1;
				}
			}
			if (!_animationFrame) {
				_animationState = Random_Query(2, 3);
			}
		}
		break;

	case 3:
		*animation = 842;
		if (_animationFrame < 2 && _var1) {
			_animationFrame = 0;
			_animationState = 0;
		} else {
			_animationFrame++;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(842) - 1) {
				_animationFrame = 0;
			} else {
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(842) - 1;
				}
			}
			if (!_animationFrame) {
				*animation = 841;
				_animationState = 2;
			}
		}
		break;

	case 4:
		*animation = 843;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(843) - 1) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(843) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = 841;
			_animationState = 2;
		}
		break;

	case 5:
		*animation = 844;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(844) - 1) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(844) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = 841;
			_animationState = 2;
		}
		break;

	case 6:
		*animation = 845;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(845) - 1) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(845) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = 841;
			_animationState = 2;
		}
		break;

	case 7:
		*animation = 845;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(845) - 1) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(845) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = 841;
			_animationState = 2;
		}
		break;

	case 8:
		*animation = 844;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(844) - 1) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(844) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = 841;
			_animationState = 2;
		}
		break;

	case 9:
		if (!_var3) {
			*animation = 838;
		}
		if (_var3 == 1) {
			*animation = 839;
		}
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame += 2;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
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
		break;

	default:
		break;
	}

	int frames = Slice_Animation_Query_Number_Of_Frames(*animation);

	if (frames != -1)
		_animationFrame %= frames;
	else
		_animationFrame = 0;

	*frame = _animationFrame;

	return true;
}

bool AIScriptIsabella::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState > 8) {
			_animationState = 0;
			_animationFrame = 0;
		} else {
			_var1 = 1;
		}
		break;

	case 3:
	case 9:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 2;
			_animationNext = 841;
			_var1 = 0;
		}
		break;

	case 10:
	case 12:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 3;
			_animationNext = 842;
			_var1 = 0;
		}
		break;

	case 11:
	case 14:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 5;
			_animationNext = 844;
			_var1 = 0;
		}
		break;

	case 13:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 4;
			_animationNext = 843;
			_var1 = 0;
		}
		break;

	case 15:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 6;
			_animationNext = 845;
			_var1 = 0;
		}
		break;

	case 16:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 7;
			_animationNext = 845;
			_var1 = 0;
		}
		break;

	case 17:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 8;
			_animationNext = 844;
			_var1 = 0;
		}
		break;

	default:
		break;
	}

	return true;
}

void AIScriptIsabella::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptIsabella::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptIsabella::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptIsabella::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
