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

AIScriptFreeSlotB::AIScriptFreeSlotB(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 0;
	_var2 = 1;
}

void AIScriptFreeSlotB::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 0;
	_var2 = 1;

	World_Waypoint_Set(527, 45, -468.46f, -616.58f, 2840.60f);
	World_Waypoint_Set(528, 45, -1024.46f, -615.49f, 2928.60f);
	World_Waypoint_Set(529, 45, -1024.46f, -615.49f, 2788.60f);
}

bool AIScriptFreeSlotB::Update() {
	if (Global_Variable_Query(kVariableChapter) > 5) {
		return false;
	}

	if (Global_Variable_Query(kVariableChapter) == 4) {
		switch (Actor_Query_Goal_Number(kActorFreeSlotB)) {
		case 300:
			Actor_Set_Goal_Number(kActorFreeSlotB, 301);
			Actor_Set_Targetable(kActorFreeSlotB, 1);
			break;

		case 301:
			if (Actor_Query_Which_Set_In(kActorFreeSlotB) == Player_Query_Current_Set()
					&& Actor_Query_Inch_Distance_From_Actor(kActorFreeSlotB, kActorMcCoy) <= 48) {
				Actor_Set_Goal_Number(kActorFreeSlotB, 302);
			}
			break;

		case 302:
			if (Actor_Query_Which_Set_In(kActorFreeSlotB) != Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorFreeSlotB, 301);
			}
			break;

		case 599:
			if (Actor_Query_Which_Set_In(kActorFreeSlotB) != Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorFreeSlotB, 300);
			}
			break;

		default:
			Actor_Set_Goal_Number(kActorFreeSlotB, 300);
			break;
		}

		return false;
	}
	if (Actor_Query_Goal_Number(kActorFreeSlotB) < 400) {
		AI_Movement_Track_Flush(kActorFreeSlotB);
		Actor_Set_Goal_Number(kActorFreeSlotB, 400);
		return true;
	} else {
		if (Actor_Query_Goal_Number(kActorFreeSlotB) != 405 || Actor_Query_Which_Set_In(kActorMcCoy) != kSetKP02) {
			if (Actor_Query_Goal_Number(kActorFreeSlotB) == 599) {
				if (Actor_Query_Which_Set_In(kActorFreeSlotB) != Player_Query_Current_Set()) {
					Non_Player_Actor_Combat_Mode_Off(kActorFreeSlotB);
					Actor_Set_Goal_Number(kActorFreeSlotB, 400);
					return true;
				}
			}
			return false;
		}
		Actor_Set_Goal_Number(kActorFreeSlotB, 406);
		Actor_Set_Targetable(kActorFreeSlotB, 1);
		return true;
	}
}

void AIScriptFreeSlotB::TimerExpired(int timer) {
	//return false;
}

void AIScriptFreeSlotB::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorFreeSlotB)) {
	case 300:
		Actor_Set_Goal_Number(kActorFreeSlotB, 301);
		break;

	case 301:
		Actor_Set_Goal_Number(kActorFreeSlotB, 300);
		break;

	case 400:
		Actor_Set_Goal_Number(kActorFreeSlotB, 405);
		break;

	case 406:
		Non_Player_Actor_Combat_Mode_On(kActorFreeSlotB, 0, 0, 0, 8, 4, 7, 8, 0, 0, 100, 5, 300, 0);
		break;

	default:
		return; //false;
	}

	return; //true;
}

void AIScriptFreeSlotB::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptFreeSlotB::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorFreeSlotB) != 599)
		return; //false;

	Actor_Face_Actor(kActorMcCoy, kActorFreeSlotB, 1);
	if (Random_Query(1, 2) == 1) {
		Actor_Says(kActorMcCoy, 8655, 16);
	} else {
		Actor_Says(kActorMcCoy, 8665, 16);
	}
}

void AIScriptFreeSlotB::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptFreeSlotB::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptFreeSlotB::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptFreeSlotB::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptFreeSlotB::ShotAtAndMissed() {
	// return false;
}

bool AIScriptFreeSlotB::ShotAtAndHit() {
	return false;
}

void AIScriptFreeSlotB::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorFreeSlotB, 599);
}

int AIScriptFreeSlotB::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptFreeSlotB::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 300:
		AI_Movement_Track_Flush(kActorFreeSlotB);
		AI_Movement_Track_Append(kActorFreeSlotB, 39, 2);
		AI_Movement_Track_Repeat(kActorFreeSlotB);
		break;

	case 301:
		AI_Movement_Track_Flush(kActorFreeSlotB);
		processGoal301();
		AI_Movement_Track_Repeat(kActorFreeSlotB);
		break;

	case 302:
		Actor_Set_Targetable(kActorFreeSlotB, 1);
		Non_Player_Actor_Combat_Mode_On(kActorFreeSlotB, 0, 0, 0, 8, 4, 7, 8, 25, 0, 75, 5, 300, 0);
		break;

	case 400:
		AI_Movement_Track_Append(kActorFreeSlotB, 39, 0);
		AI_Movement_Track_Repeat(kActorFreeSlotB);
		break;

	case 406:
		AI_Movement_Track_Flush(kActorFreeSlotB);
		AI_Movement_Track_Append(kActorFreeSlotB, 527, 0);
		AI_Movement_Track_Repeat(kActorFreeSlotB);
		break;

	case 599:
		Actor_Set_Health(kActorFreeSlotB, 20, 20);
		break;

	default:
		return false;
	}

	return true;
}

bool AIScriptFreeSlotB::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = 861;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(861)) {
			_animationFrame = 0;
		}
		break;

	case 1:
		*animation = 862;
		if (_var1) {
			_var1--;
		} else {
			_animationFrame += _var2;
			if (_animationFrame < 8) {
				_var2 = 1;
			} else {
				if (_animationFrame > 8) {
					_var2 = -1;
				} else if (Random_Query(0, 4)) {
					_var2 = -_var2;
				}
			}
			if (_animationFrame >= 7 && _animationFrame <= 9) {
				_var1 = Random_Query(0, 1);
			}
		}
		break;

	case 2:
		*animation = 862;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(862) - 1) {
			*animation = 861;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 3:
		*animation = 858;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(858)) {
			_animationFrame = 0;
		}
		break;

	case 4:
		*animation = 857;
		_animationFrame++;
		if (_animationFrame == 3) {
			int snd;
			if (Random_Query(1, 2) == 1) {
				snd = 9010;
			} else {
				snd = 9015;
			}
			Sound_Play_Speech_Line(kActorFreeSlotB, snd, 75, 0, 99);
		}
		if (_animationFrame == 3) {
			Actor_Combat_AI_Hit_Attempt(kActorFreeSlotB);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = 861;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 5:
		*animation = 874;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(874) - 1) {
			_animationState = 8;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(874) - 1;
		}
		break;

	case 6:
		if (_animationFrame == 1) {
			Ambient_Sounds_Play_Sound(kSfxRATTY3, 99, 0, 0, 20);
		}
		*animation = 860;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(860)) {
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorFreeSlotB, 0);
		}
		break;

	case 7:
		*animation = 859;
		_animationFrame++;
		if (_animationFrame == 1) {
			Ambient_Sounds_Play_Sound(kSfxRATTY5, 99, 0, 0, 25);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(859)) {
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(859);
		}
		_animationState = 8;
		break;

	case 8:
		*animation = 859;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(859) - 1;
		break;

	default:
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptFreeSlotB::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if ((unsigned int)(_animationState - 1) > 1) {
			_animationState = 0;
			_animationFrame = 0;
		} else if (_animationState == 1) {
			_animationState = 2;
		}
		break;

	case 1:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 4:
		if ((unsigned int)(_animationState - 1) > 1) {
			_animationState = 0;
			_animationFrame = 0;
		} else if (_animationState == 1) {
			_animationState = 2;
		}
		break;

	case 6:
		_animationState = 4;
		_animationFrame = 0;
		break;

	case 7:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 8:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 21:
		_animationState = 6;
		_animationFrame = 0;
		break;

	case 43:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case 48:
		_animationState = 7;
		_animationFrame = 0;
		break;
	}

	return true;
}

void AIScriptFreeSlotB::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptFreeSlotB::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptFreeSlotB::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptFreeSlotB::FledCombat() {
	// return false;
}

void AIScriptFreeSlotB::processGoal301() {
	switch (Random_Query(1, 14)) {
	case 1:
		AI_Movement_Track_Append(kActorFreeSlotB, 450, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 451, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 450, 0);
		break;

	case 2:
		World_Waypoint_Set(466, 74, 144.98f, -50.13f, -175.75f);
		World_Waypoint_Set(547, 74, 105.6f, -50.13f, -578.46f);
		World_Waypoint_Set(548, 74, 62.0f, -50.13f, -574.0f);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 548, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 5);
		break;

	case 3:
		AI_Movement_Track_Append(kActorFreeSlotB, 446, 15);
		AI_Movement_Track_Append(kActorFreeSlotB, 447, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 449, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 448, 2);
		AI_Movement_Track_Append(kActorFreeSlotB, 449, 0);
		break;

	case 4:
		World_Waypoint_Set(466, 77, -22.70f, 6.39f, 33.12f);
		World_Waypoint_Set(547, 77, -6.70f, -1.74f, -362.88f);
		World_Waypoint_Set(548, 77, 164.0f, 11.87f, -1013.0f);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 2);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 0);
		AI_Movement_Track_Append(kActorFreeSlotB, 548, 0);
		break;

	case 5:
		AI_Movement_Track_Append(kActorFreeSlotB, 457, 15);
		AI_Movement_Track_Append(kActorFreeSlotB, 458, 0);
		AI_Movement_Track_Append(kActorFreeSlotB, 459, 15);
		break;

	case 6:
		AI_Movement_Track_Append(kActorFreeSlotB, 460, 15);
		AI_Movement_Track_Append(kActorFreeSlotB, 461, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 460, 15);
		break;

	case 7:
		if (Actor_Query_In_Set(kActorClovis, kSetUG07)) {
			AI_Movement_Track_Append(kActorFreeSlotB, 39, 10);
		} else {
			World_Waypoint_Set(466, 80, -88.78f, -12.21f, -184.08f);
			World_Waypoint_Set(547, 80, 250.0f, -12.21f, -342.0f);
			World_Waypoint_Set(548, 80, -164.78f, -12.21f, -832.08f);
			AI_Movement_Track_Append(kActorFreeSlotB, 466, 5);
			AI_Movement_Track_Append(kActorFreeSlotB, 547, 1);
		}
		break;

	case 8:
		World_Waypoint_Set(466, 80, -88.78f, -12.21f, -184.08f);
		World_Waypoint_Set(547, 80, 250.0f, -12.21f, -342.0f);
		World_Waypoint_Set(548, 80, -164.78f, -12.21f, -832.08f);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
		break;

	case 9:
		World_Waypoint_Set(466, 80, -88.78f, -12.21f, -184.08f);
		World_Waypoint_Set(547, 80, 250.0f, -12.21f, -342.0f);
		World_Waypoint_Set(548, 80, -164.78f, -12.21f, -832.08f);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 548, 1);
		break;

	case 10:
		World_Waypoint_Set(466, 80, -88.78f, -12.21f, -184.08f);
		World_Waypoint_Set(547, 80, 250.0f, -12.21f, -342.0f);
		World_Waypoint_Set(548, 80, -164.78f, -12.21f, -832.08f);
		AI_Movement_Track_Append(kActorFreeSlotB, 548, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 1);
		break;

	case 11:
		World_Waypoint_Set(466, 82, 91.0f, 156.94f, -498.0f);
		World_Waypoint_Set(547, 82, -149.0f, 156.94f, -498.0f);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 1);
		break;

	case 12:
		World_Waypoint_Set(466, 82, 91.0f, 156.94f, -498.0f);
		World_Waypoint_Set(547, 82, -149.0f, 156.94f, -498.0f);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 5);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
		break;

	case 13:
		World_Waypoint_Set(466, 82, -152.51f, 277.31f, 311.98f);
		World_Waypoint_Set(547, 82, -124.51f, 275.08f, 319.98f);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 8);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
		break;

	case 14:
		World_Waypoint_Set(466, 84, -360.67f, 21.39f, 517.55f);
		World_Waypoint_Set(547, 84, -250.67f, 21.39f, 477.55f);
		World_Waypoint_Set(548, 84, -248.67f, 21.39f, -1454.45f);
		AI_Movement_Track_Append(kActorFreeSlotB, 466, 1);
		AI_Movement_Track_Append(kActorFreeSlotB, 547, 8);
		AI_Movement_Track_Append(kActorFreeSlotB, 548, 1);
		break;

	default:
		AI_Movement_Track_Append(kActorFreeSlotB, 39, Random_Query(1, 10));
		break;
	}
}

} // End of namespace BladeRunner
