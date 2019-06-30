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

AIScriptFishDealer::AIScriptFishDealer(BladeRunnerEngine *vm) : AIScriptBase(vm) {
	_flag = false;
}

void AIScriptFishDealer::Initialize() {
	_animationFrame = 0;
	_animationState = 0;
	_animationStateNext = 0;
	_animationNext = 0;

	_flag = false;
	Actor_Put_In_Set(kActorFishDealer, kSetAR01_AR02);
	Actor_Set_At_Waypoint(kActorFishDealer, 120, 424);
	Actor_Set_Goal_Number(kActorFishDealer, 0);
}

bool AIScriptFishDealer::Update() {
#if BLADERUNNER_ORIGINAL_BUGS
	if (Global_Variable_Query(kVariableChapter) == 5
	 && Actor_Query_Goal_Number(kActorFishDealer) < 400
	) {
		Actor_Set_Goal_Number(kActorFishDealer, 400);
		return true;

	} else if (Player_Query_Current_Scene() == kSceneAR01
	        && Actor_Query_Goal_Number(kActorFishDealer) != 1
	        && Actor_Query_Goal_Number(kActorFishDealer) != 2
	        && Actor_Query_Goal_Number(kActorFishDealer) != 400
	) {
		Actor_Set_Goal_Number(kActorFishDealer, 1);
		return true;
	}
	return false;
#else
	if (Global_Variable_Query(kVariableChapter) < 5) {
		// prevent Fish Dealer from blinking out while McCoy is flying out from Animoid
		if (Player_Query_Current_Scene() == kSceneAR01
		 && Actor_Query_Goal_Number(kActorFishDealer) == 3
		) {
			Actor_Set_Goal_Number(kActorFishDealer, 1);
			return true;
		}
	} else {
		if (Actor_Query_Goal_Number(kActorFishDealer) < 400) {
			Actor_Set_Goal_Number(kActorFishDealer, 400);
		} else if (Actor_Query_In_Set(kActorFishDealer, kSetAR01_AR02)) {
			// Remove the fish dealer from AR01 if she is still there in chapter 5,
			// this can happen only with older save games.
			GoalChanged(400, 400);
		}
		return true;
	}
	return false;
#endif // BLADERUNNER_ORIGINAL_BUGS
}

void AIScriptFishDealer::TimerExpired(int timer) {
	//return false;
}

void AIScriptFishDealer::CompletedMovementTrack() {
	if (Actor_Query_Goal_Number(kActorFishDealer) != 1)
		return; // false

	Actor_Set_Goal_Number(kActorFishDealer, 99); // A bug?
	Actor_Set_Goal_Number(kActorFishDealer, 1);

	//return true;
}

void AIScriptFishDealer::ReceivedClue(int clueId, int fromActorId) {
	//return false;
}

void AIScriptFishDealer::ClickedByPlayer() {
	//return false;
}

void AIScriptFishDealer::EnteredScene(int sceneId) {
	// return false;
}

void AIScriptFishDealer::OtherAgentEnteredThisScene(int otherActorId) {
	// return false;
}

void AIScriptFishDealer::OtherAgentExitedThisScene(int otherActorId) {
	// return false;
}

void AIScriptFishDealer::OtherAgentEnteredCombatMode(int otherActorId, int combatMode) {
	// return false;
}

void AIScriptFishDealer::ShotAtAndMissed() {
	// return false;
}

bool AIScriptFishDealer::ShotAtAndHit() {
	return false;
}

void AIScriptFishDealer::Retired(int byActorId) {
	// return false;
}

int AIScriptFishDealer::GetFriendlinessModifierIfGetsClue(int otherActorId, int clueId) {
	return 0;
}

bool AIScriptFishDealer::GoalChanged(int currentGoalNumber, int newGoalNumber) {
	switch (newGoalNumber) {
	case 1:
		AI_Movement_Track_Flush(kActorFishDealer);
		if (Random_Query(1, 3) == 1) {
			AI_Movement_Track_Append_With_Facing(kActorFishDealer, 121, Random_Query(2, 5), 720);
			AI_Movement_Track_Append_With_Facing(kActorFishDealer, 122, Random_Query(3, 6), 640);
			AI_Movement_Track_Append_With_Facing(kActorFishDealer, 120, Random_Query(6, 6), 640);
		} else {
			int randFace;

			if (Random_Query(1, 3) == 2) {
				AI_Movement_Track_Append_With_Facing(kActorFishDealer, 122, Random_Query(5, 8), 720);
				AI_Movement_Track_Append_With_Facing(kActorFishDealer, 120, Random_Query(2, 4), 640);
				AI_Movement_Track_Append_With_Facing(kActorFishDealer, 121, Random_Query(5, 7), 640);
				randFace = Random_Query(5, 10);
			} else {
				AI_Movement_Track_Append_With_Facing(kActorFishDealer, 122, Random_Query(3, 5), 720);
				AI_Movement_Track_Append_With_Facing(kActorFishDealer, 121, Random_Query(3, 3), 640);
				randFace = Random_Query(3, 3);
			}
			AI_Movement_Track_Append_With_Facing(kActorFishDealer, 120, randFace, 640);
		}

		AI_Movement_Track_Repeat(kActorFishDealer);
		return true;

	case 2:
		AI_Movement_Track_Flush(kActorFishDealer);
		AI_Movement_Track_Append(kActorFishDealer, 120, 30);
		AI_Movement_Track_Repeat(kActorFishDealer);
		return true;

	case 3:
		AI_Movement_Track_Flush(kActorFishDealer);
		AI_Movement_Track_Append(kActorFishDealer, 39, 0);
		AI_Movement_Track_Repeat(kActorFishDealer);
		return true;

	case 400:
#if !BLADERUNNER_ORIGINAL_BUGS
		// Movement truck is not reset and she might end-up showing up in AR01 after all.
		// This will lead to a issue with CDFRAMES in chapter 5
		AI_Movement_Track_Flush(kActorFishDealer);
#endif
		Actor_Put_In_Set(kActorFishDealer, kSetFreeSlotH);
		Actor_Set_At_Waypoint(kActorFishDealer, 40, 0);
		return true;

	default:
		return false;
	}

	return false;
}

bool AIScriptFishDealer::UpdateAnimation(int *animation, int *frame) {
	switch (_animationState) {
	case 0:
		*animation = 683;
		_animationFrame++;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(683))
			_animationFrame = 0;

		break;

	case 1:
		if (!_animationFrame && _flag) {
			*animation = 683;
			_animationState = 0;
			_flag = 0;
		} else {
			*animation = 685;
			_animationFrame++;

			if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(685))
				_animationFrame = 0;
		}
		break;

	case 2:
		*animation = 686;
		_animationFrame++;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(686)) {
			_animationFrame = 0;
			_animationState = 2;
		}
		break;

	case 3:
		*animation = 687;
		_animationFrame++;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(687)) {
			_animationFrame = 0;
			_animationState = 3;
		}
		break;

	case 4:
		*animation = 684;
		_animationFrame++;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(684)) {
			*animation = 683;
			_animationFrame = 0;
			_animationState = 0;
		}
		break;

	case 5:
		*animation = 682;
		_animationFrame++;

		if (_animationFrame >= Slice_Animation_Query_Number_Of_Frames(682))
			_animationFrame = 0;

		break;

	default:
		break;
	}

	*frame = _animationFrame;

	return true;
}

bool AIScriptFishDealer::ChangeAnimationMode(int mode) {
	switch (mode) {
	case 0:
		if (_animationState > 3) {
			_animationState = 0;
			_animationFrame = 0;
		} else {
			_flag = 1;
		}
		break;

	case 1:
		_animationState = 5;
		_animationFrame = 0;
		break;

	case 3:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
		_animationState = Random_Query(0, 2) + 1;
		_animationFrame = 0;
		break;

	case 12:
		_animationState = 2;
		_animationFrame = 0;
		break;

	case 13:
		_animationState = 3;
		_animationFrame = 0;
		break;

	case 23:
		_animationState = 4;
		_animationFrame = 0;
		break;

	default:
		return true;
	}

	return true;
}

void AIScriptFishDealer::QueryAnimationState(int *animationState, int *animationFrame, int *animationStateNext, int *animationNext) {
	*animationState     = _animationState;
	*animationFrame     = _animationFrame;
	*animationStateNext = _animationStateNext;
	*animationNext      = _animationNext;
}

void AIScriptFishDealer::SetAnimationState(int animationState, int animationFrame, int animationStateNext, int animationNext) {
	_animationState     = animationState;
	_animationFrame     = animationFrame;
	_animationStateNext = animationStateNext;
	_animationNext      = animationNext;
}

bool AIScriptFishDealer::ReachedMovementTrackWaypoint(int waypointId) {
	return true;
}

void AIScriptFishDealer::FledCombat() {
	// return false;
}

} // End of namespace BladeRunner
