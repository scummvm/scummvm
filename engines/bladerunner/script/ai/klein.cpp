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

AIScriptKlein::AIScriptKlein(BladeRunnerEngine *vm) : AIScriptBase(vm) {
}

void AIScriptKlein::Initialize() {
	_animationState = 0;
	_animationFrame = 0;
	_animationStateNext = 0;
	Actor_Put_In_Set(kActorKlein, kSetPS07);
	Actor_Set_At_XYZ(kActorKlein, 338.0f, 0.22f, -612.0f, 768);
	Actor_Set_Goal_Number(kActorKlein, 0);
}

bool AIScriptKlein::Update() {
	if (Global_Variable_Query(kVariableChapter) > 1 && Actor_Query_Goal_Number(kActorKlein) < 100) {
		Actor_Set_Goal_Number(kActorKlein, 100);
		return true;
	}
	if (Actor_Query_Goal_Number(kActorKlein) < 4 && Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) < 40) {
		Actor_Set_Goal_Number(kActorKlein, 4);
	}
	if (Player_Query_Current_Scene() == kScenePS07 && Actor_Query_Goal_Number(kActorKlein) == 0) {
		Actor_Set_Goal_Number(kActorKlein, 1);
		return true;
	}
	if ( Actor_Clue_Query(kActorMcCoy, kClueOfficersStatement)
	 && !Game_Flag_Query(kFlagMcCoyHasOfficersStatement)
	) {
		Game_Flag_Set(kFlagMcCoyHasOfficersStatement);
	}
	if ( Actor_Clue_Query(kActorMcCoy, kCluePaintTransfer)
	 && !Game_Flag_Query(kFlagMcCoyHasPaintTransfer)
	) {
		Game_Flag_Set(kFlagMcCoyHasPaintTransfer);
	}
	if ( Actor_Clue_Query(kActorMcCoy, kClueShellCasings)
	 && !Game_Flag_Query(kFlagMcCoyHasShellCasings)
	) {
		Game_Flag_Set(kFlagMcCoyHasShellCasings);
	}
	if ( Actor_Clue_Query(kActorMcCoy, kClueChromeDebris)
	 && !Game_Flag_Query(kFlagMcCoyHasChromeDebris)
	) {
		Game_Flag_Set(kFlagMcCoyHasChromeDebris);
	}
	if ( Player_Query_Current_Scene() == kScenePS07
	 &&  Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) < 35
	 && !Game_Flag_Query(kFlagKleinInsulted)
	) {
		AI_Countdown_Timer_Reset(kActorKlein, 2);
		AI_Countdown_Timer_Start(kActorKlein, 2, 5);
		Game_Flag_Set(kFlagKleinInsulted);
		return true;
	}
	if (Actor_Query_Goal_Number(kActorKlein) == 7) {
		if (Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) > 20
		 && Actor_Query_Friendliness_To_Other(kActorKlein, kActorMcCoy) < 40
		) {
			Actor_Modify_Friendliness_To_Other(kActorKlein, kActorMcCoy, 2);
		}
		AI_Movement_Track_Flush(kActorKlein);
		Actor_Set_Goal_Number(kActorKlein, 0);
		return true;
	}
	return false;
}

void AIScriptKlein::TimerExpired(int timer) {
	if (timer == 2) {
		if ( Game_Flag_Query(kFlagKleinInsulted)
		 && !Game_Flag_Query(kFlagKleinInsultedTalk)
		 &&  Actor_Query_Is_In_Current_Set(kActorKlein)
		) {
			Actor_Face_Actor(kActorKlein, kActorMcCoy, true);
			Actor_Says(kActorKlein, 10, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 4120, kAnimationModeTalk);
			Actor_Says(kActorKlein, 20, kAnimationModeTalk);
			Actor_Says(kActorMcCoy, 4125, kAnimationModeTalk);
			Game_Flag_Set(kFlagKleinInsultedTalk);
			Actor_Set_Goal_Number(kActorKlein, 4);
		} else {
			Actor_Says(kActorKlein, 10, kAnimationModeTalk);
			Actor_Set_Goal_Number(kActorKlein, 4);
		}
		// return true;
	}
	//return false;
}

void AIScriptKlein::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorKlein) == 1) {
		Actor_Set_Goal_Number(kActorKlein, 2);
		return; // true;
	}
	if (Actor_Query_Goal_Number(kActorKlein) == 2) {
		Actor_Set_Goal_Number(kActorKlein, 1);
		return; // true;
	}
	if (Actor_Query_Goal_Number(kActorKlein) == 4) {
		Actor_Set_Goal_Number(kActorKlein, 5);
		// missing return?
	}
	if (Actor_Query_Goal_Number(kActorKlein) == 5) {
		Actor_Set_Goal_Number(kActorKlein, 6);
		// missing return?
	}
	if (Actor_Query_Goal_Number(kActorKlein) == 6) {
		Actor_Set_Goal_Number(kActorKlein, 7);
		return; // true;
	}
	if (Actor_Query_Goal_Number(kActorKlein) == 7) {
		Actor_Set_Goal_Number(kActorKlein, 0);
		return; // true;
	}
	// return false;
}

void AIScriptKlein::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptKlein::ClickedByPlayer() {
	//return false;
}

void AIScriptKlein::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptKlein::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptKlein::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptKlein::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptKlein::ShotAtAndMissed() {
	// return false;
}

bool AIScriptKlein::ShotAtAndHit() {
	return false;
}

void AIScriptKlein::Retired(int byActorId) {
	// return false;
}

int AIScriptKlein::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptKlein::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 1:
		AI_Movement_Track_Flush(kActorKlein);
		AI_Movement_Track_Append(kActorKlein, 73, Random_Query(3, 20));
		AI_Movement_Track_Repeat(kActorKlein);
		break;
	case 2:
		AI_Movement_Track_Flush(kActorKlein);
		AI_Movement_Track_Append(kActorKlein, 74, Random_Query(10, 20));
		AI_Movement_Track_Repeat(kActorKlein);
		break;
	case 3:
		AI_Movement_Track_Flush(kActorKlein);
		AI_Movement_Track_Append(kActorKlein, 31, 3);
		AI_Movement_Track_Repeat(kActorKlein);
		break;
	case 4:
		AI_Movement_Track_Flush(kActorKlein);
		AI_Movement_Track_Append(kActorKlein, 32, 5);
		AI_Movement_Track_Repeat(kActorKlein);
		break;
	case 5:
		AI_Movement_Track_Flush(kActorKlein);
		AI_Movement_Track_Append(kActorKlein, 35, 60);
		AI_Movement_Track_Repeat(kActorKlein);
		break;
	case 6:
		AI_Movement_Track_Flush(kActorKlein);
		AI_Movement_Track_Append(kActorKlein, 32, 5);
		AI_Movement_Track_Repeat(kActorKlein);
		break;
	case 100:
		AI_Movement_Track_Flush(kActorKlein);
		Actor_Put_In_Set(kActorKlein, kSetFreeSlotC);
		Actor_Set_At_Waypoint(kActorKlein, 35, 0);
		break;
	}
	return false;
}

bool AIScriptKlein::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		if (Actor_Query_Goal_Number(kActorKlein) == 1 || Actor_Query_Goal_Number(kActorKlein) == 2) {
			*animation = 691;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(691)) {
				_animationFrame = 0;
			}
		} else if (!Game_Flag_Query(196) && Actor_Query_Goal_Number(kActorKlein) == 3) {
			*animation = 689;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(689)) {
				_animationFrame = 0;
				if (Random_Query(1, 10) == 1) {
					Game_Flag_Set(196);
				}
			}
		} else {
			if (Game_Flag_Query(198) && Actor_Query_Goal_Number(kActorKlein) == 3) {
				_animationFrame--;
				if (_animationFrame < 0) {
					_animationFrame = 0;
				}
			} else {
				++_animationFrame;
			}

			*animation = 690;
			if (_animationFrame <= 9) {
				if (Game_Flag_Query(198)) {
					Game_Flag_Reset(198);
				}
			}
			if (_animationFrame == 14) {
				if (Random_Query(1, 5) == 1) {
					Game_Flag_Set(197);
				}
			}
			if (_animationFrame == 15) {
				if (Game_Flag_Query(197) == 1) {
					Game_Flag_Reset(197);
					Game_Flag_Set(198);
				}
			}
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(690)) {
				_animationFrame = 0;
				Game_Flag_Reset(196);
			}
		}
		break;
	case 1:
		*animation = 688;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(688)) {
			_animationFrame = 0;
		}
		break;
	case 2:
		*animation = 692;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(692)) {
			_animationFrame = 0;
		}
		break;
	case 3:
		*animation = 693;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(693)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = 692;
		}
		break;
	case 4:
		*animation = 694;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(694)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = 692;
		}
		break;
	case 5:
		*animation = 695;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(695)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = 692;
		}
		break;
	case 6:
		*animation = 696;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(696)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = 692;
		}
		break;
	case 7:
		*animation = 697;
		_animationFrame++;
		if (_animationFrame>= Slice_Animation_Query_Number_Of_Frames(697)) {
			_animationState = 2;
			_animationFrame = 0;
			*animation = 692;
		}
		break;
	case 8:
		_animationFrame = 0;
		*animation      = _animationNext;
		_animationState = _animationStateNext;
		break;
	default:
		*animation = 399;
		break;
	}
	*frame = _animationFrame;
	return true;
}

bool AIScriptKlein::ChangeAnimationMode(int mode) {
	switch (mode) {
	case kAnimationModeIdle:
		_animationState = 0;
		_animationFrame = 0;
		break;
	case kAnimationModeWalk:
		if (_animationState > 1) {
			_animationState = 1;
			_animationFrame = 0;
		} else if (_animationState == 0) {
			_animationState = 8;
			_animationStateNext = 1;
			_animationNext = 688;
		}
		break;
	case kAnimationModeTalk:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 8;
			_animationStateNext = 2;
			_animationNext = 692;
		}
		break;
	case 12:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 8;
			_animationStateNext = 3;
			_animationNext = 693;
		}
		break;
	case 13:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 8;
			_animationStateNext = 4;
			_animationNext = 694;
		}
		break;
	case 14:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 8;
			_animationStateNext = 5;
			_animationNext = 695;
		}
		break;
	case 15:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 8;
			_animationStateNext = 6;
			_animationNext = 696;
		}
		break;
	case 16:
		if (_animationState > 0) {
			_animationState = 2;
			_animationFrame = 0;
		} else {
			_animationState = 8;
			_animationStateNext = 7;
			_animationNext = 697;
		}
		break;
	}
	return true;
}

void AIScriptKlein::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptKlein::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptKlein::ReachedMovementTrackWaypoint(int waypointId) {
	if (waypointId == 73 || waypointId == 74) {
		Actor_Face_Heading(kActorKlein, 768, false);
	}
	if (waypointId == 31) {
		Actor_Face_Heading(kActorKlein, 216, false);
	}
	return true;
}

void AIScriptKlein::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
