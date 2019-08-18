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

enum kRunciterStates {
	kRunciterStateIdle         = 0,
	kRunciterStateWalking      = 1,
	kRunciterStateDying        = 14,
	kRunciterStateDead         = 15
};

AIScriptRunciter::AIScriptRunciter(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	var_45CD78 = 0;
	var_45CD7C = 0;
	var_45CD80 = 0;
	var_45CD84 = 0;
	var_45CD88 = 0;
}

void AIScriptRunciter::Initialize() {
	_animationState = 0;
	_animationFrame = 0;
	_animationStateNext = 0;
	var_45CD78 = 0;
	var_45CD7C = 6;
	var_45CD80 = 1;
	var_45CD84 = 0;
	var_45CD88 = 0;
	Actor_Set_Goal_Number(kActorRunciter, kGoalRunciterDefault);
}

bool AIScriptRunciter::Update() {
	if (Actor_Query_Goal_Number(kActorRunciter) == kGoalRunciterDefault
	 && Game_Flag_Query(kFlagRC01PoliceDone)
	) {
		Actor_Set_Goal_Number(kActorRunciter, kGoalRunciterGoToFreeSlotGH);
	}

	if (Global_Variable_Query(kVariableChapter) == 4
	 && Actor_Query_Goal_Number(kActorRunciter) < kGoalRunciterRC02Wait
	) {
		Actor_Set_Goal_Number(kActorRunciter, kGoalRunciterRC02Wait);
	}

	return false;
}

void AIScriptRunciter::TimerExpired(int timer) {}

void AIScriptRunciter::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorRunciter) == kGoalRunciterRC02WalkAround) {
		if (Player_Query_Current_Scene() == kSceneRC02) {
			switch (Random_Query(1, 5)) {
			case 2:
			case 3:
				ADQ_Add(kActorRunciter, 530, -1);
				break;
			case 1:
			case 5:
				ADQ_Add(kActorRunciter, 80, -1);
				break;
			case 4:
				ADQ_Add(kActorRunciter, 930, -1);
				break;
			}
		}
		Actor_Set_Goal_Number(kActorRunciter, 99);
		Actor_Set_Goal_Number(kActorRunciter, kGoalRunciterRC02WalkAround);
		//return true;
	}
	//return false;
}

void AIScriptRunciter::ReceivedClue(int clueId, int fromActorId) {}

void AIScriptRunciter::ClickedByPlayer() {}

void AIScriptRunciter::EnteredScene(int sceneId) {}

void AIScriptRunciter::OtherAgentEnteredThisScene(int otherActorId) {}

void AIScriptRunciter::OtherAgentExitedThisScene(int otherActorId) {}

void AIScriptRunciter::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if ( Actor_Query_Goal_Number(kActorRunciter) == kGoalRunciterRC02Wait
	 &&  combatMode
	 && !Game_Flag_Query(kFlagRC02RunciterTalkWithGun)
	) {
		Actor_Set_Targetable(kActorRunciter, true);
		Actor_Face_Actor(kActorRunciter, kActorMcCoy, true);
		Actor_Says(kActorRunciter, 420, 12);
		Actor_Face_Actor(kActorMcCoy, kActorRunciter, true);
		Actor_Change_Animation_Mode(kActorMcCoy, kAnimationModeCombatAim);
		if (Actor_Clue_Query(kActorMcCoy, kClueZubensMotive)) {
			Actor_Says(kActorMcCoy, 4770, -1);
			Actor_Says(kActorRunciter, 590, 13);
			Actor_Says(kActorMcCoy, 4775, -1);
			Actor_Says(kActorRunciter, 600, 17);
			Sound_Play(kSfxSHOTCOK1, 100, 0, 100, 50);
			Actor_Says(kActorMcCoy, 4780, -1);
			Actor_Says(kActorRunciter, 610, 18);
			Actor_Says(kActorMcCoy, 4785, -1);
			Actor_Says(kActorRunciter, 620, 15);
			if (Game_Flag_Query(kFlagLucyIsReplicant)) {
				Actor_Says(kActorRunciter, 630, 12);
				Actor_Says(kActorRunciter, 640, 17);
				Actor_Says(kActorMcCoy, 4790, -1);
				Actor_Says(kActorRunciter, 650, 18);
				Actor_Says(kActorRunciter, 660, 19);
				Actor_Clue_Acquire(kActorMcCoy, kClueRuncitersConfession1, true, kActorRunciter);
			} else {
				Actor_Says(kActorRunciter, 670, 18);
				Actor_Says(kActorMcCoy, 4795, -1);
				Actor_Says(kActorRunciter, 730, 17);
			}
		} else if (Actor_Clue_Query(kActorMcCoy, kClueEnvelope)) {
			Actor_Says(kActorMcCoy, 4730, -1);
			Actor_Says(kActorRunciter, 480, 17);
			Actor_Says(kActorMcCoy, 4735, -1);
			Actor_Says(kActorRunciter, 490, 16);
			Sound_Play(kSfxSHOTCOK1, 100, 0, 100, 50);
			Actor_Says(kActorMcCoy, 4740, -1);
			Actor_Says(kActorRunciter, 500, 18);
			Actor_Says(kActorRunciter, 510, 19);
			Actor_Says(kActorMcCoy, 4745, -1);
			Actor_Says(kActorMcCoy, 4750, -1);
			Actor_Says(kActorRunciter, 520, 17);
			Actor_Says(kActorRunciter, 530, 18);
			Actor_Says(kActorRunciter, 540, 16);
		}
		Game_Flag_Set(kFlagRC02RunciterTalkWithGun);
	}
}

void AIScriptRunciter::ShotAtAndMissed() {}

bool AIScriptRunciter::ShotAtAndHit() {
	Actor_Set_Targetable(kActorRunciter, false);
	Actor_Change_Animation_Mode(kActorRunciter, kAnimationModeDie);
	Actor_Set_Goal_Number(kActorRunciter, kGoalRunciterDead);
	Delay(2000);
	if (Actor_Clue_Query(kActorMcCoy, kClueZubensMotive)) {
		Actor_Voice_Over(2050, kActorVoiceOver);
		Actor_Voice_Over(2060, kActorVoiceOver);
	} else {
		Actor_Voice_Over(2070, kActorVoiceOver);
		Actor_Voice_Over(2080, kActorVoiceOver);
		Actor_Voice_Over(2090, kActorVoiceOver);
	}
	Actor_Modify_Friendliness_To_Other(kActorClovis, kActorMcCoy, 3);
	return false;
}

void AIScriptRunciter::Retired(int byActorId) {}

int AIScriptRunciter::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptRunciter::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == kGoalRunciterDefault) {
		Actor_Put_In_Set(kActorRunciter, kSetRC02_RC51);
		Actor_Set_At_Waypoint(kActorRunciter, 92, 567);
		return false;
	}

	if (newGoalNumber == kGoalRunciterRC02WalkAround) {
		AI_Movement_Track_Flush(kActorRunciter);
		if (Random_Query(0, 1) == 1) {
			if (Random_Query(0, 1) == 0) {
				AI_Movement_Track_Append_With_Facing(kActorRunciter, 89, Random_Query(6, 10), 567);
			}
			AI_Movement_Track_Append_With_Facing(kActorRunciter, 93, Random_Query(2, 6), 1002);
			AI_Movement_Track_Append(kActorRunciter, 92, 5);
		} else {
			AI_Movement_Track_Append_With_Facing(kActorRunciter, 91, Random_Query(3, 10), 120);
			if (Random_Query(1, 3) == 1) {
				AI_Movement_Track_Append_With_Facing(kActorRunciter, 93, Random_Query(2, 6), 1002);
			}
			AI_Movement_Track_Append_With_Facing(kActorRunciter, 90, Random_Query(5, 10), 170);
		}
		AI_Movement_Track_Repeat(kActorRunciter);
		return true;
	}

	if (newGoalNumber == kGoalRunciterGoToFreeSlotGH) {
		AI_Movement_Track_Flush(kActorRunciter);
		AI_Movement_Track_Append(kActorRunciter, 39, 120);
		AI_Movement_Track_Append(kActorRunciter, 40, 0);
		AI_Movement_Track_Repeat(kActorRunciter);
		return false;
	}

	if (newGoalNumber == kGoalRunciterRC02Wait) {
		Actor_Put_In_Set(kActorRunciter, kSetRC02_RC51);
		Actor_Set_At_Waypoint(kActorRunciter, 93, 1007);
		return false;
	}
	return false;
}

bool AIScriptRunciter::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case kRunciterStateIdle:
		if (var_45CD78 == 0) {
			*animation = 529;
			if (var_45CD84) {
				var_45CD84--;
			} else {
				_animationFrame += var_45CD80;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(529)) {
					_animationFrame = 0;
				}
				if (_animationFrame < 0) {
					_animationFrame = Slice_Animation_Query_Number_Of_Frames(529) - 1;
				}
				--var_45CD7C;
				if (var_45CD7C == 0) {
					var_45CD80 = 2 * Random_Query(0, 1) - 1;
					var_45CD7C = Random_Query(6, 14);
					var_45CD84 = Random_Query(0, 4);
				}
				if (_animationFrame == 0) {
					if (Random_Query(0, 1) == 1) {
						var_45CD78 = Random_Query(1, 2);
						var_45CD80 = 1;
						var_45CD84 = 0;
					}
				}
			}
		} else if (var_45CD78 == 1) {
			*animation = 530;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(530)) {
				_animationFrame = 0;
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
				_animationFrame += var_45CD80;
				if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(*animation) - 1) {
					var_45CD84 = Random_Query(5, 15);
					var_45CD80 = -1;
				}
				if (_animationFrame <= 0) {
					_animationFrame = 0;
					var_45CD78 = 0;
					*animation = 529;
					var_45CD7C = Random_Query(6, 14);
					var_45CD80 = 2 * Random_Query(0, 1) - 1;
				}
			}
		}
		*frame = _animationFrame;
		break;

	case kRunciterStateWalking:
		*animation = 526;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(526)) {
			_animationFrame = 0;
		}
		*frame = _animationFrame;
		break;

	case 2:
		*animation = 533;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(533)) {
			_animationFrame = 0;
			if (var_45CD88) {
				*animation = 529;
				_animationState = 0;
				var_45CD78 = 0;
			} else {
				_animationState = 4;
			}
		}
		*frame = _animationFrame;
		break;

	case 4:
		*animation = 534;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(534)) {
			_animationFrame = 0;
			if (var_45CD88) {
				*animation = 529;
				_animationState = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 5:
		*animation = 535;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(535)) {
			_animationFrame = 0;
			if (var_45CD88) {
				*animation = 529;
				_animationState = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 6:
		*animation = 536;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(536)) {
			_animationFrame = 0;
			if (var_45CD88) {
				*animation = 529;
				_animationState = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 7:
		*animation = 537;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(537)) {
			_animationFrame = 0;
			if (var_45CD88) {
				*animation = 529;
				_animationState = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 8:
		*animation = 538;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(538)) {
			_animationFrame = 0;
			if (var_45CD88) {
				*animation = 529;
				_animationState = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 9:
		*animation = 539;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(539)) {
			_animationFrame = 0;
			if (var_45CD88) {
				*animation = 529;
				_animationState = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;
	case 10:
		*animation = 540;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(540)) {
			_animationFrame = 0;
			if (var_45CD88) {
				*animation = 529;
				_animationState = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 11:
		*animation = 541;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(541)) {
			_animationFrame = 0;
			if (var_45CD88) {
				*animation = 529;
				_animationState = 0;
				var_45CD78 = 0;
			} else {
				*animation = 533;
				_animationState = 2;
			}
		}
		*frame = _animationFrame;
		break;

	case 12:
		*animation = 532;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(532)) {
			*animation = 529;
			_animationState = 0;
			_animationFrame = 0;
			var_45CD78 = 0;
			Actor_Change_Animation_Mode(kActorRunciter, kAnimationModeCombatIdle);
		}
		*frame = _animationFrame;
		break;

	case 13:
		if (var_45CD78 == 0) {
			_animationFrame = 0;
			_animationState = _animationStateNext;
			*animation = _animationNext;
		} else if (var_45CD78 == 1) {
			*animation = 530;
			_animationFrame += 3;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(530)) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		} else if (var_45CD78 == 2) {
			*animation = 531;
			_animationFrame -= 3;
			if (_animationFrame - 3 < 0) {
				_animationFrame = 0;
				_animationState = _animationStateNext;
				*animation = _animationNext;
			}
		}
		*frame = _animationFrame;
		break;

	case kRunciterStateDying:
		*animation = 528;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(528) - 1) {
			*animation = 528;
			_animationState = kRunciterStateDead;
		}
		*frame = _animationFrame;
		break;

	case kRunciterStateDead:
		*animation = 528;
		_animationFrame = Slice_Animation_Query_Number_Of_Frames(528) - 1;
		*frame = _animationFrame;
		break;

	default:
		*animation = 399;
		_animationFrame = 0;
		*frame = _animationFrame;
		break;
	}
	return true;
}

bool AIScriptRunciter::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		if (_animationState >= 2
		 && _animationState <= 11
		) {
			var_45CD88 = 1;
		} else {
			_animationState = 0;
			_animationFrame = 0;
			var_45CD78 = 0;
		}
		break;

	case kAnimationModeWalk:
		if (_animationState > 1) {
			_animationState = 1;
			_animationFrame = 0;
		} else if (_animationState == 0) {
			_animationState = 13;
			_animationStateNext = 1;
			_animationNext = 526;
		}
		break;

	case kAnimationModeTalk:
		if (_animationState  != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 2;
			_animationNext = 526;
		}
		var_45CD88 = 0;
		break;

	case 12:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 4;
			_animationNext = 534;
		}
		var_45CD88 = 0;
		break;

	case 13:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 5;
			_animationNext = 535;
		}
		var_45CD88 = 0;
		break;

	case 14:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 6;
			_animationNext = 536;
		}
		var_45CD88 = 0;
		break;

	case 15:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 7;
			_animationNext = 537;
		}
		var_45CD88 = 0;
		break;

	case 16:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 8;
			_animationNext = 538;
		}
		var_45CD88 = 0;
		break;

	case 17:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 9;
			_animationNext = 539;
		}
		var_45CD88 = 0;
		break;

	case 18:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 10;
			_animationNext = 540;
		}
		var_45CD88 = 0;
		break;

	case 19:
		if (_animationState != 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 13;
			_animationStateNext = 11;
			_animationNext = 541;
		}
		var_45CD88 = 0;
		break;

	case 23:
		_animationState = 12;
		_animationFrame = 0;
		break;

	case kAnimationModeDie:
		_animationState = 14;
		_animationFrame = 0;
		break;
	}
	return true;
}

void AIScriptRunciter::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptRunciter::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptRunciter::ReachedMovementTrackWaypoint(int waypointId) {
	switch (waypointId) {
	case 89:
		Actor_Face_Heading(kActorRunciter, 567, true);
		break;
	case 90:
		Actor_Face_Heading(kActorRunciter, 170, true);
		break;
	case 91:
		Actor_Face_Heading(kActorRunciter, 120, true);
		break;
	case 92:
		Actor_Face_Heading(kActorRunciter, 664, true);
		break;
	case 93:
		Actor_Face_Heading(kActorRunciter, 1002, true);
		break;
	}
	return true;
}

void AIScriptRunciter::FledCombat() {}

} // End of namespace BladeRunner
