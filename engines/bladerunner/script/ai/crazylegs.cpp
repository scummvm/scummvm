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

AIScriptCrazylegs::AIScriptCrazylegs(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = false;
}

void AIScriptCrazylegs::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = false;

	World_Waypoint_Set(360, kSetHF05, -103.0f, 40.63f, -53.0f);
	Actor_Put_In_Set(kActorCrazylegs, kSetHF05);
	Actor_Set_At_XYZ(kActorCrazylegs, -33.0f, 40.63f, 16.0f, 845);
}

bool AIScriptCrazylegs::Update() {
	if (Actor_Query_Goal_Number(kActorCrazylegs) == 10)
		Actor_Set_Goal_Number(kActorCrazylegs, 11);

	return false;
}

void AIScriptCrazylegs::TimerExpired(int timer) {
	//return false;
}

void AIScriptCrazylegs::CompletedMovementTrack() {
	//return false;
}

void AIScriptCrazylegs::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptCrazylegs::ClickedByPlayer() {
	//return false;
}

void AIScriptCrazylegs::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptCrazylegs::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptCrazylegs::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptCrazylegs::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	if (Actor_Query_Goal_Number(kActorCrazylegs) != kGoalCrazyLegsLeavesShowroom && otherActorId == kActorMcCoy) {
		if (combatMode && Global_Variable_Query(kVariableChapter) < 5) {
			Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
			Actor_Face_Actor(kActorMcCoy, kActorCrazylegs, true);
			Actor_Says(kActorCrazylegs, 430, 3);
			Actor_Says_With_Pause(kActorCrazylegs, 440, 0.0f, 3);
			Actor_Says(kActorMcCoy, 1870, -1);
			Actor_Says(kActorCrazylegs, 450, 3);
			Actor_Set_Goal_Number(kActorCrazylegs, kGoalCrazyLegsMcCoyDrewHisGun);
		} else if (Actor_Query_Goal_Number(kActorCrazylegs) == kGoalCrazyLegsMcCoyDrewHisGun) {
			Actor_Face_Actor(kActorCrazylegs, kActorMcCoy, true);
			Actor_Says(kActorCrazylegs, 460, 3);
			Actor_Says(kActorCrazylegs, 470, 3);
			Actor_Set_Goal_Number(kActorCrazylegs, kGoalCrazyLegsLeavesShowroom);
		}
	}
}

void AIScriptCrazylegs::ShotAtAndMissed() {
	if (Actor_Query_Goal_Number(kActorCrazylegs) == kGoalCrazyLegsLeavesShowroom)
		return;

	Actor_Set_Goal_Number(kActorCrazylegs, kGoalCrazyLegsLeavesShowroom);
}

bool AIScriptCrazylegs::ShotAtAndHit() {
	Actor_Set_Goal_Number(kActorCrazylegs, kGoalCrazyLegsShotAndHit);
	Actor_Says(kActorMcCoy, 1875, 4);  // I wouldn't drag that bucket of bolts if you paid me.
	return false;
}

void AIScriptCrazylegs::Retired(int byActorId) {
	// return false;
}

int AIScriptCrazylegs::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptCrazylegs::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	if (newGoalNumber == kGoalCrazyLegsDefault) {
		return true;
	}

	if (newGoalNumber == kGoalCrazyLegsShotAndHit) {
		AI_Movement_Track_Flush(kActorCrazylegs);
		Actor_Set_Targetable(kActorCrazylegs, false);
		return true;
	}

	if (newGoalNumber == kGoalCrazyLegsLeavesShowroom) {
		AI_Movement_Track_Flush(kActorCrazylegs);
		AI_Movement_Track_Append(kActorCrazylegs, 360, 0);
		AI_Movement_Track_Append(kActorCrazylegs, 40, 0);
		AI_Movement_Track_Repeat(kActorCrazylegs);
		return true;
	}

	return false;
}

bool AIScriptCrazylegs::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = 454;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(454))
			_animationFrame = 0;
		break;

	case 1:
		*animation = 455;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(455))
			_animationFrame = 0;
		break;

	case 2:
		*animation = 456;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(456))
			_animationFrame = 0;
		break;

	case 3:
		*animation = 457;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(457)) {
			*animation = 454;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 4:
		*animation = 452;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(452))
			_animationFrame = 0;
		break;

	case 5:
		*animation = 453;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(453))
			_animationFrame = 0;
		break;

	case 6:
		if (!_animationFrame && _flag) {
			*animation = 454;
			_animationState = 0;
			_flag = 0;
		} else {
			*animation = 458;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(458))
				_animationFrame = 0;
		}
		break;

	case 7:
		*animation = 459;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(459)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = 458;
		}
		break;

	case 8:
		*animation = 460;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(460)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = 458;
		}
		break;

	case 9:
		*animation = 461;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(461)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = 458;
		}
		break;

	case 10:
		*animation = 462;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(462)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = 458;
		}
		break;

	case 11:
		*animation = 463;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(463)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = 458;
		}
		break;

	case 12:
		*animation = 464;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(464)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = 458;
		}
		break;

	case 13:
		*animation = 465;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(465)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = 458;
		}
		break;

	case 14:
		*animation = 466;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(466)) {
			_animationFrame = 0;
			_animationState = 6;
			*animation = 458;
		}
		break;

	case 15:
		if (!_animationFrame && _flag) {
			Actor_Change_Animation_Mode(kActorCrazylegs, 43);
			_animationState = 2;
			_flag = 0;
			*animation = 456;
		} else {
			*animation = 456;
			_animationFrame++;
			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(456))
				_animationFrame = 0;
		}
		break;

	case 16:
		*animation = 467;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(467)) {
			_animationFrame = 0;
			_animationState = 1;
			*animation = 455;
		}
		break;

	case 17:
		*animation = 468;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(468)) {
			*animation = 454;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 18:
		*animation = 469;
		_animationFrame--;
		if (_animationFrame <= Slice_Animation_Query_Number_Of_Frames(469)) { // matches original
			_animationFrame = 0;
			_animationState = 2;
			*animation = 454;
		}
		break;

	case 19:
		*animation = 469;
		_animationFrame++;
		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(469)) {
			*animation = 454;
			_animationFrame = 0;
			_animationState = 0;
			if (Actor_Query_Goal_Number(kActorCrazylegs) == 10) {
				Actor_Set_Goal_Number(kActorCrazylegs, 11);
			}
		}
		break;

	default:
		break;
	}
	*frame = _animationFrame;

	return true;
}

bool AIScriptCrazylegs::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		switch (_animationState) {
		case 0:
		case 3:
			return true;
		case 1:
			_animationState = 17;
			_animationFrame = 0;
			break;
		case 2:
			_animationState = 19;
			_animationFrame = 0;
			break;
		case 4:
		case 5:
			_animationState = 0;
			_animationFrame = 0;
			break;
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			_flag = true;
			break;
		}
		break;
	case 1:
		_animationState = 4;
		_animationFrame = 0;
		break;
	case 3:
		if (_animationState == 2) {
			_animationState = 15;
		} else {
			_animationState = 6;
		}
		_animationFrame = 0;
		_flag = false;
		break;
	case 12:
		_animationState = 7;
		_animationFrame = 0;
		_flag = false;
		break;
	case 13:
		_animationState = 8;
		_animationFrame = 0;
		_flag = false;
		break;
	case 14:
		_animationState = 9;
		_animationFrame = 0;
		_flag = false;
		break;
	case 15:
		_animationState = 10;
		_animationFrame = 0;
		_flag = false;
		break;
	case 16:
		_animationState = 11;
		_animationFrame = 0;
		_flag = false;
		break;
	case 17:
		_animationState = 12;
		_animationFrame = 0;
		_flag = false;
		break;
	case 18:
		_animationState = 13;
		_animationFrame = 0;
		_flag = false;
		break;
	case 19:
		_animationState = 14;
		_animationFrame = 0;
		_flag = false;
		break;
	case 23:
		_animationState = 3;
		_animationFrame = 0;
		break;
	case 43:
		if (_animationState != 2) {
			_animationState = 18;
			_animationFrame = Slice_Animation_Query_Number_Of_Frames(469) - 1;
		}
		break;
	}

	return true;
}

void AIScriptCrazylegs::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptCrazylegs::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptCrazylegs::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptCrazylegs::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
