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

#include "bladerunner/script/ai_15_runciter.h"

namespace BladeRunner {

AIScript_Runciter::AIScript_Runciter(BladeRunnerEngine *vm)
	: AIScriptBase(vm) {}

void AIScript_Runciter::Initialize() {
	var_45CD70_animation_state = 0;
	var_45CD74_animation_frame = 0;
	var_462800 = 0;
	var_45CD78 = 0;
	var_45CD7C = 6;
	var_45CD80 = 1;
	var_45CD84 = 0;
	var_45CD88 = 0;
	Actor_Set_Goal_Number(15, 0);
}

bool AIScript_Runciter::Update() {
	if (Actor_Query_Goal_Number(15) == 0 && Game_Flag_Query(186)) {
		Actor_Set_Goal_Number(15, 2);
	}
	if (Global_Variable_Query(1) == 4 && Actor_Query_Goal_Number(15) < 300) {
		Actor_Set_Goal_Number(15, 300);
	}
	return false;
}

void AIScript_Runciter::TimerExpired(int timer) {}

void AIScript_Runciter::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(15) == 1) {
		if (Player_Query_Current_Scene() == 79) {
			switch (Random_Query(1, 5)) {
			case 2:
			case 3:
				ADQ_Add(15, 530, -1);
				break;
			case 1:
			case 5:
				ADQ_Add(15, 80, -1);
				break;
			case 4:
				ADQ_Add(15, 930, -1);
				break;
			}
		}
		Actor_Set_Goal_Number(15, 99);
		Actor_Set_Goal_Number(15, 1);
		//return true;
	}
	//return false;
}

void AIScript_Runciter::ReceivedClue(int clueId, int fromActorId) {}

void AIScript_Runciter::ClickedByPlayer() {}

void AIScript_Runciter::EnteredScene(int sceneId) {}

void AIScript_Runciter::OtherAgentEnteredThisScene(int otherActorId) {}

void AIScript_Runciter::OtherAgentExitedThisScene(int otherActorId) {}

void AIScript_Runciter::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (Actor_Query_Goal_Number(15) == 300 && combatMode == 1 && !Game_Flag_Query(705)) {
		Actor_Set_Targetable(15, true);
		Actor_Face_Actor(15, 0, true);
		Actor_Says(15, 420, 12);
		Actor_Face_Actor(0, 15, true);
		Actor_Change_Animation_Mode(0, 5);
		if (Actor_Clue_Query(0, 158)) {
			Actor_Says(0, 4770, -1);
			Actor_Says(15, 590, 13);
			Actor_Says(0, 4775, -1);
			Actor_Says(15, 600, 17);
			Sound_Play(492, 100, 0, 100, 50);
			Actor_Says(0, 4780, -1);
			Actor_Says(15, 610, 18);
			Actor_Says(0, 4785, -1);
			Actor_Says(15, 620, 15);
			if (Game_Flag_Query(46)) {
				Actor_Says(15, 630, 12);
				Actor_Says(15, 640, 17);
				Actor_Says(0, 4790, -1);
				Actor_Says(15, 650, 18);
				Actor_Says(15, 660, 19);
				Actor_Clue_Acquire(0, 280, 1, 15);
			} else {
				Actor_Says(15, 670, 18);
				Actor_Says(0, 4795, -1);
				Actor_Says(15, 730, 17);
			}
		} else if (Actor_Clue_Query(0, 76)) {
			Actor_Says(0, 4730, -1);
			Actor_Says(15, 480, 17);
			Actor_Says(0, 4735, -1);
			Actor_Says(15, 490, 16);
			Sound_Play(492, 100, 0, 100, 50);
			Actor_Says(0, 4740, -1);
			Actor_Says(15, 500, 18);
			Actor_Says(15, 510, 19);
			Actor_Says(0, 4745, -1);
			Actor_Says(0, 4750, -1);
			Actor_Says(15, 520, 17);
			Actor_Says(15, 530, 18);
			Actor_Says(15, 540, 16);
		}
		Game_Flag_Set(705);
	}
}

void AIScript_Runciter::ShotAtAndMissed() {}

void AIScript_Runciter::ShotAtAndHit() {
	Actor_Set_Targetable(15, false);
	Actor_Change_Animation_Mode(15, 48);
	Actor_Set_Goal_Number(15, 599);
	Delay(2000);
	if (Actor_Clue_Query(0, 158)) {
		Actor_Voice_Over(2050, 99);
		Actor_Voice_Over(2060, 99);
	} else {
		Actor_Voice_Over(2070, 99);
		Actor_Voice_Over(2080, 99);
		Actor_Voice_Over(2090, 99);
	}
	Actor_Modify_Friendliness_To_Other(5, 0, 3);
}

void AIScript_Runciter::Retired(int byActorId) {}

int AIScript_Runciter::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScript_Runciter::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == 0) {
		Actor_Put_In_Set(15, 16);
		Actor_Set_At_Waypoint(15, 92, 567);
		return false;
	}
	if (newGoalNumber == 1) {
		AI_Movement_Track_Flush(15);
		if (Random_Query(0, 1) == 1) {
			if (Random_Query(0, 1) == 0) {
				AI_Movement_Track_Append_With_Facing(15, 89, Random_Query(6, 10), 567);
			}
			AI_Movement_Track_Append_With_Facing(15, 93, Random_Query(2, 6), 1002);
			AI_Movement_Track_Append(15, 92, 5);
		} else {
			AI_Movement_Track_Append_With_Facing(15, 91, Random_Query(3, 10), 120);
			if (Random_Query(1, 3) == 1) {
				AI_Movement_Track_Append_With_Facing(15, 93, Random_Query(2, 6), 1002);
			}
			AI_Movement_Track_Append_With_Facing(15, 90, Random_Query(5, 10), 170);
		}
		AI_Movement_Track_Repeat(15);
		return true;
	}
	if (newGoalNumber == 2) {
		AI_Movement_Track_Flush(15);
		AI_Movement_Track_Append(15, 39, 120);
		AI_Movement_Track_Append(15, 40, 0);
		AI_Movement_Track_Repeat(15);
		return false;
	}
	if (newGoalNumber == 300) {
		Actor_Put_In_Set(15, 16);
		Actor_Set_At_Waypoint(15, 93, 1007);
		return false;
	}
	return false;
}

bool AIScript_Runciter::UpdateAnimation(int *animation, int *frame) {

	switch (var_45CD70_animation_state) {
	case 15:
		*animation = 528;
		var_45CD74_animation_frame = Slice_Animation_Query_Number_Of_Frames(528) - 1;
		*frame = var_45CD74_animation_frame;
		break;
	case 14:
		*animation = 528;
		var_45CD74_animation_frame++;
		if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(528) - 1) {
			*animation = 528;
			var_45CD70_animation_state = 15;
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 13:
		if (var_45CD78 == 0) {
			var_45CD74_animation_frame = 0;
			var_45CD70_animation_state = var_462800;
			*animation = var_462804;
		} else if (var_45CD78 == 1) {
			*animation = 530;
			var_45CD74_animation_frame += 3;
			if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(530)) {
				var_45CD74_animation_frame = 0;
				var_45CD70_animation_state = var_462800;
				*animation = var_462804;
			}
		} else if (var_45CD78 == 2) {
			*animation = 531;
			var_45CD74_animation_frame -= 3;
			if (var_45CD74_animation_frame - 3 < 0) {
				var_45CD74_animation_frame = 0;
				var_45CD70_animation_state = var_462800;
				*animation = var_462804;
			}
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 12:
		*animation = 532;
		var_45CD74_animation_frame++;
		if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(532)) {
			*animation = 529;
			var_45CD70_animation_state = 0;
			var_45CD74_animation_frame = 0;
			var_45CD78 = 0;
			Actor_Change_Animation_Mode(15, 0);
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 11:
		*animation = 541;
		if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(541)) {
			var_45CD74_animation_frame = 0;
			if (var_45CD88) {
				*animation = 529;
				var_45CD70_animation_state = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				var_45CD70_animation_state = 2;
			}
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 10:
		*animation = 540;
		var_45CD74_animation_frame++;
		if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(540)) {
			var_45CD74_animation_frame = 0;
			if (var_45CD88) {
				*animation = 529;
				var_45CD70_animation_state = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				var_45CD70_animation_state = 2;
			}
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 9:
		*animation = 539;
		var_45CD74_animation_frame++;
		if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(539)) {
			var_45CD74_animation_frame = 0;
			if (var_45CD88) {
				*animation = 529;
				var_45CD70_animation_state = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				var_45CD70_animation_state = 2;
			}
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 8:
		*animation = 538;
		var_45CD74_animation_frame++;
		if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(538)) {
			var_45CD74_animation_frame = 0;
			if (var_45CD88) {
				*animation = 529;
				var_45CD70_animation_state = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				var_45CD70_animation_state = 2;
			}
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 7:
		*animation = 537;
		var_45CD74_animation_frame++;
		if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(537)) {
			var_45CD74_animation_frame = 0;
			if (var_45CD88) {
				*animation = 529;
				var_45CD70_animation_state = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				var_45CD70_animation_state = 2;
			}
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 6:
		*animation = 536;
		var_45CD74_animation_frame++;
		if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(536)) {
			var_45CD74_animation_frame = 0;
			if (var_45CD88) {
				*animation = 529;
				var_45CD70_animation_state = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				var_45CD70_animation_state = 2;
			}
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 5:
		*animation = 535;
		var_45CD74_animation_frame++;
		if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(535)) {
			var_45CD74_animation_frame = 0;
			if (var_45CD88) {
				*animation = 529;
				var_45CD70_animation_state = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				var_45CD70_animation_state = 2;
			}
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 4:
		*animation = 534;
		var_45CD74_animation_frame++;
		if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(534)) {
			var_45CD74_animation_frame = 0;
			if (var_45CD88) {
				*animation = 529;
				var_45CD70_animation_state = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				var_45CD70_animation_state = 2;
			}
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 2:
		*animation = 533;
		var_45CD74_animation_frame++;
		if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(533)) {
			var_45CD74_animation_frame = 0;
			if (var_45CD88) {
				*animation = 529;
				var_45CD70_animation_state = 0;
				var_45CD78 = 0;
			} else {
				var_45CD70_animation_state = 4;
			}
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 1:
		*animation = 526;
		var_45CD74_animation_frame++;
		if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(526)) {
			var_45CD74_animation_frame = 0;
		}
		*frame = var_45CD74_animation_frame;
		break;
	case 0:
		if (var_45CD78 == 0) {
			*animation = 529;
			if (var_45CD84) {
				var_45CD84--;
			} else {
				var_45CD74_animation_frame += var_45CD80;
				if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(529)) {
					var_45CD74_animation_frame = 0;
				}
				if (var_45CD74_animation_frame < 0) {
					var_45CD74_animation_frame = Slice_Animation_Query_Number_Of_Frames(529) - 1;
				}
				--var_45CD7C;
				if (var_45CD7C == 0) {
					var_45CD80 = 2 * Random_Query(0, 1) - 1;
					var_45CD7C = Random_Query(6, 14);
					var_45CD84 = Random_Query(0, 4);
				}
				if (var_45CD74_animation_frame == 0) {
					if (Random_Query(0, 1) == 1) {
						var_45CD78 = Random_Query(1, 2);
						var_45CD80 = 1;
						var_45CD84 = 0;
					}
				}
			}
		} else if (var_45CD78 == 1) {
			*animation = 530;
			var_45CD74_animation_frame++;
			if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(530)) {
				var_45CD74_animation_frame = 0;
				var_45CD78 = 0;
				*animation = 529;
				var_45CD7C = Random_Query(6, 14);
				var_45CD80 = 2 * Random_Query(0, 1) - 1;
			}
		} else if (var_45CD78 == 2) {
			*animation = 531;
			if (var_45CD84) {
				var_45CD84--;
			} else {
				var_45CD74_animation_frame += var_45CD80;
				if (var_45CD74_animation_frame >= Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
					var_45CD84 = Random_Query(5, 15);
					var_45CD80 = -1;
				}
				if (var_45CD74_animation_frame <= 0) {
					var_45CD74_animation_frame = 0;
					var_45CD78 = 0;
					*animation = 529;
					var_45CD7C = Random_Query(6, 14);
					var_45CD80 = 2 * Random_Query(0, 1) - 1;
				}
			}
		}
		*frame = var_45CD74_animation_frame;
		break;
	default:
		*animation = 399;
		var_45CD74_animation_frame = 0;
		*frame = var_45CD74_animation_frame;
		break;
	}
	return true;
}

bool AIScript_Runciter::ChangeAnimationMode(int mode) {

	switch (mode) {
	case 23:
		var_45CD70_animation_state = 12;
		var_45CD74_animation_frame = 0;
		break;
	case 19:
		if (var_45CD70_animation_state) {
			var_45CD70_animation_state = 2;
			var_45CD74_animation_frame = 0;
		} else {
			var_45CD70_animation_state = 13;
			var_462800 = 11;
			var_462804 = 541;
		}
		var_45CD88 = 0;
		break;
	case 18:
		if (var_45CD70_animation_state) {
			var_45CD70_animation_state = 2;
			var_45CD74_animation_frame = 0;
		} else {
			var_45CD70_animation_state = 13;
			var_462800 = 10;
			var_462804 = 540;
		}
		var_45CD88 = 0;
		break;
	case 17:
		if (var_45CD70_animation_state) {
			var_45CD70_animation_state = 2;
			var_45CD74_animation_frame = 0;
		} else {
			var_45CD70_animation_state = 13;
			var_462800 = 9;
			var_462804 = 539;
		}
		var_45CD88 = 0;
		break;
	case 16:
		if (var_45CD70_animation_state) {
			var_45CD70_animation_state = 2;
			var_45CD74_animation_frame = 0;
		} else {
			var_45CD70_animation_state = 13;
			var_462800 = 8;
			var_462804 = 538;
		}
		var_45CD88 = 0;
		break;
	case 15:
		if (var_45CD70_animation_state) {
			var_45CD70_animation_state = 2;
			var_45CD74_animation_frame = 0;
		} else {
			var_45CD70_animation_state = 13;
			var_462800 = 7;
			var_462804 = 537;
		}
		var_45CD88 = 0;
		break;
	case 14:
		if (var_45CD70_animation_state) {
			var_45CD70_animation_state = 2;
			var_45CD74_animation_frame = 0;
		} else {
			var_45CD70_animation_state = 13;
			var_462800 = 6;
			var_462804 = 536;
		}
		var_45CD88 = 0;
		break;
	case 13:
		if (var_45CD70_animation_state) {
			var_45CD70_animation_state = 2;
			var_45CD74_animation_frame = 0;
		} else {
			var_45CD70_animation_state = 13;
			var_462800 = 5;
			var_462804 = 535;
		}
		var_45CD88 = 0;
		break;
	case 12:
		if (var_45CD70_animation_state) {
			var_45CD70_animation_state = 2;
			var_45CD74_animation_frame = 0;
		} else {
			var_45CD70_animation_state = 13;
			var_462800 = 4;
			var_462804 = 534;
		}
		var_45CD88 = 0;
		break;
	case 3:
		if (var_45CD70_animation_state) {
			var_45CD70_animation_state = 2;
			var_45CD74_animation_frame = 0;
		} else {
			var_45CD70_animation_state = 13;
			var_462800 = 2;
			var_462804 = 526;
		}
		var_45CD88 = 0;
		break;
	case 1:
		if (var_45CD70_animation_state > 1) {
			var_45CD70_animation_state = 1;
			var_45CD74_animation_frame = 0;
		} else if (!var_45CD70_animation_state) {
			var_45CD70_animation_state = 13;
			var_462800 = 1;
			var_462804 = 526;
		}
		break;
	case 0:
		if (var_45CD70_animation_state >= 2 && var_45CD70_animation_state <= 11) {
			var_45CD88 = 1;
		} else {
			var_45CD70_animation_state = 0;
			var_45CD74_animation_frame = 0;
			var_45CD78 = 0;
		}
		break;
	case 2:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 20:
	case 21:
	case 22:
		break;
	default:
		if (mode == 48) {
			var_45CD70_animation_state = 14;
			var_45CD74_animation_frame = 0;
		}
		break;
	}
	return true;
}

void AIScript_Runciter::QueryAnimationState(int *animationState, int *animationFrame, int *a3, int *a4) {
	*animationState = var_45CD70_animation_state;
	*animationFrame = var_45CD74_animation_frame;
	*a3 = var_462800;
	*a4 = var_462804;
}

void AIScript_Runciter::SetAnimationState(int animationState, int animationFrame, int a3, int a4) {
	var_45CD70_animation_state = animationState;
	var_45CD74_animation_frame = animationFrame;
	var_462800 = a3;
	var_462804 = a4;
}

bool AIScript_Runciter::ReachedMovementTrackWaypoint(int waypointId) {
	switch (waypointId) {
	case 93:
		Actor_Face_Heading(15, 1002, true);
		break;
	case 92:
		Actor_Face_Heading(15, 664, true);
		break;
	case 91:
		Actor_Face_Heading(15, 120, true);
		break;
	case 90:
		Actor_Face_Heading(15, 170, true);
		break;
	case 89:
		Actor_Face_Heading(15, 567, true);
		break;
	default:
		return true;
	}
	return true;
}

void AIScript_Runciter::FledCombat() {}

} // End of namespace BladeRunner
