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

AIScriptFreeSlotA::AIScriptFreeSlotA(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 0;
	_var2 = 1;
	_fallSpeed = 0.0f;
	_fallHeightCurrent = 0.0f; // not initialized in original
	_fallHeightTarget = 0.0f; // not initialized in original
}

void AIScriptFreeSlotA::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 0;
	_var2 = 1;
	_fallSpeed = 0.0f;
	_fallHeightCurrent = 0.0f; // not initialized in original
	_fallHeightTarget = 0.0f; // not initialized in original

	World_Waypoint_Set(525, 45, -780.0f, -615.49f, 2611.0f);
	World_Waypoint_Set(526, 45, -780.0f, -615.49f, 2759.0f);
}

bool AIScriptFreeSlotA::Update() {
	switch (Global_Variable_Query(kVariableChapter)) {
	case 4:
		if (Actor_Query_Which_Set_In(kActorMcCoy) == kSetUG15
		 && Actor_Query_Which_Set_In(kActorFreeSlotA) == kSetUG15
		) {
			int goal = Actor_Query_Goal_Number(kActorFreeSlotA);
			if ((goal == kGoalFreeSlotAUG15RunToOtherSide
			  || goal == kGoalFreeSlotAUG15RunBack
			 )
			 && Actor_Query_Inch_Distance_From_Actor(kActorFreeSlotA, kActorMcCoy) <= 48
			) {
				Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAUG15Attack);
			} else if (goal == kGoalFreeSlotAUG15Fall) {
				float x, y, z;

				Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
				_fallHeightCurrent += _fallSpeed;
				if (_fallHeightTarget < _fallHeightCurrent) {
					_fallSpeed -= 0.2f;
				} else {
					_fallHeightCurrent = _fallHeightTarget;
					Actor_Set_Goal_Number(kActorFreeSlotA, 0);
				}
				Actor_Set_At_XYZ(kActorFreeSlotA, x, _fallHeightCurrent, z, Actor_Query_Facing_1024(kActorFreeSlotA));
			}

			return true;
		}

		switch (Actor_Query_Goal_Number(kActorFreeSlotA)) {
		case kGoalFreeSlotAWalkAround:
			if (Actor_Query_Which_Set_In(kActorFreeSlotA) == Player_Query_Current_Set()
				&& Actor_Query_Inch_Distance_From_Actor(kActorFreeSlotA, kActorMcCoy) <= 48
			) {
				Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAAttackMcCoy);
			}
			break;

		case kGoalFreeSlotAAttackMcCoy:
			if (Actor_Query_Which_Set_In(kActorFreeSlotA) != Player_Query_Current_Set()) {
				Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAWalkAround);
			}
			break;

		case kGoalFreeSlotAGone:
			if (Actor_Query_Which_Set_In(kActorFreeSlotA) != Player_Query_Current_Set()) {
				Game_Flag_Reset(kFlagRatWalkingAround);
				Game_Flag_Reset(kFlagUG15BridgeWillBreak);
				Actor_Set_Goal_Number(kActorFreeSlotA, false);
			}
			break;

		default:
			if (!Game_Flag_Query(kFlagRatWalkingAround)) {
				Game_Flag_Set(kFlagRatWalkingAround);
				Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAWalkAround);
				Actor_Set_Targetable(kActorFreeSlotA, true);
			}
		}

		return true;

	case 5:
		if (Actor_Query_Goal_Number(kActorFreeSlotA) < 400) {
			AI_Movement_Track_Flush(kActorFreeSlotA);
			Actor_Set_Goal_Number(kActorFreeSlotA, 400);
		} else if (Actor_Query_Goal_Number(kActorFreeSlotA) == 405
		        && Actor_Query_Which_Set_In(kActorMcCoy) == kSceneKP05
		) {
			Actor_Set_Targetable(kActorFreeSlotA, 1);
			Actor_Set_Goal_Number(kActorFreeSlotA, 406);
		}
		return true;

	default:
		return false;
	}
}

void AIScriptFreeSlotA::TimerExpired(int timer) {
	//return false;
}

void AIScriptFreeSlotA::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorFreeSlotA)) {
	case kGoalFreeSlotAUG15WalkOut:
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAUG15RunToOtherSide);
		break;

	case kGoalFreeSlotAUG15RunToOtherSide:
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAUG15RunBack);
		break;

	case kGoalFreeSlotAUG15RunBack:
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAUG15Wait);
		break;

	case kGoalFreeSlotAWalkAround:
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAWalkAroundRestart);
		break;

	case kGoalFreeSlotAWalkAroundRestart:
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAWalkAround);
		break;

	case 400:
		Actor_Set_Goal_Number(kActorFreeSlotA, 405);
		break;

	case 406:
		Non_Player_Actor_Combat_Mode_On(kActorFreeSlotA, 0, 0, 0, 8, 4, 7, 8, 0, 0, 100, 5, 300, 0);
		break;

	default:
		return; //false;
	}

	return; //true;
}

void AIScriptFreeSlotA::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptFreeSlotA::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorFreeSlotA) == kGoalFreeSlotAGone) {
		Actor_Face_Actor(kActorMcCoy, kActorFreeSlotA, true);
		if (Random_Query(1, 2) == 1) {
			Actor_Says(kActorMcCoy, 8655, 16);
		} else {
			Actor_Says(kActorMcCoy, 8665, 16);
		}
	}
	//return false;
}

void AIScriptFreeSlotA::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptFreeSlotA::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptFreeSlotA::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptFreeSlotA::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptFreeSlotA::ShotAtAndMissed() {
	if (Actor_Query_In_Set(kActorFreeSlotA, kSetUG15)) {
		checkIfOnBridge();
	}
}

bool AIScriptFreeSlotA::ShotAtAndHit() {
	if (Actor_Query_In_Set(kActorFreeSlotA, kSetUG15)) {
		checkIfOnBridge();
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAUG15Die);
		return true;
	}

	return false;
}

void AIScriptFreeSlotA::Retired(int byActorId) {
	Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAGone);
}

int AIScriptFreeSlotA::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptFreeSlotA::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case kGoalFreeSlotAUG15Wait:
		AI_Movement_Track_Flush(kActorFreeSlotA);
		Actor_Change_Animation_Mode(kActorFreeSlotA, kAnimationModeIdle);
		Actor_Set_Targetable(kActorFreeSlotA, false);
		break;

	case kGoalFreeSlotAUG15WalkOut:
		Actor_Force_Stop_Walking(kActorMcCoy);
		AI_Movement_Track_Flush(kActorFreeSlotA);
		World_Waypoint_Set(444, 87, -48.75f, 44.66f, 87.57f);
		AI_Movement_Track_Append(kActorFreeSlotA, 444, 1);
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		break;

	case kGoalFreeSlotAUG15RunToOtherSide:
		AI_Movement_Track_Flush(kActorFreeSlotA);
		World_Waypoint_Set(444, 87, -237.0f, 48.07f, 208.0f);
		AI_Movement_Track_Append(kActorFreeSlotA, 444, 1);
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		Actor_Set_Targetable(kActorFreeSlotA, true);
		break;

	case kGoalFreeSlotAUG15RunBack:
		AI_Movement_Track_Flush(kActorFreeSlotA);
		World_Waypoint_Set(444, 87, 3.52f, 52.28f, 90.68f);
		AI_Movement_Track_Append(kActorFreeSlotA, 444, 0);
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		break;

	case kGoalFreeSlotAUG15Attack:
		Player_Loses_Control();
		Actor_Force_Stop_Walking(kActorMcCoy);
		AI_Movement_Track_Flush(kActorFreeSlotA);
		Actor_Face_Actor(kActorFreeSlotA, kActorMcCoy, true);
		Actor_Change_Animation_Mode(kActorFreeSlotA, kAnimationModeCombatAttack);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeDie);
		break;

	case kGoalFreeSlotAUG15Die:
		AI_Movement_Track_Flush(kActorFreeSlotA);
		Actor_Set_Targetable(kActorFreeSlotA, false);
		Game_Flag_Set(kFlagUG15RatShot);
		_animationState = 7;
		_animationFrame = 0;
		break;

	case kGoalFreeSlotAWalkAround:
		AI_Movement_Track_Flush(kActorFreeSlotA);
		goToRandomUGxx();
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		break;

	case kGoalFreeSlotAWalkAroundRestart:
		AI_Movement_Track_Flush(kActorFreeSlotA);
		AI_Movement_Track_Append(kActorFreeSlotA, 39, 1);
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		break;

	case kGoalFreeSlotAAttackMcCoy:
		Actor_Set_Targetable(kActorFreeSlotA, true);
		Non_Player_Actor_Combat_Mode_On(kActorFreeSlotA, 0, 0, 0, 8, 4, 7, 8, 25, 0, 75, 5, 300, 0);
		break;

	case kGoalFreeSlotAUG15Fall:
		Actor_Force_Stop_Walking(kActorFreeSlotA);
		AI_Movement_Track_Flush(kActorFreeSlotA);
		_fallHeightCurrent = 52.46f;
		_fallSpeed = -4.0f;
		_fallHeightTarget = -10.0f;
		if (_animationState != 7
		 && _animationState != 8
		) {
			_animationState = 7;
			_animationFrame = 0;
		}
		break;

	case kGoalFreeSlotAUG15Prepare:
		AI_Movement_Track_Flush(kActorFreeSlotA);
		Actor_Put_In_Set(kActorFreeSlotA, kSetUG15);
		Actor_Set_At_XYZ(kActorFreeSlotA, 3.52f, 52.28f, 90.68f, 700);
		Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAUG15Wait);
		break;

	case 400:
		AI_Movement_Track_Append(kActorFreeSlotA, 39, 0);
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		break;

	case 406:
		AI_Movement_Track_Flush(kActorFreeSlotA);
		AI_Movement_Track_Append(kActorFreeSlotA, 525, 0);
		AI_Movement_Track_Repeat(kActorFreeSlotA);
		break;

	case kGoalFreeSlotAGone:
		Actor_Set_Health(kActorFreeSlotA, 20, 20);
		Actor_Set_Friendliness_To_Other(kActorFreeSlotA, kActorMcCoy, 40);
		break;

	default:
		return false;
	}

	return true;
}

bool AIScriptFreeSlotA::UpdateAnimation(int *animation, int *frame) {
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
		if (_animationFrame == 1) {
			int snd;
			if (Random_Query(1, 2) == 1) {
				snd = 9010;
			} else {
				snd = 9015;
			}
			Sound_Play_Speech_Line(64, snd, 75, 0, 99);
		}
		if (_animationFrame == 3) {
			Ambient_Sounds_Play_Sound(438, 99, 0, 0, 20);
			Actor_Combat_AI_Hit_Attempt(kActorFreeSlotA);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(857)) {
			_animationState = 0;
			_animationFrame = 0;
			Actor_Change_Animation_Mode(kActorFreeSlotA, kAnimationModeCombatIdle);
		}
		break;

	case 5:
		*animation = 874;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(874) - 1) { // bug? shuld not be '-1'
			Actor_Change_Animation_Mode(kActorFreeSlotA, kAnimationModeIdle);
		}
		break;

	case 6:
		if (_animationFrame == 1) {
			Ambient_Sounds_Play_Sound(437, 99, 0, 0, 20);
		}
		*animation = 860;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(860)) {
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorFreeSlotA, kAnimationModeIdle);
		}
		break;

	case 7:
		*animation = 859;
		_animationFrame++;
		if (_animationFrame == 0) {
			Ambient_Sounds_Play_Sound(439, 99, 0, 0, 25);
		}
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(859) - 1) { // bug? shuld not be '-1'
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(859) - 1;
			_animationState = 8;
			Actor_Set_Goal_Number(kActorFreeSlotA, kGoalFreeSlotAGone);
		}
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

bool AIScriptFreeSlotA::ChangeAnimationMode(int mode) {
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

void AIScriptFreeSlotA::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptFreeSlotA::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptFreeSlotA::ReachedMovementTrackWaypoint(int waypointId) {
	if (Actor_Query_Which_Set_In(kActorFreeSlotA) == kSetUG01) {
		if (waypointId == 465) {
			Actor_Change_Animation_Mode(kActorFreeSlotA, 43);
		}
	} else if (Actor_Query_Goal_Number(kActorFreeSlotA) == 302) {
		Actor_Face_Actor(kActorFreeSlotA, kActorMcCoy, true);
	}

	return true;
}

void AIScriptFreeSlotA::FledCombat() {
	// return false;
}

void AIScriptFreeSlotA::checkIfOnBridge() {
	float x, y, z;
	Actor_Query_XYZ(kActorFreeSlotA, &x, &y, &z);
	// bug? this should probably check if McCoy is close enough because bridge will break long after rat died and player tries to walk through
	if (-150.0 <= x && x < -30.0f) {
		Game_Flag_Set(kFlagUG15BridgeWillBreak);
	}
}

void AIScriptFreeSlotA::goToRandomUGxx() {
	switch (Random_Query(1, 14)) {
	case 1:
		AI_Movement_Track_Append(kActorFreeSlotA, 450, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 451, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 450, 0);
		break;

	case 2:
		World_Waypoint_Set(463, 74, 144.98f, -50.13f, -175.75f);
		World_Waypoint_Set(464, 74, 105.6f, -50.13f, -578.46f);
		World_Waypoint_Set(465, 74, 62.0f, -50.13f, -574.0f);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 465, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 5);
		break;

	case 3:
		AI_Movement_Track_Append(kActorFreeSlotA, 446, 15);
		AI_Movement_Track_Append(kActorFreeSlotA, 447, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 449, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 448, 2);
		AI_Movement_Track_Append(kActorFreeSlotA, 449, 0);
		break;

	case 4:
		World_Waypoint_Set(463, 77, -22.7f, 6.39f, 33.12f);
		World_Waypoint_Set(464, 77, -6.70f, -1.74f, -362.88f);
		World_Waypoint_Set(465, 77, 164.0f, 11.87f, -1013.0f);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 2);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 0);
		AI_Movement_Track_Append(kActorFreeSlotA, 465, 0);
		break;

	case 5:
		AI_Movement_Track_Append(kActorFreeSlotA, 457, 15);
		AI_Movement_Track_Append(kActorFreeSlotA, 458, 0);
		AI_Movement_Track_Append(kActorFreeSlotA, 459, 15);
		break;

	case 6:
		AI_Movement_Track_Append(kActorFreeSlotA, 460, 15);
		AI_Movement_Track_Append(kActorFreeSlotA, 461, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 460, 15);
		break;

	case 7:
		if (Actor_Query_In_Set(kActorClovis, kSetUG07)) {
			AI_Movement_Track_Append(kActorFreeSlotA, 39, 10);
		} else {
			World_Waypoint_Set(463, 80, -88.78f, -12.21f, -184.08f);
			World_Waypoint_Set(464, 80, 250.0f, -12.21f, -342.0f);
			World_Waypoint_Set(465, 80, -164.78f, -12.21f, -832.08f);
			AI_Movement_Track_Append(kActorFreeSlotA, 463, 5);
			AI_Movement_Track_Append(kActorFreeSlotA, 464, 1);
		}
		break;

	case 8:
		World_Waypoint_Set(463, 80, -88.78f, -12.21f, -184.08f);
		World_Waypoint_Set(464, 80, 250.0f, -12.21f, -342.0f);
		World_Waypoint_Set(465, 80, -164.78f, -12.21f, -832.08f);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
		break;

	case 9:
		World_Waypoint_Set(463, 80, -88.78f, -12.21f, -184.08f);
		World_Waypoint_Set(464, 80, 250.0f, -12.21f, -342.0f);
		World_Waypoint_Set(465, 80, -164.78f, -12.21f, -832.08f);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 465, 1);
		break;

	case 10:
		World_Waypoint_Set(463, 80, -88.78f, -12.21f, -184.08f);
		World_Waypoint_Set(464, 80, 250.0f, -12.21f, -342.0f);
		World_Waypoint_Set(465, 80, -164.78f, -12.21f, -832.08f);
		AI_Movement_Track_Append(kActorFreeSlotA, 465, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 1);
		break;

	case 11:
		World_Waypoint_Set(463, 82, 91.0f, 156.94f, -498.0f);
		World_Waypoint_Set(464, 82, -149.0f, 156.94f, -498.0f);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 1);
		break;

	case 12:
		World_Waypoint_Set(463, 82, 91.0f, 156.94f, -498.0f);
		World_Waypoint_Set(464, 82, -149.0f, 156.94f, -498.0f);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 5);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
		break;

	case 13:
		World_Waypoint_Set(463, 82, -152.51f, 277.31f, 311.98f);
		World_Waypoint_Set(464, 82, -124.51f, 275.08f, 319.98f);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 8);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
		break;

	case 14:
		World_Waypoint_Set(463, 84, -360.67f, 21.39f, 517.55f);
		World_Waypoint_Set(464, 84, -250.67f, 21.39f, 477.55f);
		World_Waypoint_Set(465, 84, -248.67f, 21.39f, -1454.45f);
		AI_Movement_Track_Append(kActorFreeSlotA, 463, 1);
		AI_Movement_Track_Append(kActorFreeSlotA, 464, 8);
		AI_Movement_Track_Append(kActorFreeSlotA, 465, 1);
		break;

	default:
		AI_Movement_Track_Append(kActorFreeSlotA, 39, Random_Query(1, 10));
		break;
	}
}

} // End of namespace BladeRunner
