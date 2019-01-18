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

AIScriptHasan::AIScriptHasan(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 6;
	_var2 = 1;
	_var3 = 0;
	_var4 = 0;
	_var5 = 0;
	_var6 = 0;
}

void AIScriptHasan::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 6;
	_var2 = 1;
	_var3 = 0;
	_var4 = 0;
	_var5 = 0;
	_var6 = 0;

	Actor_Put_In_Set(kActorHasan, kSetAR01_AR02);
	Actor_Set_At_XYZ(kActorHasan, -214.0f, 0.0f, -1379.0f, 371);
	Actor_Set_Goal_Number(kActorHasan, 0);
}

bool AIScriptHasan::Update() {
	if (Global_Variable_Query(kVariableChapter) != 3 || Actor_Query_Goal_Number(kActorHasan) >= 300)
		return false;

	Actor_Set_Goal_Number(kActorHasan, 300);
	return true;
}

void AIScriptHasan::TimerExpired(int timer) {
	//return false;
}

void AIScriptHasan::CompletedMovementTrack() {
	//return false;
}

void AIScriptHasan::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptHasan::ClickedByPlayer() {
	//return false;
}

void AIScriptHasan::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptHasan::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptHasan::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptHasan::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptHasan::ShotAtAndMissed() {
	// return false;
}

bool AIScriptHasan::ShotAtAndHit() {
	return false;
}

void AIScriptHasan::Retired(int byActorId) {
	// return false;
}

int AIScriptHasan::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptHasan::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == 300) {
		Actor_Put_In_Set(kActorHasan, kSetFreeSlotH);
		Actor_Set_At_Waypoint(kActorHasan, 40, 0);
	}
	return false;
}

bool AIScriptHasan::UpdateAnimation(int *animation, int *frame) {
	if (_var4) {
		_var4--;
	}
	if (_var5) {
		_var5--;
	}

	switch (_animationState) {
	case 0:
		if (_var6 == 1) {
			*animation = 922;
			if (_var3) {
				_var3--;
			} else {
				_animationFrame++;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(922)) {
					_animationFrame = 0;
					_var6 = 0;
					*animation = 921;
					_var1 = Random_Query(6, 14);
					_var2 = 2 * Random_Query(0, 1) - 1;
					_var4 = Random_Query(40, 60);
				}
				if (_animationFrame >= 10 && _animationFrame <= 14) {
					_var3 = Random_Query(0, 1);
				}
			}
		} else if (_var6 == 2) {
			*animation = 923;
			if (_var3) {
				_var3--;
			} else {
				_animationFrame++;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(923)) {
					_animationFrame = 0;
					_var6 = 0;
					*animation = 921;
					_var1 = Random_Query(6, 14);
					_var2 = 2 * Random_Query(0, 1) - 1;
					_var5 = Random_Query(40, 60);
				}
				if (_animationFrame == 14) {
					_var3 = Random_Query(3, 10);
				}
				if (_animationFrame == 23) {
					_var3 = Random_Query(0, 4);
				}
			}
		} else if (_var6 == 0) {
			*animation = 921;
			if (_var3) {
				_var3--;
			} else {
				_animationFrame += _var2;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(921)) {
					_animationFrame = 0;
				}
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(921) - 1;
				}
				if (!--_var1) {
					_var2 = 2 * Random_Query(0, 1) - 1;
					_var1 = Random_Query(6, 14);
					_var3 = Random_Query(0, 4);
				}
				if (!_animationFrame) {
					_var6 = Random_Query(0, 2);
				}
				if (_var6 == 1 && _var4) {
					_var6 = 0;
				}
				if (_var6 == 2 && _var5) {
					_var6 = 0;
				}
			}
		}
		break;

	case 1:
		*animation = 925;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(925)) {
			_animationFrame = 0;
		}
		break;

	case 2:
		*animation = 926;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(926)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = 925;
		}
		break;

	case 3:
		*animation = 927;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(927)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = 925;
		}
		break;

	case 4:
		*animation = 928;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(928)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = 925;
		}
		break;

	case 5:
		*animation = 929;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(929)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = 925;
		}
		break;

	case 6:
		*animation = 930;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(930)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = 925;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptHasan::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		_animationState = 0;
		_var6 = 0;
		_animationFrame = 0;
		break;

	case 3:
		_animationState = 1;
		_var6 = 0;
		_animationFrame = 0;
		break;

	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
		_animationState = 6;
		_var6 = 0;
		_animationFrame = 0;
		break;

	default:
		break;
	}
	return true;
}

void AIScriptHasan::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptHasan::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptHasan::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptHasan::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
