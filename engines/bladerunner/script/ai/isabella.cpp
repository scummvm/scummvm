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
	_varNumOfTimesToHoldCurrentFrame = 0;
	// _varChooseIdleAnimation can have valid values: 0, 
	_varChooseIdleAnimation = 0;
	_var4 = 1;
}

void AIScriptIsabella::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 0;
	_varNumOfTimesToHoldCurrentFrame = 0;
	_varChooseIdleAnimation = 0;
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

void AIScriptIsabella::EnteredSet(int setId) {
	// return false;
}

void AIScriptIsabella::OtherAgentEnteredThisSet(int otherActorId) {
	// return false;
}

void AIScriptIsabella::OtherAgentExitedThisSet(int otherActorId) {
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
		if (_varChooseIdleAnimation == 1) {
			*animation = kModelIsabellaPutsSpicesInSoup;
			++_animationFrame;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelIsabellaPutsSpicesInSoup) - 1) {
				_animationFrame = 0;
			}
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelIsabellaPutsSpicesInSoup) - 1;
			}
			if (_animationFrame == 0) {
				_varChooseIdleAnimation = 0;
				_var4 = 2 * Random_Query(0, 1) - 1;
			}
		} else if (_varChooseIdleAnimation == 0) {
			*animation = kModelIsabellaIdle;
			if (_varNumOfTimesToHoldCurrentFrame > 0) {
				--_varNumOfTimesToHoldCurrentFrame;
				if (_varNumOfTimesToHoldCurrentFrame == 0) {
					_var4 = 2 * Random_Query(0, 1) - 1;
				}
			} else {
				_animationFrame += _var4;
				if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelIsabellaIdle) - 1) {
					_animationFrame = 0;
				}
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelIsabellaIdle) - 1;
				}
				if (_animationFrame == 1) {
					if (!Random_Query(0, 1)) {
						_varNumOfTimesToHoldCurrentFrame = Random_Query(4, 8);
					}
				}
				if (_animationFrame == 11) {
					if (!Random_Query(0, 1)) {
						_varNumOfTimesToHoldCurrentFrame = Random_Query(4, 8);
					}
				}
				if (_animationFrame == 16) {
					if (!Random_Query(0, 1)) {
						_varNumOfTimesToHoldCurrentFrame = Random_Query(4, 8);
					}
				}
				if (_animationFrame == 0) {
					if (!Random_Query(0, 2)) {
						_varChooseIdleAnimation = 1;
					}
				}
			}
		}
		break;

	case 1:
		*animation = kModelIsabellaGestureGiveOrTake;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelIsabellaGestureGiveOrTake) - 1) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelIsabellaGestureGiveOrTake) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = kModelIsabellaIdle;
			_animationState = 0;
		}
		break;

	case 2:
		*animation = kModelIsabellaCalmTalk;
		if (_animationFrame < 2 && _var1) {
			_animationFrame = 0;
			_animationState = 0;
		} else {
			++_animationFrame;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelIsabellaCalmTalk) - 1) {
				_animationFrame = 0;
			} else {
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelIsabellaCalmTalk) - 1;
				}
			}
			if (_animationFrame == 0) {
				_animationState = Random_Query(2, 3);
			}
		}
		break;

	case 3:
		*animation = kModelIsabellaSuggestTalk;
		if (_animationFrame < 2 && _var1) {
			_animationFrame = 0;
			_animationState = 0;
		} else {
			++_animationFrame;
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelIsabellaSuggestTalk) - 1) {
				_animationFrame = 0;
			} else {
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelIsabellaSuggestTalk) - 1;
				}
			}
			if (_animationFrame == 0) {
				*animation = kModelIsabellaCalmTalk;
				_animationState = 2;
			}
		}
		break;

	case 4:
		*animation = kModelIsabellaProtestTalk;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelIsabellaProtestTalk) - 1) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelIsabellaProtestTalk) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = kModelIsabellaCalmTalk;
			_animationState = 2;
		}
		break;

	case 5:
		*animation = kModelIsabellaMoreCalmTalk;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelIsabellaMoreCalmTalk) - 1) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelIsabellaMoreCalmTalk) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = kModelIsabellaCalmTalk;
			_animationState = 2;
		}
		break;

	case 6:
		*animation = kModelIsabellaLaughTalk;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelIsabellaLaughTalk) - 1) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelIsabellaLaughTalk) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = kModelIsabellaCalmTalk;
			_animationState = 2;
		}
		break;

	case 7:
		*animation = kModelIsabellaLaughTalk;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelIsabellaLaughTalk) - 1) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelIsabellaLaughTalk) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = kModelIsabellaCalmTalk;
			_animationState = 2;
		}
		break;

	case 8:
		*animation = kModelIsabellaMoreCalmTalk;
		++_animationFrame;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(kModelIsabellaMoreCalmTalk) - 1) {
			flag = true;
			_animationFrame = 0;
		} else {
			if (_animationFrame < 0) {
				_animationFrame = Slice_Animation_Query_Number_Of_Frames(kModelIsabellaMoreCalmTalk) - 1;
				flag = true;
			} else {
				flag = false;
			}
		}
		if (flag) {
			*animation = kModelIsabellaCalmTalk;
			_animationState = 2;
		}
		break;

	case 9:
		if (_varChooseIdleAnimation == 0) {
			*animation = kModelIsabellaIdle;
		}
		if (_varChooseIdleAnimation == 1) {
			*animation = kModelIsabellaPutsSpicesInSoup;
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
		// fall through
	case 9:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 2;
			_animationNext = kModelIsabellaCalmTalk;
			_var1 = 0;
		}
		break;

	case 10:
		// fall through
	case 12:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 3;
			_animationNext = kModelIsabellaSuggestTalk;
			_var1 = 0;
		}
		break;

	case 11:
		// fall through
	case 14:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 5;
			_animationNext = kModelIsabellaMoreCalmTalk;
			_var1 = 0;
		}
		break;

	case 13:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 4;
			_animationNext = kModelIsabellaProtestTalk;
			_var1 = 0;
		}
		break;

	case 15:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 6;
			_animationNext = kModelIsabellaLaughTalk;
			_var1 = 0;
		}
		break;

	case 16:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 7;
			_animationNext = kModelIsabellaLaughTalk;
			_var1 = 0;
		}
		break;

	case 17:
		if (_animationState < 2 || _animationState > 8) {
			_animationState = 9;
			_animationStateNext = 8;
			_animationNext = kModelIsabellaMoreCalmTalk;
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
