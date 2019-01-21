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

AIScriptLucy::AIScriptLucy(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = 0;
}

void AIScriptLucy::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = 0;

	Actor_Set_Goal_Number(kActorLucy, 0);
}

bool AIScriptLucy::Update() {
	float x, y, z;

	if (Global_Variable_Query(kVariableChapter) == 3 && Actor_Query_Goal_Number(kActorLucy) < 200) {
		Actor_Set_Goal_Number(kActorLucy, 200);
	}
	if (Actor_Query_Goal_Number(kActorLucy) == 230 && Player_Query_Current_Scene() == 37) {
		Actor_Set_Goal_Number(kActorLucy, 233);
	}
	if (Global_Variable_Query(kVariableChapter) == 4 && Actor_Query_Goal_Number(kActorLucy) < 300) {
		Actor_Set_Goal_Number(kActorLucy, 300);
	}
	if (Global_Variable_Query(kVariableChapter) == 4
			&& Actor_Query_Goal_Number(kActorLucy) == 599
			&& Actor_Query_Which_Set_In(kActorLucy) != 99) {
		if (Actor_Query_Which_Set_In(kActorLucy) != Player_Query_Current_Set()) {
			Actor_Put_In_Set(kActorLucy, kSetFreeSlotI);
			Actor_Set_At_Waypoint(kActorLucy, 41, 0);
		}
	}
	if (Game_Flag_Query(616) && Actor_Query_Goal_Number(kActorLucy) == 201) {
		Actor_Put_In_Set(kActorLucy, kSetHF03);
		Actor_Set_At_Waypoint(kActorLucy, 371, 156);
		Actor_Set_Goal_Number(kActorLucy, 250);
	}
	if (Actor_Query_Goal_Number(kActorLucy) > 229
			&& Actor_Query_Goal_Number(kActorLucy) < 239
			&& Actor_Query_Goal_Number(kActorLucy) != 232
			&& Player_Query_Current_Scene() == 37
			&& Actor_Query_Which_Set_In(kActorLucy) == 40
			&& !Game_Flag_Query(701)
			&& Actor_Query_Inch_Distance_From_Actor(kActorLucy, kActorMcCoy) < 84
			&& !Player_Query_Combat_Mode()
			&& Actor_Query_Friendliness_To_Other(kActorLucy, kActorMcCoy) > 40) {
		Actor_Set_Goal_Number(kActorLucy, 232);
	}
	if (Actor_Query_Goal_Number(kActorLucy) == 235) {
		Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
		if (z > -875.0f) {
			Game_Flag_Set(586);
			Actor_Set_Goal_Number(kActorLucy, 236);
		}
	}
	if (Actor_Query_Goal_Number(kActorLucy) == 237) {
		Actor_Query_XYZ(kActorMcCoy, &x, &y, &z);
		if (x > 350.0f) {
			Game_Flag_Set(585);
			Actor_Set_Goal_Number(kActorLucy, 238);
		}
	}

	return false;
}

void AIScriptLucy::TimerExpired(int timer) {
	AI_Countdown_Timer_Reset(kActorLucy, 0);
	if (!timer && Actor_Query_Goal_Number(kActorLucy) == 205) {
		if (Player_Query_Current_Scene() == 36) {
			AI_Countdown_Timer_Start(kActorLucy, 0, 20);
		} else {
			Actor_Set_Goal_Number(kActorLucy, 200);
		}
	}
}

void AIScriptLucy::CompletedMovementTrack() {
	switch (Actor_Query_Goal_Number(kActorLucy)) {
	case 205:
		if (!Game_Flag_Query(616) || Global_Variable_Query(40) != 3) {
			AI_Countdown_Timer_Reset(kActorLucy, 0);
			AI_Countdown_Timer_Start(kActorLucy, 0, 30);
			return; //false;
		}
		Actor_Set_Goal_Number(kActorLucy, 250);
		return; //true;

	case 210:
		Actor_Set_Goal_Number(kActorLucy, 211);
		break;

	case 211:
		Game_Flag_Set(593);
		Actor_Set_Goal_Number(kActorLucy, 299);
		break;

	case 212:
		Actor_Set_Goal_Number(kActorLucy, 213);
		break;

	case 213:
		if (Actor_Clue_Query(kActorLucy, 219) && Global_Variable_Query(40) != 3) {
			Game_Flag_Set(593);
		} else {
			Actor_Set_Goal_Number(kActorLucy, 230);
			Game_Flag_Reset(584);
		}
		break;

	case 214:
		Actor_Set_Goal_Number(kActorLucy, 215);
		break;

	case 215:
		Actor_Set_Goal_Number(kActorLucy, 201);
		break;

	case 220:
	case 225:
		Actor_Set_Goal_Number(kActorLucy, 200);
		break;

	case 233:
		Game_Flag_Set(585);
		Actor_Set_Goal_Number(kActorLucy, 234);
		break;

	case 234:
		Actor_Set_Goal_Number(kActorLucy, 235);
		break;

	case 236:
		Actor_Set_Goal_Number(kActorLucy, 237);
		break;

	case 238:
		Game_Flag_Set(593);
		Actor_Put_In_Set(kActorLucy, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorLucy, 33, 0);
		Actor_Set_Health(kActorLucy, 30, 30);
		if (Global_Variable_Query(40) == 3) {
			Actor_Set_Goal_Number(kActorSteele, 240);
		}
		break;

	case 239:
		Game_Flag_Set(593);
		break;

	default:
		break;
	}
	return; //false;
}

void AIScriptLucy::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptLucy::ClickedByPlayer() {
	if (Actor_Query_Goal_Number(kActorLucy) == 599) {
		Actor_Face_Actor(kActorMcCoy, kActorLucy, 1);
		Actor_Says(kActorMcCoy, 8630, 3);
	}
}

void AIScriptLucy::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptLucy::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptLucy::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptLucy::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptLucy::ShotAtAndMissed() {
	checkCombat();
}

bool AIScriptLucy::ShotAtAndHit() {
	checkCombat();

	return false;
}

void AIScriptLucy::Retired(int byActorId) {
	if (byActorId == kActorMcCoy) {
		Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, -6);
	}
	if ((byActorId == kActorSteele || byActorId == kActorMcCoy)
			&& Actor_Query_In_Set(kActorSteele, kSetHF06)
			&& Actor_Query_In_Set(kActorMcCoy, kSetHF06)) {
		Non_Player_Actor_Combat_Mode_On(kActorSteele, kActorCombatStateUncover, true, kActorMcCoy, 15, kAnimationModeCombatIdle, kAnimationModeCombatWalk, kAnimationModeCombatRun, 0, 0, 100, 25, 300, false);
	}
	if (Query_Difficulty_Level() && byActorId == kActorMcCoy && Game_Flag_Query(46)) {
		Global_Variable_Increment(2, 200);
	}
	Actor_Set_Goal_Number(kActorLucy, 599);
}

int AIScriptLucy::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptLucy::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (!newGoalNumber) {
		Actor_Put_In_Set(kActorLucy, kSetFreeSlotA);
		return false;
	}

	switch (newGoalNumber) {
	case 200:
		if (Global_Variable_Query(40) == 3) {
			if (Game_Flag_Query(591) && Game_Flag_Query(592) && Player_Query_Current_Scene() != 36) {
				Actor_Set_Goal_Number(kActorLucy, 205);
			} else {
				if (Random_Query(1, 2) - 1) {
					Actor_Set_Goal_Number(kActorLucy, 225);
				} else {
					Actor_Set_Goal_Number(kActorLucy, 220);
				}
			}
		} else {
			int rnd = Random_Query(1, 4) - 1;

			if (rnd) {
				if (rnd == 1) {
					Actor_Set_Goal_Number(kActorLucy, 225);
				} else if (Player_Query_Current_Scene() == 36) {
					Actor_Set_Goal_Number(kActorLucy, 220);
				} else {
					Actor_Set_Goal_Number(kActorLucy, 205);
				}
			} else {
				Actor_Set_Goal_Number(kActorLucy, 220);
			}
		}
		break;

	case 205:
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append_With_Facing(kActorLucy, 371, 0, 156);
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 210:
		Actor_Set_Immunity_To_Obstacles(kActorLucy, 1);
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append_Run(kActorLucy, 377, 0);
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 211:
		Actor_Set_Immunity_To_Obstacles(kActorLucy, 0);
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append_Run(kActorLucy, 372, 0);
		AI_Movement_Track_Append(kActorLucy, 33, 0);
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 212:
		Actor_Set_Immunity_To_Obstacles(kActorLucy, 1);
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append_Run(kActorLucy, 378, 0);
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 213:
		Actor_Set_Immunity_To_Obstacles(kActorLucy, 0);
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append_Run(kActorLucy, 373, 0);
		AI_Movement_Track_Append(kActorLucy, 33, 0);
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 214:
		Actor_Says(kActorLucy, 320, 16);
		Actor_Set_Goal_Number(kActorHolloway, 242);
		Actor_Set_Immunity_To_Obstacles(6, 1);
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append(kActorLucy, 378, 0);
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 215:
		Actor_Set_Immunity_To_Obstacles(kActorLucy, 0);
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append_Run(kActorLucy, 373, 0);
		AI_Movement_Track_Append(kActorLucy, 33, 30);
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 220:
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append(kActorLucy, 39, Random_Query(5, 10));
		AI_Movement_Track_Append(kActorLucy, 33, Random_Query(5, 10));
		AI_Movement_Track_Append(kActorLucy, 39, Random_Query(5, 10));
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 225:
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append(kActorLucy, 39, Random_Query(5, 15));
		AI_Movement_Track_Append(kActorLucy, 33, Random_Query(10, 30));
		AI_Movement_Track_Append(kActorLucy, 40, Random_Query(15, 30));
		AI_Movement_Track_Append(kActorLucy, 42, Random_Query(10, 20));
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 230:
		AI_Movement_Track_Flush(kActorLucy);
		Actor_Put_In_Set(kActorLucy, kSetHF04);
		Actor_Set_At_Waypoint(kActorLucy, 518, 0);
		Actor_Set_Targetable(kActorLucy, 1);
		Actor_Set_Health(kActorLucy, 5, 5);
		break;

	case 232:
		Player_Loses_Control();
		Actor_Says(kActorMcCoy, 1700, 16);
		AI_Movement_Track_Flush(kActorLucy);
		Actor_Face_Actor(kActorLucy, 0, 1);
		Actor_Face_Actor(kActorMcCoy, kActorLucy, 1);
		Actor_Says(kActorLucy, 350, 13);
		Actor_Says(kActorMcCoy, 1705, 13);
		Actor_Says(kActorLucy, 360, 13);
		Actor_Says(kActorMcCoy, 1710, 13);

		if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsLucy) {
			Actor_Says(kActorLucy, 940, 13);
			Actor_Says(kActorMcCoy, 6780, 12);
			Actor_Says(kActorLucy, 950, 12);
			Actor_Says(kActorLucy, 960, 14);
			Actor_Says(kActorMcCoy, 6785, 13);
			Actor_Says(kActorLucy, 970, 16);
			Actor_Says(kActorLucy, 980, 13);
			if (Game_Flag_Query(kFlagDektoraIsReplicant)) {
				Actor_Says(kActorLucy, 990, 15);
			}
			Actor_Says(kActorMcCoy, 6790, 13);
			if (Game_Flag_Query(kFlagDektoraIsReplicant)
			 && Game_Flag_Query(kFlagLucyIsReplicant)
			) {
				Actor_Says(kActorLucy, 1000, 12);
			}
			Actor_Says(kActorLucy, 1010, 15);
			Actor_Says(kActorLucy, 1020, 12);
			Actor_Says(kActorMcCoy, 6795, 18);
			Actor_Says(kActorLucy, 1030, 15);
			Actor_Says(kActorMcCoy, 6800, 16);
			Actor_Says(kActorLucy, 1040, 12);
			Actor_Says(kActorMcCoy, 6805, 15);
			Actor_Says(kActorLucy, 1050, 12);
		}
		Actor_Says(kActorLucy, 370, 14);
		Actor_Set_Goal_Number(kActorLucy, 239);
		if (Global_Variable_Query(40) == 3) {
			Actor_Set_Goal_Number(kActorSteele, 243);
			Game_Flag_Set(593);
		}
		Player_Gains_Control();
		break;

	case 233:
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append_Run(kActorLucy, 519, 0);
		AI_Movement_Track_Repeat(kActorLucy);
		Actor_Set_Health(kActorLucy, 5, 5);
		break;

	case 234:
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append_Run(kActorLucy, 520, 0);
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 236:
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append_Run(kActorLucy, 521, 0);
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 238:
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append_Run(kActorLucy, 522, 0);
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 239:
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append_Run(kActorLucy, 523, 0);
		AI_Movement_Track_Append(kActorLucy, 33, 0);
		AI_Movement_Track_Repeat(kActorLucy);
		Player_Gains_Control();
		Actor_Set_Health(kActorLucy, 30, 30);
		break;

	case 240:
		if (Global_Variable_Query(40) == 3) {
			Actor_Set_Goal_Number(kActorLucy, 599);
			Actor_Set_Goal_Number(kActorSteele, 240);
		} else {
			Actor_Set_Goal_Number(kActorLucy, 299);
			Game_Flag_Set(593);
		}
		break;

	case 250:
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append(kActorLucy, 372, 0);
		AI_Movement_Track_Append_With_Facing(kActorLucy, 371, 0, 156);
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 300:
		Actor_Put_In_Set(kActorLucy, kSetFreeSlotA);
		Actor_Set_At_Waypoint(kActorLucy, 33, 0);
		if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsLucy) {
			Actor_Set_Goal_Number(kActorLucy, 310);
		}
		break;

	case 310:
		Actor_Put_In_Set(kActorLucy, kSetUG01);
		Actor_Set_At_Waypoint(kActorLucy, 544, 651);
		break;

	case 311:
		voightKempTest();
		break;

	case 312:
		AI_Movement_Track_Flush(kActorLucy);
		AI_Movement_Track_Append_Run(kActorLucy, 545, 0);
		AI_Movement_Track_Append(kActorLucy, 33, 0);
		AI_Movement_Track_Repeat(kActorLucy);
		break;

	case 599:
		Game_Flag_Set(593);
		break;
	}

	return false;
}

bool AIScriptLucy::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = 260;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(260)) {
			_animationFrame = 0;
		}
		break;

	case 1:
	case 2:
	case 3:
	case 4:
		if (_animationState == 1) {
			*animation = 253;
		}
		if (_animationState == 2) {
			*animation = 254;
		}
		if (_animationState == 3) {
			*animation = 255;
		}
		if (_animationState == 4) {
			*animation = 256;
		}
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
		}
		break;

	case 5:
	case 6:
		if (_animationState == 5) {
			*animation = 257;
		}
		if (_animationState == 6) {
			*animation = 258;
		}
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			*animation = 260;
			_animationFrame = 0;
			_animationState = 0;
			Actor_Change_Animation_Mode(kActorLucy, 0);
		}
		break;

	case 7:
		*animation = 259;
		if (_animationFrame < Slice_Animation_Query_Number_Of_Frames(259) - 1) {
			++_animationFrame;
		}
		break;

	case 8:
		if (!_animationFrame && _flag) {
			*animation = 260;
			_animationState = 0;
			_flag = 0;
		} else {
			*animation = 263;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(263)) {
				_animationFrame = 0;
			}
		}
		break;

	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
		if (_animationState == 9) {
			*animation = 264;
		}
		if (_animationState == 10) {
			*animation = 265;
		}
		if (_animationState == 11) {
			*animation = 266;
		}
		if (_animationState == 12) {
			*animation = 267;
		}
		if (_animationState == 13) {
			*animation = 268;
		}
		if (_animationState == 14) {
			*animation = 269;
		}
		if (_animationState == 15) {
			*animation = 270;
		}
		if (_animationState == 16) {
			*animation = 271;
		}
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation)) {
			_animationFrame = 0;
			_animationState = 8;
			*animation = 263;
		}
		break;

	case 17:
		*animation = 272;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(272)) {
			_animationFrame = 0;
		}
		break;

	case 18:
		if (!_animationFrame && _flag) {
			_animationState = 17;
			_flag = 0;
			*animation = 272;
		} else {
			*animation = 273;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(273)) {
				_animationFrame = 0;
			}
		}
		break;

	case 19:
		*animation = 274;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(274)) {
			*animation = 260;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 20:
		*animation = 275;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(275)) {
			_animationFrame = 0;
		}
		break;

	case 21:
		*animation = 276;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(276)) {
			_animationFrame = 0;
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptLucy::ChangeAnimationMode(int mode) {
	switch (mode) {
		case 0:
	if (_animationState >= 8 && _animationState <= 16) {
			_flag = 1;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;

	case 1:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case 2:
		_animationState = 2;
		_animationFrame = 0;
		break;

	case 3:
		_flag = 0;
		_animationState = 8;
		_animationFrame = 0;
		break;

	case 4:
		if (_animationState >= 8 && _animationState <= 16) {
			_flag = 1;
		} else {
			_animationState = 0;
			_animationFrame = 0;
		}
		break;

	case 7:
		_animationState = 1;
		_animationFrame = 0;
		break;

	case 8:
		_animationState = 2;
		_animationFrame = 0;
		break;

	case 12:
		_flag = 0;
		_animationState = 9;
		_animationFrame = 0;
		break;

	case 13:
		_flag = 0;
		_animationState = 10;
		_animationFrame = 0;
		break;

	case 14:
		_flag = 0;
		_animationState = 11;
		_animationFrame = 0;
		break;

	case 15:
		_flag = 0;
		_animationState = 12;
		_animationFrame = 0;
		break;

	case 16:
		_flag = 0;
		_animationState = 13;
		_animationFrame = 0;
		break;

	case 17:
		_flag = 0;
		_animationState = 14;
		_animationFrame = 0;
		break;

	case 18:
		_flag = 0;
		_animationState = 15;
		_animationFrame = 0;
		break;

	case 19:
		_flag = 0;
		_animationState = 16;
		_animationFrame = 0;
		break;

	case 21:
		if (Random_Query(1, 2) == 1) {
			_animationState = 5;
		} else {
			_animationState = 6;
		}
		_animationFrame = 0;
		break;

	case 22:
		if (Random_Query(1, 2) == 1) {
			_animationState = 5;
		} else {
			_animationState = 6;
		}
		_animationFrame = 0;
		break;

	case 48:
		_animationState = 7;
		_animationFrame = 0;
		break;
	}

	return true;
}

void AIScriptLucy::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptLucy::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptLucy::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptLucy::FledCombat() {
	if (Global_Variable_Query(kVariableChapter) == 5 && Actor_Query_Goal_Number(kActorLucy) == 450) {
		Actor_Put_In_Set(kActorLucy, kSetFreeSlotG);
		Actor_Set_At_Waypoint(kActorLucy, 39, 0);
		Actor_Set_Goal_Number(kActorLucy, 599);
	}

	return; //true;
}

void AIScriptLucy::voightKempTest() {
	Player_Loses_Control();
	Actor_Face_Actor(kActorMcCoy, kActorLucy, 1);
	Actor_Says(kActorMcCoy, 6815, 11);
	Actor_Face_Actor(kActorLucy, kActorMcCoy, 1);
	Actor_Says(kActorLucy, 1060, 16);
	Actor_Says(kActorLucy, 1070, 17);
	Delay(1000);
	Actor_Says(kActorLucy, 1080, 14);
	Actor_Says(kActorMcCoy, 6820, 16);
	Actor_Says(kActorLucy, 1090, 13);
	if (!Game_Flag_Query(378)) {
		Actor_Says(kActorMcCoy, 6825, 13);
	}
	Actor_Says(kActorMcCoy, 6830, 12);
	Actor_Says(kActorLucy, 1100, 14);
	Actor_Says(kActorMcCoy, 6835, 14);
	Actor_Says(kActorLucy, 1110, 15);
	Actor_Says(kActorMcCoy, 6840, 13);
	Delay(1000);
	Actor_Says(kActorMcCoy, 6845, 12);
	Delay(500);
	Actor_Says(kActorMcCoy, 6850, 12);
	Actor_Says(kActorLucy, 1120, 14);
	Actor_Says(kActorMcCoy, 6855, 13);
	Actor_Says(kActorMcCoy, 6860, 13);
	Actor_Says(kActorLucy, 1130, 14);
	Music_Stop(2);
	Player_Gains_Control();
	Voight_Kampff_Activate(6, 40);
	Player_Loses_Control();
	if (Actor_Clue_Query(kActorMcCoy, 271)) {
		Actor_Says(kActorMcCoy, 6865, 13);
		Actor_Says(kActorLucy, 1140, 14);
		Actor_Says(kActorMcCoy, 6865, 14);
		Actor_Says(kActorLucy, 1150, 16);
		Actor_Says(kActorMcCoy, 6870, 14);
		Delay(500);
		Actor_Says(kActorMcCoy, 6875, 13);
		Actor_Says(kActorLucy, 1160, 16);
	} else {
		Actor_Says(kActorMcCoy, 6880, 13);
		Actor_Says(kActorLucy, 1170, 13);
		Actor_Says(kActorLucy, 1180, 16);
		Actor_Says(kActorMcCoy, 6890, 15);
		Actor_Says(kActorLucy, 1190, 15);
		Actor_Says(kActorLucy, 1200, 17);
		Actor_Says(kActorMcCoy, 6885, 13);
		Actor_Says(kActorLucy, 1210, 17);
	}
	Actor_Says(kActorMcCoy, 6895, 15);
	Actor_Says(kActorMcCoy, 6900, 11);
	Actor_Says(kActorLucy, 1220, 16);
	Actor_Says(kActorMcCoy, 6905, 13);
	Actor_Says(kActorLucy, 1230, 17);
	Actor_Says(kActorMcCoy, 6910, 13);
	Delay(2000);
	Player_Gains_Control();
	Actor_Set_Goal_Number(kActorLucy, 312);
}

void AIScriptLucy::checkCombat() {
	Game_Flag_Set(701);
	if (Actor_Query_In_Set(kActorLucy, kSetHF01)
	 && Global_Variable_Query(kVariableChapter) == 5
	 && Actor_Query_Goal_Number(kActorLucy) != 450
	) {
		if (Global_Variable_Query(kVariableAffectionTowards) == kAffectionTowardsLucy) {
			Global_Variable_Set(kVariableAffectionTowards, kAffectionTowardsNone);
		}
		Actor_Set_Goal_Number(kActorLucy, 450);
		Non_Player_Actor_Combat_Mode_On(kActorLucy, kActorCombatStateIdle, false, kActorMcCoy, 4, kAnimationModeIdle, kAnimationModeWalk, kAnimationModeRun, -1, 0, 0, 10, 300, false);
	}
}

} // End of namespace BladeRunner
