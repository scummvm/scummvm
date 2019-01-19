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

AIScriptHanoi::AIScriptHanoi(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_var1 = 0;
	_var2 = 0;
	_var3 = 0;
	_var4 = 1;
}

void AIScriptHanoi::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_var1 = 0;
	_var2 = 0;
	_var3 = 0;
	_var4 = 1;

	Actor_Set_Goal_Number(kActorHanoi, 0);
}

bool AIScriptHanoi::Update() {
	if (Actor_Query_Goal_Number(kActorHolloway) == 240) {
		AI_Countdown_Timer_Reset(kActorHanoi, 0);
	}
	if (Global_Variable_Query(kVariableChapter) == 3 && Actor_Query_Goal_Number(kActorHanoi) < 200) {
		Actor_Set_Goal_Number(kActorHanoi, 210);
	}
	if (Player_Query_Current_Scene() != 56 && Actor_Query_Goal_Number(kActorHanoi) == 236) {
		Actor_Set_Goal_Number(kActorHanoi, 210);
	}
	if (Player_Query_Current_Scene() == 56
			&& Actor_Query_Goal_Number(kActorHanoi) != 215
			&& Actor_Query_Goal_Number(kActorHanoi) != 230
			&& Actor_Query_Goal_Number(kActorHanoi) != 235
			&& Actor_Query_Goal_Number(kActorHanoi) != 236) {
		if (Actor_Query_Inch_Distance_From_Waypoint(kActorMcCoy, 364) < 420) {
			if (Actor_Query_Goal_Number(kActorHanoi) == 210) {
				Actor_Set_Goal_Number(kActorHanoi, 211);
			}
		} else if (Actor_Query_Goal_Number(kActorHanoi) == 211) {
			Actor_Set_Goal_Number(kActorHanoi, 210);
		}
		if (Actor_Query_Inch_Distance_From_Waypoint(kActorMcCoy, 361) < 240) {
			if (Actor_Query_Goal_Number(kActorHanoi) == 210) {
				Actor_Set_Goal_Number(kActorHanoi, 212);
			}
		} else if (Actor_Query_Goal_Number(kActorHanoi) == 212) {
			Actor_Set_Goal_Number(kActorHanoi, 210);
		}
		if (Actor_Query_Inch_Distance_From_Actor(kActorMcCoy, kActorHysteriaPatron1) < 120
				&& Actor_Query_Which_Set_In(kActorHanoi) == 55
				&& Actor_Query_Goal_Number(kActorHanoi) != 213) {
			Actor_Set_Goal_Number(kActorHanoi, 213);
		}
	}

	return false;
}

void AIScriptHanoi::TimerExpired(int timer) {
	if (timer == 0) {
		if (Actor_Query_Goal_Number(kActorHanoi) == 215) {
			Actor_Set_Goal_Number(kActorHanoi, 210);
			return; //true;
		}

		if (Actor_Query_Goal_Number(kActorHanoi) == 220)
			return; //false;

		Actor_Set_Goal_Number(kActorHanoi, 202);
		return; //true;
	}
	return; //false;
}

void AIScriptHanoi::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorHanoi)) {
	case 235:
		Actor_Set_Goal_Number(kActorHanoi, 236);
		break;

	case 240:
		Actor_Set_Goal_Number(kActorHanoi, 241);
		break;

	case 202:
		Actor_Says(kActorHanoi, 130, 3);
		Actor_Says(kActorDektora, 540, 30);
		Actor_Set_Goal_Number(kActorHanoi, 203);
		break;

	case 203:
		Actor_Face_Actor(kActorHanoi, kActorMcCoy, 1);
		Actor_Face_Actor(kActorMcCoy, kActorHanoi, 1);
		Actor_Change_Animation_Mode(kActorHanoi, 23);
		Actor_Set_Invisible(kActorMcCoy, 1);
		Actor_Says(kActorMcCoy, 3595, 3);
		Actor_Says(kActorHanoi, 140, 3);
		Actor_Set_Goal_Number(kActorHanoi, 220);
		break;

	case 213:
		Actor_Set_Goal_Number(kActorHanoi, 210);
		break;

	default:
		return; //false;
	}

	return; //true;
}

void AIScriptHanoi::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptHanoi::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorHanoi) == 230 || Actor_Query_Goal_Number(kActorHanoi) == 235) {
		Actor_Face_Actor(kActorMcCoy, kActorHanoi, 1);
		Actor_Says(kActorMcCoy, 8915, 11);

		if (Actor_Query_Goal_Number(kActorHanoi) == 230) {
			Actor_Says(kActorHanoi, 210, 3);
		}
	}
}

void AIScriptHanoi::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptHanoi::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptHanoi::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptHanoi::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (Player_Query_Current_Scene() != 56 || otherActorId || combatMode != 1) {
		return; //false;
	}
	Player_Set_Combat_Mode(0);
	Player_Loses_Control();
	Actor_Set_Goal_Number(kActorHanoi, 220);

	return; //true;
}

void AIScriptHanoi::ShotAtAndMissed() {
	// return false;
}

bool AIScriptHanoi::ShotAtAndHit() {
	return false;
}

void AIScriptHanoi::Retired(int byActorId) {
	// return false;
}

int AIScriptHanoi::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptHanoi::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (!newGoalNumber) {
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append(kActorHanoi, 39, 0);
		AI_Movement_Track_Repeat(kActorHanoi);

		return true;
	}

	switch (newGoalNumber) {
	case 200:
		AI_Countdown_Timer_Start(kActorHanoi, 0, 45);
		break;

	case 201:
		AI_Countdown_Timer_Reset(kActorHanoi, 0);
		break;

	case 202:
		if (Actor_Query_Which_Set_In(kActorMcCoy) == kSetNR07 && Actor_Query_In_Set(kActorDektora, kSetNR07)) {
			Player_Loses_Control();
			Actor_Put_In_Set(kActorHanoi, kSetNR07);
			Actor_Set_At_XYZ(kActorHanoi, -102.0f, -73.5f, -233.0f, 0);
			Async_Actor_Walk_To_Waypoint(kActorMcCoy, 338, 0, 0);
			AI_Movement_Track_Flush(kActorHanoi);
			AI_Movement_Track_Append(kActorHanoi, 336, 1);
			AI_Movement_Track_Repeat(kActorHanoi);
		} else {
			Actor_Set_Goal_Number(kActorHanoi, 0);
		}
		break;

	case 203:
		if (Actor_Query_Which_Set_In(kActorMcCoy) != kSetNR07) {
			return false;
		}
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append(kActorHanoi, 337, 0);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case 204:
		Actor_Says(kActorHanoi, 210, 3);
		Actor_Change_Animation_Mode(kActorHanoi, 23);
		break;

	case 210:
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append_With_Facing(kActorHanoi, 362, 0, 300);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case 211:
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append_With_Facing(kActorHanoi, 363, 0, 500);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case 212:
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append_With_Facing(kActorHanoi, 361, 0, 457);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case 213:
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append_With_Facing(kActorHanoi, 365, Random_Query(15, 20), 600);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case 215:
		Actor_Put_In_Set(kActorHanoi, kSetNR03);
		Actor_Set_At_Waypoint(kActorHanoi, 362, 300);
		AI_Countdown_Timer_Reset(kActorHanoi, 0);
		AI_Countdown_Timer_Start(kActorHanoi, 0, 6);
		break;

	case 220:
		Game_Flag_Set(604);
		AI_Countdown_Timer_Reset(kActorHanoi, 0);
		Player_Loses_Control();
		Player_Set_Combat_Mode(0);
		Actor_Force_Stop_Walking(kActorMcCoy);
		Actor_Change_Animation_Mode(kActorMcCoy, 48);
		Actor_Set_Invisible(kActorMcCoy, 1);
		AI_Movement_Track_Flush(kActorHanoi);
		Actor_Put_In_Set(kActorHanoi, kSetNR01);
		Actor_Set_At_XYZ(kActorHanoi, -444.0f, 24.0f, -845.0f, 512);
		Actor_Change_Animation_Mode(kActorHanoi, 78);
		Set_Enter(kSetNR01, kSceneNR01);
		break;

	case 230:
		AI_Movement_Track_Flush(kActorHanoi);
		Actor_Put_In_Set(kActorHanoi, kSetNR05_NR08);
		Actor_Set_At_XYZ(kActorHanoi, -1387.51f, 0.32f, 288.16f, 292);
		break;

	case 235:
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append(kActorHanoi, 439, 0);
		AI_Movement_Track_Append(kActorHanoi, 39, 45);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case 236:
		break;

	case 240:
		Actor_Put_In_Set(kActorHanoi, kSetNR04);
		Actor_Set_At_XYZ(kActorHanoi, -47.0f, 0.0f, 334.0f, 535);
		AI_Movement_Track_Flush(kActorHanoi);
		AI_Movement_Track_Append(kActorHanoi, 549, 0);
		AI_Movement_Track_Repeat(kActorHanoi);
		break;

	case 241:
		Actor_Face_Actor(kActorHanoi, kActorMcCoy, 1);
		Actor_Change_Animation_Mode(kActorHanoi, 6);
		Actor_Retired_Here(kActorMcCoy, 12, 12, 1, -1);
		break;

	case 9999:
		AI_Movement_Track_Flush(kActorHanoi);
		break;

	default:
		return false;
	}

	return true;
}

bool AIScriptHanoi::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (_var2 == 1) {
			*animation = 649;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(649)) {
				*animation = 648;
				_animationFrame = 0;
				_var2 = 0;
			}
		} else if (_var2 == 0) {
			*animation = 648;
			if (_var3) {
				_var3--;
				if (!Random_Query(0, 6)) {
					_var4 = -_var4;
				}
			} else {
				_animationFrame += _var4;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(648)) {
					_animationFrame = 0;
				}
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(648) - 1;
				}
				if (_animationFrame == 5 || _animationFrame == 15 || _animationFrame == 11 || !_animationFrame) {
					_var3 = Random_Query(5, 12);
				}
				if (_animationFrame >= 10 && _animationFrame <= 13) {
					_var3 = Random_Query(0, 1);
				}
				if (!_animationFrame) {
					if (!Random_Query(0, 4)) {
						_var2 = 1;
					}
				}
			}
		}
		break;

	case 1:
		if (_var2) {
			*animation = 649;
			if ( Slice_Animation_Query_Number_Of_Frames(649) < Slice_Animation_Query_Number_Of_Frames(649)) {
				_animationFrame += 2;
			} else {
				_animationFrame -= 2;
			}
			if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(649) - 1
					|| _animationFrame <= 0) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		} else {
			_animationFrame = 0;
			_animationState = _animationStateNext;
			*animation = _animationNext;
		}
		break;

	case 2:
		*animation = 657;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(657)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 658;
		}
		break;

	case 3:
		*animation = 658;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(658)) {
			_animationFrame = 0;
		}
		break;

	case 4:
		*animation = 659;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(659)) {
			_animationFrame = 0;
			_animationState = 3;
			*animation = 658;
		}
		break;

	case 5:
		*animation = 657;
		_animationFrame--;
		if (_animationFrame == 0) {
			_animationState = 0;
			_animationFrame = 0;
			*animation = 648;
			Actor_Face_Actor(kActorMcCoy, kActorHanoi, 1);
			Actor_Set_Invisible(kActorMcCoy, 0);

			if (Actor_Query_In_Set(kActorHanoi, kSetNR01)) {
				AI_Movement_Track_Flush(kActorHanoi);
				AI_Movement_Track_Append(kActorHanoi, 350, 0);
				AI_Movement_Track_Append(kActorHanoi, 39, 0);
				AI_Movement_Track_Repeat(kActorHanoi);
			}
		}
		break;

	case 6:
		*animation = 345;
		_animationFrame++;
		if (_animationFrame > 26) {
			Actor_Change_Animation_Mode(kActorHanoi, 0);
			_animationState = 0;
			_animationFrame = 0;
			*animation = 648;
			Actor_Set_Goal_Number(kActorMcCoy, 210);
			Actor_Set_Goal_Number(kActorHanoi, 210);
		}
		break;

	case 7:
		*animation = 645;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(645)) {
			_animationFrame = 0;
		}
		break;

	case 8:
		*animation = 642;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(642) - 1) {
			_animationFrame = 0;
		}
		break;

	case 9:
		*animation = 643;
		_animationFrame++;
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(643) - 1) {
			Actor_Change_Animation_Mode(kActorHanoi, 4);
			_animationState = 8;
			_animationFrame = 0;
			*animation = 642;
			Actor_Set_Goal_Number(kActorHanoi, 241);
		}
		break;

	case 10:
		*animation = 644;
		_animationFrame++;
		if (_animationFrame == 4) {
			Ambient_Sounds_Play_Sound(492, 77, 0, 0, 20);
		}
		if (_animationFrame == 6) {
			Ambient_Sounds_Play_Sound(493, 97, 0, 0, 20);
		}
		if (_animationFrame == 5) {
			Actor_Force_Stop_Walking(kActorMcCoy);
			Actor_Change_Animation_Mode(kActorMcCoy, 48);
		}
		if (_animationFrame > Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
			Actor_Change_Animation_Mode(kActorHanoi, 4);
			_animationFrame = 0;
			_animationState = 8;
			*animation = 642;
		}
		break;

	case 11:
		*animation = 660;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(660)) {
			*animation = 648;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 12:
		*animation = 646;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(646)) {
			*animation = 642;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 13:
		*animation = 647;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(647) - 1) {
			_animationFrame++;
		}
		break;

	case 14:
		*animation = 650;
		if (!_animationFrame && _var1) {
			_animationState = 0;
		} else {
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(650)) {
				_animationFrame = 0;
			}
		}
		break;

	case 15:
		*animation = 651;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(651)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 650;
		}
		break;

	case 16:
		*animation = 652;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(652)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 650;
		}
		break;

	case 17:
		*animation = 653;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(653)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 650;
		}
		break;

	case 18:
		*animation = 654;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(654)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 650;
		}
		break;

	case 19:
		*animation = 655;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(655)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 650;
		}
		break;

	case 20:
		*animation = 656;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(656)) {
			_animationFrame = 0;
			_animationState = 14;
			*animation = 650;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptHanoi::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if ((unsigned int)(_animationState - 2) > 1) {
			_animationState = 0;
		} else {
			_animationState = 3;
		}
		_animationFrame = 0;
		break;

	case kAnimationModeWalk:
		_animationState = 7;
		_animationFrame = 0;
		break;

	case kAnimationModeTalk:
		if (_animationState == 3) {
			_animationState = 4;
			_animationFrame = 0;
		} else {
			_animationStateNext = 14;
			_animationNext = 650;
			_animationState = 1;
		}
		break;

	case kAnimationModeCombatIdle:
		_animationState = 8;
		_animationFrame = 0;
		break;

	case kAnimationModeCombatAttack:
		_animationState = 10;
		_animationFrame = 0;
		break;

	case 12:
		_animationStateNext = 15;
		_animationNext = 651;
		_animationState = 1;
		break;

	case 13:
		_animationStateNext = 16;
		_animationNext = 652;
		_animationState = 1;
		break;

	case 14:
		_animationStateNext = 17;
		_animationNext = 653;
		_animationState = 1;
		break;

	case 15:
		_animationStateNext = 18;
		_animationNext = 654;
		_animationState = 1;
		break;

	case 16:
		_animationStateNext = 18;
		_animationNext = 654;
		_animationState = 1;
		break;

	case 17:
		_animationStateNext = 20;
		_animationNext = 656;
		_animationState = 1;
		break;

	case kAnimationModeHit:
	case kAnimationModeCombatHit:
		_animationState = 12;
		_animationFrame = 0;
		break;

	case 23:
		if (_animationState != 3 && _animationState != 4) {
			Actor_Set_Invisible(kActorMcCoy, true);
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 5;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(657) - 1;
		}
		break;

	case kAnimationModeDie:
		_animationState = 13;
		_animationFrame = 0;
		break;

	case 71:
		_animationState = 9;
		_animationFrame = 0;
		break;

	case 78:
		_animationState = 6;
		_animationFrame = 16;
		break;
	}

	return true;
}

void AIScriptHanoi::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptHanoi::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptHanoi::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 365) {
		Actor_Face_Actor(kActorHanoi, kActorHysteriaPatron1, true);
	}

	return true;
}

void AIScriptHanoi::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
