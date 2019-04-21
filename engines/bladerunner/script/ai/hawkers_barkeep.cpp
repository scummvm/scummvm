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

AIScriptHawkersBarkeep::AIScriptHawkersBarkeep(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 0;
	_var2 = 0;
	_var3 = 1;
	_flag = false;
}

void AIScriptHawkersBarkeep::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 0;
	_var2 = 0;
	_var3 = 1;
	_flag = false;

	Actor_Put_In_Set(kActorHawkersBarkeep, kSetHC01_HC02_HC03_HC04);
	Actor_Set_At_XYZ(kActorHawkersBarkeep, -225.0f, 0.14f, 39.0f, 284);
}

bool AIScriptHawkersBarkeep::Update() {
	return false;
}

void AIScriptHawkersBarkeep::TimerExpired(int timer) {
	//return false;
}

void AIScriptHawkersBarkeep::CompletedMovementTrack() {
	//return false;
}

void AIScriptHawkersBarkeep::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptHawkersBarkeep::ClickedByPlayer() {
	//return false;
}

void AIScriptHawkersBarkeep::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptHawkersBarkeep::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptHawkersBarkeep::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptHawkersBarkeep::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptHawkersBarkeep::ShotAtAndMissed() {
	// return false;
}

bool AIScriptHawkersBarkeep::ShotAtAndHit() {
	return false;
}

void AIScriptHawkersBarkeep::Retired(int byActorId) {
	// return false;
}

int AIScriptHawkersBarkeep::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptHawkersBarkeep::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == 1)  {
		_animationState = 8;
		_animationFrame = -1;
		Actor_Set_Goal_Number(kActorHawkersBarkeep, 0);

		return true;
	} else if (newGoalNumber == 2) {
		_animationState = 9;
		_animationFrame = -1;
		Actor_Set_Goal_Number(kActorHawkersBarkeep, 0);

		return true;
	}

	return false;
}

bool AIScriptHawkersBarkeep::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_var1 == 0) {
			*animation = 705;

			if (_var2) {
				_var2--;

				if (Random_Query(0, 6) == 0) {
					_var3 = -_var3;
				}
			} else {
				_animationFrame += _var3;

				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(705))
					_animationFrame = 0;

				if (_animationFrame < 0)
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(705) - 1;

				if (!Random_Query(0, 4))
					_var2 = 1;

				if (_animationFrame == 13 || _animationFrame == 5 || _animationFrame == 9)
					_var2 = Random_Query(2, 8);

				if (!Random_Query(0, 5)) {
					if (!_animationFrame || _animationFrame == 11) {
						_animationFrame = 0;

						if (Random_Query(0, 1)) {
							*animation = 706;
							_var1 = 1;
						} else {
							*animation = 707;
							_var1 = 2;
						}
					}
				}
			}
		} else if (_var1 == 1) {
			*animation = 706;
			if (_animationFrame <= 3)
				_var3 = 1;

			if (_animationFrame == 11) {
				if (Random_Query(0, 2))
					_var3 = -1;
			}

			_animationFrame += _var3;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(706)) {
				*animation = 705;
				_animationFrame = 0;
				_var1 = 0;
			}
		} else if (_var1 == 2) {
			*animation = 707;

			if (_var2) {
				_var2--;

				if (_var2 == 0)
					_var3 = 2 * Random_Query(0, 1) - 1;
			} else {
				if (_animationFrame <= 11)
					_var3 = 1;

				if (_animationFrame == 14) {
					if (Random_Query(0, 2) != 0) {
						_var3 = -1;
					}
				}

				if (_animationFrame == 18) {
					_var2 = Random_Query(5, 15);
				}

				_animationFrame++;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(707)) {
					*animation = 705;
					_animationFrame = 0;
					_var1 = 0;
					_var2 = 0;
				}
			}
		}
		break;

	case 1:
		if (_var1 == 0) {
			_animationFrame = 0;
			_animationState = _animationStateNext;
			*animation = _animationNext;
		} else if (_var1 == 1) {
			*animation = 706;
			_animationFrame++;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(706)) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		} else if (_var1 == 2) {
			*animation = 707;
			_animationFrame += 2;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(707)) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		}

		break;

	case 2:
		*animation = 710;

		if (_animationFrame == 0 && _flag) {
			_animationState = 0;
			_var1 = 0;
		} else {
			_animationFrame++;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(710)) {
				_animationFrame = 0;
			}
		}
		break;

	case 3:
		*animation = 711;
		_animationFrame++;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(711)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 710;
		}
		break;

	case 4:
		*animation = 712;
		_animationFrame++;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(712)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 710;
		}
		break;

	case 5:
		*animation = 713;
		_animationFrame++;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(713)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 710;
		}
		break;

	case 6:
		*animation = 714;
		_animationFrame++;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(714)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 710;
		}
		break;

	case 7:
		*animation = 715;
		_animationFrame++;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(715)) {
			_animationFrame = 0;
			_animationState = 2;
			*animation = 710;
		}
		break;

	case 8:
		*animation = 708;
		_animationFrame++;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(708)) {
			*animation = 705;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 9:
		*animation = 709;
		_animationFrame++;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(709)) {
			*animation = 705;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	default:
		break;
	}

	*frame = _animationFrame;

	return true;
}

bool AIScriptHawkersBarkeep::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState >= 2 && _animationState <= 7) {
			_flag = true;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;

	case 3:
		if (_animationState) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 2;
			_animationNext = 710;
		}
		_flag = false;
		break;

	case 12:
		if (_animationState) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 3;
			_animationNext = 711;
		}
		_flag = false;
		break;

	case 13:
		if (_animationState) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 4;
			_animationNext = 712;
		}
		_flag = false;
		break;

	case 14:
		if (_animationState) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 5;
			_animationNext = 713;
		}
		_flag = false;
		break;

	case 15:
		if (_animationState) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 6;
			_animationNext = 714;
		}
		_flag = false;
		break;

	case 16:
		if (_animationState) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 1;
			_animationStateNext = 7;
			_animationNext = 715;
		}
		_flag = false;
		break;

	default:
		break;
	}

	return true;
}

void AIScriptHawkersBarkeep::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptHawkersBarkeep::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptHawkersBarkeep::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptHawkersBarkeep::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
